#!/bin/bash
if [ $# == 0 ]
then
	printf "Not enough arguments\n"
	exit
fi

dd if=/dev/urandom of=$2/data/10K.data bs=10K count=1
dd if=/dev/urandom of=$2/data/100M.data bs=100M count=1

python3 $2/scripts/cpmega.py $1 $2
python3 $2/scripts/cpkilo.py $1 $2