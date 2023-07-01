#!/bin/bash
make clean

make

./server

while true
do
  sleep 1
done