#!/bin/bash

function src_memcpy() {
	for element in $(ls $2); do
		dir_or_file=$2"/"$element
		if [[ -d $dir_or_file ]]; then
			src_memcpy $1 $dir_or_file
		elif [[ $dir_or_file == *".c" ]]; then
			#echo "$dir_or_file"
			#echo "2:$2"
			mkdir -p $1 && cp $dir_or_file $1
		elif [[ $dir_or_file == *".h" ]]; then
			#echo "$dir_or_file"
			#echo "2:$2"
			mkdir -p $1 && cp $dir_or_file $1
		fi
	done
}

