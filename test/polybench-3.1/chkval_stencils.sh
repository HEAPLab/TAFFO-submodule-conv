#!/bin/bash

if [ "x$FORMAT" = "x" ]; then
        FORMAT='\033[33m%15s\033[39m%10s%10s\033[%sm%9s%9s\033[39m%11s%13s\n'
fi
source common_chkval.sh
check "adi"
check "fdtd-2d"
check "fdtd-apml"
check "jacobi-1d-imper"
check "jacobi-2d-imper"
check "seidel-2d"
