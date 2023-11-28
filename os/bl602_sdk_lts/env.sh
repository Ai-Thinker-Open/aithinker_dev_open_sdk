#!/bin/bash

## Sdk dir
lsdk_dir="$PWD"
## Tool dir
tool_dir="$lsdk_dir/tools"


if [ -d "$tool_dir/flash_tool" ]; then

echo ""

else

cd $tool_dir && tar -zxvf flash_tool.tar.gz

fi

cd $lsdk_dir && ./patch.sh
