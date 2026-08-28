#!/bin/bash

cd sdk/ameba-rtos
#清理工作目录
git checkout -- .
#git clean -f -d
#git clean -Xfd

#copy lib
cp ../../patch/lib/libdev_rtl8711dn.a amebadplus_gcc_project/project_km4/asdk/lib/application/ -a


function apply_patch(){
    for element in `ls $1`
    do  
        dir_or_file=$1"/"$element
        if [[ -d $dir_or_file ]]
        then 
            apply_patch $dir_or_file 
        elif [[ $dir_or_file = *".patch" ]]
        then
            git apply $dir_or_file >/dev/null 2>&1
	    if [ $? -ne 0 ]
	    then
		    echo -e "\e[31mpatch $dir_or_file error\e[0m"
		    return 1
	    fi
        fi  
    done
}

apply_patch ../../patch
if [ $? -ne 0 ]
then
	echo  -e "\e[31mpatch error\e[0m"
	exit 1
fi

cd ../../

echo -e "\e[32mpatch success\e[0m"

exit 0

