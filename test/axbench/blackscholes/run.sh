#!/bin/bash

rm -rf data/output
mkdir data/output
benchmark=blackscholes

for f in data/input/*.data
do
	filename=$(basename "$f")
	extension="${filename##*.}"
	filename="${filename%.*}"

	./bin/${benchmark}.out ${f} data/output/${filename}_${benchmark}_out.data
	./bin/${benchmark}.out.fixp ${f} data/output/${filename}_${benchmark}_out.data.fixp
	python ./scripts/qos.py data/output/${filename}_${benchmark}_out.data data/output/${filename}_${benchmark}_out.data.fixp
done
