#!/bin/bash

cd "$(dirname "$0")"

sudo apt-get update
sudo apt-get install doxygen
sudo apt-get install graphviz



if [ -d "./out/html" ]
then
	rm -rf ./out/html
fi

doxygen Doxyfile

rm -rf ./out/latex

echo
echo "## doxygen ok, enjoy on out file"
echo

if [ -d "./out/html" ]
then
	rm -rf ../../docs/html && mv ./out/html ../../docs/
fi
