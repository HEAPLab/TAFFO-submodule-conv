#!/bin/bash

source common_compile.sh

ROOT='linear-algebra/kernels'
compile "2mm" $D_STANDARD_DATASET 28 32
compile "3mm" $D_STANDARD_DATASET 30 32
compile "atax" $D_STANDARD_DATASET 31 32
compile "bicg" $D_STANDARD_DATASET 31 32
compile "cholesky" $D_STANDARD_DATASET 30 32
compile "doitgen" $D_STANDARD_DATASET 11 32
compile "gemm" $D_STANDARD_DATASET 19 32
compile "gemver" $D_STANDARD_DATASET 28 32
compile "gesummv" $D_SMALL_DATASET 18 32
compile "mvt" $D_STANDARD_DATASET 8 32
compile "symm" $D_STANDARD_DATASET 29 32
compile "syr2k" $D_STANDARD_DATASET 26 32
compile "syrk" $D_STANDARD_DATASET 25 32
compile "trisolv" $D_STANDARD_DATASET 29 32
compile "trmm" $D_STANDARD_DATASET 29 32
