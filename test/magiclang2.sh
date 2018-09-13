#!/bin/bash

if [[ -z $PASSLIB ]]; then
  echo -e '\033[31m'"Error"'\033[39m'" please set PASSLIB to the location of LLVMFloatToFixed.so";
fi
if [[ -z $LLVM_DIR ]]; then
  echo -e '\033[33m'"Warning"'\033[39m'" using default llvm/clang";
else
  llvmbin="$LLVM_DIR/bin/";
fi

parse_state=0
raw_opts="$@"
input_files=
output_file=
optimization=
opts=
optflags=
dontlink=
for opt in $raw_opts; do
  case $parse_state in
    0)
      case $opt in
        -Xopt)
          parse_state=2;
          ;;
        -o*)
          if [[ ${#opt} -eq 2 ]]; then
            parse_state=1;
          else
            output_file=`echo "$opt" | cut -b 2`;
          fi;
          ;;
        -O*)
          optimization=$opt;
          ;;
        -fixp*)
          optflags="$optflags $opt";
          ;;
        -c)
          dontlink="$opt";
          ;;
        -*)
          opts="$opts $opt";
          ;;
        *.c | *.cpp | *.cc)
          input_files="$input_files $opt";
          ;;
        *)
          opts="$opts $opt";
          ;;
      esac;
      ;;
    1)
      output_file="$opt";
      parse_state=0;
      ;;
    2)
      optflags="$optflags $opt";
      parse_state=0;
      ;;
  esac;
done

set -x

"${llvmbin}clang" \
  $opts -O0 \
  -c -emit-llvm \
  ${input_files} \
  -S -o "${output_file}.1.ll" || exit $?
"${llvmbin}opt" \
  -load "$PASSLIB" \
  -flttofix -dce \
  ${optflags} \
  -S -o "${output_file}.2.ll" "${output_file}.1.ll" || exit $?
"${llvmbin}clang" \
  $opts ${optimization} \
  -c \
  "${output_file}.2.ll" \
  -S -o "$output_file.s" || exit $?
"${llvmbin}clang" \
  $opts ${optimization} \
  ${dontlink} \
  "${output_file}.2.ll" \
  -o "$output_file" || exit $?


