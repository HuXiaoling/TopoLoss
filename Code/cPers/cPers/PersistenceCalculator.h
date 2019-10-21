#ifndef INCLUDED_PERSISTENCE_CALCULATOR_H
#define INCLUDED_PERSISTENCE_CALCULATOR_H

// Trick:
// Rather than operating on an existing matrix, we record the columns
// to be cleared, and clear them once we have calculated the matrix.
// We need to store only one matrix at a time.

#include "Reduction.h"

// By switching FiltrationGeneratorType it should be possible to use for example simplicial complexes
template<int dim, typename FiltrationGeneratorType = CubicalFiltration<dim> >
struct PersistenceCalculator {	

	typedef blitz::TinyVector<int, dim> Vertex;
	typedef vector<PersPair<Vertex> > PersResultContainer;

	template<typename NDArray>
	void SavePersistence(
		NDArray * phi,
		const vector<Vertex> &vList,
		vector< int > & lowerCellList, 
		vector< int > & low_1D_v2e, 
		int &count_pairs, 
		vector< CellNrType > & upperCellList, 
		const double pers_thd, 									 
		PersResultContainer &veList, 
		//NDArray &persRobM,
		/* for reduction list*/
		vector< MatrixListType > & red_list,
		vector< MatrixListType > & red_cell2v_list,
		vector< MatrixListType > & final_red_list,
		vector< MatrixListType > & bd_list,
		vector< MatrixListType > & bd_cell2v_list,
		vector< MatrixListType > & final_boundary_list
		) 
	{
		assert( final_red_list.empty() );
		assert( final_boundary_list.empty() );

//                 cout << " What is going on? " << endl;
//                 for (int i = 0; i < 10; ++i){
//                     for(int j = 0; j < 10; ++j)
//                         cout << (*phi)(i,j) << " ";
//                     cout << endl;
//                 }
// 

		// output vertex-edge pairs whose persistence is bigger than pers_thd
		for (size_t i=0;i<lowerCellList.size();i++){
			int tmp_int=low_1D_v2e[i];
			if (tmp_int==BIG_INT){
				continue;
			}
			++count_pairs;

			int vBirth=lowerCellList[i];
			int vDeath=upperCellList[tmp_int];

			double tmp_death=(*phi)(vList[vDeath]);
			double tmp_birth=(*phi)(vList[vBirth]);
			double tmp_pers=tmp_death-tmp_birth;

			MY_ASSERT(tmp_pers>=0);

			if (tmp_pers > pers_thd){				
				//write persistence pair into veList
				veList.push_back(PersPair<Vertex>(vList[vBirth],
					vList[vDeath],tmp_pers,tmp_birth, tmp_death));				

// 				cout << "BIRTH: " << vList[vBirth]+1 << " -- " << tmp_birth<< endl;
// 				cout << "DEATH: " << vList[vDeath]+1 << " -- " << tmp_death << endl;
// 
//				persRobM(vList[vBirth])+=tmp_pers;
//				persRobM(vList[vDeath])-=tmp_pers;				
				
				//save the reduction lists
				MatrixListType tmp_list;
				MY_ASSERT( ! red_list[ tmp_int ].empty() );
				for( MatrixListType::iterator tmpiter = red_list[ tmp_int ].begin(); tmpiter != red_list[ tmp_int ].end(); tmpiter ++ ){
					tmp_list = list_union( tmp_list, red_cell2v_list[ * tmpiter ] );
				}
				final_red_list.push_back( tmp_list );
//				cout << "red list: ";
//				for( MatrixListType::iterator tmpiter = red_list[tmp_int].begin(); tmpiter != red_list[tmp_int].end(); tmpiter ++ )
//					cout << red_cell2v_list[* tmpiter][0] << " " << red_cell2v_list[* tmpiter][red_cell2v_list[* tmpiter].size()-1] << " | ";
//				cout << endl;

//				for( MatrixListType::iterator tmpitit = tmp_list.begin(); tmpitit != tmp_list.end(); tmpitit ++ )
//				{
//					cout << vList[ *tmpitit] + 1 << "--" << (*phi)(vList[ *tmpitit]) << endl;
//				}	

				//save the boundary lists
				MatrixListType tmp_boundary_list;
				MY_ASSERT( ! bd_list[ tmp_int ].empty() );
				for( MatrixListType::iterator tmpiter = bd_list[ tmp_int ].begin(); tmpiter != bd_list[ tmp_int ].end(); tmpiter ++ ){
	//				if( tmp_boundary_list.empty() )
	//					tmp_boundary_list = bd_cell2v_list[ * tmpiter ];
	//				else 
						tmp_boundary_list = list_union( tmp_boundary_list, bd_cell2v_list[ * tmpiter ] );
				}
				final_boundary_list.push_back( tmp_boundary_list );
//				cout << "bd list: ";
//				for( MatrixListType::iterator tmpiter = bd_list[tmp_int].begin(); tmpiter != bd_list[tmp_int].end(); tmpiter ++ )
//					cout << bd_cell2v_list[* tmpiter][0] << " " << bd_cell2v_list[* tmpiter][bd_cell2v_list[* tmpiter].size()-1] << " | " ;
//				cout << endl;


			}		
		}		
	}

