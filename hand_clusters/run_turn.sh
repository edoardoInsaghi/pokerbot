#!/bin/bash

g++-14 -std=c++20 -fopenmp -O3 dist_turn.cpp ../src/utils.cpp ../src/hash.cpp -o turn_main
echo "Running main..."
./turn_main
end=`date +%s.%N`
rm turn_main
