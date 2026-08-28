# 简介

该demo主要是wifi 开启sta功能。 连接WIFI名称默认为：myssid,密码：mypassword，频段在2.4G。 （可通过menuconfig修改相关参数）

1、支持配置STA模式连接wifi热点
2、可配置连接wifi名称、密码、连接指定信道等参数

# 适用平台

BL602、BL616、BL618、RTL8720DX、RTL8711DX

# 不支持平台

BS21

# 硬件介绍

BL602、BL616、BL618、RTL8720DX、RTL8711DX开发板

# 编译方法

1. 进入sdk根目录

2. 执行 ./build.sh <platform> wifi_sta_demo menuconfig ,不用修改直接退出，生成默认config （也可以自主修改连接参数）

   eg：./build.sh bl602 wifi_sta_demo menuconfig

3. 执行 ./build.sh <platform> wifi_sta_demo cn debug 开始编译

   eg：./build.sh bl602 wifi_sta_demo cn debug

# 用法

1. 烧录固件

2、手动复位模组，波特率：921600。通过串口调试助手查看log信息，查看是否能够连接路由获取IP成功。