#!/bin/bash

cd "$(dirname "$0")"

echo "ln882h os_make:"
echo ""

###############################################################################################################################################
## ./os_make.sh    $application_dir $application_tmp_app_dir $rtos_dir $application_tmp_typ_dir $root_dir $application $make_dir $adt_dir $chip
###############################################################################################################################################
cur_dir=$PWD
chip=$9
debugging_level=$10
root_dir=$5
application=$6
application_dir=$1
application_tmp_app_dir=$2
application_src_dir=$application_dir/src
application_sdk_config_dir=$application_dir/vendor/$chip/sdk_config
adt_os_dir=$8/adt_os
adt_tools_dir=$8/adt_tools
adapter_chip=$adt_os_dir/adapter_chip
adapter_core=$adt_os_dir/adapter_core
adapter_main=$adt_os_dir/adapter_main
adapter_include=$adt_os_dir/adapter_include
adapter_chip_chip_dir=$adapter_chip/chip
adapter_chip_chip_common_dir=$adapter_chip/chip_common

adt_os_dir__=$root_dir/tmp/adt_os_src
adapter_chip__=$adt_os_dir__/adapter_chip
adapter_core__=$adt_os_dir__/adapter_core
adapter_main__=$adt_os_dir__/adapter_main
adapter_include__=$adt_os_dir__/adapter_include
adapter_chip_chip_dir__=$adapter_chip__/chip
adapter_chip_chip_common_dir__=$adapter_chip__/chip_common

bin_out_dir=$root_dir/out/$chip/$application

sdk_dir=$3/sdk/ln882h

source $adt_tools_dir/chip_week_func_gen.sh
source $cur_dir/../shell_func/shell_func.sh

if [ -f $root_dir/tmp/kconfig_h/$chip/$application/aiio_autoconf.h ]; then

	echo ""
else
	echo -e "\033[31merror: no ./build.sh \$chip \$application menuconfig\033[0m"
	exit 1

fi

src_to_weak_c 			$application_tmp_app_dir/src $adapter_chip_chip_dir__/include
src_to_weak_h 			$application_tmp_app_dir/src $adapter_chip_chip_dir__/include $adapter_chip_chip_common_dir__ $adapter_chip_chip_dir__/spec/$chip
wsrc_call_func_to_weak_c 	$application_tmp_app_dir/src $adapter_chip_chip_dir__/port/$chip
wsrc_call_func_to_ssrc_c 	$application_tmp_app_dir/src $adapter_chip_chip_dir__/port/$chip

adapter_core_h_include_h  	$application_tmp_app_dir/src $adt_os_dir__/adapter_core


src_memcpy 			$application_tmp_app_dir/src $adapter_chip_chip_dir__/include
src_memcpy 			$application_tmp_app_dir/src $adapter_chip_chip_dir__/spec/$chip
src_memcpy 			$application_tmp_app_dir/src $adapter_chip_chip_common_dir__
src_memcpy 			$application_tmp_app_dir/src $adapter_core__
src_memcpy 			$application_tmp_app_dir/src $adapter_main__
src_memcpy 			$application_tmp_app_dir/src $adapter_include__
src_memcpy 			$application_tmp_app_dir/src $application_src_dir

## Platform Config
cp -a $application_sdk_config_dir/* 					$application_tmp_app_dir
cp -a $root_dir/tmp/kconfig_h/$chip/$application/aiio_autoconf.h	$application_tmp_app_dir/src

## Cp To SDK
rm -rf $sdk_dir/project/dev_project && cp -a $application_tmp_app_dir $sdk_dir/project/dev_project

## Make
cd $sdk_dir/ && python3 start_build.py rebuild

## rm -rf $root_dir/out/$chip/$application && mkdir -p $root_dir/out/$chip/$application && cp -a $sdk_dir/build-dev_project-release/bin $root_dir/out/$chip/$application

cd $cur_dir && ./bin_out.sh $sdk_dir $bin_out_dir

