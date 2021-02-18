#!/bin/bash

echo Q4 - Tempos > ./output/mandelbrot.txt 2>&1

for num_procs in 1 2 4 8
do
    (
        echo mandelbrot_mpi2_nox_dist_n$num_procs
        for i in `seq 30`
        do
            time mpirun -n $num_procs ./bin/mandelbrot_mpi2_nox_dist
        done
    ) >> ./output/mandelbrot.txt 2>&1

    (
        echo mandelbrot_mpi1_nox_dist_n$num_procs
        for i in `seq 30`
        do
            time mpirun -n $num_procs ./bin/mandelbrot_mpi1_nox_dist
        done
    ) >> ./output/mandelbrot.txt 2>&1
done
