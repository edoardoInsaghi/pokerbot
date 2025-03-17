#!/bin/bash

g++-14 -std=c++20 -Wstack-usage=8192 -fopenmp -O3 -mtune=native canon_distributions_flop.cpp ../src/utils.cpp ../src/hash.cpp -o canon_distributions_flop
echo "Running main..."
./canon_distributions_flop
rm canon_distributions_flop
