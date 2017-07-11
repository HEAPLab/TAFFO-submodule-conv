#!/bin/bash

./magiclang.sh 'linear-algebra/kernels/atax/atax.c' '-O3' '-I utilities -I linear-algebra/kernels/atax -DPOLYBENCH_TIME -DSMALL_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'atax_out' 'utilities/polybench.c' '-fixpfracbitsamt=8 -fixpbitsamt=32' 



