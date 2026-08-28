# 简介

该demo是rd01模组连接mqtt服务器并上报检测结果

1、log tx 为IO4   波特率921600
2、数据串口波特率为256000
    IO16	RX
    IO17	TX

只适用BL602平台
# 编译

1. 进入sdk根目录

2. ./build.sh bl602 rd01_demo menuconfig

3. ./build.sh bl602 rd01_demo cn debug

# 测试

1. 烧录固件

2、手动复位模组，波特率：921600。通过串口调试助手查看mqtt的log信息

注意：需要修改MQTT连接参数、Topic及上报消息
