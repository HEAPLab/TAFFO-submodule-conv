#!/bin/bash


do_conf() {
  ./compile_everything.sh $1 --stats-only 2> /dev/null
  mv stats $OUT/$2-stats
  ./compile_everything.sh $1 2> $OUT/$2-compile.log
  ./chkval_everything.sh > $OUT/$2-error.txt
  ./chkval_everything_better.py 5 > $OUT/$2-times.txt
  mv raw-times $OUT/$2-raw-times
}


main() {
  rm -r ./raw-times
  rm -r ./build
  rm -r ./stats
  OUT="./multiconf_results"
  mkdir -p $OUT

  echo '**** good'
  do_conf CONF_GOOD good
  echo '**** bad'
  do_conf CONF_BAD bad
  echo '**** worse'
  do_conf CONF_WORSE worse
  
  cp multiconf_readme_template.txt $OUT/README.txt
  hostname > $OUT/MACHINE.txt
}


trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM
main & wait

