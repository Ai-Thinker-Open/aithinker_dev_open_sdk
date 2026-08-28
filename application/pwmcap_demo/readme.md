# 简介

这个demo是用来演示pwm捕获的demo

适用型号：LN882H

# 环境搭建

参考主工程的readme，搭建开发环境后可以正常 demo 例程表示环境搭建完毕

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh <platform> pwmcap_demo menuconfig ,不用修改直接退出，生成默认config

   eg：./build.sh ln882h pwmcap_demo menuconfig

3. 执行 ./build.sh <platform> pwmcap_demo cn debug 开始编译

   eg：./build.sh ln882h pwmcap_demo cn debug

# 测试

## 接线

PA7<———————————>PB5

## 现象

PB5引脚会输出0x17，~0x17，0x25，~0x25的NEC红外码;PA7引脚会对红外码进行捕获，并在log中输出捕获到的码值