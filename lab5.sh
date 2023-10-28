#!/bin/bash

director=$1
file=$2
count=0
sum=0

if test $# -ne 2
then
    echo "Eroare : nu avem 2 argumente"
    exit 1
    fi

if test -d "$director"
then
    echo "Primul argument este director"
    else
        echo "Eroare: primul argument nu este director"
        exit 1
        fi

if test -f "$file"
then
    echo "Al doilea argument este fisier"
    else
        echo "Eroare: al doilea argument nu este fisier"
        exit 1
        fi

for f in $director/*
do
    if test -h "$f"
       then
         ((count++))
    elif test -f "$f"
        res=`cat "$f" | grep -E "^[A-Z]{1,1}[A-Za-z0-9\ ,]*\.$" | grep -v "(si[ ]*) | (si  +)"  | grep -v "[n][pb]"`
        then
            echo "$f" >> $2
            fi
    if test -d "$f"
        then
            count_rec=$(bash $0 $f $file)
            ((count+=$count_rec))
            fi
done

echo $count > "$file"
