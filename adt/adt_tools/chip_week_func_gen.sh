#!/bin/bash

function week_func_auto_h() {
	for element in $(ls $2); do
		dir_or_file=$2"/"$element
		if [[ -d $dir_or_file ]]; then
			week_func_auto_h $1 $dir_or_file
		elif [[ $dir_or_file == *".h" ]]; then
			echo "#include \"${dir_or_file##*/}\"" >>$1/chip_week_func.h
		fi
	done
}
function week_func_auto_c() {
	for element in $(ls $2); do
		dir_or_file=$2"/"$element
		if [[ -d $dir_or_file ]]; then
			week_func_auto_c $1 $dir_or_file
		elif [[ $dir_or_file == *".h" ]]; then
			#echo "$dir_or_file"
			while read line; do
				if [[ "$line" == "CHIP_API"*";" ]]; then
					line_tmp1=${line#*CHIP_API }
					line_tmp2=${line_tmp1%;*}
					echo $line_tmp2" __attribute__((weak, alias(\"week_func_ndef\")));" >>$1/chip_week_func.c
				fi
			done <$dir_or_file
		fi
	done
}
function wsrc_call_auto_c() {
	for element in $(ls $2); do
		dir_or_file=$2"/"$element
		if [[ -d $dir_or_file ]]; then
			wsrc_call_auto_c $1 $dir_or_file
		elif [[ $dir_or_file == *".c" ]]; then
			tmp1=${dir_or_file##*/}
			tmp2=${tmp1%.*}"_wsrc_call"
			echo "" >>$1/chip_week_func.c
			echo "	extern void $tmp2(void);" >>$1/chip_week_func.c
			echo "	$tmp2();" >>$1/chip_week_func.c

		fi
	done
}
function components_include_gen(){
    for element in `ls $2`
    do  
        dir_or_file=$2"/"$element
        if [[ -d $dir_or_file ]]
        then 
            components_include_gen $1 $dir_or_file
        elif [[ $dir_or_file = *".h" ]]
        then
            # echo "$dir_or_file"
            echo "#include \"${dir_or_file##*/}\"" >> $1/components_include.h

        fi  
    done
}
function src_to_weak_h() {
	mkdir -p $1 && touch "$1/chip_week_func.h" && : >$1/chip_week_func.h

	echo "#ifndef __CHIP_WEEK_FUNC_H_" >>$1/chip_week_func.h
	echo "#define __CHIP_WEEK_FUNC_H_" >>$1/chip_week_func.h
	echo "" >>$1/chip_week_func.h

	if [ xx"$1" == xx ]; then
		exit 1
	fi
	if [ xx"$2" == xx ]; then
		echo -e "\c"
	else
		week_func_auto_h $1 $2
	fi
	if [ xx"$3" == xx ]; then
		echo -e "\c"
	else
		week_func_auto_h $1 $3
	fi
	if [ xx"$4" == xx ]; then
		echo -e "\c"
	else
		week_func_auto_h $1 $4
	fi
	if [ xx"$5" == xx ]; then
		echo -e "\c"
	else
		week_func_auto_h $1 $5
	fi

	echo "" >>$1/chip_week_func.h
	echo "typedef int32_t (*week_func_p)(void);" >>$1/chip_week_func.h

	echo "" >>$1/chip_week_func.h
	echo "#endif" >>$1/chip_week_func.h
}
function src_to_weak_c() {
	if [ xx"$1" == xx ]; then
		exit 1
	fi

	mkdir -p $1 && touch "$1/chip_week_func.h" && : >$1/chip_week_func.c

	echo "#include \"chip_week_func.h\"" >>$1/chip_week_func.c
	echo "" >>$1/chip_week_func.c
	echo "#ifndef AIIO_FUNC_NO_DEFINE" >>$1/chip_week_func.c
	echo "#define AIIO_FUNC_NO_DEFINE -2" >>$1/chip_week_func.c
	echo "#endif" >>$1/chip_week_func.c
	echo "" >>$1/chip_week_func.c
	echo "int32_t week_func_ndef(void){aiio_log_e(\"WEEK_FUNC_NO_DEFINE\"); return AIIO_FUNC_NO_DEFINE;}" >>$1/chip_week_func.c
	echo "" >>$1/chip_week_func.c

	if [ xx"$2" == xx ]; then
		echo -e "\c"
	else
		week_func_auto_c $1 $2
	fi
	if [ xx"$3" == xx ]; then
		echo -e "\c"
	else
		week_func_auto_c $1 $3
	fi
	if [ xx"$4" == xx ]; then
		echo -e "\c"
	else
		week_func_auto_c $1 $4
	fi
	if [ xx"$5" == xx ]; then
		echo -e "\c"
	else
		week_func_auto_c $1 $5
	fi

}
function wsrc_call_func_to_weak_c() {
	echo "" >>$1/chip_week_func.c
	echo "#if 1" >>$1/chip_week_func.c
	echo "void aiio_wsrc_call(void)" >>$1/chip_week_func.c
	echo "{" >>$1/chip_week_func.c

	wsrc_call_auto_c $1 $2

	echo "" >>$1/chip_week_func.c
	echo "	return;" >>$1/chip_week_func.c

	echo "}" >>$1/chip_week_func.c
	echo "#endif" >>$1/chip_week_func.c

}
function wsrc_call_func_to_ssrc_c(){
    for element in `ls $2`
    do  
        dir_or_file=$2"/"$element
        if [[ -d $dir_or_file ]]
        then 
            wsrc_call_func_to_ssrc_c $1 $dir_or_file
        elif [[ $dir_or_file = *".c" ]]
        then
            #echo "$dir_or_file"
            #echo "2:$2"
            mkdir -p $1 && cp  $dir_or_file $1
            
            tmp1=${dir_or_file##*/}
            tmp2=${tmp1%.*}"_wsrc_call"
            
            echo ""			>> $1/$tmp1
            echo "void $tmp2(void)"   	>> $1/$tmp1
            echo "{"   		>> $1/$tmp1
            echo ""			>> $1/$tmp1
            echo "}"   		>> $1/$tmp1
            echo ""			>> $1/$tmp1
            
        fi  
    done
}

function adapter_core_h_include_h()
{
	mkdir -p $1 && touch $1/components_include.h && :> $1/components_include.h

	echo "#ifndef COMPONENTS_INCLUDE_H" 		>> $1/components_include.h
	echo "#define COMPONENTS_INCLUDE_H" 		>> $1/components_include.h
	echo ""				    	>> $1/components_include.h

	components_include_gen 			$1 $2

	echo "" 					>> $1/components_include.h
	echo "#endif" 					>> $1/components_include.h

}


