#ifndef GENERAL_FILTRATION_INCLUDED
#define GENERAL_FILTRATION_INCLUDED

#include "STLUtils.h"
#include <numeric>
#include <functional>
#include <climits>

//TODO: try to get rid of the in_bounds thing??

typedef vector<int> MatrixListType;

// It's a bit hackery generator of certain {-1,0,1}^dim vectors meeting certain properties
template<int dim, typename type = int>
struct delta_generator
{
	typedef blitz::TinyVector<int, dim> vec;

	static std::vector<vec> generate(int max_abs_sum, bool positive_only = false)
	{
		std::vector<vec> vs;

		size_t all_bits_set = (1 << (2*dim)) - 1;

		int mapping[4] = {0,1,-1,0}; // Actually, we'd prefer a radix-3 system (if it was natively supported).

		for (size_t m = 0; m <= all_bits_set; m++)
		{
			vec v;
			int s = 0;
			for (int pos = 0; pos < dim; pos++)
			{
				int bit0 = !!(m&(1<<(2*pos)));
				int bit1 = !!(m&(1<<(2*pos+1)));
				int ind = 2*bit0 + bit1;
				assert(ind >= 0 && ind <= 3);
				v[pos] = mapping[ind];

				if (ind == 3)
				{
					s = INT_MAX; // We want just one representation of '0'.
					break;
				}

				s += abs(v[pos]);
			}
			if (s <= max_abs_sum)
				vs.push_back(v);
		}

		return vs;
	}
};

bool in_bounds(int x, int y, int z, int X, int Y, int Z)
{
	return x >= 0 && y >= 0 && z >= 0 && x < X && y < Y && z < Z;
}

template<typename IndexT>
bool in_bounds(const IndexT &ind, const IndexT &bounds)
{
	for (int i = 0; i < IndexT::numElements; i++)
		if (ind[i] < 0 || ind[i] >= bounds[i]) // > or >=
			return false;
	return true;
}

template<int dim>
class CubicalFiltration
{
	typedef blitz::TinyVector<int, dim> Vertex;
	template<typename ArrayType>
	struct PhiComparator
	{
		const ArrayType &phi;
		PhiComparator(const ArrayType * const p) : phi(*p) {}

		template<typename VertexType>
		bool operator()(const VertexType &a, const VertexType &b) const
		{
			return phi(a) < phi(b);
		}
	};
	// We use this as a generalized n-D index in our arrays.
	typedef blitz::TinyVector<int, dim> Index;

	// Dimension of the original image. Lower bounds will probably stay at 0,0,...0
	const blitz::TinyVector<int, dim> lowerOrigBounds;
	const blitz::TinyVector<int, dim> upperOrigBounds;

	// Dimension of the 'extended' image, where each input voxel is split into cells (vertices, edges etc.)
	const blitz::TinyVector<int, dim> lowerBigBounds;
	const blitz::TinyVector<int, dim> upperBigBounds;

	// The filter function
	const blitz::Array<double, dim> *const phi;

	// The number of cells in a given dimension.
	int cellCount[dim+1];

	// The index on the filtration list.
	blitz::Array<int, dim> filtrationOrder;

	// The maximum value of the generic function among all neighbouring vertices.
	blitz::Array<int, dim> maxValue;
public:	
		  CubicalFiltration(const blitz::Array<double, dim> *const p) :
	  phi(p),
		  lowerOrigBounds(p->lbound()),
		  upperOrigBounds(p->ubound()),
		  lowerBigBounds(p->lbound()),
		  upperBigBounds((2 * p->ubound()) + 1), // this is correct, note that upper bounds are exclusive in blitz!
		  filtrationOrder(upperBigBounds),
		  maxValue(upperBigBounds)
	  {
		  fill_n(cellCount, dim+1, 0);
		  fill_n(maxValue.begin(), getBigTotalSize(), 0);
	  }

	  int getSizeInDim(int d)
	  {
		  assert(d >= 0 && d <= dim);
		  return cellCount[d];
	  }

	  void init(vector< Vertex > * vList)
	  {
		  if (vList->empty())
		  {
			  constructSortedVertexList(vList);
		  }

		  propagateMaxValue(vList);

		  assignNumbersToCells(vList);
	  }

