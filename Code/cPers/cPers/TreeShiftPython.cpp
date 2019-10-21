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

// python thingy
#include <pybind11/pybind11.h>

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

std::vector<std::vector<double> > TreeShift(std::vector< double > &entries, std::vector<int> dims, double thd, int delta, bool verbose_mode ) {
	
        int dim = dims.size();
        assert(dim <= 2);
        assert(dim > 0);
        if (dim < 2)
            dims.append(1);

        std::vector<std::vector< double > > ret(dim[0] std::vector<double>(dim[1]));

        int counter = 0;
        if  (verbose_mode){
            cout << " thd = " << thd << endl;
            cout << " delta = " << delta << endl;
            cout << " verbose = " << verbose_mode << endl;
            cout << " matrix size = " << dims[0] << " X " << dims[1] << endl;
            cout << "Matrix: " << endl;
            counter = 0;
            for(int i = 0; i < dims[0]; ++i){
                for(int j = 0; j < dims[1]; ++j){
                    cout << entries[counter++];
                }
            }
	    cout << "All calculations computed !" << endl;
        }
        	
            counter = 0;
            for(int i = 0; i < dims[0]; ++i){
                for(int j = 0; j < dims[1]; ++j){
                    ret[i][j] = entries[counter++];
	return ret;
}

PYBIND11_PLUGIN(TreeShiftPython) {
        py::module m("TreeShiftPython", "python binding for tree shift algorithm");

//    m.def("kw_func4", &kw_func4, py::arg("myList") = list);
    m.def("TreeShift", &TreeShift);
    return m.ptr();
}
