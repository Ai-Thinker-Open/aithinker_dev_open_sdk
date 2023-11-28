#!/bin/bash

adt_os_dir=$1
adt_os_tmp_dir=$2
prj_config=$3

function adt_src_memcpy() {
	rm -rf $2 && mkdir -p $2 && cp -a $1/* $2/
}

function adt_modules_conf_read_cpy() {
	adt_modules_flag=0

	while read line || [[ -n $LINE ]]; do
		if [[ "$line" == "# adt_modules"* ]]; then

			adt_modules_flag=1

			#echo $line

		fi

		if [[ "$line" == "CONFIG_"*"=y" ]] && [[ $adt_modules_flag == 1 ]]; then

			adt_modules_next=0

			#echo $line
			modules_tmp=${line#*CONFIG_}
			modules=${modules_tmp%=y*}

			#echo $modules

			cat $1/adapter_modules | jq '.adapter_chip','.adapter_core' | jq '.[]' >$1/adapter_modules_tmp

			while read adapter_modules_tmp_line || [[ -n $LINE ]]; do

				if [[ "$adapter_modules_tmp_line" == \"name\":* ]]; then

					name_tmp=${adapter_modules_tmp_line#*:}
					name=${name_tmp%,*}
					name_kfg=$(echo $name | sed 's/\"//g')

					if [[ $name_kfg == $modules ]]; then

						#echo "cc:"$name_kfg
						adt_modules_next=1
					fi
				fi

				if [[ "$adapter_modules_tmp_line" == \"path\":* ]] && [[ $adt_modules_next == 1 ]]; then

					path=${adapter_modules_tmp_line#*:}
					path_kfg=$(echo $path | sed 's/\"//g')

					#echo $path_kfg
					#echo $1/$path_kfg
					#echo $2/$path_kfg
					
					rm -rf $2/$path_kfg && mkdir -p $2/$path_kfg 
					
					if [ "`ls -A $1/$path_kfg`" = "" ]; then
  						echo ""
					else
  						cp -a $1/$path_kfg/. $2/$path_kfg
					fi
					
					
					adt_modules_next=0
				fi

			done <$1/adapter_modules_tmp

		fi

		if [[ "$line" == "# end of adt_modules"* ]]; then

			adt_modules_flag=0

			#echo $line

		fi
	done <$3
	
	rm -rf $1/adapter_modules_tmp
}

function adt_os_src_gen_func()
{
	rm -rf $2
	
	adt_modules_conf_read_cpy $1 $2 $3
	
	rm -rf $2/adapter_main && mkdir -p $2/adapter_main && cp -a $1/adapter_main $2/adapter_main
	
	rm -rf $2/adapter_include && mkdir -p $2/adapter_include && cp -a $1/adapter_include $2/adapter_include
}

adt_os_src_gen_func "$adt_os_dir" "$adt_os_tmp_dir" "$prj_config"

