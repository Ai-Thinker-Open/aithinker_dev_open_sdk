#!/bin/bash

cd sdk/bouffalo_sdk

function apply_patch(){
    for element in `ls $1`
    do  
        dir_or_file=$1"/"$element
        if [[ -d $dir_or_file ]]
        then 
            apply_patch $dir_or_file 
        elif [[ $dir_or_file = *".patch" ]]
        then
            git apply $dir_or_file >/dev/null 2>&1
        fi  
    done
}

apply_patch ../../patch/

cd ../../