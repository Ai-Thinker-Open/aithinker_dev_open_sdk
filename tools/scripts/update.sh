#!/bin/bash

#########################################################

##The Version Must To Download All Support OS

#########################################################

cd "$(dirname "$0")"

root_dir=$1
tool_dir=$1/tools

#echo "root:"$root_dir
#echo "tool:"$tool_dir

os_version=$root_dir/os/os_version
adt_os_version=$root_dir/adt/adt_os_version
application_version=$root_dir/application/application_version

## FUNC: Update SDK && Checkout version
function os_sdk_update() {
	cd $4/..

	## Check whether the sdk has a directory
	if [ -d "$1" ]; then
		echo ""
		cd ./$1 && rm -rf ./sdk/*

		git reset --hard HEAD && git checkout $3

		git pull
		git submodule init
		git submodule update
		cd - >>/dev/null
	else
		git clone --recurse-submodules $2
	fi

	cd $4/.. && ./*version_ctrl.sh
}

## FUNC: Read SDK Version Info && TODO Update SDK Func
function os_sdk_version_read() {
	while read line || [[ -n $LINE ]]; do
		if [[ "$line" == "name:"* ]]; then

			sdk_name=${line#*:}
			#echo $sdk_name
		fi

		if [[ "$line" == "branch:"* ]]; then

			sdk_branch=${line#*:}
			#echo $sdk_branch
		fi

		if [[ "$line" == "repository:"* ]]; then

			coding_https=$(echo ${line#*:})
			#echo $coding_https
		fi
	done <$1

	echo ""
	echo -e "\033[33m---------------------------------------------------------------------------------- \033[0m"
	echo "sdk_name:      $sdk_name"
	echo "sdk_branch:    $sdk_branch"
	echo "coding_https:  $coding_https"

	os_sdk_update "$sdk_name" "$coding_https" "$sdk_branch" $2
}

## Update Coding

## 开放版本代码，已经将（adt_os、application）合并到主库，考虑芯片SDK体积较大，仍然保留单独仓库方式提供
## 部分仓库权限，请联系商务进行开放

for file in $os_version/*"_version.txt"; do
	os_sdk_version_read $file $os_version
done

for file in $adt_os_version/*"_version.txt"; do
	os_sdk_version_read $file $adt_os_version
done

for file in $application_version/*"_version.txt"; do
	os_sdk_version_read $file $application_version
done

cd $tool_dir
