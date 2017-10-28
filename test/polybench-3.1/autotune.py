#!/usr/bin/env python3


import sys
import math
import os
from decimal import *
import resource
import argparse


def nothing(*args, **kwargs): pass
vprint = nothing 
nvprint = print


def execute(command):
  import subprocess
  limit = '65532' if os.uname()[0] == 'Darwin' else 'unlimited'
  rcmd = 'ulimit -s ' + limit + '; ' + command
  pr = subprocess.run(rcmd, stdout=subprocess.DEVNULL, stdin=subprocess.DEVNULL, shell=True)
  if pr.returncode == -2:
    vprint('sigint! stopping')
    os.exit(0)
  return pr.returncode


def ReadValues(filename):
  with open(filename, 'r') as f:
    l = f.readline()
    while l != '':
      for v in l.strip().split():
        if v != '':
          yield v
      l = f.readline()
  
  
def errorMetric(flo, fix):
  if len(fix) == 0:
    return 2.0  # the process failed
  maxflo = max([abs(flov) for flov in flo])
  maxdiff = max([abs(fixv - flov) for (fixv, flov) in zip(flo, fix)])
  return maxdiff / maxflo
  
  
flovals_cache = {}
em_cache = {}
def buildExecuteAndGetErrorMetric(benchname, fracbsize, bitness, doubleflt=False):
  global flovals_cache
  global em_cache
  
  cachekey = 'bench='+benchname+';fbs='+str(fracbsize)
  cached = em_cache.get(cachekey)
  if not (cached is None):
    vprint(cachekey, ' is cached')
    return cached
    
  execute('./compile_everything.sh --only=%s --frac=%d --tot=%d %s 2>> ' \
          'build.log > /dev/null' % (benchname, fracbsize, bitness, \
          '64bit' if doubleflt else ''))
  
  flovals = flovals_cache.get(benchname)
  if flovals is None:
    fn = './output-data-32/%s_out_not_opt.output.csv' % benchname
    execute('./build/%s_out_not_opt 2> %s > /dev/null' % (benchname, fn))
    flovals = [float(v) for v in ReadValues(fn)]
    flovals_cache[benchname] = flovals
    
  fn = './output-data-32/%s_out.output.csv' % benchname
  res = execute('./build/%s_out 2> %s > /dev/null' % (benchname, fn))
  if res == 0:
    fixvals = [float(v) for v in ReadValues(fn)]
  else:
    fixvals = []
  
  cached = errorMetric(flovals, fixvals)
  em_cache[cachekey] = cached
  return cached
  
  
def plotErrorMetric(benchname, bitness=32, doubleflt=False):
  import matplotlib.pyplot as plt
  fracbits = range(bitness)
  err = []
  for b in fracbits:
    vprint(b)
    err += [buildExecuteAndGetErrorMetric(benchname, b, bitness, doubleflt)]
    vprint(err)
  print(err)
  plt.plot(fracbits, err)
  plt.yscale('log')
  plt.show()
  
  
def autotune(benchname, bitness=32, doubleflt=False):
  steps = 0
  
  vprint('## lwall & rwall search')
  lwall, rwall = -1, bitness
  minimum = 1
  
  queue = [(lwall, rwall)]
  while len(queue) > 0:
    lwall, rwall = queue.pop(0)
    center = int((lwall + rwall) / 2)
    vprint('lwall, center, rwall = ', lwall, center, rwall)
    if rwall - lwall <= 1:
      vprint('empty interval')
      continue
    queue += [(lwall, center), (center, rwall)]
    
    minimum = buildExecuteAndGetErrorMetric(benchname, center, bitness, doubleflt)
    vprint('minimum = ', minimum)
    steps += 1
    if minimum < 0.1:
      vprint('ok!')
      break
  vprint('lwall = ', lwall, '; rwall = ', rwall)
  
  vprint('## min search')
  while rwall - lwall > 1:
    center = int((lwall + rwall) / 2)
    vprint('lwall, center, rwall = ', lwall, center, rwall)
    sample = buildExecuteAndGetErrorMetric(benchname, center, bitness, doubleflt)
    vprint('sample = ', sample)
    steps += 1
    if sample <= minimum:
      lwall = center
      minimum = sample
      vprint('sample is new best')
    else:
      rwall = center
      vprint('sample is new rwall')
      if sample <= 0.1:
        vprint('(spike at ', center, ')')
      
  vprint('optimal frac bits = ', lwall, '; total # iterations = ', steps)
  vprint('worst value error = ', minimum)  
  return lwall, steps
  
  
parser = argparse.ArgumentParser(description='autotune for polybench')
parser.add_argument('benchnames', metavar='name', type=str, nargs='+',
                    help='the benchmarks to tune')
parser.add_argument('--plot', action='store_true',
                    help='plot a frac bit / error graph instead')
parser.add_argument('--bit', dest='bitness', type=int,
                    default=32, help='specifies the fixed point data type size'
                    'to use (default=32)')
parser.add_argument('--flt-double', dest='double', action='store_true',
                    help='compare fixed point with double-precision floats '
                    'instead than with single-precision floats')
parser.add_argument('--verbose', action='store_true',
                    help='be verbose')
args = parser.parse_args()

if args.verbose:
  vprint = print
  nvprint = nothing

for bench_name in args.benchnames:
  vprint('#### ', bench_name)
  if args.plot:
    plotErrorMetric(bench_name, args.bitness, args.double)
  else:
    res, steps = autotune(bench_name, args.bitness, args.double)
    nvprint(bench_name, res, steps)
  vprint('\n')


