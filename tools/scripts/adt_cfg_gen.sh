#!/bin/bash

config_src=$1
config_dts=$2

adt_config=$(cat $1/adapter_modules)

echo $adt_config

echo $adt_config | jq  '.adapter_chip','.adapter_core' | jq '.[]' > $2/adt_config_tmp



function adt_config_tmp_read()
{
	while read line || [[ -n $LINE ]]
	do
		if [[ "$line" == \"name\":* ]]; then
		
			name_tmp=${line#*:}
			name=${name_tmp%,*}
			name_kfg=$(echo $name | sed 's/\"//g')
			echo "config $name_kfg" 	>> $2/MKconfig
			echo "bool \"$name_kfg\"" 	>> $2/MKconfig
			echo "default y"  		>> $2/MKconfig
		fi
		
		if [[ "$line" == \"path\":* ]]; then
		
			path=${line#*:}
			path_kfg=$(echo $path | sed 's/\"//g')
		fi			
	done < $1
	
	#echo ""
	#echo "name:      	$name"
	#echo "path:    	$path"
	
}




###########################################################################
:> 					   $2/MKconfig
echo "menu \"adt_modules\"" 		>> $2/MKconfig

for file in $2/adt_config_tmp
do
	adt_config_tmp_read $file $2
done

echo "endmenu" 			>> $2/MKconfig
############################################################################

rm -rf $2/adt_config_tmp
