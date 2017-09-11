#!/bin/bash

source common_compile.sh

ROOT='stencils'
compile "adi" $D_MINI_DATASET 12 64   # won't actually work in 32 bit mode, set 64bit on the command line
compile "fdtd-2d" $D_SMALL_DATASET 24 32
compile "fdtd-apml" $D_MINI_DATASET 12 32
compile "jacobi-1d-imper" $D_STANDARD_DATASET 24 32
compile "jacobi-2d-imper" $D_STANDARD_DATASET 18 32
compile "seidel-2d" $D_STANDARD_DATASET 16 32
