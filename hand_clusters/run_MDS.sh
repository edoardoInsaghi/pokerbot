#!/bin/bash

g++-14 -std=c++20 -fopenmp -O3 -mtune=native MDS.cpp ../src/utils.cpp ../src/hash.cpp -o MDS_main
echo "Running main..."
./MDS_main
rm MDS_main


