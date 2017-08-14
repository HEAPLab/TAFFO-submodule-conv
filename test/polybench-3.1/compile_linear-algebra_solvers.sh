#!/bin/bash

echo durbin
./magiclang.sh 'linear-algebra/solvers/durbin/durbin.c' '-O3' '-I utilities -I linear-algebra/solvers/durbin -DPOLYBENCH_TIME -DLARGE_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'durbin_out' 'utilities/polybench.c' '-fixpfracbitsamt=8 -fixpbitsamt=32' 

echo dynprog
./magiclang.sh 'linear-algebra/solvers/dynprog/dynprog.c' '-O3' '-I utilities -I linear-algebra/solvers/dynprog -DPOLYBENCH_TIME -DSTANDARD_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'dynprog_out' 'utilities/polybench.c' '-fixpfracbitsamt=16 -fixpbitsamt=32' 

echo gramschmidt
./magiclang.sh 'linear-algebra/solvers/gramschmidt/gramschmidt.c' '-O3' '-I utilities -I linear-algebra/solvers/gramschmidt -lm -DPOLYBENCH_TIME -DSMALL_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'gramschmidt_out' 'utilities/polybench.c' '-fixpfracbitsamt=18 -fixpbitsamt=32' 

echo lu
./magiclang.sh 'linear-algebra/solvers/lu/lu.c' '-O3' '-I utilities -I linear-algebra/solvers/lu -DPOLYBENCH_TIME -DSTANDARD_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'lu_out' 'utilities/polybench.c' '-fixpfracbitsamt=20 -fixpbitsamt=32' 

echo ludcmp
./magiclang.sh 'linear-algebra/solvers/ludcmp/ludcmp.c' '-O3' '-I utilities -I linear-algebra/solvers/ludcmp -DPOLYBENCH_TIME -DLARGE_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'ludcmp_out' 'utilities/polybench.c' '-fixpfracbitsamt=16 -fixpbitsamt=32'
