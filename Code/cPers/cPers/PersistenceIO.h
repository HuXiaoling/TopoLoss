#ifndef PERSISTENCEIO_H
#define PERSISTENCEIO_H

#include <blitz/array.h>
#include <blitz/tinyvec-et.h>
#include <functional>
#include <vector>
//#include <malloc.h>
#include <ctime>
#include "Debugging.h"

//using namespace std;

typedef vector<int> MatrixListType;

/************************************************************************/
/* Save persistence values to binary file                               */
/************************************************************************/
int savePersistenceResults(const char *fileName,unsigned int *data, const vector<int> &header, int nrPairs);
int saveReductionResults(const char *fileName,unsigned int *data, const vector<int> &header, int nrPairs);

/************************************************************************/
/* Read persistence values from binary file                             */
/************************************************************************/
// int ReadPersistencyResults(char *fileName);

template<int d>
struct TextPersistentPairsSaver
{
template<typename ContT>
void savePers(vector<ContT> &res, const char * output_fname){

	OUTPUT_MSG( endl<< "---------- writing result ---------------" );

	cout << "writing text output to:  " << output_fname << endl;

	fstream output_filestr(output_fname, fstream::out | fstream::trunc);

	string names[] = {"Vertex", "Edge", "Face", "Cube", "4D-Cell", "5D-Cell"};

	for (int i = 0; i < d; i++)
	{
		//sort(res[i].begin(), res[i].end());
		output_filestr << names[i] << " " << names[i+1] << " Pairs, Number = " << res[i].size() << endl;
		typename ContT::iterator veiter;
		for(veiter=res[i].begin();veiter!=res[i].end();veiter++)
			output_filestr << veiter->birth << "\t" << veiter->death << endl;
	}	

	OUTPUT_MSG( endl<< "---------- writing result finished ---------------" );
}

};

