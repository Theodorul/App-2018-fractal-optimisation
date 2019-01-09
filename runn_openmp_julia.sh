#!/bin/bash

echo "2 Threads"
export OMP_NUM_THREADS=2
make run-parallel-openmp-julia

echo "4 Threads"
export OMP_NUM_THREADS=4
make run-parallel-openmp-julia

echo "6 Threads"
export OMP_NUM_THREADS=6
make run-parallel-openmp-julia

echo "8 Threads"
export OMP_NUM_THREADS=8
make run-parallel-openmp-julia

echo "12 Threads"
export OMP_NUM_THREADS=12
make run-parallel-openmp-julia

echo "16 Threads"
export OMP_NUM_THREADS=16
make run-parallel-openmp-julia

echo "20 Threads"
export OMP_NUM_THREADS=20
make run-parallel-openmp-julia

echo "24 Threads"
export OMP_NUM_THREADS=24
make run-parallel-openmp-julia
