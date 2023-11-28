#!/bin/bash

CONFIG_FILE=otapack.ini

echo "CONFIG_FILE = ${CONFIG_FILE}"

PACK_METADATA=pack_metadata

PACK_DATA_MAX_LEN=128


# 添加文件头, 0xaa 0x55 0xff 0x44 0x88 0xaa
echo aa | xxd -r -ps >>${PACK_METADATA}
echo 55 | xxd -r -ps >>${PACK_METADATA}
echo ff | xxd -r -ps >>${PACK_METADATA}
echo 44 | xxd -r -ps >>${PACK_METADATA}
echo 88 | xxd -r -ps >>${PACK_METADATA}
echo aa | xxd -r -ps >>${PACK_METADATA}

# 添加文件头长度偏移量
printf "%04x" ${PACK_DATA_MAX_LEN} | xxd -r -ps >>${PACK_METADATA}


# 添加OTA文件类型
cat ${CONFIG_FILE} | grep "OTA type"
ota_type=$(cat ${CONFIG_FILE} | grep "OTA type=" | cut -d "=" -f 2 | tr -d '\n')
echo "ota_type = ${ota_type}"
echo ${ota_type} | xxd -r -ps >>${PACK_METADATA}


# 添加OTA版本
ota_version=$(cat ${CONFIG_FILE} | grep "OTA version=" | cut -d "=" -f 2 | tr -d '\n')
echo ${ota_version} | tr -d '\n' >>${PACK_METADATA}
if [ ${#ota_version} -lt 8 ]; then
    for i in $(seq ${#ota_version} 7); do
        echo 00 | xxd -r -ps >>${PACK_METADATA}
    done
fi

# 添加OTA原文件数据长度
cat ${CONFIG_FILE} | grep "Bin Path="
bin_path=$(cat ${CONFIG_FILE} | grep "Bin Path=" | cut -d "=" -f 2 | tr -d '\n')
echo "bin_path = ${bin_path}"
file_size=$(du -b ${bin_path} | awk '{print $1}')
echo "file_size = ${file_size}"
printf "%08x" ${file_size} | xxd -r -ps >>${PACK_METADATA}

# reserved
for i in $(seq 1 11); do
    echo 00 | xxd -r -ps >>${PACK_METADATA}
done

# 添加产品ID
produtcid=$(cat ${CONFIG_FILE} | grep "Product ID=" | cut -d "=" -f 2 | tr -d '\n')
echo ${produtcid} | tr -d '\n' >>${PACK_METADATA}
if [ ${#produtcid} -lt 64 ]; then
    for i in $(seq ${#produtcid} 63); do
        echo 00 | xxd -r -ps >>${PACK_METADATA}
    done
fi

# reserved
for i in $(seq 1 24); do
    echo 00 | xxd -r -ps >>${PACK_METADATA}
done


#添加OTA原文件数据的CRC
cat ${bin_path} >>all_bin
crc32 ${bin_path} | xxd -r -ps >>${PACK_METADATA}

#添加OTA文件头数据的CRC
crc32 ${PACK_METADATA} | xxd -r -ps >>${PACK_METADATA}

# 获取ota文件名
ota_file_name=$(cat ${CONFIG_FILE} | grep "File name=" | cut -d "=" -f 2 | tr -d '\n')
echo "ota_file_name = ${ota_file_name}"

if [ -f ${ota_file_name}.bin ]
then
    echo "rm ${ota_file_name}.bin"
    rm -r ${ota_file_name}.bin
    echo "create ${ota_file_name}.bin"
    cat ${PACK_METADATA} all_bin >${ota_file_name}.bin
else
    echo "create ${ota_file_name}.bin"
    cat ${PACK_METADATA} all_bin >${ota_file_name}.bin
fi

rm -r ${PACK_METADATA}
rm -r all_bin