#!/bin/bash

ROOT=$(dirname "$0")

if [ $# -ne 1 ]; then
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
PASSLIB="$ROOT/../build/LLVMFloatToFixed/Debug/LLVMFloatToFixed.$SOEXT"
OUTNAME=$(echo "$1" | sed -E 's/\.[^\.]$//')

$CLANG -S -emit-llvm "$1" -o "_tmp0.$1.ll"
$OPT -load="$PASSLIB" -S -flttofix -debug-only=flttofix -dce "_tmp0.$1.ll" -o "_tmp1.$1.ll"
$LLC -o "_tmp2.$1.o" "_tmp1.$1.ll" -filetype=obj
$CLANG -o "$OUTNAME" "_tmp2.$1.o"

