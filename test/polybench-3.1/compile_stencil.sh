#!/bin/bash

source common_compile.sh

ROOT='stencils'
compile "adi" $D_STANDARD_DATASET             21 32 53 64
compile "fdtd-2d" $D_STANDARD_DATASET         23 32 56 64
compile "fdtd-apml" $D_STANDARD_DATASET       18 32 49 64
compile "jacobi-1d-imper" $D_STANDARD_DATASET 29 32 61 64
compile "jacobi-2d-imper" $D_STANDARD_DATASET 18 32 50 64
compile "seidel-2d" $D_STANDARD_DATASET       17 32 49 64
