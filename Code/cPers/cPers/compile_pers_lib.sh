#!/usr/bin/env bash

g++ -O3 -w -shared -std=c++11 -I ../ -I pybind11-stable/include `python3.6-config --cflags --ldflags --libs` PersistencePython.cpp Debugging.cpp PersistenceIO.cpp -o ../../TDFPython/PersistencePython.so
