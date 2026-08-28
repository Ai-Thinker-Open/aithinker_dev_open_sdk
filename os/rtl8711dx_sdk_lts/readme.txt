编译环境部署
    sudo apt-get install gcc make libssl-dev binutils python3
    //sudo chown xxx:xxx /opt
    sudo chmod 777 /opt
    mkdir /opt/rtk-toolchain
    cd /opt/rtk-toolchain
    tar -xvf xxx/rtk-toolchain/asdk-10.3.1-linux-newlib-build-3682-x86_64.tar.bz2 /opt/rtk-toolchain
    sudo dpkg-reconfigure dash		//选择No

编译
    cd sdk/ameba-rtos/amebadplus_gcc_project
    make all
下载
    配置使用
        ImageTool for Ameba D Plus\DeviceProfiles\RTL8721F_FreeRTOS_NOR.rdev
    镜像使用
        ameba-rtos/amebadplus_gcc_project/km4_boot_all.bin
        ameba-rtos/amebadplus_gcc_project/km0_km4_app.bin

