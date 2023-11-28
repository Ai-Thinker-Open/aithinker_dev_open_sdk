# 简介

该demo主要是实现ble主机功能，需要配合ble从机demo测试

1、支持ble扫描、ble连接、ble服务发现、ble接收数据

# 环境搭建

参考主工程的readme，搭建开发环境后可以正常 demo 例程表示环境搭建完毕

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh platform ble_master_demo menuconfig ,不用修改直接退出，生成默认config

   eg：./build.sh bl602 ble_master_demo menuconfig

3. 执行 ./build.sh platform ble_master_demo cn debug 开始编译

   eg：./build.sh bl602 ble_master_demo cn debug

# 测试

1、烧录固件（同时另一块开发板烧录从机固件）

2、手动复位模组，模组将自动扫描并连接名称为"AXK"的ble从设备

3、连接成功后模组将收到并打印从设备发送的数据
