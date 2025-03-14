#!/bin/bash

g++-14 -std=c++20 -fopenmp -O3 -mtune=native dist_turn.cpp ../src/utils.cpp ../src/hash.cpp -o turn_main
echo "Running main..."
./turn_main
rm turn_main
