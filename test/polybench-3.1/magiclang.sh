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
  echo 'please set $LLVM_DIR to a prefix where LLVM 4.0 can be found';
  exit;
fi

SOEXT="so"
if [ $(uname -s) = "Darwin" ]; then
  SOEXT="dylib";
fi

CLANG=$LLVM_DIR/bin/clang
OPT=$LLVM_DIR/bin/opt
LLC=$LLVM_DIR/bin/llc
PASSLIB="$ROOT/../../build/LLVMFloatToFixed/Debug/LLVMFloatToFixed.$SOEXT"
OUTNAME=$(echo "$5" | sed -E 's/\.[^\.]$//')

$CLANG -S -emit-llvm "$1" -o "_tmp0.$5.ll" $3 $4
$OPT -load="$PASSLIB" -S -flttofix -debug-only=flttofix -dce "_tmp0.$5.ll" -o "_tmp1.$5.ll" $7
$CLANG -S -o "_tmp2.$5.s" "_tmp1.$5.ll" $2 $3
$CLANG -o "$OUTNAME" "_tmp2.$5.s" $2 $3 $6

$CLANG -S -o "_tmp2_not_opt.$5.s" "_tmp0.$5.ll" $2 $3
$CLANG -o "$OUTNAME._not_opt" "_tmp2_not_opt.$5.s" $2 $3 $6
