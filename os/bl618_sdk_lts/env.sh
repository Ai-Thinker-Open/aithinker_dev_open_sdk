#!/bin/bash

## Sdk dir
lsdk_dir="$PWD"
## Tool dir
tool_dir="$lsdk_dir/tools"
## Name of the decompressed package
tool_pag_name="toolchain_gcc_t-head_linux-master"
## Undecompressed compressed package
tool_tar_name="toolchain_gcc_t-head_linux-master.zip"
## The path of the environment variable to be added
tool_add_path="$tool_dir/$tool_pag_name/bin/"

patch_check_result=$(echo $PATH | grep "$tool_add_path")

if [[ "$patch_check_result" != "" ]]; then
	echo ""
else
	export PATH=$tool_add_path:$PATH
fi

## Check the package
if [ -d "$tool_dir/$tool_pag_name" ]; then

	# du -s * | sort -nr
	#cd $tool_dir && find $tool_pag_name -type f | xargs ls -gGS --time-style=long-iso | sort -nr -k 3  >$tool_dir/list.tmp

	#md5_file=$(md5sum $tool_dir/list.tmp)
	#md5_list=$(md5sum $tool_dir/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux_list.txt)

	#echo ${md5_file%%/*}
	#echo ${md5_list%%/*}

	#if [[ "${md5_file%%/*}" == "${md5_list%%/*}" ]]; then

		#echo ""
	#else
		## The file package is incomplete
		## rm -rf $tool_dir/$tool_pag_name && mkdir -p $tool_dir/$tool_pag_name && tar -jxvf $tool_dir/$tool_tar_name -C $tool_dir/$tool_pag_name 
	#fi

	#rm -rf $tool_dir/list.tmp

	#cd $lsdk_dir
	
	echo ""

else
	## The file package is incomplete
	rm -rf $tool_dir/$tool_pag_name && mkdir -p $tool_dir/$tool_pag_name && cd $tool_dir && unzip $tool_dir/$tool_pag_name
	cd $lsdk_dir
fi

./patch.sh