template<int d>
struct BinaryPersistentPairsSaver
{
typedef blitz::TinyVector<int, d> Vertex; 

template<typename ContT>
void saveOutput(vector<ContT> &res, vector< Vertex > & vList, const char * output_fname)
{	
	assert(res.size() == d);	

	int count = 0;
	for (int i = 0; i < res.size(); i++){
		count += res[i].size();
	}

	string names[] = {"Vertex", "Edge", "Face", "Cube", "4D-Cell", "5D-Cell"};

	for (int i = 0; i < d; i++)			
		cout << names[i] << " " << names[i+1] << " Pairs = " << res[i].size() << endl;		

	unsigned int *pairArray = new unsigned int[count*2*d];

	int index = 0;

	vector<int> header(d);

	for (int i = 0; i < d; i++)
	{
			
		// do not sort
//		sort(res[i].begin(), res[i].end());

		typename ContT::iterator veiter;
		for(veiter=res[i].begin();veiter!=res[i].end();veiter++)
		{
/*
			cout <<" birth " << "\t";
			for( size_t ii = 0; ii < veiter->vertDim; ii ++ )
				cout << veiter->birthV[ ii ] << " " ;
			cout << endl;
			cout <<" death " << "\t";
			for( size_t ii = 0; ii < veiter->vertDim; ii ++ )
				cout << veiter->deathV[ ii ] << " " ;
			cout << endl;
			cout.flush();
*/
//			pairArray[index++] = veiter->birth;
//			pairArray[index++] = veiter->death;
			
			//Vertex birthcoord = vList[ vListveiter->birthV ];
			Vertex birthcoord = veiter->birthV ;
			for( size_t ii = 0; ii < d; ii ++ )
				pairArray[index++] = birthcoord[d-1-ii]+1;
	//		cout << "birth :" << birthcoord+1 << veiter->birth << endl;
			//Vertex deathcoord = vList[ vListveiter->deathV ];
			Vertex deathcoord = veiter->deathV;
			for( size_t ii = 0; ii < d; ii ++ )
				pairArray[index++] = deathcoord[ d-1-ii ]+1;
	//		cout << "death :" << deathcoord+1 << veiter->death << endl;

		}
		header[i] = res[i].size();
		cout << endl;
	}	

	savePersistenceResults(output_fname, pairArray, header, index);
}
void saveOneDimReduction(const vector< MatrixListType > & final_red_list, vector< Vertex > & vList, const char * output_fname, const int dSave)
{	
	MY_ASSERT( dSave > 0 );
	MY_ASSERT( dSave <= d );

	int count = 0;
	for( int j = 0; j < final_red_list.size(); j ++ ){
		assert( ! final_red_list[j].empty() );
		count += 1;
		count += final_red_list[j].size();
	}

	unsigned int *redArray = new unsigned int[count * d];

	int index = 0;

	vector<int> header(d);

	header[0] = final_red_list.size();

		for( vector< MatrixListType >::const_iterator red_list_iter = final_red_list.begin(); red_list_iter!=final_red_list.end();red_list_iter++)
		{
			// size of the reduction list of this dot
			assert( red_list_iter->size() != 0 );
			redArray[ index++ ] = red_list_iter->size();
			for( int j = 1; j < d; j ++ )
				redArray[ index++ ] = 0;

//			cout << red_list_iter - final_red_lists[i].begin() << " : " << (int) (red_list_iter->size()) << " -- ";

			// for each vertex in the red list, write its coordinates
			for( MatrixListType::const_iterator cellid_iter = red_list_iter->begin(); cellid_iter != red_list_iter->end(); cellid_iter ++ ){

				Vertex coord = vList[ * cellid_iter ];	
//				cout << * cellid_iter << " [";
				for( size_t ii = 0; ii < d; ii ++ ){
					redArray[ index++ ] = coord[ d-1-ii ] + 1;
//					cout << (int)(coord[ ii ] )+1 << "," ;
				}
//				cout << "] \n";
			}

//			cout << endl;

		}
//		cout << endl;

	saveReductionResults(output_fname, redArray, header, index);
}
void saveReduction(vector<vector< MatrixListType > > final_red_lists, vector< Vertex > & vList, const char * output_fname)
{	
	assert(final_red_lists.size() == d);	

	int count = 0;
	for (int i = 0; i < final_red_lists.size(); i++){
//		count += 2;	// dimension and number of persistence dots to save
		for( int j = 0; j < final_red_lists[ i ].size(); j ++ ){
			assert( ! final_red_lists[i][j].empty() );
			count += 1;
			count += final_red_lists[i][j].size();
		}
	}

	string names[] = {"Vertex", "Edge", "Face", "Cube", "4D-Cell", "5D-Cell"};

	for (int i = 0; i < d; i++)			
		cout << names[i] << " " << names[i+1] << " Pairs = " << final_red_lists[i].size() << endl;		

	unsigned int *redArray = new unsigned int[count * d];

	int index = 0;

	vector<int> header(d);

	for (int i = 0; i < d; i++)
	{
/*		// all zeros as deliminator
		for( int j = 0; j < d ; j ++ )
			redArray[ index++ ] = 0;

		redArray[ index++ ] = i+1;	// dimension
		redArray[ index++ ] = final_red_lists[ i ].size();	// number of dots
		for( int j = 2; j < d; j ++ )
			redArray[ index++ ] = 0;
*/
		for( vector< MatrixListType >::iterator red_list_iter = final_red_lists[i].begin(); red_list_iter!=final_red_lists[i].end();red_list_iter++)
		{
			// size of the reduction list of this dot
			assert( red_list_iter->size() != 0 );
			redArray[ index++ ] = red_list_iter->size();
			for( int j = 1; j < d; j ++ )
				redArray[ index++ ] = 0;

//			cout << red_list_iter - final_red_lists[i].begin() << " : " << (int) (red_list_iter->size()) << " -- ";

			// for each vertex in the red list, write its coordinates
			for( MatrixListType::iterator cellid_iter = red_list_iter->begin(); cellid_iter != red_list_iter->end(); cellid_iter ++ ){

				Vertex coord = vList[ * cellid_iter ];	
//				cout << * cellid_iter << " [";
				for( size_t ii = 0; ii < d; ii ++ ){
					redArray[ index++ ] = coord[ d-1-ii ] + 1;
//					cout << (int)(coord[ ii ] )+1 << "," ;
				}
//				cout << "] \n";
			}

//			cout << endl;

		}
		header[i] = final_red_lists[i].size();
//		cout << endl;
	}	

	saveReductionResults(output_fname, redArray, header, index);
}
};



#endif
