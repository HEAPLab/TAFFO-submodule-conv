#!/bin/bash

echo correlation
./magiclang.sh 'datamining/correlation/correlation.c' '-O3' '-I utilities -I dataminiming/correlation -lm -DPOLYBENCH_TIME -DSTANDARD_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'correlation_out' 'utilities/polybench.c' '-fixpfracbitsamt=21 -fixpbitsamt=32' 

echo covariance
./magiclang.sh 'datamining/covariance/covariance.c' '-O3' '-I utilities -I dataminiming/covariance -lm -DPOLYBENCH_TIME -DSTANDARD_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'covariance_out' 'utilities/polybench.c' '-fixpfracbitsamt=16 -fixpbitsamt=32'
