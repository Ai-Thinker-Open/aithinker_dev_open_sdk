#!/bin/bash

# 检查和转换指定路径下所有txt文件的行结束符（支持递归）
# 确保dos2unix已经安装

# 检查是否提供了路径参数
if [ "$#" -lt 1 ]; then
    echo "使用方法: $0 [路径]"
    exit 1
fi

# 获取路径
directory=$1

# 检查dos2unix是否安装
if ! command -v dos2unix > /dev/null 2>&1; then
    echo "错误: dos2unix 未安装，请先安装它。"
    exit 1
fi

# 判断路径是否存在
if [ ! -d "$directory" ]; then
    echo "指定的路径不存在: $directory"
    exit 1
fi

# 初始化统计数据
converted_count=0
total_count=0

echo "正在处理目录: $directory"

# 使用find递归查找所有txt文件并转换
while IFS= read -r -d '' file; do
    total_count=$((total_count + 1))
    if dos2unix -k -o "$file"; then
        echo "已转换文件: $file"
        converted_count=$((converted_count + 1))
    else
        echo "转换失败: $file" >&2
    fi
done < <(find "$directory" -type f -name "*.txt" -print0)

# 打印处理结果
echo "处理完成。总文件数: $total_count, 成功转换: $converted_count"
