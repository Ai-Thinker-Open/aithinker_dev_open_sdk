# 简介

该demo主要是wifi ap和sta 共存功能。连接WIFI名称默认为：myssid,密码：mypassword，频段在2.4G。 ap名称默认为：test_ap。

1、支持AP模式与STA模式共存
2、可配置wifi名称、密码、信道等参数

note: ln882h平台 不支持 ap与sta共存

# 环境搭建

参考主工程的readme，搭建开发环境后可以正常 demo 例程表示环境搭建完毕

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh <platform> wifi_apsta_demo menuconfig ,不用修改直接退出，生成默认config （也可以自主修改连接参数）

   eg：./build.sh bl602 wifi_apsta_demo menuconfig

3. 执行 ./build.sh <platform> wifi_apsta_demo cn debug 开始编译

   eg：./build.sh bl602 wifi_apsta_demo cn debug

# 测试

1. 烧录固件

2、手动复位模组，波特率：921600。通过串口调试助手查看log信息，注意：如果使用手机开启热点的方式，必须是一个手机开热点，另一个手机去连接模组的ap。