	double calcPersistence( blitz::Array<double, dim> * phi, const double pers_thd, 
		// blitz::Array<double, dim> * const persRobM, 
		vector<PersResultContainer> &result_lists, vector<Vertex> & _vList, const InputFileInfo &info)
	{

		time_t wholestart, wholeend, redstart, redend;
		double wholetime=0, redtime=0;

		time(&wholestart);		
		
		vector<Vertex> *vList = &_vList;

/***********   compute dummyVList, sizes and cell2v_lists *******/
		// we could save some memory by storing only one, but there's no point...
		vector<int> dummyV(vList->size());
		for (int i = 0; i < dummyV.size(); i++)
			dummyV[i] = i;
		vector<vector<int> > birth_lists(dim+1);
		birth_lists[0] = dummyV;

		vector< MatrixListType > dummyVList( vList->size() );
		for (int i = 0; i < dummyVList.size(); i++)
			dummyVList[i].push_back(i);
		vector< vector<MatrixListType > > cell2v_lists(dim+1);
		cell2v_lists[0] = dummyVList;

		int sizes[dim+1] = {0};

		{
			FiltrationGeneratorType filtration(phi);
			filtration.init(vList);

			for (int i = 0; i <= dim; i++){
				sizes[i] = filtration.getSizeInDim(i);
				filtration.initList(vList, &birth_lists[i], &cell2v_lists[i], i);
			}
		}						  

/****************************************************************/

		vector<vector<int> > low_arrays(dim+1);
		for (int i = 1; i <= dim; i++)
			low_arrays[i].assign(sizes[i-1], BIG_INT);					  					  
		vector<vector< MatrixListType > > boundaries(dim+1);
		
		vector<bool> willBeCleared(sizes[dim], false);						  
/********** reduction list *******************/

		// save for each negative simplex the simplices used to reduce it
		vector< MatrixListType > reduction_list;
		vector< MatrixListType > final_reduction_list;
		vector< MatrixListType > final_boundary_list;
/********************************************/

		int num_pairs[dim] = {0};

		for (int d = dim; d >= 1; d--)
		{						  
			{
				FiltrationGeneratorType filtration(phi);
				filtration.init(vList);								  
//				filtration.initList(vList, &birth_lists[d], &cell2v_lists[d], d);								  
				filtration.calculateBoundaries(vList, &boundaries[d], d, willBeCleared);
			}							  
			willBeCleared.assign(sizes[d-1], false);
			time(& redstart);

			reduceND(willBeCleared, birth_lists[d], boundaries[d], low_arrays[d], reduction_list);

			time(& redend);
			redtime += difftime(redend,redstart);

			cout << "reduced dimension " << d << endl;
			
			// save persistence, boundaries, red_list for this dim
			// so that the memory could be cleaned
			SavePersistence(phi, *vList, birth_lists[d-1], low_arrays[d], num_pairs[d-1], birth_lists[d], pers_thd, result_lists[d-1], 
				//*persRobM,
				reduction_list, cell2v_lists[d], final_reduction_list, boundaries[d], cell2v_lists[d-1], final_boundary_list);

		if( !info.from_python ){
			BinaryPersistentPairsSaver<dim> binSaver;
			stringstream output_red_file;
			output_red_file << info.input_path;
			output_red_file << ".red." << d;
			binSaver.saveOneDimReduction(final_reduction_list, (* vList), output_red_file.str().c_str(), d);		
			stringstream output_boundary_file;
			output_boundary_file << info.input_path;
			output_boundary_file << ".bnd." << d;
			binSaver.saveOneDimReduction(final_boundary_list, (* vList), output_boundary_file.str().c_str(), d);	
		}


			cout << "saved dimension " << d << endl;

			reduction_list.clear();
			final_reduction_list.clear();
			boundaries[d].clear();
			final_boundary_list.clear();
		}

		OUTPUT_MSG( "Reduction done" );

		time(& wholeend);
		wholetime = difftime(wholeend,wholestart);
		cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
		cout << "Filtration building time = " << (wholetime-redtime) / 60.0 << " Min" << endl;
		cout << "Reduction  time          = " << redtime / 60.0 << " Min" << endl;
		cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;

		OUTPUT_MSG( "Recording persistence pairs" )

/*
		for (int d = 0; d < dim; d++)
		{						  
			SavePersistence(phi, *vList, birth_lists[d], low_arrays[d+1], num_pairs[d], birth_lists[d+1], 
				pers_thd, result_lists[d], *persRobM,
				reduction_lists[d+1], cell2v_lists[d+1], final_reduction_lists[d]);

			cout << "saved dimension " << d << endl;
		}
*/
		MY_ASSERT(num_pairs[0]+1==birth_lists[0].size());
		for (int i = 1; i < dim - 1; i++)
			MY_ASSERT(num_pairs[i] + num_pairs[i+1]==birth_lists[i+1].size());
		MY_ASSERT(num_pairs[dim-1]==birth_lists[dim].size());		

		OUTPUT_MSG( "Finished");

		return 0;
	}

};

#endif
