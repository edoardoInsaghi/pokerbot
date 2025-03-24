#!/bin/bash

g++-14 -std=c++20 -Wstack-usage=8192 -fopenmp -O3 -mtune=native canonicalise_distributions.cpp ../src/utils.cpp ../src/hash.cpp -o main
echo "Running main..."
./main
rm main
