#!/bin/bash

for((j=0;j<5;j++)); do
	for((i=0;i<8;i++)); do
		a="./maxcut maxcut500.txt result"
	  a+=$i$j
	  a+=".txt &"
		eval $a
	done
	sleep 200
done
