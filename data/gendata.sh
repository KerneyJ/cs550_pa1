#!/bin/bash
dd if=/dev/urandom of=./1K.data bs=1K count=1
dd if=/dev/urandom of=./1M.data bs=1M count=1
dd if=/dev/urandom of=./1G.data bs=1G count=1

mkdir kilo
mkdir mega
mkdir giga

python3 cpmega.py
python3 cpkilo.py

exit
for i in {0..9}
do
	dd if=/dev/urandom of=./giga/$i.data bs=1G count=1
done
