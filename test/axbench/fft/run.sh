#!/bin/bash

rm -rf data/output
mkdir data/output
benchmark=fft


./bin/${benchmark}.out 2048 data/output/${benchmark}_out.data
./bin/${benchmark}.out.fixp 2048 data/output/${benchmark}_out.data.fixp
python ./scripts/qos.py data/output/${benchmark}_out.data data/output/${benchmark}_out.data.fixp

