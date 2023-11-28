#!/bin/bash

cd "$(dirname "$0")" && ./split_after.sh

: >split_log.txt
: >split_log_md5.txt

rootdir=.

workdir=$rootdir/../

split_list=$(find $workdir -path $workdir".git" -prune -o -type f -size +100M -print)

for split_path in $split_list; do

	echo $split_path >>split_log.txt
	echo $(md5sum $split_path) >>split_log_md5.txt

	split -b 100M -d $split_path $split_path"_pkg" && rm -rf $split_path

done

