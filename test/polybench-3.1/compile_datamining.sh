#!/bin/bash

D_MINI_DATASET="MINI_DATASET"
D_SMALL_DATASET="SMALL_DATASET"
D_STANDARD_DATASET="STANDARD_DATASET"
D_LARGE_DATASET="LARGE_DATASET"
D_EXTRALARGE_DATASET="EXTRALARGE_DATASET"
D_M=1
D_DATA_TYPE='__attribute__((annotate("no_float")))float'

for arg; do
  case $arg in
    64bit)
      D_M=2
      D_DATA_TYPE='__attribute__((annotate("no_float")))double'
      ;;
    [A-Z]*_DATASET)
      D_MINI_DATASET=$arg
      D_SMALL_DATASET=$arg
      D_STANDARD_DATASET=$arg
      D_LARGE_DATASET=$arg
      D_EXTRALARGE_DATASET=$arg
      ;;
    *)
      echo Unrecognized option $arg
      exit 1
  esac
done


echo correlation
./magiclang.sh "datamining/correlation/correlation.c" "-O3" "-I utilities -I dataminiming/correlation -lm -DPOLYBENCH_TIME -D$D_SMALL_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS -DDATA_TYPE=$D_DATA_TYPE" "" "correlation_out" "utilities/polybench.c" "-fixpfracbitsamt=$((27 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo covariance
./magiclang.sh "datamining/covariance/covariance.c" "-O3" "-I utilities -I dataminiming/covariance -lm -DPOLYBENCH_TIME -D$D_STANDARD_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS -DDATA_TYPE=$D_DATA_TYPE" "" "covariance_out" "utilities/polybench.c" "-fixpfracbitsamt=$((16 * $D_M)) -fixpbitsamt=$((32 * $D_M))"
