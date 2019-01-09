#!/bin/bash

make

./load_calc ../gnuplot.dat 2 2_th_load.txt
./load_calc ../gnuplot.dat 4 4_th_load.txt
./load_calc ../gnuplot.dat 6 6_th_load.txt
./load_calc ../gnuplot.dat 8 8_th_load.txt
./load_calc ../gnuplot.dat 12 12_th_load.txt
./load_calc ../gnuplot.dat 16 16_th_load.txt
./load_calc ../gnuplot.dat 20 20_th_load.txt
./load_calc ../gnuplot.dat 24 24_th_load.txt
