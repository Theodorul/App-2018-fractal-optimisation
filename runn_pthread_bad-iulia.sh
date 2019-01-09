#!/bin/bash

echo "2 Threads"
export TH_NUM=2
make run-parallel-pthreads-julia

echo "4 Threads"
export TH_NUM=4
make run-parallel-pthreads-julia

echo "6 Threads"
export TH_NUM=6
make run-parallel-pthreads-julia

echo "8 Threads"
export TH_NUM=8
make run-parallel-pthreads-julia

echo "12 Threads"
export TH_NUM=12
make run-parallel-pthreads-julia

echo "16 Threads"
export TH_NUM=16
make run-parallel-pthreads-julia

echo "20 Threads"
export TH_NUM=20
make run-parallel-pthreads-julia

echo "24 Threads"
export TH_NUM=24
make run-parallel-pthreads-julia
