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


echo adi
./magiclang.sh "stencils/adi/adi.c" "-O3" "-I utilities -I stencils/adi -lm -DPOLYBENCH_TIME -D$D_MINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "adi_out" "utilities/polybench.c" "-fixpfracbitsamt=32 -fixpbitsamt=64" 

echo fdtd-2d
./magiclang.sh "stencils/fdtd-2d/fdtd-2d.c" "-O3" "-I utilities -I stencils/fdtd-2d -DPOLYBENCH_TIME -D$D_SMALL_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "fdtd-2d_out" "utilities/polybench.c" "-fixpfracbitsamt=$((24 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo fdtd-apml
./magiclang.sh "stencils/fdtd-apml/fdtd-apml.c" "-O3" "-I utilities -I stencils/fdtd-apml -DPOLYBENCH_TIME -D$D_MINI_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "fdtd-apml_out" "utilities/polybench.c" "-fixpfracbitsamt=$((16 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo jacobi-1d-imper
./magiclang.sh "stencils/jacobi-1d-imper/jacobi-1d-imper.c" "-O3" "-I utilities -I stencils/jacobi-1d-imper -DPOLYBENCH_TIME -D$D_STANDARD_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "jacobi-1d-imper_out" "utilities/polybench.c" "-fixpfracbitsamt=$((24 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo jacobi-2d-imper
./magiclang.sh "stencils/jacobi-2d-imper/jacobi-2d-imper.c" "-O3" "-I utilities -I stencils/jacobi-2d-imper -DPOLYBENCH_TIME -D$D_STANDARD_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "jacobi-2d-imper_out" "utilities/polybench.c" "-fixpfracbitsamt=$((18 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo seidel-2d
./magiclang.sh "stencils/seidel-2d/seidel-2d.c" "-O3" "-I utilities -I stencils/seidel-2d -DPOLYBENCH_TIME -D$D_STANDARD_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "seidel-2d_out" "utilities/polybench.c" "-fixpfracbitsamt=$((16 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 
