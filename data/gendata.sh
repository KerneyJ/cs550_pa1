#!/bin/bash
dd if=/dev/urandom of=./1K.data bs=1K count=1
dd if=/dev/urandom of=./1M.data bs=1M count=1
dd if=/dev/urandom of=./1G.data bs=1G count=1

mkdir kilo
mkdir mega
mkdir giga

for i in {1..1000000}
do
	dd if=/dev/urandom of=./kilo/$(printf "%07d" $i).data bs=1K count=1
done

for i in {1..1000}
do
	dd if=/dev/urandom of=./kilo/$(printf "%04d" $i).data bs=1M count=1
done

for i in {1..10}
do
	dd if=/dev/urandom of=./kilo/$(printf "%02d" $i).data bs=1G count=1
done
