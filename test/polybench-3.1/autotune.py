#!/usr/bin/env python3


import sys
import math
import os
from decimal import *
import matplotlib.pyplot as plt
import resource


def ReadValues(filename):
  print(filename)
  with open(filename, 'r') as f:
    l = f.readline()
    while l != '':
      for v in l.strip().split():
        if v != '':
          yield v
      l = f.readline()
  
  
def errorMetric(flo, fix):
  maxflo = max(flo)
  maxdiff = max([abs(fixv - flov) for (fixv, flov) in zip(flo, fix)])
  return maxdiff / maxflo
  
  
flovals = None
def buildExecuteAndGetErrorMetric(benchname, fracbsize):
  global flovals
  os.system('./compile_everything.sh --only=%s --frac=%d --tot=32 2>> build.log' % (benchname, fracbsize))
  if flovals is None:
    fn = './output-data-32/%s_out_not_opt.output.csv' % benchname
    os.system('./build/%s_out_not_opt 2> %s' % (benchname, fn))
    flovals = [float(v) for v in ReadValues(fn)]
  fn = './output-data-32/%s_out.output.csv' % benchname
  os.system('./build/%s_out 2> %s' % (benchname, fn))
  fixvals = [float(v) for v in ReadValues(fn)]
  return errorMetric(flovals, fixvals)
  
  
def plotErrorMetric(benchname):
  fracbits = range(0, 30)
  err = []
  for b in fracbits:
    print(b)
    err += [buildExecuteAndGetErrorMetric(benchname, b)]
    print(err)
  plt.plot(fracbits, err)
  plt.show()
  

bench_name = sys.argv[1]
plotErrorMetric(bench_name)





