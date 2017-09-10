# NOT TO BE EXECUTED BY ITSELF

D_MINI_DATASET="MINI_DATASET"
D_SMALL_DATASET="SMALL_DATASET"
D_STANDARD_DATASET="STANDARD_DATASET"
D_LARGE_DATASET="LARGE_DATASET"
D_EXTRALARGE_DATASET="EXTRALARGE_DATASET"
D_M=1
D_DATA_TYPE='__attribute__((annotate("no_float")))float'
ONLY='*'
FRAC=''
TOT=''

for arg; do
  case $arg in
    64bit)
      D_M=2
      D_DATA_TYPE='__attribute__((annotate("no_float")))double'
      ;;
    [A-Z]*_DATASET)
      D_MINI_DATASET=$arg
      D_SMALL_DATASET=$arg
      D_STANDARD_DATASET=$arg
      D_LARGE_DATASET=$arg
      D_EXTRALARGE_DATASET=$arg
      ;;
    --only=*)
      ONLY="${arg#*=}"
      ;;
    --frac=*)
      FRAC="${arg#*=}"
      ;;
    --tot=*)
      FIX="${arg#*=}"
      ;;
    *)
      echo Unrecognized option $arg
      exit 1
  esac
done


#param: ROOT=where/the/c/files/are
#       $1=bench-name
#       $2=$D_XXX_DATASET
#       $3=fixpfracbitsamt
#       $4=fixpbitsamt
compile() {
  if [[ "$1" == $ONLY ]]; then
    fracx=$3
    totx=$4
    if [ 'x'$FRAC != 'x' ]; then
      fracx=$FRAC;
    fi
    if [ 'x'$TOT != 'x' ]; then
      totx=$TOT;
    fi
    echo $1
    ./magiclang.sh "$ROOT/$1/$1.c" "-O3" \
      "-I utilities -I $ROOT/$1 -DPOLYBENCH_TIME -D$2 -DDATA_TYPE=$D_DATA_TYPE -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_STACK_ARRAYS" \
      "" "$1_out" "utilities/polybench.c" "-fixpfracbitsamt=$(($fracx * $D_M)) -fixpbitsamt=$(($totx * $D_M))"; 
  fi
}

