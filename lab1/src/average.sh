#!/bin/bash

sum=0
count=0
for var in $(cat $1)
do
sum=$(($sum+$var))
count=$(($count+1))
done
echo $(($sum/$count))
