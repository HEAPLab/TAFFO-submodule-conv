#!/usr/bin/env python3

import sys
import math
import os
import resource
import argparse
import itertools


def execute(command, capture_stdout=False):
  import subprocess
  limit = '65532' if os.uname()[0] == 'Darwin' else 'unlimited'
  rcmd = 'ulimit -s ' + limit + '; ' + command
  if capture_stdout:
    outchan = subprocess.PIPE
  else:
    outchan = subprocess.DEVNULL
  pr = subprocess.run(rcmd, stdout=outchan, stdin=subprocess.DEVNULL, shell=True, universal_newlines=True)
  if pr.returncode == -2:
    vprint('sigint! stopping')
    os.exit(0)
  if capture_stdout:
    return pr.stdout if pr.returncode == 0 else ''
  return pr.returncode
  
  
def ParseInstrStatFile(filename):
  out = {}
  with open(filename, 'r') as f:
    l = f.readline()
    while l != '':
      items = l.split()
      if len(items) < 2:
        continue
      out[items[0]] = int(items[1])
      l = f.readline()
  
  out2 = {}
  total = out['*']
  for k, v in out.items():
    out2[k] = out[k] / total
  return out2
  
  
def MaterializeTable(iterableofiterables):
  tab = []
  sizes = []
  for iterable in iterableofiterables:
    row = []
    for item, i in zip(iterable, itertools.count()):
      stritem = str(item)
      row += [stritem]
      if len(sizes) <= i:
        sizes += [len(stritem)]
      else:
        sizes[i] = max(sizes[i], len(stritem))
    tab += [row]

  for row in tab:
    print('  '.join([item.rjust(size) for item, size in zip(row, sizes)]))
    
    
def TableGenerator(stats, allinst):
  instlist = sorted(list(allinst))
  instlist.remove('*')
  firstrow = ['']
  firstrow += instlist
  yield firstrow
  benchs = sorted(stats.keys())
  for bench in benchs:
    row = [bench]
    for inst in instlist:
      stat = stats[bench].get(inst)
      row += ['0.0' if stat is None else '%.1f'%(stat*100)]
    yield row
  
  
benchs = execute(command='./compile_everything.sh --dump-bench-list', capture_stdout=True)
benchnames = benchs.split()

stats = {}
allinst = set()
for bench in benchnames:
  fix = ParseInstrStatFile('./stats/' + bench + '_out_ic_fix.txt')
  flo = ParseInstrStatFile('./stats/' + bench + '_out_ic_float.txt')
  stats[bench+'_fix'] = fix
  stats[bench+'_flo'] = flo
  allinst |= set(fix.keys())
  allinst |= set(flo.keys())

MaterializeTable(TableGenerator(stats, allinst))
  
  
  
  
  
  
