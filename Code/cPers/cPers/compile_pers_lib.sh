#!/usr/bin/env bash

# Sammy's setup: use `/usr/local/Cellar/python/2.7.13/bin/python-config`
g++ -O3 -w -shared -std=c++11 -I ../ -I pybind11-stable/include `python3.6-config --cflags --ldflags --libs` PersistencePython.cpp Debugging.cpp PersistenceIO.cpp -o ../../TDFPython/PersistencePython.so
