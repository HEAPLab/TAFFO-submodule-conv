#!/bin/bash

source common_compile.sh

ROOT='linear-algebra/kernels'
compile "2mm" $D_STANDARD_DATASET       29 32 61 64
compile "3mm" $D_STANDARD_DATASET       30 32 62 64
compile "atax" $D_STANDARD_DATASET      31 32 63 64
compile "bicg" $D_STANDARD_DATASET      31 32 63 64
compile "cholesky" $D_STANDARD_DATASET  29 32 62 64
compile "doitgen" $D_STANDARD_DATASET   11 32 43 64
compile "gemm" $D_STANDARD_DATASET      19 32 51 64
compile "gemver" $D_STANDARD_DATASET    28 32 60 64
compile "gesummv" $D_STANDARD_DATASET   28 32 60 64
compile "mvt" $D_STANDARD_DATASET        8 32 40 64
compile "symm" $D_STANDARD_DATASET      29 32 61 64
compile "syr2k" $D_STANDARD_DATASET     29 32 61 64
compile "syrk" $D_STANDARD_DATASET      25 32 57 64
compile "trisolv" $D_STANDARD_DATASET   29 32 57 64
compile "trmm" $D_STANDARD_DATASET      29 32 61 64
