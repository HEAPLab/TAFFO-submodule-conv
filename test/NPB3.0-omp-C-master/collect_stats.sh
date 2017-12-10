#!/bin/bash

mkdir -p stats

ISTRTYPE="../../build/tool/Debug/istr_type";
if [ ! -e "$ISTRTYPE" ]; then 
	ISTRTYPE="../../build/tool/istr_type"; 
fi; 

$ISTRTYPE ./BT/bt.o.2.ll > ./BT/stats.txt
$ISTRTYPE ./CG/cg.o.2.ll > ./CG/stats.txt
$ISTRTYPE ./EP/ep.o.2.ll > ./EP/stats.txt
$ISTRTYPE ./LU/lu.o.2.ll > ./LU/stats.txt
$ISTRTYPE ./SP/sp.o.2.ll > ./SP/stats.txt


