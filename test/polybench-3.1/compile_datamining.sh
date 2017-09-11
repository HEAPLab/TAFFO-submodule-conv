#!/bin/bash

source common_compile.sh

ROOT='datamining'
compile "correlation" $D_STANDARD_DATASET 27 32
compile "covariance" $D_STANDARD_DATASET 19 32
