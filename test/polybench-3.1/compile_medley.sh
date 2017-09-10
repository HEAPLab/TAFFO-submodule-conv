#!/bin/bash

source common_compile.sh

ROOT='medley'
compile "floyd-warshall" $D_SMALL_DATASET 23 32
compile "reg_detect" $D_LARGE_DATASET 10 32
