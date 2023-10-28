#!bin/bash

dir=$1
ch=$2

if test "$#" -eq 2
then
    echo "Primeste numar bun de argumente"
    else
        echo "Eroare: numar gresit de argumente"
        exit 1
        fi

if test -d "$dir"
then
    echo "Primul argument este director"
    else
        echo "Eroare: primul argument nu este director"
        exit 1
        fi

#nerecursiv
for file in $dir/*.txt
do
    if test $2 = 'r' -o $2 = 'w' -o $2 = 'x'
        then
            chmod "+$2" "$file"
            fi
done

#recursiv
for file in $dir/*
do 
    if test -d "$dir"
    then
        chmod "+$2" "$file"
        else 
            bash $0 $file $caracter
            fi  
done

