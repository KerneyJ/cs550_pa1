#!/bin/bash

parallel-ssh -O StrictHostKeyChecking=no -h host-file.txt -t 0 'pwd'
