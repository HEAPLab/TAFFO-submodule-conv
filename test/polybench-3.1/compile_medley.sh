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


./magiclang.sh "medley/floyd-warshall/floyd-warshall.c" "-O3" "-I utilities -I medley/floyd-warshall -DPOLYBENCH_TIME -D$D_SMALL_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "floyd-warshall_out" "utilities/polybench.c" "-fixpfracbitsamt=$((23 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

./magiclang.sh "medley/reg_detect/reg_detect.c" "-O3" "-I utilities -I medley/reg_detect -DPOLYBENCH_TIME -D$D_LARGE_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "reg_detect_out" "utilities/polybench.c" "-fixpfracbitsamt=$((10 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 
