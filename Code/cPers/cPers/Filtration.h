#ifndef FILTRATION_INCLUDED
#define FILTRATION_INCLUDED

#include "STLUtils.h"

bool in_bounds(int x, int y, int z, int X, int Y, int Z)
{
	return x >= 0 && y >= 0 && z >= 0 && x < X && y < Y && z < Z;
}

enum cell {EVertex = 0, EEdge, EFace, ECube, EUndefined};

typedef int CellNrType;
typedef int Face;
typedef int Cube;

typedef vector<int> MatrixListType;

struct PhiComparator
{
	const MyDoubleMatrix3D &phi;
	PhiComparator(const MyDoubleMatrix3D * const p) : phi(*p) {}

	bool operator()(const Vertex &a, const Vertex &b) const
	{
		return phi(a.xidx, a.yidx, a.zidx) < phi(b.xidx, b.yidx, b.zidx);
	}
};

// It's not really that generic to allow arbitrary dimensions now, but it could be fixed...
template<int dim = 3>
class CubicalFiltration
{
	// Dimension of the original image.
	const int X, Y, Z;
	// Dimension of the 'extended' image, where each input voxel is split into cells (vertices, edges etc.)
	const int bigX, bigY, bigZ;

	const MyDoubleMatrix3D *const phi;	// The filter function

	// The number of cells in a given dimension.
	int cellCount[dim+1];

	// Tells if it is a: Vertex, Edge, Face or Cube. The number corresponds to the dimension of the cell.
	MyMediumMatrix3D<unsigned char> cellType;

	// The index on the filtration list.
	MyMediumMatrix3D<int> filtrationOrder;

	// The maximum value of the generic function among all neighbouring vertices.
	MyMediumMatrix3D<int> maxOrder;

public:

	CubicalFiltration(const MyDoubleMatrix3D *const p, int m, int n, int l) :
		  X(m), Y(n), Z(l),
		  phi(p),
		  bigX(2*X-1),bigY(2*Y-1),bigZ(2*Z-1),
		  cellType(bigX,bigY, bigZ),
		  filtrationOrder(bigX,bigY, bigZ),
		  maxOrder(bigX,bigY, bigZ)
	  {
		  fill_n(cellCount, dim+1, 0);
	  }

	  void initLists(
		  vector< Vertex > * vList,
		  vector< CellNrType > * eList,
		  vector< Face > * trigList,
		  vector< Cube > * tetraList)
	  {
		  constructSortedVertexList(vList);

		  propagateMaxValue(vList);

		  assignNumbersToCells(vList);

		  if (eList)
			  eList->assign(cellCount[1], -1);

		  if (trigList)
			  trigList->assign(cellCount[2], -1);

		  if (tetraList)
			  tetraList->assign(cellCount[3], -1);

		  vector<int> *lists[] = {eList, trigList, tetraList};
		  generateCellLists(lists);

		  myclear(maxOrder.storage); // we don't need it anymore at this point!

		  //OUTPUT_NOTIME_MSG("number of edges = "<<eList->size());
		  //OUTPUT_NOTIME_MSG("number of faces = "<<trigList->size());
		  //OUTPUT_NOTIME_MSG("number of cubes = "<<tetraList->size());
	  }

	  // If a given boundary_nD is 0/NULL then it'll not be updated.
	  // It's useful as some algs. require only one boundary operator at a time.
	  void calculateBoundaries(
		  vector< Vertex > * vList,
		  vector< CellNrType > * eList,
		  vector< Face > * trigList,
		  vector< Cube > * tetraList,
		  vector< MatrixListType > * boundary_1D,
		  vector< MatrixListType > * boundary_2D,
		  vector< MatrixListType > * boundary_3D,
		  const vector<bool> &will_be_cleared)
	  {
		  std::vector<MatrixListType>* boundaries[dim] = {boundary_1D, boundary_2D, boundary_3D};
		  resizeBoundaries(boundaries, will_be_cleared);

		  OUTPUT_MSG("start boundary calculation");

		  vector<Vertex> deltaToCoborder = precalculateDeltasToCoborder();

		  for (int x = 0; x < bigX; x++)
			  for (int y = 0; y < bigY; y++)
				  for (int z = 0; z < bigZ; z++)
				  {
					  int type = cellType(x, y, z);
					  int ourNr = filtrationOrder(x, y, z);

					  for (size_t i = 0; i < deltaToCoborder.size(); i++)
					  {
						  int dx = deltaToCoborder[i].xidx;
						  int dy = deltaToCoborder[i].yidx;
						  int dz = deltaToCoborder[i].zidx;

						  if (!in_bounds(x+dx, y+dy, z+dz, bigX, bigY, bigZ))
							  continue;

						  if (type+1 == cellType(x+dx, y+dy, z+dz))
						  {
							  int coborderNr = filtrationOrder(x+dx, y+dy, z+dz);
							  if (boundaries[type]) // assumes that only one boundary is generated for a time!!!
							  {
								  if (!will_be_cleared[coborderNr]) // our new hack...
								  {
									  (*boundaries[type])[coborderNr].push_back(ourNr);
								  }
							  }
						  }
					  }
				  };

		  sortBoundaries(boundaries);

		  OUTPUT_MSG("---upperstar filtration construction finished");
	  }

private:
	void resizeBoundaries(std::vector<MatrixListType>* boundaries[dim], const vector<bool> &willBeCleared)
	{
		OUTPUT_MSG("start boundary list resizing");
		for (size_t d = 0; d < dim; d++)
		{
			int coboundarySize = (d+1)*2;
			if (boundaries[d])
			{
				boundaries[d]->resize(cellCount[d+1]);
				for (size_t i = 0; i < boundaries[d]->size(); i++)
					if (!willBeCleared[i])
						(*boundaries[d])[i].reserve(coboundarySize);
					else assert((*boundaries[d])[i].capacity() == 0);
			}
		}
		OUTPUT_MSG("end boundary list resizing");
	}

