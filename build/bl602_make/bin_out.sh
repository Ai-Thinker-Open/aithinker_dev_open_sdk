#!/bin/bash

sdk_dir=$1
bin_out_dir=$2

bin_out_elf=$bin_out_dir/elf
bin_out_bin=$bin_out_dir/bin
bin_out_map=$bin_out_dir/map

# Platform adapter
bin_src_elf=$sdk_dir/customer_app/dev_project/build_out/src.elf
bin_src_bin=$sdk_dir/customer_app/dev_project/build_out/src.bin
bin_src_map=$sdk_dir/customer_app/dev_project/build_out/src.map


function bin_out()
{
	rm -rf   $1

	mkdir -p $2 && cp -a $5 $2/dev_project.bin >/dev/null 2>&1
	mkdir -p $3 && cp -a $6 $3/dev_project.elf >/dev/null 2>&1
	mkdir -p $4 && cp -a $7 $4/dev_project.map >/dev/null 2>&1
}

bin_out $bin_out_dir $bin_out_bin $bin_out_elf $bin_out_map $bin_src_bin $bin_src_elf $bin_src_map
