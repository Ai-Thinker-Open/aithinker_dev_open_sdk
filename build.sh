#!/bin/bash

cd "$(dirname "$0")"

###########################################################################################################
#
# aithinker Development Team
#
# shell command:
#
#	./build.sh help
#	./build.sh docs
#	./build.sh clean
#	./build.sh reset
#	./build.sh update
#	./build.sh version
#	./build.sh environment
#	./build.sh $chip $application clean
#	./build.sh $chip $application menuconfig
#	./build.sh $chip $application $country_code $debugging_level(debug/debug_ram/release)
#
###########################################################################################################

###########################################################################################################
# default config					  		                                  							  #
###########################################################################################################
default_chip="bl602"
default_application="demo"
default_country_code="cn"
default_debugging_level="debug"

root_dir=$PWD
start_time=$(date +%s)
export LC_ALL=en_US.utf-8 && export LANG=en_US.utf-8

###########################################################################################################
# os check && args check					  		                          							  #
###########################################################################################################
if [ "$(uname)" = "Linux" ]; then
    CUR_OS="Linux"
else
    echo -e "\033[31merror: unkonw os\033[0m"
    exit 1
fi

if [[ $# != 0 ]] && [[ $# != 1 ]] && [[ $# != 3 ]] && [[ $# != 4 ]]; then
    echo -e "\033[31merror: arguments error\033[0m"
    exit 1
fi

###########################################################################################################
# command: ./build.sh help					  		                          							  #
###########################################################################################################
if [[ $# = 1 ]] && [[ "$1" == "help" ]]; then

    echo -e ""
    echo -e "shell command:\n"

    echo -e "\t./build.sh help"
    echo -e "\t./build.sh docs"
    echo -e "\t./build.sh clean"
    echo -e "\t./build.sh reset"
    echo -e "\t./build.sh update"
    echo -e "\t./build.sh version"
    echo -e "\t./build.sh environment"
    echo -e "\t./build.sh docker_install"
    echo -e "\t./build.sh docker_image_pull"
    echo -e "\t./build.sh docker_container_exit"
    echo -e "\t./build.sh docker_container_entry"
    echo -e "\t./build.sh \$chip \$application clean"
    echo -e "\t./build.sh \$chip \$application menuconfig"
    echo -e "\t./build.sh \$chip \$application \$country_code \$debugging_level(debug/debug_ram/release)"

    echo -e ""
    exit 0
fi

###########################################################################################################
# command: ./build.sh docs  					  		                          						  #
###########################################################################################################
if [[ $# == 1 ]] && [[ "$1" == "docs" ]]; then

    tools/doxygen/doxygen.sh
    exit 0
fi

###########################################################################################################
# command: ./build.sh docker_install  					  		                   						  #
###########################################################################################################
if [[ $# == 1 ]] && [[ "$1" == "docker_install" ]]; then

    sudo /bin/bash $root_dir/tools/ubuntu_docker_install.sh && /bin/bash $root_dir/tools/docker_env.sh
    exit 0
fi

###########################################################################################################
# command: ./build.sh docker_image_pull 					  		           							  #
###########################################################################################################
if [[ $# == 1 ]] && [[ "$1" == "docker_image_pull" ]]; then

    docker pull zifengchenll/ubuntu:aithinker_middleware
    exit 0
fi

###########################################################################################################
# command: ./build.sh docker_image_pull 					  		           							  #
###########################################################################################################
if [[ $# == 1 ]] && [[ "$1" == "docker_container_exit" ]]; then

    exit
    exit 0
fi

###########################################################################################################
# command: ./build.sh docker_image_pull 					  		           							  #
###########################################################################################################
if [[ $# == 1 ]] && [[ "$1" == "docker_container_entry" ]]; then

    docker run -it --user $(id -u ${USER}):$(id -g ${USER}) --hostname docker -v /etc/passwd:/etc/passwd -v /etc/group:/etc/group -v $root_dir:/home/docker/aithinker_dev_sdk zifengchenll/ubuntu:aithinker_middleware /bin/bash
    exit 0
fi

###########################################################################################################
# command: ./build.sh clean  					  		                          						  #
###########################################################################################################
if [[ $# == 1 ]] && [[ "$1" == "clean" ]]; then

    for env_file in $root_dir/os/*_sdk_lts; do

        if [[ -d "$env_file" ]] && [[ -f $env_file/env.sh ]]; then
            cd $env_file && source env.sh
        fi
    done

    $root_dir/tools/scripts/clean.sh $root_dir/out/* && ls $root_dir/build/*_make/os_clean.sh | bash
    exit 0
fi

###########################################################################################################
# command: ./build.sh reset	  					  		                  								  #
###########################################################################################################
if [[ $# = 1 ]] && [[ "$1" == "reset" ]]; then

    rm -rf adt application build docs license.txt os out pre_commit.sh readme.md test thirds tmp tools

    git reset --hard HEAD && cd $root_dir && ./build.sh update

    echo -e ""
    exit 0
fi

###########################################################################################################
# command: ./build.sh update	  					  		                  							  #
###########################################################################################################
if [[ $# = 1 ]] && [[ "$1" == "update" ]]; then

    #	echo -e ""
    #	echo -e "\033[31mbuild.sh update command will be pulled to update and overwrite your local repository\033[0m"
    #	echo -e ""
    #	echo -e "\033[31mreset repository:\033[0m"
    #	echo -e "\033[31m\t./adt/adt_os\033[0m"
    #	echo -e "\033[31m\t./os/xxx_sdk_lts\033[0m"
    #	echo -e "\033[31m\t./application/xxx_demo\033[0m"
    #	echo -e ""
    #	echo -e "\033[31mAre You Sure Running? [Y/N] \033[0m\c"
    #	read -r -e input
    #
    #	case $input in
    #    		[yY][eE][sS]|[yY])
    #		#echo "Yes"
    #		;;
    #
    #    		[nN][oO]|[nN])
    #		#echo "No"
    #		exit 1
    #       	;;
    #
    #   		*)
    #		echo "Invalid input..."
    #		exit 1
    #		;;
    #	esac

    git config --global credential.helper store
    $root_dir/tools/scripts/update.sh $root_dir
    exit 0
fi

###########################################################################################################
# command: ./build.sh version		  					  		          								  #
###########################################################################################################
if [[ $# = 1 ]] && [[ "$1" == "version" ]]; then

    echo -e "bulish.sh tool_version: v1.0.2\n"
    exit 0
fi

###########################################################################################################
# command: ./build.sh environment		  					  		 	 								  #
###########################################################################################################
if [[ $# = 1 ]] && [[ "$1" == "environment" ]]; then

    sudo apt-get install git wget flex \
        bison gperf python3 python3-venv python3-setuptools \
        cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 jq

    pip install fpvgcc
    exit 0
fi

###########################################################################################################
# command: ./build.sh $chip $application clean 		  					  								  #
###########################################################################################################
if [[ $# == 3 ]] && [[ "$3" == "clean" ]]; then
    if [ xx"$1" == xx ]; then
        chip=$default_chip
    else
        chip=$1
    fi

    if [ xx"$2" == xx ]; then
        application=$default_application
    else
        application=$2
    fi

    ## sdk
    rtos_dir=$root_dir/os/$chip"_sdk_lts"

    ## application demo
    application_dir=$root_dir/application/$application

    ## sdk_make
    make_dir=$root_dir/build/$chip"_make"

    ## chip model(tmp)
    application_tmp_typ_dir=$root_dir/tmp/src/$chip

    if [[ -d "$rtos_dir" ]] && [[ -d "$make_dir" ]]; then
        echo ""
    else
        echo ""
        echo -e "\033[31merror: no adaptation the chip about $chip\033[0m"
        echo ""
        exit
    fi

    if [ -d "$application_dir" ]; then
        echo ""
    else
        echo ""
        echo -e "\033[31merror: no adaptation the application about $application\033[0m"
        echo ""
        exit
    fi

    cd $rtos_dir && source env.sh

    $root_dir/tools/scripts/clean.sh $root_dir/out/$chip/$application && $make_dir/os_clean.sh

    exit 0
fi

###########################################################################################################
# command: ./build.sh $chip $application defconfig 		  					  							  #
###########################################################################################################
if [[ $# == 3 ]] && [[ "$3" == "defconfig" ]]; then

    if [ xx"$1" == xx ]; then
        chip=$default_chip
    else
        chip=$1
    fi

    if [ xx"$2" == xx ]; then
        application=$default_application
    else
        application=$2
    fi

    ## adt
    adt_dir=$root_dir/adt

    ## sdk
    rtos_dir=$root_dir/os/$chip"_sdk_lts"

    ## application demo
    application_dir=$root_dir/application/$application

    ## sdk_make
    make_dir=$root_dir/build/$chip"_make"

    ## chip model(tmp)
    application_tmp_typ_dir=$root_dir/tmp/src/$chip

    ## application demo(tmp)
    application_tmp_app_dir=$root_dir/tmp/src/$chip/$application

    if [[ -d "$rtos_dir" ]] && [[ -d "$make_dir" ]]; then
        echo ""
    else
        echo ""
        echo -e "\033[31merror: no adaptation the chip about $chip\033[0m"
        echo ""
        exit
    fi

    if [ -d "$application_dir" ]; then
        echo ""
    else
        echo ""
        echo -e "\033[31merror: no adaptation the application about $application\033[0m"
        echo ""
        exit
    fi

    cd $root_dir

    mkdir -p $root_dir/tmp/kconfig_h/$chip/$application && cp -a $application_dir/vendor/$chip/app_config/aiio_autoconf.h $root_dir/tmp/kconfig_h/$chip/$application

    cd $root_dir

    exit 0
fi
###########################################################################################################
# command: ./build.sh $chip $application menuconfig 		  					  						  #
###########################################################################################################
if [[ $# == 3 ]] && [[ "$3" == "menuconfig" ]]; then

    if [ xx"$1" == xx ]; then
        chip=$default_chip
    else
        chip=$1
    fi

    if [ xx"$2" == xx ]; then
        application=$default_application
    else
        application=$2
    fi

    ## adt
    adt_dir=$root_dir/adt

    ## sdk
    rtos_dir=$root_dir/os/$chip"_sdk_lts"

    ## application demo
    application_dir=$root_dir/application/$application

    ## sdk_make
    make_dir=$root_dir/build/$chip"_make"

    ## chip model(tmp)
    application_tmp_typ_dir=$root_dir/tmp/src/$chip

    ## application demo(tmp)
    application_tmp_app_dir=$root_dir/tmp/src/$chip/$application

    if [[ -d "$rtos_dir" ]] && [[ -d "$make_dir" ]]; then
        echo ""
    else
        echo ""
        echo -e "\033[31merror: no adaptation the chip about $chip\033[0m"
        echo ""
        exit
    fi

    if [ -d "$application_dir" ]; then
        echo ""
    else
        echo ""
        echo -e "\033[31merror: no adaptation the application about $application\033[0m"
        echo ""
        exit
    fi

    cd $root_dir

    ## clean menuconfig
    mkdir -p $root_dir/tmp/kconfig/adt_kconfig && touch $root_dir/tmp/kconfig/adt_kconfig/Kconfig && >$root_dir/tmp/kconfig/adt_kconfig/Kconfig
    #>$root_dir/tmp/kconfig/os_kconfig/Kconfig
    #>$root_dir/tmp/kconfig/os_kconfig/global_osKconfig
    mkdir -p $root_dir/tmp/kconfig/application_kconfig && touch $root_dir/tmp/kconfig/application_kconfig/Kconfig && >$root_dir/tmp/kconfig/application_kconfig/Kconfig
    mkdir -p $root_dir/tmp/kconfig/common_kconfig && touch $root_dir/tmp/kconfig/common_kconfig/Kconfig && >$root_dir/tmp/kconfig/common_kconfig/Kconfig

    #rm $root_dir/tmp/kconfig_h/autoconf.h -rf

    if [ -d $root_dir/tmp/kconfig_h/$chip/$application ]; then
        rm $root_dir/tmp/kconfig_h/$chip/$application/aiio_autoconf.h -rf
    else
        mkdir -p $root_dir/tmp/kconfig_h/$chip/$application

    fi

    rm $root_dir/tools/platform_config/kconfig_flinux/.config -rf
    rm $root_dir/tools/platform_config/kconfig_flinux/include/ -rf

    #cd $root_dir/tools/platform_config && ./kconfig_gen.sh $application_dir/config/kconfig/"$chip"_config $root_dir $chip

    ## cp Kconfig to tmp
    cp -a $root_dir/adt/adt_kconfig/Kconfig $root_dir/tmp/kconfig/adt_kconfig/
    cp -a $root_dir/application/$application/vendor/$chip/app_config/Kconfig $root_dir/tmp/kconfig/application_kconfig/
    cp -a $root_dir/application/common_kconfig/Kconfig $root_dir/tmp/kconfig/common_kconfig/

    $root_dir/tools/scripts/adt_cfg_gen.sh $root_dir/adt/adt_os $root_dir/tmp/kconfig/adt_kconfig
    #cp -a $root_dir/os/os_kconfig/$chip"_sdk_lts_kconfig"/Kconfig $root_dir/tmp/kconfig/os_kconfig/
    #cp -a $root_dir/os/os_kconfig/global_osKconfig $root_dir/tmp/kconfig/os_kconfig/
    #cp -a $application_dir/config/kconfig/$chip"_config"/Kconfig $root_dir/tmp/kconfig/application_kconfig/

    ## cp last config
    #if [ -f $application_dir/config/kconfig/"$chip"_config/.config ]; then
    #	cp -a $application_dir/config/kconfig/"$chip"_config/.config $root_dir/tools/platform_config/kconfig_flinux/
    #fi

    if [ -f $application_dir/vendor/$chip/app_config/.config ]; then
        cp -a $application_dir/vendor/$chip/app_config/.config $root_dir/tools/platform_config/kconfig_flinux/
    fi

    cd $root_dir/tools/platform_config/kconfig_flinux

    ## menuconfig
    make menuconfig

    ## update autoconf.h
    make >/dev/null 2>&1

    ## cp autoconf.h to tmp dir
    cp -a ./include/generated/autoconf.h $root_dir/tmp/kconfig_h/$chip/$application/aiio_autoconf.h
    cp -a ./include/generated/autoconf.h $application_dir/vendor/$chip/app_config/aiio_autoconf.h

    ## cp last config
    cp -a .config $application_dir/vendor/$chip/app_config/

    cd $root_dir

    exit 0
fi

function build_sdk() {
    echo ""

    if [ -d $2 ]; then
        rm -rf $2
    fi

    cd $7 && ./os_make.sh $1 $2 $3 $4 $5 $6 $7 $8 $9 $10

    cd $5
}

function build_end() {

    end_time=$(date +%s)
    cost_time=$(($end_time - $start_time))

    echo ""
    echo "aithinker_dev_sdk build time is $(($cost_time / 60))min $(($cost_time % 60))s"
}

###########################################################################################################
# command: ./build.sh $chip $application $country_code $debugging_level(debug/release) 		  			  #
###########################################################################################################
if [ $# = 4 ]; then

    if [ xx"$1" == xx ]; then
        chip=$default_chip
    else
        chip=$1
    fi

    if [ xx"$2" == xx ]; then
        application=$default_application
    else
        application=$2
    fi

    if [ xx"$3" == xx ]; then
        country_code=$default_country_code
    else
        country_code=$3
    fi

    if [ xx"$4" == xx ]; then
        debugging_level=$default_debugging_level
    else
        debugging_level=$4
    fi

    ## adt
    adt_dir=$root_dir/adt

    ## sdk
    rtos_dir=$root_dir/os/$chip"_sdk_lts"

    ## application demo
    application_dir=$root_dir/application/$application

    ## sdk_make
    make_dir=$root_dir/build/$chip"_make"

    ## chip model(tmp)
    application_tmp_typ_dir=$root_dir/tmp/src/$chip

    ## application demo(tmp)
    application_tmp_app_dir=$root_dir/tmp/src/$chip/$application

    if [[ $debugging_level != "debug" ]] && [[ $debugging_level != "debug_tmp" ]] && [[ $debugging_level != "debug_ram" ]] && [[ $debugging_level != "release" ]]; then

        echo ""
        echo -e "\033[31merror: arguments error\033[0m"
        echo ""
        exit
    fi

    if [[ $country_code != "en" ]] && [[ $country_code != "cn" ]]; then

        echo ""
        echo -e "\033[31merror: arguments error\033[0m"
        echo ""
        exit
    fi

    if [[ -d "$rtos_dir" ]] && [[ -d "$make_dir" ]]; then
        cd $rtos_dir && source env.sh
        cd $root_dir
    else
        echo ""
        echo -e "\033[31merror: no adaptation the chip about $chip\033[0m"
        echo ""
        exit
    fi

    if [ -d "$application_dir" ]; then
        echo ""
    else
        echo ""
        echo -e "\033[31merror: no adaptation the application about $application\033[0m"
        echo ""
        exit
    fi

    echo "----------------------------------------------------------------------------------------------------"
    echo "PATH: =$PATH"
    echo "PCOS: ${CUR_OS}"
    echo "INFO: chip=$chip application=$application country_code=$country_code debugging_level=$debugging_level"
    echo "----------------------------------------------------------------------------------------------------"

    $root_dir/tools/scripts/tmp_cp.sh "$adt_dir/adt_os" "$root_dir/tmp/adt_os_src" "$root_dir/application/$application/vendor/$chip/app_config/.config"

    build_sdk $application_dir $application_tmp_app_dir $rtos_dir $application_tmp_typ_dir $root_dir $application $make_dir $adt_dir $chip $debugging_level

    if [[ $debugging_level == "debug" ]]; then

        rm -rf $root_dir/tmp/adt_os_src

        cd $make_dir && ./tmp_clean.sh $application_tmp_app_dir $rtos_dir
        cd $root_dir

    fi

    if [[ $debugging_level == "debug_tmp" ]]; then
        echo ""
    fi

    if [[ $debugging_level == "debug_ram" ]]; then

        cd $make_dir && ./tmp_clean.sh $application_tmp_app_dir $rtos_dir
        cd $root_dir

        if [ -f "$root_dir/out/$chip/$application/map/dev_project.map" ]; then

            fpvgcc $root_dir/out/$chip/$application/map/dev_project.map --sar
            fpvgcc $root_dir/out/$chip/$application/map/dev_project.map --ssec
        fi
    fi

    echo ""
    echo "build output:"
    echo ""
    ls -ogh $root_dir/out/$chip/$application/bin/*
    ls -ogh $root_dir/out/$chip/$application/elf/*
    ls -ogh $root_dir/out/$chip/$application/map/*

    build_end

    exit 0
fi

echo -e "\033[31merror: arguments error\033[0m"
exit 1
