#!/bin/bash

cd "$(dirname "$0")"

echo "bl602 os_make:"
echo ""

###############################################################################################################################################
## ./os_make.sh    $application_dir $application_tmp_app_dir $rtos_dir $application_tmp_typ_dir $root_dir $application $make_dir $adt_dir $chip
###############################################################################################################################################
cur_dir=$PWD
chip=$9
debugging_level=${10}
root_dir=$5
application=$6
application_dir=$1			#应用根目录
application_tmp_app_dir=$2	#源码整合复制后的临时目录，"$root_dir/tmp/adt_os_src"
application_tmp_app_dir_src=$application_tmp_app_dir/src
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

adt_os_dir__=$root_dir/tmp/adt_os_src       #不直接操作原代码，通过复制一份到临时目录进行操作
adapter_chip__=$adt_os_dir__/adapter_chip
adapter_core__=$adt_os_dir__/adapter_core
adapter_main__=$adt_os_dir__/adapter_main
adapter_include__=$adt_os_dir__/adapter_include
adapter_chip_chip_dir__=$adapter_chip__/chip
adapter_chip_chip_common_dir__=$adapter_chip__/chip_common

bin_out_dir=$root_dir/out/$chip/$application

###############################################################################################################################################
## 需要根据不同的芯片和应用进行修改
###############################################################################################################################################
sdk_dir=$3/sdk/bl_iot_sdk	#SDK目录
commonent_tmp_app_dir_adt_os=$root_dir/tmp/adt_os_staged
adt_os_sdk_dir=$sdk_dir/components/adt_os

source $adt_tools_dir/chip_week_func_gen.sh
source $cur_dir/../shell_func/shell_func.sh

if [ -f $root_dir/tmp/kconfig_h/$chip/$application/aiio_autoconf.h ]; then
    
    echo ""
else
    echo -e "\033[31merror: no ./build.sh \$chip \$application menuconfig\033[0m"
    exit 1
    
fi

adt_os_commit_id=$(git -C "$adt_os_dir" rev-parse --short HEAD 2>/dev/null || echo "unknown")

rm $commonent_tmp_app_dir_adt_os -rf    #每次编译前清空临时组件目录

src_to_weak_c 			$commonent_tmp_app_dir_adt_os $adapter_chip_chip_dir__/include
src_to_weak_h 			$commonent_tmp_app_dir_adt_os $adapter_chip_chip_dir__/include $adapter_chip_chip_common_dir__ $adapter_chip_chip_dir__/spec/$chip
wsrc_call_func_to_weak_c 	$commonent_tmp_app_dir_adt_os $adapter_chip_chip_dir__/port/$chip
wsrc_call_func_to_ssrc_c 	$commonent_tmp_app_dir_adt_os $adapter_chip_chip_dir__/port/$chip

adapter_core_h_include_h  	$commonent_tmp_app_dir_adt_os $adt_os_dir__/adapter_core


src_memcpy 			$commonent_tmp_app_dir_adt_os $adapter_chip_chip_dir__/include
src_memcpy 			$commonent_tmp_app_dir_adt_os $adapter_chip_chip_dir__/spec/$chip
src_memcpy 			$commonent_tmp_app_dir_adt_os $adapter_chip_chip_common_dir__
src_memcpy 			$commonent_tmp_app_dir_adt_os $adapter_core__
src_memcpy 			$commonent_tmp_app_dir_adt_os $adapter_main__
src_memcpy 			$commonent_tmp_app_dir_adt_os $adapter_include__

src_memcpy 			$application_tmp_app_dir_src $application_src_dir	#第三方开发者编写的应用源码，这个版本将应用代码进行分离，不再和SDK源码混合在一起

## Platform Config
cp -a $application_sdk_config_dir/* $application_tmp_app_dir
cp -a $root_dir/tmp/kconfig_h/$chip/$application/aiio_autoconf.h $commonent_tmp_app_dir_adt_os
echo "#ifndef ADT_OS_COMMIT_ID" >> $commonent_tmp_app_dir_adt_os/aiio_autoconf.h
echo "#define ADT_OS_COMMIT_ID \"$adt_os_commit_id\"" >> $commonent_tmp_app_dir_adt_os/aiio_autoconf.h
echo "#endif" >> $commonent_tmp_app_dir_adt_os/aiio_autoconf.h

###############################################################################################################################################
## 需要根据不同的芯片和应用进行修改
###############################################################################################################################################
## Cp To SDK
cp -a $application_tmp_app_dir/adt_os/bouffalo.mk $commonent_tmp_app_dir_adt_os
if [[ "$debugging_level" == "debug_tmp" && -d "$adt_os_sdk_dir" ]] && command -v rsync >/dev/null 2>&1; then
    rsync -a --no-times --checksum "$commonent_tmp_app_dir_adt_os/" "$adt_os_sdk_dir/"
else
    rm -rf "$adt_os_sdk_dir" && cp -a "$commonent_tmp_app_dir_adt_os" "$adt_os_sdk_dir"
fi
dev_proj="$sdk_dir/customer_app/dev_project"
if [[ "$debugging_level" == "debug_tmp" && -d "$dev_proj" ]] && command -v rsync >/dev/null 2>&1; then
    rsync -a --no-times --checksum "$application_tmp_app_dir/" "$dev_proj/"
else
    rm -rf "$dev_proj" && cp -a "$application_tmp_app_dir" "$dev_proj"
fi

## Make
cd "$dev_proj" && ./genromap

## Cp To out
cd $cur_dir && ./bin_out.sh $sdk_dir $bin_out_dir

