#!/bin/bash

sdk_dir=$1
bin_out_dir=$2
boot2_vendor_override=$3

bin_out_elf=$bin_out_dir/elf
bin_out_bin=$bin_out_dir/bin
bin_out_map=$bin_out_dir/map

# Platform adapter
bin_src_elf=$sdk_dir/examples/dev_project/build/build_out/dev_project_bl616cl.elf
bin_src_bin=$sdk_dir/examples/dev_project/build/build_out/dev_project_bl616cl.bin
bin_src_map=$sdk_dir/examples/dev_project/build/build_out/dev_project_bl616cl.map
partition_src=$sdk_dir/examples/dev_project/build/build_out/partition.bin
bin_src_ota=$sdk_dir/examples/dev_project/build/build_out/dev_project_bl616cl.bin.ota
bin_src_hash=$sdk_dir/examples/dev_project/build/build_out/dev_project_bl616cl.bin.hash
bin_src_xz=$sdk_dir/examples/dev_project/build/build_out/dev_project_bl616cl.xz
bin_src_xz_hash=$sdk_dir/examples/dev_project/build/build_out/dev_project_bl616cl.xz.hash
bin_src_xz_ota=$sdk_dir/examples/dev_project/build/build_out/dev_project_bl616cl.xz.ota

# Only use the application's own vendor boot2 image; no fallback to the SDK's builtin one.
if [ -n "$boot2_vendor_override" ] && [ -f "$boot2_vendor_override" ]; then
    boot2_src="$boot2_vendor_override"
    boot2_type="vendor override"
else
    echo "Error: vendor boot2 file not found: $boot2_vendor_override"
    exit 1
fi


function bin_out()
{
	rm -rf   $1

	mkdir -p $2 && cp -a $5 $2/dev_project.bin >/dev/null 2>&1
	mkdir -p $3 && cp -a $6 $3/dev_project.elf >/dev/null 2>&1
	mkdir -p $4 && cp -a $7 $4/dev_project.map >/dev/null 2>&1
	mkdir -p $1/ota
	
	# Copy boot2 and partition files
	if [ -n "$boot2_src" ] && [ -f "$boot2_src" ]; then
		cp -a "$boot2_src" $2/boot2.bin >/dev/null 2>&1
		echo "boot2.bin copied ($boot2_type version)"
	else
		echo "Warning: boot2 file not found, skipping boot2.bin"
	fi
	
	if [ -f "$partition_src" ]; then
		cp -a "$partition_src" $2/partition.bin >/dev/null 2>&1
		echo "partition.bin copied"
	else
		echo "Warning: partition file not found: $partition_src"
	fi
	
	# ===================== 核心修改 =====================
	# 所有OTA文件 只拷贝到 ota 目录，不再拷贝到 bin 目录
	# ====================================================
	if [ -f "$bin_src_ota" ]; then
		cp -a "$bin_src_ota" $1/ota/dev_project_bl616cl.bin.ota >/dev/null 2>&1
		echo "dev_project.bin.ota copied"
	else
		echo "Warning: ota file not found: $bin_src_ota"
	fi
	
	if [ -f "$bin_src_hash" ]; then
		cp -a "$bin_src_hash" $1/ota/dev_project_bl616cl.bin.hash >/dev/null 2>&1
		echo "dev_project.bin.hash copied"
	else
		echo "Warning: hash file not found: $bin_src_hash"
	fi
	
	if [ -f "$bin_src_xz" ]; then
		cp -a "$bin_src_xz" $1/ota/dev_project_bl616cl.xz >/dev/null 2>&1
		echo "dev_project.xz copied"
	else
		echo "Warning: xz file not found: $bin_src_xz"
	fi
	
	if [ -f "$bin_src_xz_hash" ]; then
		cp -a "$bin_src_xz_hash" $1/ota/dev_project_bl616cl.xz.hash >/dev/null 2>&1
		echo "dev_project.xz.hash copied"
	else
		echo "Warning: xz hash file not found: $bin_src_xz_hash"
	fi

	if [ -f "$bin_src_xz_ota" ]; then
		cp -a "$bin_src_xz_ota" $1/ota/dev_project_bl616cl.xz.ota >/dev/null 2>&1
		echo "dev_project.xz.ota copied"
	else
		echo "Warning: xz ota file not found: $bin_src_xz_ota"
	fi

	# ===================== 已删除：ai.ota 生成代码 =====================

	# Create flash programming config file
	flash_cfg_file=$bin_out_dir/flash_prog_cfg.ini
	cat > $flash_cfg_file << 'EOF'
[cfg]
# 0: no erase, 1:programmed section erase, 2: chip erase
# Use chip erase to ensure both dynamic partition table copies (0xE000 and 0xF000) are cleared.
erase = 2
# skip mode set first para is skip addr, second para is skip len, multi-segment region with ; separated
skip_mode = 0x0, 0x0
# 0: not use isp mode, #1: isp mode
boot2_isp_mode = 0

[boot2]
filedir = ./bin/boot2.bin
address = 0x000000

[partition]
filedir = ./bin/partition.bin
address = 0xE000

[FW]
filedir = ./bin/dev_project.bin
address = @partition
EOF
	echo "Created flash_prog_cfg.ini"
}

bin_out $bin_out_dir $bin_out_bin $bin_out_elf $bin_out_map $bin_src_bin $bin_src_elf $bin_src_map