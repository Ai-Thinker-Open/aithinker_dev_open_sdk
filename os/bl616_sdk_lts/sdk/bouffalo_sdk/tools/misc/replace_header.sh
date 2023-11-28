#!/bin/sh

# 该工具可以检索当前路径下的所有C文件和头文件，并且将C文件里include <xxx.h>替换为 include <aaa/bbb/xxx.h>

for inc in `find . -name "*.[ch]" | xargs awk '/#include [<"](.+)[">]/ {print $2}' | sort | uniq`;
do
  include_str="#include $inc"
  inc=`echo $inc | sed 's/[<>\"]//g'`
  basename_str=`basename $inc`
  relative_path=`find . -name $basename_str`
  count=`echo $relative_path | wc -w`

  if [ $count -ne 1 ]
  then
    echo "$basename_str wont process!"
    continue
  fi

  relative_path=`echo $relative_path | sed 's/\.\///g'`
  new_include_str="#include \"$relative_path\""

  sed_command=`echo "s?$include_str?$new_include_str?g" | sed 's/\"/\\"/g'`
  find . -name "*.[ch]" | xargs sed -i "$sed_command"
done
