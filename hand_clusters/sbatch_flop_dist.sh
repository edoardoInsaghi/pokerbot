#!/bin/bash

#SBATCH --job-name=RLpoker_flop
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=128
#SBATCH --cpus-per-task=1
#SBATCH --time=2:00:00
#SBATCH --partition=EPYC
#SBATCH --output=flop_out.out
#SBATCH --error=flop_err.err   

mkdir ~/tmp
export TMPDIR=~/tmp

g++ -std=c++20 -fopenmp -O3 -march=native -mtune=native -funroll_loops dist_flop.cpp ../src/utils.cpp ../src/hash.cpp -o flop_main

./flop_main

rm flop_main
