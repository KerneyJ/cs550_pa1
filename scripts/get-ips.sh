#!/bin/bash
echo "" > peer-ips.csv
echo "" > host-file.txt
for i in {1..16}; do
        IP=$(lxc exec "vm$i" -- ip a show scope global | grep -Po "inet \K[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*" | head -1);
        echo "$i,$IP" >> peer-ips.csv
	echo "root@$IP" >> host-file.txt
done
