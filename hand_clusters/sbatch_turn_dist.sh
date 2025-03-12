#!/bin/bash

#SBATCH --job-name=RLpoker_turn
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=128
#SBATCH --cpus-per-task=1
#SBATCH --time=2:00:00
#SBATCH --partition=EPYC
#SBATCH --output=out.out
#SBATCH --error=err.err   
mkdir ~/tmp
export TMPDIR=~/tmp
g++ -std=c++20 -fopenmp -O3 dist_turn.cpp ../src/utils.cpp ../src/hash.cpp -o turn_main
./turn_main
rm turn_main
