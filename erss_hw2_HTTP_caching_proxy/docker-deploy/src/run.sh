#!/bin/bash
make clean

make

./haoserver

while true
do
	sleep 1
done

