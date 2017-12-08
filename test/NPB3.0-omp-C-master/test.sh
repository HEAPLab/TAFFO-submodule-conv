#!/bin/bash

ncpus=$(getconf _NPROCESSORS_ONLN)
mkdir -p bin

make clean
rm bin/*
echo MAKE floating point
make -j ${ncpus} bt cg ep lu sp CLASS=A > build.log 2>&1
echo RUN  floating point
./bin/bt_A  > float_res.txt
./bin/cg_A >> float_res.txt
./bin/ep_A >> float_res.txt
./bin/lu_A >> float_res.txt
./bin/sp_A >> float_res.txt

make clean
rm bin/*
echo MAKE fixed point
make -j ${ncpus} bt cg ep lu sp CLASS=a >> build.log 2>&1
echo RUN  fixed point
./bin/bt_a  > fix_res.txt
./bin/cg_a >> fix_res.txt
./bin/ep_a >> fix_res.txt
./bin/lu_a >> fix_res.txt
./bin/sp_a >> fix_res.txt
