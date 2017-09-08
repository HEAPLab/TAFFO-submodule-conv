#!/bin/bash

source common_chkval.sh
if [ "x$FORMAT" = "x" ]; then
        FORMAT='\033[33m%11s\033[39m%10s%10s\033[%sm%11s%11s\033[39m%11s%13s\n'
fi
check "correlation"
check "covariance"
