#!/bin/bash

g++-14 -std=c++20 -fopenmp -Ofast dist_flop.cpp ../src/utils.cpp ../src/hash.cpp -o flop_main
echo "Running main..."
time ./flop_main
end=`date +%s.%N`
rm flop_main
rm flop_distributions_*
rm features_*
