#!/bin/bash

g++-14 -std=c++20 -fopenmp -O3 -mtune=native test_canon.cpp ../src/utils.cpp ../src/hash.cpp -o test_canon
echo "Running main..."
./test_canon
rm test_canon
