#!/bin/bash

echo adi
./magiclang.sh 'stencils/adi/adi.c' '-O3' '-I utilities -I stencils/adi -lm -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'adi_out' 'utilities/polybench.c' '-fixpfracbitsamt=32 -fixpbitsamt=64' 

echo fdtd-2d
./magiclang.sh 'stencils/fdtd-2d/fdtd-2d.c' '-O3' '-I utilities -I stencils/fdtd-2d -DPOLYBENCH_TIME -DSTANDARD_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'fdtd-2d_out' 'utilities/polybench.c' '-fixpfracbitsamt=24 -fixpbitsamt=32' 

echo fdtd-apml
./magiclang.sh 'stencils/fdtd-apml/fdtd-apml.c' '-O3' '-I utilities -I stencils/fdtd-apml -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'fdtd-apml_out' 'utilities/polybench.c' '-fixpfracbitsamt=16 -fixpbitsamt=32' 

echo jacobi-1d-imper
./magiclang.sh 'stencils/jacobi-1d-imper/jacobi-1d-imper.c' '-O3' '-I utilities -I stencils/jacobi-1d-imper -DPOLYBENCH_TIME -DSTANDARD_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'jacobi-1d-imper_out' 'utilities/polybench.c' '-fixpfracbitsamt=24 -fixpbitsamt=32' 

echo jacobi-2d-imper
./magiclang.sh 'stencils/jacobi-2d-imper/jacobi-2d-imper.c' '-O3' '-I utilities -I stencils/jacobi-2d-imper -DPOLYBENCH_TIME -DSTANDARD_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'jacobi-2d-imper_out' 'utilities/polybench.c' '-fixpfracbitsamt=18 -fixpbitsamt=32' 

echo seidel-2d
./magiclang.sh 'stencils/seidel-2d/seidel-2d.c' '-O3' '-I utilities -I stencils/seidel-2d -DPOLYBENCH_TIME -DSTANDARD_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'seidel-2d_out' 'utilities/polybench.c' '-fixpfracbitsamt=16 -fixpbitsamt=32' 
