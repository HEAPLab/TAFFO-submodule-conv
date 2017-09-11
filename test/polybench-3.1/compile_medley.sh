#!/bin/bash

source common_compile.sh

ROOT='medley'
compile "floyd-warshall" $D_STANDARD_DATASET 16 32
compile "reg_detect" $D_STANDARD_DATASET 13 32
