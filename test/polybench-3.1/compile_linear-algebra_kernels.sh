#!/bin/bash

echo 2mm
./magiclang.sh 'linear-algebra/kernels/2mm/2mm.c' '-O3' '-I utilities -I linear-algebra/kernels/2mm -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' '2mm_out' 'utilities/polybench.c' '-fixpfracbitsamt=8 -fixpbitsamt=32' 

echo 3mm
./magiclang.sh 'linear-algebra/kernels/3mm/3mm.c' '-O3' '-I utilities -I linear-algebra/kernels/3mm -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' '3mm_out' 'utilities/polybench.c' '-fixpfracbitsamt=8 -fixpbitsamt=32' 

echo atax
./magiclang.sh 'linear-algebra/kernels/atax/atax.c' '-O3' '-I utilities -I linear-algebra/kernels/atax -DPOLYBENCH_TIME -DSMALL_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'atax_out' 'utilities/polybench.c' '-fixpfracbitsamt=8 -fixpbitsamt=32' 

echo bicg
./magiclang.sh 'linear-algebra/kernels/bicg/bicg.c' '-O3' '-I utilities -I linear-algebra/kernels/bicg -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'bicg_out' 'utilities/polybench.c' '-fixpfracbitsamt=8 -fixpbitsamt=32' 

echo cholesky
./magiclang.sh 'linear-algebra/kernels/cholesky/cholesky.c' '-O3' '-I utilities -I linear-algebra/kernels/cholesky -lm -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'cholesky_out' 'utilities/polybench.c' '-fixpfracbitsamt=30 -fixpbitsamt=32'

echo doitgen
./magiclang.sh 'linear-algebra/kernels/doitgen/doitgen.c' '-O3' '-I utilities -I linear-algebra/kernels/doitgen -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'doitgen_out' 'utilities/polybench.c' '-fixpfracbitsamt=8 -fixpbitsamt=32' 

echo gemm
./magiclang.sh 'linear-algebra/kernels/gemm/gemm.c' '-O3' '-I utilities -I linear-algebra/kernels/gemm -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'gemm_out' 'utilities/polybench.c' '-fixpfracbitsamt=8 -fixpbitsamt=32' 

echo gemver
./magiclang.sh 'linear-algebra/kernels/gemver/gemver.c' '-O3' '-I utilities -I linear-algebra/kernels/gemver -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'gemver_out' 'utilities/polybench.c' '-fixpfracbitsamt=18 -fixpbitsamt=32' 

echo gesummv
./magiclang.sh 'linear-algebra/kernels/gesummv/gesummv.c' '-O3' '-I utilities -I linear-algebra/kernels/gesummv -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'gesummv_out' 'utilities/polybench.c' '-fixpfracbitsamt=8 -fixpbitsamt=32' 

echo mvt
./magiclang.sh 'linear-algebra/kernels/mvt/mvt.c' '-O3' '-I utilities -I linear-algebra/kernels/mvt -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'mvt_out' 'utilities/polybench.c' '-fixpfracbitsamt=8 -fixpbitsamt=32' 

echo symm
./magiclang.sh 'linear-algebra/kernels/symm/symm.c' '-O3' '-I utilities -I linear-algebra/kernels/symm -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'symm_out' 'utilities/polybench.c' '-fixpfracbitsamt=12 -fixpbitsamt=32' 

echo syr2k
./magiclang.sh 'linear-algebra/kernels/syr2k/syr2k.c' '-O3' '-I utilities -I linear-algebra/kernels/syr2k -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'syr2k_out' 'utilities/polybench.c' '-fixpfracbitsamt=8 -fixpbitsamt=32' 

echo syrk
./magiclang.sh 'linear-algebra/kernels/syrk/syrk.c' '-O3' '-I utilities -I linear-algebra/kernels/syrk -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'syrk_out' 'utilities/polybench.c' '-fixpfracbitsamt=8 -fixpbitsamt=32' 

echo trisolv
./magiclang.sh 'linear-algebra/kernels/trisolv/trisolv.c' '-O3' '-I utilities -I linear-algebra/kernels/trisolv -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'trisolv_out' 'utilities/polybench.c' '-fixpfracbitsamt=8 -fixpbitsamt=32' 

echo trmm
./magiclang.sh 'linear-algebra/kernels/trmm/trmm.c' '-O3' '-I utilities -I linear-algebra/kernels/trmm -DPOLYBENCH_TIME -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS' '' 'trmm_out' 'utilities/polybench.c' '-fixpfracbitsamt=24 -fixpbitsamt=32' 

