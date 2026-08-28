#!/bin/bash
# update.sh: 更新 SDK 版本
# Usage:
#   ./update.sh <root_dir>               # 全量更新：os_version、adt_os_version、application_version
#   ./update.sh <root_dir> <chip>        # 芯片模式：仅更新 os_version 中匹配 <chip> 的版本文件，跳过其它目录
#   ./update.sh <root_dir> adt_os        # ADT 模式：仅更新 adt_os_version，跳过 os_version 和 application_version
#   ./update.sh <root_dir> application   # 应用模式：仅更新 application_version，跳过 os_version 和 adt_os_version

set -e  # 出现错误时退出

cd "$(dirname "$0")" || exit 1

root_dir=$1
tool_dir=$1/tools

# 简单检查 root_dir 是否存在
if [[ -z "$root_dir" ]]; then
    echo -e "\033[31m[update.sh] 错误：缺少 root_dir 参数\033[0m" >&2
    exit 1
fi

echo -e "\033[32m[update.sh] Received parameters count: $#\033[0m"
echo -e "\033[32m[update.sh] All parameters: $*\033[0m"

# 解析第二个参数：可能是芯片名，也可能是 "application"
param="$2"
mode_full=false
mode_application=false
mode_adt_os=false
chip=""

if [[ -n "$param" ]]; then
    if [[ "$param" == "application" ]]; then
        mode_application=true
        echo -e "\033[32m[update.sh] 模式: 仅更新 application_version\033[0m"
    elif [[ "$param" == "adt_os" ]]; then
        mode_adt_os=true
        echo -e "\033[32m[update.sh] 模式: 仅更新 adt_os_version\033[0m"
    else
        chip="$param"
        # 校验芯片名格式：仅字母数字和下划线
        if ! [[ "$chip" =~ ^[a-zA-Z0-9_]+$ ]]; then
            echo -e "\033[31m[update.sh] 错误：芯片名 '$chip' 格式非法\033[0m" >&2
            exit 1
        fi
        echo -e "\033[32m[update.sh] 模式: 仅更新 os_version 中与芯片 '$chip' 相关内容，跳过其它目录\033[0m"
    fi
else
    mode_full=true
    echo -e "\033[32m[update.sh] 模式: 全量更新（os_version、adt_os_version、application_version）\033[0m"
fi

# 各版本目录
os_version=$root_dir/os/os_version
adt_os_version=$root_dir/adt/adt_os_version
application_version=$root_dir/application/application_version

# 检查目录是否存在（可选）
if [[ "$mode_full" == true || -n "$chip" ]]; then
    if [[ ! -d "$os_version" ]]; then
        echo -e "\033[31m[update.sh] 错误：os_version 目录不存在: $os_version\033[0m" >&2
        exit 1
    fi
fi
if [[ "$mode_full" == true || "$mode_adt_os" == true ]]; then
    if [[ ! -d "$adt_os_version" ]]; then
        echo -e "\033[31m[update.sh] 错误：adt_os_version 目录不存在: $adt_os_version\033[0m" >&2
        exit 1
    fi
fi
if [[ "$mode_full" == true ]]; then
    if [[ ! -d "$application_version" ]]; then
        echo -e "\033[31m[update.sh] 错误：application_version 目录不存在: $application_version\033[0m" >&2
        exit 1
    fi
fi
if [[ "$mode_application" == true ]]; then
    if [[ ! -d "$application_version" ]]; then
        echo -e "\033[31m[update.sh] 错误：application_version 目录不存在: $application_version\033[0m" >&2
        exit 1
    fi
fi

##############################
# 定义函数：os_sdk_update, os_sdk_version_read
##############################

