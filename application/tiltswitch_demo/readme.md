# 简介

这个demo是用来演示使用tilt传感器的demo

# 环境搭建

参考主工程的readme，搭建开发环境后可以正常 demo 例程表示环境搭建完毕

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh <platform> tiltswitch_demo menuconfig ,不用修改直接退出，生成默认config

   eg：./build.sh bl602 tiltswitch_demo menuconfig

3. 执行 ./build.sh <platform> tiltswitch_demo cn debug 开始编译

   eg：./build.sh bl602 tiltswitch_demo cn debug

# 测试

1. 将传感器和模组连接

2. 烧录固件

3. 查看log输出的开关量信息

## log
开关打开打印:
```
tiltswitch on!
```

开关关闭打印:
```
tiltswitch off!
```
