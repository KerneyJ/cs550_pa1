#!/bin/bash
if [ $# == 0 ]
then
	printf "Not enough arguments\n"
	exit
fi

dd if=/dev/urandom of=../data/1K.data bs=1K count=1
dd if=/dev/urandom of=../data/1M.data bs=1M count=1

python3 cpmega.py $1
python3 cpkilo.py $1

for i in {0..9}
do
	dd if=/dev/urandom of=../data/vm$1_1G_$(printf "%06d" $i).data bs=1G count=1
done
