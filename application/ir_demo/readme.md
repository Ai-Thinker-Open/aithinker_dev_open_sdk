# 简介

这个demo是用来演示输出红外NEC码的demo，适用模组LN882H

# 环境搭建

参考主工程的readme，搭建开发环境后可以正常 demo 例程表示环境搭建完毕

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh <platform> ir_demo menuconfig ,不用修改直接退出，生成默认config

   eg：./build.sh bl602 ir_demo menuconfig

3. 执行 ./build.sh <platform> ir_demo cn debug 开始编译

   eg：./build.sh bl602 ir_demo cn debug

# 测试

1. 烧录固件

2. 模组PB5引脚在开机时会输出NEC红外编码，码值为0x17，~0x17，0x25，~0x25，可以通过逻辑分析仪抓取相关数据
