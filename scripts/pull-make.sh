#!/bin/bash

for i in {1..17}; do
        lxc exec "vm$i" -- bash -lc "cd /home/root/cs550_pa1 && git pull && make" &
done
