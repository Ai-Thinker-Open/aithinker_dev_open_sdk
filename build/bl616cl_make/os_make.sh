#!/bin/bash

cd "$(dirname "$0")"

echo "bl616cl os_make:"
echo ""

###############################################################################################################################################
## ./os_make.sh    $application_dir $application_tmp_app_dir $rtos_dir $application_tmp_typ_dir $root_dir $application $make_dir $adt_dir $chip
###############################################################################################################################################
cur_dir=$PWD
chip=$9
debugging_level=${10}
root_dir=$5
application=$6
application_dir=$1
application_tmp_app_dir=$2
application_src_dir=$application_dir/src

## This make dir only serves the bl616cl platform; $chip may be a board variant (bl616cl-48/-32),
## but the adt adapter (port/spec) always uses the bl616cl platform.
platform=bl616cl
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

sdk_dir=$3/sdk/bouffalo_sdk

source $adt_tools_dir/chip_week_func_gen.sh
source $cur_dir/../shell_func/shell_func.sh

if [ -f $root_dir/tmp/kconfig_h/$chip/$application/aiio_autoconf.h ]; then

	echo ""
else
	echo -e "\033[31merror: no ./build.sh \$chip \$application menuconfig\033[0m"
	exit 1

fi

src_to_weak_c 			$application_tmp_app_dir/src $adapter_chip_chip_dir__/include
src_to_weak_h 			$application_tmp_app_dir/src $adapter_chip_chip_dir__/include $adapter_chip_chip_common_dir__ $adapter_chip_chip_dir__/spec/$platform
wsrc_call_func_to_weak_c 	$application_tmp_app_dir/src $adapter_chip_chip_dir__/port/$platform
wsrc_call_func_to_ssrc_c 	$application_tmp_app_dir/src $adapter_chip_chip_dir__/port/$platform

adapter_core_h_include_h  	$application_tmp_app_dir/src $adt_os_dir__/adapter_core


src_memcpy 			$application_tmp_app_dir/src $adapter_chip_chip_dir__/include
src_memcpy 			$application_tmp_app_dir/src $adapter_chip_chip_dir__/spec/$platform
src_memcpy 			$application_tmp_app_dir/src $adapter_chip_chip_common_dir__
src_memcpy 			$application_tmp_app_dir/src $adapter_core__
src_memcpy 			$application_tmp_app_dir/src $adapter_main__
src_memcpy 			$application_tmp_app_dir/src $adapter_include__
src_memcpy 			$application_tmp_app_dir/src $application_src_dir

