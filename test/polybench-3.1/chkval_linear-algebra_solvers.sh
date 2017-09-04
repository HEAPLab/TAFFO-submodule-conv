#!/bin/bash

PRINTF="./printfx.py"
if [ "x$FORMAT" = "x" ]; then
        FORMAT='\033[33m%11s\033[39m%10s%10s\033[%sm%11s%11s\033[39m%11s%13s\n'
fi

TASKSET=""
which taskset > /dev/null
if [ $? -eq 0 ]; then
        TASKSET="taskset -c 0 "
fi

check() {
        OPT="./$1_out"
        NOPT="./$1_out._not_opt"
        OPT_OUT="$OPT.output.csv"
        NOPT_OUT="$NOPT.output.csv"
        
        FIXT=$($TASKSET $OPT 2> $OPT_OUT)
        FLOT=$($TASKSET $NOPT 2> $NOPT_OUT)
        
        RESDIFF=($(./resultdiff.py "$OPT_OUT" "$NOPT_OUT"))
        
        OFLC_OPT=${RESDIFF[0]}
        OFLC_NOPT=${RESDIFF[1]}
        if [ "$OFLC_OPT" = "$OFLC_NOPT" ]; then
                OFLC='32'
        else
                OFLC='31'
        fi
        ERROR=${RESDIFF[2]}
        ABSERROR=${RESDIFF[3]}
        
        $PRINTF $FORMAT $1 $FIXT $FLOT $OFLC $OFLC_OPT $OFLC_NOPT $ERROR $ABSERROR
}

if [ "x$NOHEADER" = "x" ]; then
        $PRINTF $FORMAT '' 'fix T' 'flo T' '39' '# ofl fix' '# ofl flo' 'avg err %' 'avg abs err'
fi

if [ "$#" = "1" ]; then
        check $1;
        exit 0;
fi;
check "durbin"
check "dynprog"
check "gramschmidt"
check "lu"
check "ludcmp"
