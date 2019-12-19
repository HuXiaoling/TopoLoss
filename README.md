# Topological loss

Theoretically speaking, the loss function can be incorporated into any suitable framework.
The function is used in PyTorch. And there are two ways to incorporate this loss function into your framework:  
1) Update the total gradient (e.g. cross entropy gradient + lambda * topo gradient) when backpropagation;  
2) Our loss function is actually defined on critical pixels, and you can conduct your total loss (e.g. cross entropy loss + lambda * topo loss) based on the repository. And do the else as usual.

## Content directory
Data/:                                   data folder

Results/:                                storing results

Code/TDFPython/TDFMain.py:               main python script (run it to start), results are all written in Results folder

Code/TDFPython/PersistencePython.so:     persistence code, wrapped as a static library (most likely you would have to recompile this library by yourself)

Code/cPers/:                             c++ persistence code

Code/cPers/cPers/compile_pers_lib.sh:    script to compile the library

Code/cPers/cPers/PersistencePython.cpp:  root file for the python library

pybind11-stable.zip:                     tools used for compiling c++ code into library (python wrapper)
