#!/bin/bash
echo "" > peer_ips.csv
for i in {1..17}; do
        IP=$(lxc exec "vm$i" -- ip a show scope global | grep -Po "inet \K[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*" | head -1);
        echo "$i,$IP" >> peer_ips.csv
done
