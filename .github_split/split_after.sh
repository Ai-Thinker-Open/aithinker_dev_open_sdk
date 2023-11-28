#!/bin/bash

cd "$(dirname "$0")"

rootdir=$(pwd)

workdir=$rootdir/../

while read line || [[ -n ${line} ]]; do

	cd $rootdir

	cd $(dirname ${line}) 

	cat $(basename ${line})"_pkg"* >$(basename ${line}) #&& rm -rf cat $(basename ${line})"_pkg"* 
	
	cd $rootdir && echo $(md5sum ${line})

done <$rootdir/split_log.txt

