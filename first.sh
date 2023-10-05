#!/bin/bash

file=$1
dir1=$2
count=0
sum=0

if test $# -lt 3
then
    echo "Error: Not enough arguments"
    exit 1
fi

shift 2

for arguments in "$@"
do
    sum=$((sum + arguments))
    if test $arguments -gt 10
    then
        ((count++))
    fi
done

if test -f "$file" 
then
    echo "Regular file!"
fi

echo "Number of arguments greater than 10: $count"
echo "Sum of arguments: $sum"
echo $sum | wc -c 

echo "Number of arguments greater than 10: $count, sum of arguments: $sum" > $file

