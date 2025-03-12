#!/bin/bash
rm flop_distributions.csv 
g++-14 -std=c++20 -fopenmp -O2 main.cpp utils.cpp hash.cpp -o main
./main

