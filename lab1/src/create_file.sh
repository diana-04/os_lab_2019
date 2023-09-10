#!/usr/bin/bash
for((i = 0; i < 150; i++))
do
od -A n -t d -N 1 /dev/random
done
