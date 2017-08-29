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


echo durbin
./magiclang.sh "linear-algebra/solvers/durbin/durbin.c" "-O3" "-I utilities -I linear-algebra/solvers/durbin -DPOLYBENCH_TIME -D$D_SMALL_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "durbin_out" "utilities/polybench.c" "-fixpfracbitsamt=$((8 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo dynprog
./magiclang.sh "linear-algebra/solvers/dynprog/dynprog.c" "-O3" "-I utilities -I linear-algebra/solvers/dynprog -DPOLYBENCH_TIME -D$D_STANDARD_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "dynprog_out" "utilities/polybench.c" "-fixpfracbitsamt=$((16 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo gramschmidt
./magiclang.sh "linear-algebra/solvers/gramschmidt/gramschmidt.c" "-O3" "-I utilities -I linear-algebra/solvers/gramschmidt -lm -DPOLYBENCH_TIME -D$D_SMALL_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "gramschmidt_out" "utilities/polybench.c" "-fixpfracbitsamt=$((26 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo lu
./magiclang.sh "linear-algebra/solvers/lu/lu.c" "-O3" "-I utilities -I linear-algebra/solvers/lu -DPOLYBENCH_TIME -D$D_STANDARD_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "lu_out" "utilities/polybench.c" "-fixpfracbitsamt=$((20 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo ludcmp
./magiclang.sh "linear-algebra/solvers/ludcmp/ludcmp.c" "-O3" "-I utilities -I linear-algebra/solvers/ludcmp -DPOLYBENCH_TIME -D$D_SMALL_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "ludcmp_out" "utilities/polybench.c" "-fixpfracbitsamt=$((16 * $D_M)) -fixpbitsamt=$((32 * $D_M))"