	  void initList(
		  vector< Vertex > * vList,
		  vector< int > * list,
		  vector< MatrixListType > * cell2v_list,
		  int d)
	  {
		  assert(!vList->empty());		  
		  list->assign(cellCount[d], -1);		  
		  cell2v_list->assign(cellCount[d], vector<int>());		  
		  generateCellLists(*list, *cell2v_list, d);		  
	  }

	  // If a given boundary_nD is 0/NULL then it'll not be updated.
	  // It's useful as some algorithms require only one boundary operator at a time.
	  void calculateBoundaries(
		  vector< Vertex > * vList,
		  vector< MatrixListType > * boundary,
		  int d,
		  const vector<bool> &will_be_cleared)
	  {
		  maxValue.free();
		  resizeBoundary(*boundary, d, will_be_cleared);

		  OUTPUT_MSG("start boundary calculation");

		  vector<Index> deltaToCoborder = delta_generator<dim>::generate(1);
		  const size_t deltaSize = deltaToCoborder.size();

		  for (typename blitz::Array<int, dim>::const_iterator it = filtrationOrder.begin(), end = filtrationOrder.end(); it != end; ++it)
		  {
			  Index ind = it.position();
			  int cellDim = dimFromCoords(ind);

			  if (cellDim+ 1 != d) // we should iterate over specific cells, but that's not a bottleneck (for now)
				  continue;

			  int ourNr = *it;

			  for (size_t i = 0; i < deltaSize; i++)
			  {
				  Index newInd = ind + deltaToCoborder[i];

				  if (!in_bounds(newInd, upperBigBounds))
					  continue;

				  int newDim = dimFromCoords(newInd);

				  if (cellDim+1 == newDim)
				  {
					  int coborderNr = filtrationOrder(newInd);

					  if (!will_be_cleared[coborderNr])
					  {
						  (*boundary)[coborderNr].push_back(ourNr);
					  }
				  }
			  }
		  };

		  for (std::vector<MatrixListType>::iterator it = boundary->begin(), end = boundary->end(); it != end; ++it)
			  mysort(*it);

		  OUTPUT_MSG("---filtration construction finished");
	  }

private:
	int abs_sum(const Index &delta) const
	{
		int sabs = 0;
		for (int d = 0; d < dim; d++)
			sabs += abs(delta[d]);

		return sabs;
	}

	int dimFromCoords(const Index &ind)
	{
		return abs_sum(ind % 2);
	}

	void resizeBoundary(std::vector<MatrixListType> &boundary, int d, const vector<bool> &willBeCleared)
	{
		OUTPUT_MSG("start boundary list resizing");

		int coboundarySize = d*2;

		boundary.resize(cellCount[d]);
		for (size_t i = 0; i < boundary.size(); i++)
			if (!willBeCleared[i])
				boundary[i].reserve(coboundarySize);
		else assert(boundary[i].capacity() == 0);

		OUTPUT_MSG("end boundary list resizing");
	}

	void propagateMaxValue(const vector<Vertex> *const vList)
	{
		for (size_t i=0; i < vList->size(); i++)
		{
			const Vertex v = 2 * vList->at(i);
			maxValue(v) = i; //NOT symmetric
		}
		

		OUTPUT_MSG("start propagating maximum values from vertices");

		blitz::TinyVector<int, dim> stride(2);

		blitz::Array<int, dim> vertices = maxValue(blitz::StridedDomain<dim>(lowerBigBounds, upperBigBounds, stride));
		std::vector<Index> neighbours = delta_generator<dim>::generate(dim);
		const size_t nsz = neighbours.size();

		for (typename blitz::Array<int,dim>::iterator it = vertices.begin(), end = vertices.end(); it != end; ++it)
		{
			int val = *it;
			Index ind = 2 * it.position();

			for (size_t i = 0; i < nsz; i++)
			{
				Index newIndex = ind + neighbours[i];
				// propagate from vertex to its nighbouring elements
				if (in_bounds(newIndex, upperBigBounds))
					maxValue(newIndex) = max(val, maxValue(newIndex));
			}
		};

		OUTPUT_MSG("end propagating maximum values from vertices");
	}

