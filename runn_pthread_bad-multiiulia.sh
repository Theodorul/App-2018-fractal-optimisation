#!/bin/bash

echo "2 Threads"
export TH_NUM=2
make run-parallel-pthreads-multiiulia

echo "4 Threads"
export TH_NUM=4
make run-parallel-pthreads-multiiulia

echo "6 Threads"
export TH_NUM=6
make run-parallel-pthreads-multiiulia

echo "8 Threads"
export TH_NUM=8
make run-parallel-pthreads-multiiulia

echo "12 Threads"
export TH_NUM=12
make run-parallel-pthreads-multiiulia

echo "16 Threads"
export TH_NUM=16
make run-parallel-pthreads-multiiulia

echo "20 Threads"
export TH_NUM=20
make run-parallel-pthreads-multiiulia

echo "24 Threads"
export TH_NUM=24
make run-parallel-pthreads-multiiulia
