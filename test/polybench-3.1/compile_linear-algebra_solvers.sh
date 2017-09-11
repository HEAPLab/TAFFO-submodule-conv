#!/bin/bash

source common_compile.sh

ROOT='linear-algebra/solvers'
compile "durbin" $D_SMALL_DATASET 10 32
compile "dynprog" $D_STANDARD_DATASET 16 32
compile "gramschmidt" $D_SMALL_DATASET 26 32
compile "lu" $D_STANDARD_DATASET 20 32
compile "ludcmp" $D_SMALL_DATASET 16 32
