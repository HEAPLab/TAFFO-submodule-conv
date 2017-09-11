#!/bin/bash

source common_compile.sh

ROOT='medley'
compile "floyd-warshall" $D_SMALL_DATASET 16 32
compile "reg_detect" $D_LARGE_DATASET 13 32
