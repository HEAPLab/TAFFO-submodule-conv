#!/bin/bash

FORMAT='\033[33m%16s\033[39m%10s%10s\033[%sm%11s%11s\033[39m\n'

check() {
        OPT="./$1_out"
        NOPT="./$1_out._not_opt"
        OPT_OUT="$OPT.output.csv"
        NOPT_OUT="$NOPT.output.csv"
        
        FIXT=$($OPT 2> $OPT_OUT)
        FLOT=$($NOPT 2> $NOPT_OUT)
                
        OFLC_OPT=$(grep '-' $OPT_OUT | wc -l)
        OFLC_NOPT=$(grep '(-)|(nan)' $NOPT_OUT | wc -l)
        if [ "$OFLC_OPT" = "$OFLC_NOPT" ]; then
                OFLC='32'
        else
                OFLC='31'
        fi
        
        printf $FORMAT $1 $FIXT $FLOT $OFLC $OFLC_OPT $OFLC_NOPT
}

printf $FORMAT '' 'fix T' 'flo T' '39' '# ofl fix' '# ofl flo'

check "2mm"
check "3mm"
check "atax"
check "bicg"
check "cholesky"
check "doitgen"
check "gemm"
check "gemver"
check "gesummv"
check "mvt"
check "symm"
check "syr2k"
check "syrk"
check "trisolv"
check "trmm"

