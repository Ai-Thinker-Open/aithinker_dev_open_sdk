#!/bin/bash

chip=$1
adt_os_dir=$2
adapter_core_components_dir=$adt_os_dir/adapter_core/components
adapter_core_components_include_dir=$adt_os_dir/adapter_core/components_include

function components_include_gen(){
    for element in `ls $1`
    do  
        dir_or_file=$1"/"$element
        if [[ -d $dir_or_file ]]
        then 
            components_include_gen $dir_or_file
        elif [[ $dir_or_file = *".h" ]]
        then
            # echo "$dir_or_file"
            echo "#include \"${dir_or_file##*/}\"" >> $adapter_core_components_include_dir/components_include.h

        fi  
    done
}


################################################################################################
:> $adapter_core_components_include_dir/components_include.h

echo "#ifndef COMPONENTS_INCLUDE_H" >> $adapter_core_components_include_dir/components_include.h
echo "#define COMPONENTS_INCLUDE_H" >> $adapter_core_components_include_dir/components_include.h
echo ""				    >> $adapter_core_components_include_dir/components_include.h

components_include_gen $adapter_core_components_dir

echo "" 	>> $adapter_core_components_include_dir/components_include.h
echo "#endif" 	>> $adapter_core_components_include_dir/components_include.h
################################################################################################










