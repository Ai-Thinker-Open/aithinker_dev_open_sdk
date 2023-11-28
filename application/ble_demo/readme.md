# 简介

该demo主要是实现ble从机功能。

支持ble广播、ble连接、ble notify

# 环境搭建

参考主工程的readme，搭建开发环境后可以正常 demo 例程表示环境搭建完毕

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh platform ble_demo menuconfig ,不用修改直接退出，生成默认config

   eg：./build.sh bl602 ble_demo menuconfig

3. 执行 ./build.sh platform ble_demo cn debug 开始编译

   eg：./build.sh bl602 ble_demo cn debug

# 测试

1、烧录固件

2、手动复位模组，然后打开蓝牙调试工具，连接设备，名称：AXK。

3、连接成功后调试工具将周期性收到内容为"test"的notify数据。
