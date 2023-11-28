
# 简介

这个demo是用来演示nvs_flash 读、写、擦除的demo

1、支持通过key值名读、写、擦除指定长度数据

# 环境搭建

参考主工程的readme，搭建开发环境后可以正常 demo 例程表示环境搭建完毕

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh <platform> nvs_flash_demo menuconfig ,不用修改直接退出，生成默认config

   eg：./build.sh bl602 nvs_flash_demo menuconfig

3. 执行 ./build.sh <platform> nvs_flash_demo cn debug 开始编译

   eg：./build.sh bl602 nvs_flash_demo cn debug

# 测试

1. 烧录固件

2. 可以查看log，读取出来的数据与存储的数据对比成功，则读写成功