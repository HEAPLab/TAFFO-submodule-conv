#!/bin/bash -x

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


echo 2mm
./magiclang.sh "linear-algebra/kernels/2mm/2mm.c" "-O3" "-I utilities -I linear-algebra/kernels/2mm -DPOLYBENCH_TIME -D$D_STANDARD_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "2mm_out" "utilities/polybench.c" "-fixpfracbitsamt=$((28 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo 3mm
./magiclang.sh "linear-algebra/kernels/3mm/3mm.c" "-O3" "-I utilities -I linear-algebra/kernels/3mm -DPOLYBENCH_TIME -D$D_STANDARD_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "3mm_out" "utilities/polybench.c" "-fixpfracbitsamt=$((30 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo atax
./magiclang.sh "linear-algebra/kernels/atax/atax.c" "-O3" "-I utilities -I linear-algebra/kernels/atax -DPOLYBENCH_TIME -D$D_SMALL_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "atax_out" "utilities/polybench.c" "-fixpfracbitsamt=$((8 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo bicg
./magiclang.sh "linear-algebra/kernels/bicg/bicg.c" "-O3" "-I utilities -I linear-algebra/kernels/bicg -DPOLYBENCH_TIME -D$D_MINI_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "bicg_out" "utilities/polybench.c" "-fixpfracbitsamt=$((8 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo cholesky
./magiclang.sh "linear-algebra/kernels/cholesky/cholesky.c" "-O3" "-I utilities -I linear-algebra/kernels/cholesky -lm -DPOLYBENCH_TIME -D$D_MINI_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "cholesky_out" "utilities/polybench.c" "-fixpfracbitsamt=$((30 * $D_M)) -fixpbitsamt=$((32 * $D_M))"

echo doitgen
./magiclang.sh "linear-algebra/kernels/doitgen/doitgen.c" "-O3" "-I utilities -I linear-algebra/kernels/doitgen -DPOLYBENCH_TIME -D$D_MINI_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "doitgen_out" "utilities/polybench.c" "-fixpfracbitsamt=$((8 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo gemm
./magiclang.sh "linear-algebra/kernels/gemm/gemm.c" "-O3" "-I utilities -I linear-algebra/kernels/gemm -DPOLYBENCH_TIME -D$D_MINI_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "gemm_out" "utilities/polybench.c" "-fixpfracbitsamt=$((8 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo gemver
./magiclang.sh "linear-algebra/kernels/gemver/gemver.c" "-O3" "-I utilities -I linear-algebra/kernels/gemver -DPOLYBENCH_TIME -D$D_MINI_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "gemver_out" "utilities/polybench.c" "-fixpfracbitsamt=$((18 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo gesummv
./magiclang.sh "linear-algebra/kernels/gesummv/gesummv.c" "-O3" "-I utilities -I linear-algebra/kernels/gesummv -DPOLYBENCH_TIME -D$D_MINI_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "gesummv_out" "utilities/polybench.c" "-fixpfracbitsamt=$((8 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo mvt
./magiclang.sh "linear-algebra/kernels/mvt/mvt.c" "-O3" "-I utilities -I linear-algebra/kernels/mvt -DPOLYBENCH_TIME -D$D_MINI_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "mvt_out" "utilities/polybench.c" "-fixpfracbitsamt=$((8 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo symm
./magiclang.sh "linear-algebra/kernels/symm/symm.c" "-O3" "-I utilities -I linear-algebra/kernels/symm -DPOLYBENCH_TIME -D$D_MINI_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "symm_out" "utilities/polybench.c" "-fixpfracbitsamt=$((12 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo syr2k
./magiclang.sh "linear-algebra/kernels/syr2k/syr2k.c" "-O3" "-I utilities -I linear-algebra/kernels/syr2k -DPOLYBENCH_TIME -D$D_MINI_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "syr2k_out" "utilities/polybench.c" "-fixpfracbitsamt=$((8 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo syrk
./magiclang.sh "linear-algebra/kernels/syrk/syrk.c" "-O3" "-I utilities -I linear-algebra/kernels/syrk -DPOLYBENCH_TIME -D$D_MINI_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "syrk_out" "utilities/polybench.c" "-fixpfracbitsamt=$((8 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo trisolv
./magiclang.sh "linear-algebra/kernels/trisolv/trisolv.c" "-O3" "-I utilities -I linear-algebra/kernels/trisolv -DPOLYBENCH_TIME -D$D_MINI_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "trisolv_out" "utilities/polybench.c" "-fixpfracbitsamt=$((29 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

echo trmm
./magiclang.sh "linear-algebra/kernels/trmm/trmm.c" "-O3" "-I utilities -I linear-algebra/kernels/trmm -DPOLYBENCH_TIME -D$D_MINI_DATASET -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" "" "trmm_out" "utilities/polybench.c" "-fixpfracbitsamt=$((24 * $D_M)) -fixpbitsamt=$((32 * $D_M))" 

