#!/bin/bash

sdk_dir=$1
bin_out_dir=$2

bin_out_elf=$bin_out_dir/elf
bin_out_bin=$bin_out_dir/bin
bin_out_map=$bin_out_dir/map

# Platform adapter
bin_src_elf=$sdk_dir/amebadplus_gcc_project/project_km4/asdk/image
bin_src_bin=$sdk_dir/amebadplus_gcc_project
bin_src_map=$sdk_dir/amebadplus_gcc_project/project_km4/asdk/image


function bin_out()
{
	rm -rf $1

	mkdir -p $2 && cp -a $5/km4_boot_all.bin $5/km0_km4_app.bin $5/ota_all.bin $2
	# mkdir -p $3 && cp -a $6 $3/dev_project.elf >/dev/null 2>&1
	# mkdir -p $4 && cp -a $7 $4/dev_project.map >/dev/null 2>&1
}

bin_out $bin_out_dir $bin_out_bin $bin_out_elf $bin_out_map $bin_src_bin $bin_src_elf $bin_src_map
