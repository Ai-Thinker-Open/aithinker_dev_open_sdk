# 简介

该demo主要是实现蓝牙 blufi配网功能。 需要下载EspBlufi软件或使用有blufi功能的微信小程序

1、支持BLUFI配网方式，模组端开启蓝牙广播，可被APP端扫描到并连接，APP端通过蓝牙发送wifi ssid 与password数据进行连接对应热点

# 环境搭建

参考主工程的readme，搭建开发环境后可以正常 demo 例程表示环境搭建完毕

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh <platform> blufi_demo menuconfig ,不用修改直接退出，生成默认config

   eg：./build.sh bl602 blufi_demo menuconfig

3. 执行 ./build.sh <platform> blufi_demo cn debug 开始编译

   eg：./build.sh bl602 blufi_demo cn debug

# 测试

1. 烧录固件

2、手动复位模组，然后打开EspBlufi，连接设备，名称：BLUFI_DEVICE。

3、然后通过EspBlufi去连接蓝牙，发送wifi ssid 与password数据进行配网。

4、查看是否配网连接路由获取IP成功。