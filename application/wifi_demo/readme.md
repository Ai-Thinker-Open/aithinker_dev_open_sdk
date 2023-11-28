# 简介

该demo主要是测试 wifi 各项功能。连接WIFI名称默认为：myssid,密码：mypassword，频段在2.4G。 ap名称默认为：test_ap。


1、可配置sta连接wifi名称、密码、连接指定信道等参数
2、可配置ap开启wifi名称、密码、信道等参数
3、可配置wifi扫描参数


# 环境搭建

参考主工程的readme，搭建开发环境后可以正常 demo 例程表示环境搭建完毕

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh <platform> wifi_demo menuconfig ,不用修改直接退出，生成默认config （也可以自主修改连接参数）

   eg：./build.sh bl602 wifi_demo menuconfig

3. 执行 ./build.sh <platform> wifi_demo cn debug 开始编译

   eg：./build.sh bl602 wifi_demo cn debug

# 测试

1. 烧录固件

2、手动复位模组，波特率：921600。通过串口调试助手查看log信息

3、模组上电自动开启WIFI扫描，扫描完成后自动连接默认wifi,获取到IP后等待5秒，关闭sta模式，开启AP模式，开启成功后等待客户端连接热点；使用手机连接AP成功后，再断开此热点，AP模式关闭。

