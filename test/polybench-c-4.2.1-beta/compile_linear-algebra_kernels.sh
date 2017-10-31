#!/bin/bash

source common_compile.sh

ROOT='linear-algebra/kernels'
compile "2mm" $D_STANDARD_DATASET       29 32 61 64
compile "3mm" $D_STANDARD_DATASET       30 32 62 64
compile "atax" $D_STANDARD_DATASET      31 32 63 64
compile "bicg" $D_STANDARD_DATASET      31 32 63 64
compile "doitgen" $D_STANDARD_DATASET   11 32 43 64
compile "mvt" $D_STANDARD_DATASET        8 32 40 64
