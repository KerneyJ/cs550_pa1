#!/bin/bash
if [ $# == 0 ]
then
	printf "Not enough arguments\n"
	exit
fi

dd if=/dev/urandom of=$2/data/1K.data bs=1K count=1
dd if=/dev/urandom of=$2/data/1M.data bs=1M count=1

python3 $2/scripts/cpmega.py $1 $2
python3 $2/scripts/cpkilo.py $1 $2

#for i in {0..9}
#do
#	dd if=/dev/urandom of=$2/data/vm$1_1G_$(printf "%06d" $i).data bs=1G count=1
#done