# os_sdk_update: 根据 version 文件信息更新或 clone SDK
# 参数：
#   $1 = sdk_name
#   $2 = repository URL (coding_https)
#   $3 = branch
#   $4 = base_dir （版本文件所在目录，例如 os_version、adt_os_version、application_version 的父目录）
#   $5 = commit
function os_sdk_update() {
    local sdk_name="$1"
    local repo_url="$2"
    local branch="$3"
    local base_dir="$4"
    local commit="$5"

    # 进入 base_dir 父目录。例如 base_dir="/path/.../os/os_version"，则 cd 到 "/path/.../os"
    cd "$base_dir/.." || return

    # 如果目录 sdk_name 存在，则进入目录，否则 clone
    if [[ -d "$sdk_name" ]]; then
        echo "[update.sh][os_sdk_update] 目录已存在: $sdk_name，执行更新流程"
        cd "./$sdk_name" || return
        rm -rf ./sdk/* 2>/dev/null || true

        echo "[update.sh][os_sdk_update] 当前路径: $(pwd)"
        # 恢复仓库状态并切换分支
        git reset --hard HEAD
        git checkout "$branch" || { echo "[update.sh][os_sdk_update] git checkout $branch 失败" >&2; }
        git pull || echo "[update.sh][os_sdk_update] git pull 失败" >&2

        if [[ "$commit" =~ ^[[:space:]]*$ ]]; then
            echo -e "\033[31m[update.sh][os_sdk_update] 警告：commit 为空\033[0m"
        fi

        git reset --hard "$commit" || echo "[update.sh][os_sdk_update] git reset --hard $commit 失败" >&2

        # 更新子模块
        git submodule init
        git submodule update --progress

        cd - >>/dev/null || return
    else
        # clone 到 sdk_name 目录
        echo "[update.sh][os_sdk_update] 目录不存在，执行 git clone: $repo_url 分支 $branch 到目录 $sdk_name"
        git clone --branch "$branch" "$repo_url" "$sdk_name" || { echo "[update.sh][os_sdk_update] git clone 失败" >&2; return; }
        cd "./$sdk_name" || return

        git reset --hard HEAD
        git checkout "$branch" || echo "[update.sh][os_sdk_update] git checkout $branch 失败" >&2
        git pull || echo "[update.sh][os_sdk_update] git pull 失败" >&2

        if [[ "$commit" =~ ^[[:space:]]*$ ]]; then
            echo -e "\033[31m[update.sh][os_sdk_update] 警告：commit 为空\033[0m"
        fi

        git reset --hard "$commit" || echo "[update.sh][os_sdk_update] git reset --hard $commit 失败" >&2

        git submodule init
        git submodule update --progress

        cd - >>/dev/null || return
    fi

    # 返回上一层并调用 version_ctrl.sh（如果存在）
    # 例如 base_dir="/path/.../os/os_version"，则 cd "/path/.../os" 后执行 "./*version_ctrl.sh"
    cd "$base_dir/.." || return
    # 如果目录下存在 version_ctrl.sh 或符合通配的 version_ctrl 脚本，则执行
    # 注意：确保不会误执行过多脚本，通常项目中只有一个 version_ctrl.sh
    for script in ./*version_ctrl.sh; do
        if [[ -f "$script" && -x "$script" ]]; then
            echo "[update.sh][os_sdk_update] 运行版本控制脚本: $script"
            "$script" || echo "[update.sh][os_sdk_update] 脚本 $script 执行失败" >&2
        fi
    done
}

# os_sdk_version_read: 读取 version 文件并调用 os_sdk_update
# 参数：
#   $1 = version_file 完整路径
#   $2 = base_dir，即 version_file 所在目录
function os_sdk_version_read() {
    local version_file="$1"
    local base_dir="$2"

    # 提前检查
    if [[ ! -f "$version_file" ]]; then
        echo -e "\033[31m[update.sh][os_sdk_version_read] 错误：版本文件不存在: $version_file\033[0m" >&2
        return 1
    fi

    # 初始化局部变量
    local sdk_name=""
    local sdk_branch=""
    local sdk_commit=""
    local coding_https=""

    # 读取 version_file
    while IFS= read -r line || [[ -n "$line" ]]; do
        if [[ "$line" == name:* ]]; then
            sdk_name="${line#*:}"
            sdk_name="${sdk_name//[[:space:]]/}"  # 去掉前后空白
        elif [[ "$line" == branch:* ]]; then
            sdk_branch="${line#*:}"
            sdk_branch="${sdk_branch//[[:space:]]/}"
        elif [[ "$line" == commit:* ]]; then
            sdk_commit="${line#*:}"
            sdk_commit="${sdk_commit//[[:space:]]/}"
        elif [[ "$line" == repository:* ]]; then
            coding_https="${line#*:}"
            coding_https="${coding_https//[[:space:]]/}"
        fi
    done <"$version_file"

    echo ""
    echo -e "\033[33m----------------------------------------------------------------------------------\033[0m"
    echo "[update.sh][os_sdk_version_read] 版本文件: $version_file"
    echo "[update.sh][os_sdk_version_read] sdk_name:      $sdk_name"
    echo "[update.sh][os_sdk_version_read] sdk_branch:    $sdk_branch"
    echo "[update.sh][os_sdk_version_read] sdk_commit:    $sdk_commit"
    echo "[update.sh][os_sdk_version_read] coding_https:  $coding_https"

    # 调用更新函数
    os_sdk_update "$sdk_name" "$coding_https" "$sdk_branch" "$base_dir" "$sdk_commit"
}

##############################
# 主流程：按模式处理各目录
##############################

# 1. 处理 os_version
if [[ "$mode_application" == true ]]; then
    echo "[update.sh] 跳过 os_version 更新 (application-only 模式)"
elif [[ "$mode_adt_os" == true ]]; then
    echo "[update.sh] 跳过 os_version 更新 (adt_os-only 模式)"
elif [[ -n "$chip" ]]; then
    # 芯片模式，只处理 os_version 下匹配的文件
    # 根据实际文件命名规则调整 target
    target="$os_version/${chip}_sdk_lts_version.txt"
    if [[ -f "$target" ]]; then
        echo "[update.sh] 处理 OS 版本文件: $target"
        os_sdk_version_read "$target" "$os_version"
    else
        # 通配匹配：文件名中包含芯片名且以 _version.txt 结尾
        matched=( "$os_version/"*"$chip"*"_version.txt" )
        found=false
        for f in "${matched[@]}"; do
            if [[ -f "$f" ]]; then
                echo "[update.sh] 匹配到 OS 版本文件: $f"
                os_sdk_version_read "$f" "$os_version"
                found=true
            fi
        done
        if ! $found; then
            echo -e "\033[31m[update.sh] 未找到 os_version 下与芯片 '$chip' 对应的版本文件\033[0m" >&2
            exit 1
        fi
    fi
else
    # 全量或 application-only 不到这里，mode_full 情况才会执行
    if [[ "$mode_full" == true ]]; then
        for file in "$os_version/"*"_version.txt"; do
            [[ -f "$file" ]] || continue
            echo "[update.sh] 处理 OS 版本文件: $file"
            os_sdk_version_read "$file" "$os_version"
        done
    fi
fi

# 2. 处理 adt_os_version
if [[ "$mode_full" == true || "$mode_adt_os" == true ]]; then
    for file in "$adt_os_version/"*"_version.txt"; do
        [[ -f "$file" ]] || continue
        echo "[update.sh] 处理 ADT OS 版本文件: $file"
        os_sdk_version_read "$file" "$adt_os_version"
    done
else
    echo "[update.sh] 跳过 adt_os_version 更新"
fi

# 3. 处理 application_version
if [[ -n "$chip" ]]; then
    echo "[update.sh] 跳过 application_version 更新 (芯片模式)"
elif [[ "$mode_adt_os" == true ]]; then
    echo "[update.sh] 跳过 application_version 更新 (adt_os-only 模式)"
else
    # mode_full 或 mode_application 都处理 application_version
    for file in "$application_version/"*"_version.txt"; do
        [[ -f "$file" ]] || continue
        echo "[update.sh] 处理 应用 版本文件: $file"
        os_sdk_version_read "$file" "$application_version"
    done
fi

# 返回 tool_dir
cd "$tool_dir" || exit 1

echo -e "\033[32m[update.sh] 完成\033[0m"