	// We number all cells according to the function value.
	// We iterate through all vertices in sorted order, and go through all
	// the neighbours such that the current vertex is the maximum and update the value.
	// The 'filtrationOrder' vector is in fact the order of filtration seperate for each cell (edge, face...).
	void assignNumbersToCells(const vector<Vertex> *const vList)
	{
		OUTPUT_MSG("start cell numbering ");

		std::vector<Index> neighbours = delta_generator<dim>::generate(dim);
		const size_t nsz = neighbours.size();

		for (size_t v = 0; v < vList->size(); v++)
		{
			Index index = 2 * vList->at(v);			

			for (size_t i = 0; i < nsz; i++)
			{
				const Index &delta = neighbours[i];				
				Index newIndex = index + delta;

				if (in_bounds(newIndex, upperBigBounds)
					&& maxValue(newIndex) == maxValue(index))
				{
					int sabs = abs_sum(delta);
					filtrationOrder(newIndex) = cellCount[sabs]++;					
				}
			}
		}
		OUTPUT_MSG("end cell numbering ");
	}

	void constructSortedVertexList(vector<Vertex> *vList)
	{
		OUTPUT_MSG("start vList construction and sorting");		

		vList->reserve(getSmallTotalSize());				

		blitz::TinyVector<int, dim> stride(2); // = 2,2,...,2

		blitz::Array<int, dim> vertices = filtrationOrder(blitz::StridedDomain<dim>(lowerBigBounds, upperBigBounds, stride));

		// constructing vertex list: vList
		for (typename blitz::Array<int,dim>::iterator it = vertices.begin(), end = vertices.end(); it != end; ++it)
		{			
			vList->push_back(it.position());
		}

		// sort vZist accordiYg to fuYctioY vaZues
		OUTPUT_MSG("start sorting vList by f. value");
		sort(vList->begin(), vList->end(), PhiComparator<blitz::Array<double,dim> >(this->phi));
		OUTPUT_MSG("end sorting vList by f. value");

		OUTPUT_MSG("end vList constructed and sorting");
		OUTPUT_NOTIME_MSG("Number of vertices = "<< vList->size());
	}

	int getSmallTotalSize() 
	{
		int size = 1;
		for (int i = 0; i < dim; i++)
			size *= upperOrigBounds[i];

		return size;
	}

	int getBigTotalSize() 
	{
		int size = 1;
		for (int i = 0; i < dim; i++)
			size *= upperBigBounds[i];

		return size;
	}

	void generateCellLists(vector<int> &list, vector<MatrixListType> &cell2v_list, int d)
	{
		OUTPUT_MSG("start explicit cell generation");

		blitz::TinyVector<int, dim> stride(2);

		blitz::Array<int, dim> vertices = filtrationOrder(blitz::StridedDomain<dim>(lowerBigBounds, upperBigBounds, stride));
		std::vector<Index> positive_deltas = delta_generator<dim>::generate(dim, true);
		size_t nsz = positive_deltas.size();		

		for (typename blitz::Array<int,dim>::iterator it = vertices.begin(), end = vertices.end(); it != end; ++it)
		{
			Index ind = it.position();			
			ind *= 2;

			for (size_t i = 0; i < nsz; i++)
			{
				const Index &delta = positive_deltas[i];
				Index newIndex = ind + delta;				
				if (in_bounds(newIndex, upperBigBounds))
				{
					int cellDim = dimFromCoords(newIndex);
					if(cellDim == d)
					{						
						int order = filtrationOrder(newIndex);						
						list[order] = maxValue(newIndex);

						cell2v_list[order].push_back( filtrationOrder( ind ) );
					}
				}
			}
		};

		cout << d<< " " <<(int) cell2v_list.size() << endl;
		for( size_t i = 0; i < cell2v_list.size(); i ++ ){
			sort( cell2v_list[ i ].begin(), cell2v_list[ i ].end() );
			MatrixListType::iterator tmpiter = unique(cell2v_list[ i ].begin(), cell2v_list[ i ].end() );
			cell2v_list[ i ].resize( tmpiter - cell2v_list[ i ].begin() );
			MY_ASSERT_MORE( cell2v_list[ i ].size() == pow((double)2, d), "ERROR: real size = %d, %d\n ", cell2v_list[ i ].size(), d  );
//			for( size_t j = 0; j < cell2v_list[ i ].size(); j ++ )
//				cout << cell2v_list[i][j] << " ";
//			cout << endl;
		}

		OUTPUT_MSG("end explicit cell generation");
	}
};

#endif
