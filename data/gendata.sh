#!/bin/bash
dd if=/dev/urandom of=./1K.data bs=1K count=1
dd if=/dev/urandom of=./1M.data bs=1M count=1
dd if=/dev/urandom of=./1G.data bs=1G count=1
