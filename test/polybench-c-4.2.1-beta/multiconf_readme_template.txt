good-*
  optimized benchmarks
bad-*
  benchmarks with unnecessary precision changes added
worse-*
  incomplete conversion (extra fix-to-float and float-to-fix casts added)
  
*-compile.log
  compilation logs (useless except for debugging)
*-error.txt
  table of error data (disregard time columns)
*-times.txt
  table of execution times averaged across 5 executions
  (disregard error columns)
*-raw-times/
  data of each of the 5 execution runs averaged to produce *-times.txt
*-stats/[bench]_ic_fix.txt
  instruction mix stats for the fixed-point version
*-stats/[bench]_ic_float.txt
  instruction mix stats for the floating-point version (should be the same
  for good/bad/worse)
*-stats/[bench].txt
  LLVM statistics produced by the conversion pass + lines of LLVM IR code
  output after conversion

  