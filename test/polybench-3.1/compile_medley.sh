#!/bin/bash

source common_compile.sh

ROOT='medley'
compile "floyd-warshall" $D_STANDARD_DATASET  20 32 52 64
compile "reg_detect" $D_STANDARD_DATASET      13 32 26 64
