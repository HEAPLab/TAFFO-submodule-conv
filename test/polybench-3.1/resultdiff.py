#!/usr/bin/env python3

import sys
import math


def ReadValues(filename):
  with open(filename, 'r') as f:
    l = f.readline()
    while l != '':
      for v in l.strip().split():
        if v != '':
          yield v
      l = f.readline()


n = 0
accerr = 0
accval = 0
fix_nofl = 0
flo_nofl = 0

for svfix, svflo in zip(ReadValues(sys.argv[1]), ReadValues(sys.argv[2])):
  vfix, vflo = float(svfix), float(svflo)
  if math.isnan(vfix):
    fix_nofl += 1
  elif math.isnan(vflo):
    flo_nofl += 1
    fix_nofl += 1
  elif abs(vflo) > 0.01 and ( \
         (abs(vflo + vfix) != abs(vflo) + abs(vfix)) or \
         (vflo > 1.0 and abs(vflo - vfix) > abs(vflo) * 2.0)):
    fix_nofl += 1
  else:
    n += 1
    accerr += abs(vflo - vfix)
    accval += vflo
  
print(fix_nofl, flo_nofl, \
      '%.5f' % (accerr / accval * 100) if accval > 0 and n > 0 else '-', \
      '%.5e' % (accerr / n) if n > 0 else '-')

