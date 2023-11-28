#!/bin/bash

function code_style_func(){
    for element in `ls $1`
    do  
        dir_or_file=$1"/"$element
        if [[ -d $dir_or_file ]]
        then 
            code_style_func $dir_or_file
        elif [[ $dir_or_file = *".c" ]]
        then
            echo $dir_or_file
            python3 ./formatting/formatting.py $dir_or_file
            ./as.sh $dir_or_file

        elif [[ $dir_or_file = *".h" ]]
        then
            echo $dir_or_file
            python3 ./formatting/formatting.py $dir_or_file  
            ./as.sh $dir_or_file         
        fi  
    done
}

adapter_dir="$1/adt/adt_os"
application_dir="$1/application"

code_style_func $adapter_dir
code_style_func $application_dir
