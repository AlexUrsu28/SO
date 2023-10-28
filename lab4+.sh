#!/bin/bash

while read line
do
    echo $line | grep -E "^[A-Z]{1,1}[A-Za-z0-9\ ,]*\.$" | grep -v "(si[ ]*) | (si  +)"  | grep -v "[n][pb]"
done