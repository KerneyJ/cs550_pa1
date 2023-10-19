#!/bin/bash

for i in {1..17}
do
	lxc stop "vm$i"
	lxc delete "vm$i"
done
