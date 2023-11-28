#!/bin/bash

coding_name=$1

if [ $# -eq 1 ]; then

	if [ -d "$coding_name" ]; then

		cp -a demo/* $coding_name && echo "successful initialization (coding_name: $coding_name)"

	fi

	exit
else

	echo "Input parameter error"
fi

