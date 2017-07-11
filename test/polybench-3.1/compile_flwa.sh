#!/bin/bash


./magiclang.sh 'medley/floyd-warshall/floyd-warshall.c' '-O3' '-I utilities -I medley/floyd-warshall -DPOLYBENCH_TIME -DSMALL_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'floyd-warshall_out' 'utilities/polybench.c' '-fixpfracbitsamt=23 -fixpbitsamt=32' 


