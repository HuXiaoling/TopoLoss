// Persistent Homology code by Hubert Wagner based on an implementation by Chao Chen
#include <cmath>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <deque>
#include <cstring>
#include <ctime>
#include <blitz/array.h>
#include <blitz/tinyvec-et.h>

using namespace std;

const int BIG_INT = 0x7FFFFFFF;	//be careful, could be too small compared to # of cubes
typedef int CellNrType; // it could be long for really large inputs...

#include "PersistenceIO.h"
#include "Debugging.h"
#include "GeneralFiltration.h"

#include "InputRunner.h"

#include "PersistentPair.h"
#include "DataReaders.h"

#include "PersistenceCalculator.h"
#include "PersistenceCalcRunner.h"


// python thingy
#include <pybind11/pybind11.h>
#include <iostream>

namespace py = pybind11;

#include <pybind11/stl.h>

// std::vector<std::vector<double> > kw_func4(std::vector<std::vector<double> > &entries) {
//     std::vector<std::vector<double> > try_vec(2);
//     std::cout << "kw_func4: " << std::endl;
//     for (std::vector<double> i : entries){
//         for (double j : i){
//             std::cout << j << " ";
//             try_vec[0].push_back(j);
//             try_vec[1].push_back(j*j);
//         }
//         std::cout << endl;
//     }
//     return try_vec;
// }
// 

std::vector<std::vector<double> > cubePers(std::vector< double > &entries, std::vector<int> dims, double pers_thd ) {
	string lfile = "log.txt";	
	string efile = "error.txt";	
	DebuggerClass::init( true, lfile, efile );
	
        int dim = dims.size();
	InputFileInfo input_file_info(dim);
	
	int max_pers_pts = BIG_INT; // the maximal number of persistence pairs recorded
        
	time_t startTime1, startTime2, endTime;		
	time(&startTime1);		

        std::vector<std::vector< double > > ret;

	switch(input_file_info.dimension)
	{
	case 1:
		ret = InputRunner<1>::run(input_file_info, dims, entries, pers_thd);
		break;
	case 2:
		ret = InputRunner<2>::run(input_file_info, dims, entries, pers_thd);
		break;
	case 3:
		ret = InputRunner<3>::run(input_file_info, dims, entries, pers_thd);
		break;
	case 4:
		ret = InputRunner<4>::run(input_file_info, dims, entries, pers_thd);
		break;
	case 5:
		ret = InputRunner<5>::run(input_file_info, dims, entries, pers_thd);
		break;
	case 6:
		ret = InputRunner<6>::run(input_file_info, dims, entries, pers_thd);
		break;	
	case 7:
		ret = InputRunner<7>::run(input_file_info, dims, entries, pers_thd);
		break;	
	case 8:
		ret = InputRunner<8>::run(input_file_info, dims, entries, pers_thd);
		break;	
        default:
                assert(false);
	}

	time(&endTime);
	double ellapsed1 = difftime (endTime,startTime1);	

	cout << "All calculations computed in " << setprecision(2) << ellapsed1/60.0 << endl;
	DebuggerClass::finish();
	
	return ret;
}

PYBIND11_PLUGIN(PersistencePython) {
        py::module m("PersistencePython", "python binding for persistence computation (cubical complex)");

//    m.def("kw_func4", &kw_func4, py::arg("myList") = list);
    m.def("cubePers", &cubePers);
    return m.ptr();
}
