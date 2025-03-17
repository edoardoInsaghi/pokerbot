#!/bin/bash

g++-14 -std=c++20 -fopenmp -O3 -mtune=native test_canon_duplicates_flop.cpp ../src/utils.cpp ../src/hash.cpp -o main
echo "Running main..."
./main
rm main