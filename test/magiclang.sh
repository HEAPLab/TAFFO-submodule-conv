#!/bin/bash

ROOT=$(dirname "$0")

if [ $# -lt 1 ]; then
  echo "usage: $0 <filename.c>";
  echo 'compiles a binary but by converting floats to fixed point first';
  echo 'output to filename (without the extension) and to various _tmp* files';
  echo 'any existing files will be overwritten so be careful';
  exit;
fi

if [ -z ${LLVM_DIR+x} ]; then
  echo 'please set $LLVM_DIR to a prefix where LLVM 6.0 can be found';
  exit;
fi

SOEXT="so"
if [ $(uname -s) = "Darwin" ]; then
  SOEXT="dylib";
fi

CLANG=$LLVM_DIR/bin/clang
OPT=$LLVM_DIR/bin/opt
LLC=$LLVM_DIR/bin/llc
if [ ! -e "$PASSLIB" ]; then
  PASSLIB="$ROOT/../build/LLVMFloatToFixed/LLVMFloatToFixed.$SOEXT";
fi
OUTNAME=$(echo "$1" | sed -E 's/\.[^\.]$//')

ISDEBUG=$("$OPT" --version | grep DEBUG | wc -l)
DEBUGONLYFLAG="-debug-only=flttofix"
if [ $ISDEBUG != '1' ]; then
  DEBUGONLYFLAG='';
fi

$CLANG -S -emit-llvm "$1" -o "_tmp0.$1.ll" $3 -O0
$OPT -load="$PASSLIB" -S -flttofix -dce -debug -stats "_tmp0.$1.ll" -o "_tmp1.$1.ll" 
$CLANG -S  -o "_tmp2.$1.s" "_tmp1.$1.ll" $2 $3
$CLANG -o "$OUTNAME" "_tmp2.$1.s" $2 $3 -lm

$CLANG -S -o "_tmp2_not_opt.$1.s" "_tmp0.$1.ll" $2 $3
$CLANG -o "$OUTNAME._not_opt" "_tmp2_not_opt.$1.s" $2 $3 -lm
