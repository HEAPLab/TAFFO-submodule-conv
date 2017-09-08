#!/bin/bash


./compile_linear-algebra_kernels.sh "$@" &
./compile_linear-algebra_solvers.sh "$@" &
./compile_datamining.sh "$@" &
./compile_medley.sh "$@" &
./compile_stencil.sh "$@" &
wait


