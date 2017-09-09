# NOT TO BE EXECUTED BY ITSELF

DATADIR='./result-data'
mkdir -p $DATADIR

PRINTF="./printfx.py"

TASKSET=""
which taskset > /dev/null
if [ $? -eq 0 ]; then
        TASKSET="taskset -c 0 "
fi
ulimit -s 65532

check() {
        OPT="./build/$1_out"
        NOPT="./build/$1_out_not_opt"
        OPT_OUT="$DATADIR/$1_out.output.csv"
        NOPT_OUT="$DATADIR/$1_out_not_opt.output.csv"
        
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

