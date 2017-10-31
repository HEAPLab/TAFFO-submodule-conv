#!/bin/bash

source common_compile.sh

ROOT='linear-algebra/blas'
compile "gemm" $D_STANDARD_DATASET      19 32 51 64
compile "gemver" $D_STANDARD_DATASET    28 32 60 64
compile "gesummv" $D_STANDARD_DATASET   28 32 60 64
compile "symm" $D_STANDARD_DATASET      29 32 61 64
compile "syr2k" $D_STANDARD_DATASET     29 32 61 64
compile "syrk" $D_STANDARD_DATASET      25 32 57 64
compile "trmm" $D_STANDARD_DATASET      29 32 61 64
