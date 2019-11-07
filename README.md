# C++ code for topological loss

Data/:                                   data folder

Results/:                                storing results

Code/TDFPython/TDFMain.py:               main python script (run it to start), results are all written in Results folder

Code/TDFPython/PersistencePython.so:     persistence code, wrapped as a static library (most likely you would have to recompile this library by yourself)

Code/cPers/:                             c++ persistence code

Code/cPers/cPers/compile_pers_lib.sh:    script to compile the library

Code/cPers/cPers/PersistencePython.cpp:  root file for the python library

pybind11-stable.zip:                     tools used for compiling c++ code into library (python wrapper)
