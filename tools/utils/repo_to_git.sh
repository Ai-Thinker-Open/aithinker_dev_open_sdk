#!/bin/bash

# 检查命令行参数
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <path-to-sdk>"
    exit 1
fi

sdk_path="$1"

# 验证指定的路径是否存在
if [ ! -d "$sdk_path" ]; then
    echo "The specified SDK path does not exist: $sdk_path"
    exit 1
fi

# 检查是否安装了git
if ! command -v git &> /dev/null; then
    echo "git is not installed. Please install git and try again."
    exit 1
fi

# 导出Git日志历史
echo "Exporting Git log history in $sdk_path..."
git -C "$sdk_path" log > "$sdk_path/git_log_history.txt"

# 删除.repo目录
echo "Removing .repo directory..."
rm -rf "$sdk_path/.repo"

# 查找并报告超过100MB的大文件
echo "Searching for files larger than 100MB in $sdk_path..."
big_files=$(find "$sdk_path" -type f -size +100M -print0)

if [ -n "$big_files" ]; then
    echo "Large files found. The script will terminate. Here are the paths:"
    echo "$big_files" | xargs -0
    exit 1
else
    echo "No large files found. Continuing..."
fi

# 删除现有的Git配置文件和文件夹
echo "Removing existing .git directories and .gitignore files..."
find "$sdk_path" -name ".git" -print0 | xargs -0 rm -rf
find "$sdk_path" -name ".gitignore" -print0 | xargs -0 rm -rf

# 初始化新的Git仓库
echo "Initializing new Git repository in $sdk_path..."
git -C "$sdk_path" init

# 添加所有文件到暂存区
echo "Adding all files to staging area..."
git -C "$sdk_path" add -A

# 提交更改
echo "Committing changes..."
git -C "$sdk_path" commit -m "repo to git by zifeng"

echo "Git repository has been successfully initialized and updated."
