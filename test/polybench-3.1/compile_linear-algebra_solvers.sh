#!/bin/bash

source common_compile.sh

ROOT='linear-algebra/solvers'
compile "durbin" $D_STANDARD_DATASET      27 32 59 64
compile "dynprog" $D_STANDARD_DATASET     23 32 55 64
compile "gramschmidt" $D_STANDARD_DATASET 26 32 58 64
compile "lu" $D_STANDARD_DATASET          19 32 52 64
compile "ludcmp" $D_STANDARD_DATASET      16 32 49 64
