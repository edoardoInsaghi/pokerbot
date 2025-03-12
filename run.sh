#!/bin/bash

if test -d "build"; then
    rm -rf build
fi

mkdir build && cd build
cmake ..
make
echo "Running main..."
./main

if test -f "logs.csv"; then
    cat logs.csv > ../logs.csv
    rm logs.csv
fi

if test -f "results.csv"; then
    cat results.csv > ../results.csv
    rm results.csv
fi

rm main
