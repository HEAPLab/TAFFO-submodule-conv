#!/usr/bin/env python3


import sys
import math
import os
from decimal import *
import resource
import argparse


def ReadValues(filename):
  with open(filename, 'r') as f:
    l = f.readline()
    while l != '':
      for v in l.strip().split():
        if v != '':
          yield v
      l = f.readline()
  
  
def errorMetric(flo, fix):
  maxflo = max([abs(flov) for flov in flo])
  maxdiff = max([abs(fixv - flov) for (fixv, flov) in zip(flo, fix)])
  return maxdiff / maxflo
  
  
flovals = None
em_cache = {}
def buildExecuteAndGetErrorMetric(benchname, fracbsize):
  global flovals
  global em_cache
  
  cachekey = 'bench='+benchname+';fbs='+str(fracbsize)
  cached = em_cache.get(cachekey)
  if not (cached is None):
    print(cachekey, ' is cached')
    return cached
    
  os.system('./compile_everything.sh --only=%s --frac=%d --tot=32 2>> build.log > /dev/null' % (benchname, fracbsize))
  
  if flovals is None:
    fn = './output-data-32/%s_out_not_opt.output.csv' % benchname
    os.system('./build/%s_out_not_opt 2> %s > /dev/null' % (benchname, fn))
    flovals = [float(v) for v in ReadValues(fn)]
    
  fn = './output-data-32/%s_out.output.csv' % benchname
  os.system('./build/%s_out 2> %s > /dev/null' % (benchname, fn))
  fixvals = [float(v) for v in ReadValues(fn)]
  
  cached = errorMetric(flovals, fixvals)
  em_cache[cachekey] = cached
  return cached
  
  
def plotErrorMetric(benchname):
  import matplotlib.pyplot as plt
  fracbits = range(0, 31)
  err = []
  for b in fracbits:
    print(b)
    err += [buildExecuteAndGetErrorMetric(benchname, b)]
    print(err)
  plt.plot(fracbits, err)
  plt.yscale('log')
  plt.show()
  
  
def autotune(benchname):
  steps = 0
  
  # 1: find initial lwall and rwall by sampling
  print('## lwall & rwall search')
  lwall, rwall = -1, 32
  minimum = 1
  
  queue = [(lwall, rwall)]
  while len(queue) > 0:
    lwall, rwall = queue.pop(0)
    center = int((lwall + rwall) / 2)
    print('lwall, center, rwall = ', lwall, center, rwall)
    if rwall - lwall <= 1:
      print('empty interval')
      continue
    queue += [(lwall, center), (center, rwall)]
    
    minimum = buildExecuteAndGetErrorMetric(benchname, center)
    print('minimum = ', minimum)
    steps += 1
    if minimum < 0.1:
      print('ok!')
      break
  print('lwall = ', lwall, '; rwall = ', rwall)
  
  print('## min search')
  while rwall - lwall > 1:
    center = int((lwall + rwall) / 2)
    print('lwall, center, rwall = ', lwall, center, rwall)
    sample = buildExecuteAndGetErrorMetric(benchname, center)
    print('sample = ', sample)
    steps += 1
    if sample <= minimum:
      lwall = center
      minimum = sample
      print('sample is new best')
    else:
      rwall = center
      print('sample is new rwall')
      if sample <= 0.1:
        print('(spike at ', center, ')')
      
  print('optimal frac bits = ', lwall, '; total # iterations = ', steps)
  print('worst value error = ', minimum)  
  return lwall
  
  
parser = argparse.ArgumentParser(description='autotune for polybench')
parser.add_argument('benchnames', metavar='name', type=str, nargs='+',
                    help='the benchmarks to tune')
parser.add_argument('--plot', dest='plot', action='store_const',
                    const=True, default=False,
                    help='plot a frac bit / error graph instead')
args = parser.parse_args()

for bench_name in args.benchnames:
  if args.plot:
    plotErrorMetric(bench_name)
  else:
    autotune(bench_name)



