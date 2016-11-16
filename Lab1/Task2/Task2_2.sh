#!/bin/sh

for i in $(seq 0 100)
do 
./21 >> output2_1.txt 2>> &1
done 

exit 0 
