#ifndef INCLUDED_REDUCTION_H
#define INCLUDED_REDUCTION_H

#include "GeneralFiltration.h"

// This function reduces a boundary matrix represented by its 'low_array'.
void reduceND(vector<bool> &willBeCleared, vector<CellNrType> &upperList, vector<MatrixListType> &boundary_upper, vector<int> &low_array, vector< MatrixListType > & reduction_list) 
{
	OUTPUT_MSG("Reducing cells, total number = " << upperList.size());
	for(size_t i=0, sz = upperList.size(); i < sz; i++){
		reduction_list.push_back( MatrixListType() );
		MY_ASSERT( reduction_list.size() == i + 1 );
		
		if (boundary_upper[i].empty())
			continue;

		reduction_list[i].push_back( i );
		// initialize the reduction list first
		
		int low = boundary_upper[i].back();
		int column_used=0;
		while (!boundary_upper[i].empty() && low_array[low]!=BIG_INT){
			assert(low_array[low] < i);
			assert(low == boundary_upper[low_array[low]].back());
			assert(!boundary_upper[i].empty());
			assert(!boundary_upper[low_array[low]].empty());

			boundary_upper[i]=list_sym_diff(boundary_upper[i], boundary_upper[low_array[low]]);
			// update the reduction list as well
			reduction_list[i]=list_sym_diff(reduction_list[i], reduction_list[low_array[low]]);
			if(!boundary_upper[i].empty()){
				int old_low=low;
				low = boundary_upper[i].back();
				assert(low<old_low);
			}

			column_used++;		
		}
		if (!boundary_upper[i].empty()){
			assert(low>=0);
			assert(low_array[low]==BIG_INT);
			low_array[low]=i;								  

			willBeCleared[low] = true;			
		}else{
			// a column has to remain non-empty after reduction, 
			// because this is a negative column
			assert( false );
		}
		
	}

	// MY_ASSERT(num_lower_creator==num_upper_destroyer);
	// MY_ASSERT(num_upper_destroyer==upperList->size());

	//myclear(boundary_upper);
}

#endif