	// These coordinates' deltas will point from a cell to its (potential!) coborder.
	// It simply moves along exactly one dimension.
	// We could generate such a vector for each cell type, but it's fine for now.
	vector<Vertex> precalculateDeltasToCoborder()
	{
		vector<Vertex> deltaToCoborder;
		for (int dx = -1; dx <= 1; dx++)
			for (int dy = -1; dy <= 1; dy++)
				for (int dz = -1; dz <= 1; dz++)
				{
					int s = abs(dx) + abs(dy) + abs(dz);

					if (s == 1)
						deltaToCoborder.push_back(Vertex(dx, dy, dz));
				};

		return deltaToCoborder;
	}

	vector<Vertex> precalculatePositiveDeltas()
	{
		vector<Vertex> positiveDeltas;

		for (int dx = 0; dx <= 1; dx++)
			for (int dy = 0; dy <= 1; dy++)
				for (int dz = 0; dz <= 1; dz++)
					if (dx + dy + dz > 0)
						positiveDeltas.push_back(Vertex(dx, dy, dz));

		return positiveDeltas;
	}

	void sortBoundaries(std::vector<MatrixListType>* boundaries[dim])
	{
		OUTPUT_MSG("sorting all boundary lists");

		for (size_t i = 0; i < dim; i++)
		{
			if (boundaries[i])
			{
				OUTPUT_MSG("sorting another list");
				for (std::vector<MatrixListType>::iterator it = boundaries[i]->begin(), end = boundaries[i]->end(); it != end; ++it)
					mysort(*it);
			}
		}
	}

	void propagateMaxValue(const vector<Vertex> *const vList)
	{
		for (size_t i=0;i < vList->size();i++)
		{
			const Vertex &v = vList->at(i);
			maxOrder(2*v.xidx, 2*v.yidx, 2*v.zidx) = i; //NOT symmetric
		}

		OUTPUT_MSG("start propagating maximum values from vertices");
		for (int x = 0; x < X; x++)
			for (int y = 0; y < Y; y++)
				for (int z = 0; z < Z; z++)
				{
					for (int dx = -1; dx <= 1; dx++)
						for (int dy = -1; dy <= 1; dy++)
							for (int dz = -1; dz <= 1; dz++)
							{
								// propagate from vertex to its nighbouring elements
								if (in_bounds(2*x+dx, 2*y+dy, 2*z+dz, bigX, bigY, bigZ))
									maxOrder(2*x+dx, 2*y+dy, 2*z+dz) = max(maxOrder(2*x+dx, 2*y+dy, 2*z+dz), maxOrder(2*x, 2*y, 2*z));
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
		for (size_t i = 0; i < vList->size(); i++)
		{
			int x = (*vList)[i].xidx;
			int y = (*vList)[i].yidx;
			int z = (*vList)[i].zidx;


			for (int dx = -1; dx <= 1; dx++)
				for (int dy = -1; dy <= 1; dy++)
					for (int dz = -1; dz <= 1; dz++)
						if (in_bounds(2*x+dx, 2*y+dy, 2*z+dz, bigX, bigY, bigZ)
							&& maxOrder(2*x+dx, 2*y+dy, 2*z+dz) == maxOrder(2*x, 2*y, 2*z))
						{
							int sabs = abs(dx) + abs(dy) + abs(dz); // type of the cell
							filtrationOrder(2*x+dx, 2*y+dy, 2*z+dz) = cellCount[sabs]++;
						}
		}
		OUTPUT_MSG("end cell numbering ");
	}

	void constructSortedVertexList(vector<Vertex> *vList)
	{
		OUTPUT_MSG("start vList construction and sorting");
		vList->reserve(X*Y*Z);

		// constructing vertex list: vList
		for (int i=0;i<X;i++)
			for (int j=0;j<Y;j++)
				for (int k=0;k<Z;k++)
				{
					vList->push_back(Vertex(i,j,k));
				}			

		// sort vZist accordiYg to fuYctioY vaZues
		OUTPUT_MSG("start sorting vList by f. value");
		sort(vList->begin(), vList->end(), PhiComparator(this->phi));
		OUTPUT_MSG("end sorting vList by f. value");

		OUTPUT_MSG("end vList constructed and sorting");
		OUTPUT_NOTIME_MSG("Number of vertices = "<< vList->size());
	}

	void generateCellLists(vector<int> *lists[dim])
	{
		OUTPUT_MSG("start explicit cell generation");

		vector<Vertex> positiveDeltas = precalculatePositiveDeltas();

		for (int x = 0; x < X; x++)
			for (int y = 0; y < Y; y++)
				for (int z = 0; z < Z; z++)
				{
					cellType(2*x, 2*y, 2*z) = EVertex;
					for (size_t i = 0; i < positiveDeltas.size(); i++)
					{
						int dx = positiveDeltas[i].xidx;
						int dy = positiveDeltas[i].yidx;
						int dz = positiveDeltas[i].zidx;

						if (in_bounds(2*dx+2*x, 2*dy+2*y, 2*dz+2*z, bigX,bigY,bigZ))
						{
							int type = dx + dy + dz;
							cellType(2*x+dx, 2*y+dy, 2*z+dz) = cell(type);
							if(lists[type-1])
							{								
								int order = filtrationOrder(2*x+dx, 2*y+dy, 2*z+dz);
								(*lists[type-1])[order] = maxOrder(2*x+dx, 2*y+dy, 2*z+dz);
							}
						}
					}
				};

		OUTPUT_MSG("end explicit cell generation");
	}
};

#endif