## Platform Config
cp -a $application_sdk_config_dir/* $application_tmp_app_dir
cp -a $root_dir/tmp/kconfig_h/$chip/$application/aiio_autoconf.h $application_tmp_app_dir/src

## Vendor lib: libdev_bl616cl.a (e.g. syn_user_info / bind-check) lives under OS tree patch/.
## Dropping it only under patch/lib does NOT link automatically; CMake must pull it into `app`.
patched_lib="$3/patch/lib/libdev_bl616cl.a"
dev_cmake="$application_tmp_app_dir/CMakeLists.txt"
if [[ -f "$patched_lib" ]]; then
	if [[ ! -f "$dev_cmake" ]]; then
		echo -e "\033[31merror: $dev_cmake missing; cannot attach libdev_bl616cl.a\033[0m"
		exit 1
	fi
	mkdir -p "$application_tmp_app_dir/lib"
	cp -a "$patched_lib" "$application_tmp_app_dir/lib/"
	# Repair older injections that used PRIVATE (CMake forbids mixing plain vs keyword on same target).
	sed -i 's/target_link_libraries(app PRIVATE \(.*lib\/libdev_bl616cl\.a\))/target_link_libraries(app \1)/g' "$dev_cmake" 2>/dev/null || true
	if ! grep -q 'lib/libdev_bl616cl\.a' "$dev_cmake"; then
		# Must match Bouffalo's plain target_link_libraries(app ...) — PRIVATE mixes signatures and CMake errors.
		inject='target_link_libraries(app ${CMAKE_CURRENT_SOURCE_DIR}/lib/libdev_bl616cl.a)'
		if grep -q '^project(' "$dev_cmake"; then
			tmp_cmake="$dev_cmake.$$"
			awk -v inj="$inject" '
				/^project\(/ && !inserted {
					print inj
					inserted = 1
				}
				{ print }
				END {
					if (!inserted)
						exit 2
				}
			' "$dev_cmake" >"$tmp_cmake" && mv "$tmp_cmake" "$dev_cmake"
		else
			printf '\n%s\n' "$inject" >>"$dev_cmake"
		fi
		echo "Linked libdev_bl616cl.a via CMakeLists in dev project tree"
	fi
	tool_nm="$(command -v riscv64-unknown-elf-nm 2>/dev/null)"
	if [[ -n "$tool_nm" ]] && ! "$tool_nm" --defined-only -g "$patched_lib" 2>/dev/null | grep -q 'syn_user_info'; then
		echo -e "\033[33mWarning: archive $patched_lib has no exported syn_user_info; link may still fail unless another object provides it.\033[0m"
	fi
else
	echo -e "\033[33mWarning: $patched_lib not found — bind/vendor symbols (e.g. syn_user_info) will stay unresolved.\033[0m"
fi

## Board macro: let firmware branch per variant (bl616cl-48 -> AIIO_BOARD_BL616CL_48, etc.). Idempotent.
board_macro="AIIO_BOARD_$(printf '%s' "$chip" | sed 's/[^A-Za-z0-9]/_/g' | tr 'a-z' 'A-Z')"
if [[ -f "$dev_cmake" ]] && ! grep -q "$board_macro" "$dev_cmake"; then
	printf '\n%s\n' "target_compile_definitions(app PUBLIC ${board_macro}=1)" >>"$dev_cmake"
	echo "Defined board macro ${board_macro} via CMakeLists in dev project tree"
fi

## Partition config (prefer application vendor partition table; avoid modifying SDK)
# bouffalo_sdk's build system generates `partition.bin` from the `partition*.toml` file
# located in the project root (examples/dev_project after copy).
#
# For at_demo on bl616cl, we allow overriding the partition table from application vendor,
# so users can customize flash layout without touching SDK board config.
#
# Use at_demo's chip-matched vendor partition table only.
# (No dependency on bl618 vendor files.)
partition_override="$application_dir/vendor/$chip/partition_cfg_4M.toml"

# Remove any existing partition toml in temp project to avoid
# "[Error] More than one partition file found" during build.
rm -f "$application_tmp_app_dir"/partition*.toml >/dev/null 2>&1

if [ -f "$partition_override" ]; then
    cp -a "$partition_override" "$application_tmp_app_dir/partition_cfg_4M.toml"
    echo "Using partition table: $partition_override"
else
    echo -e "\033[33mWarning: no vendor partition_cfg_4M.toml override found; SDK default may be used.\033[0m"
fi

# Tell bflb_fw_post_proc to use project-local ptfile instead of board config.
# This makes `make combine` generate partition.bin from the vendor-provided toml,
# without editing `${BL_SDK_BASE}/bsp/board/.../config/partition_cfg_4M.toml`.
if [ -f "$application_tmp_app_dir/partition_cfg_4M.toml" ] && [ -f "$application_tmp_app_dir/defconfig" ]; then
    # Normalize to Makefile style with spaces around '=' (matches other defconfig entries).
    if grep -q '^CONFIG_OVERRIDE_PTFILE' "$application_tmp_app_dir/defconfig"; then
        sed -i 's/^CONFIG_OVERRIDE_PTFILE[[:space:]]*=.*/CONFIG_OVERRIDE_PTFILE = partition_cfg_4M.toml/' "$application_tmp_app_dir/defconfig"
    else
        echo "CONFIG_OVERRIDE_PTFILE = partition_cfg_4M.toml" >> "$application_tmp_app_dir/defconfig"
    fi
    echo "Set CONFIG_OVERRIDE_PTFILE in defconfig:"
    grep '^CONFIG_OVERRIDE_PTFILE' "$application_tmp_app_dir/defconfig" || true

    # Force-add --ptfile to bflb_fw_post_proc command line.
    # This is more reliable than relying on CONFIG_OVERRIDE_PTFILE being propagated
    # through the Makefile->CMake generated config chain in all environments.
    if grep -q '^CONFIG_FW_POST_PROC_CUSTOM' "$application_tmp_app_dir/defconfig"; then
        sed -i 's/^CONFIG_FW_POST_PROC_CUSTOM[[:space:]]*=.*/CONFIG_FW_POST_PROC_CUSTOM = --ptfile=partition_cfg_4M.toml/' "$application_tmp_app_dir/defconfig"
    else
        echo "CONFIG_FW_POST_PROC_CUSTOM = --ptfile=partition_cfg_4M.toml" >> "$application_tmp_app_dir/defconfig"
    fi
    echo "Set CONFIG_FW_POST_PROC_CUSTOM in defconfig:"
    grep '^CONFIG_FW_POST_PROC_CUSTOM' "$application_tmp_app_dir/defconfig" || true
fi

## Cp To SDK
dev_proj="$sdk_dir/examples/dev_project"
if [[ "$debugging_level" == "debug_tmp" && -d "$dev_proj" ]] && command -v rsync >/dev/null 2>&1; then
    rsync -a --no-times --checksum "$application_tmp_app_dir/" "$dev_proj/"
else
    rm -rf "$dev_proj" && cp -a "$application_tmp_app_dir" "$dev_proj"
    if [ -f "$dev_proj/partition_cfg_4M.toml" ]; then
        rm -rf "$dev_proj/build" >/dev/null 2>&1
    fi
fi

## Make
cd "$dev_proj" && make

## Cp To out
## boot2_vendor_override: use application's own boot2 image instead of the SDK's builtin one.
## Matched by pattern (not a fixed name) so swapping in a new boot2_isp_bl616cl_*_release.bin
## commit build under vendor/$chip/ needs no script change.
boot2_vendor_override=$(ls "$application_dir"/vendor/$chip/boot2_isp_bl616cl_*_release.bin 2>/dev/null | head -n 1)
cd $cur_dir && ./bin_out.sh $sdk_dir $bin_out_dir "$boot2_vendor_override"

