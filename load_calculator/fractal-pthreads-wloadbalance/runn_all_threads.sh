#!/bin/bash

cd ../../
if [ "$1" = 'input1-mandel' ]; then
        make run-serial-mandel;
fi

if [ "$1" = "input2-julia" ]; then
        make run-serial-julia;
fi

if [ "$1" = "input3-multiiulia" ]; then
        make run-serial-multiiulia;
fi
cd -

cd ..
./find_all_dem_loads.sh
cd -

make


echo "2 Threads"
export TH_NUM=2
time ./fractal-pthreads-wloadbalance $(cat $1)

echo "4 Threads"
export TH_NUM=4
time ./fractal-pthreads-wloadbalance $(cat $1)

echo "6 Threads"
export TH_NUM=6
time ./fractal-pthreads-wloadbalance $(cat $1)

echo "8 Threads"
export TH_NUM=8
time ./fractal-pthreads-wloadbalance $(cat $1)

echo "12 Threads"
export TH_NUM=12
time ./fractal-pthreads-wloadbalance $(cat $1)

echo "16 Threads"
export TH_NUM=16
time ./fractal-pthreads-wloadbalance $(cat $1)

echo "20 Threads"
export TH_NUM=20
time ./fractal-pthreads-wloadbalance $(cat $1)

echo "24 Threads"
export TH_NUM=24
time ./fractal-pthreads-wloadbalance $(cat $1)

