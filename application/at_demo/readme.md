# 简介

该demo主要是安信可自主生态Combo-AT指令集at_demo.实现AT指令控制各功能实现

1、默认AT指令串口为：USB-TYPE-C线连接模组与电脑所使用的串口，可使用menuconfig->adt->AT_Config->IO 中修改。
2、AT串口波特率为115200 

3、如所需使用debug，查看模组log Debug串口可使用 menuconfig->adt->ALOG_Config->Choice uart 中修改。
4、Debug串口波特率默认为921600
BL602:
用一个串口工具与模组连接，默认连接方式:
模组	串口工具
GND		GND
IO4		RX
IO3		TX
3V3		VCC

BL618/616:
用一个串口工具与模组连接，默认连接方式:
模组	串口工具
GND		GND
IO16	RX
IO17	TX
3V3		VCC


# 环境搭建

参考主工程的readme，搭建开发环境后可以正常 demo 例程表示环境搭建完毕

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh <platform> at_demo menuconfig ,不用修改直接退出，生成默认config （也可以自主修改连接参数）

   eg：./build.sh bl602 at_demo menuconfig

3. 执行 ./build.sh <platform> at_demo cn debug 开始编译

   eg：./build.sh bl602 at_demo cn debug

# 测试

1. 烧录固件

2、手动复位模组，波特率：921600。通过串口调试助手查看log信息，查看AT指令是否正常

3、相关AT指令，可输入AT+HELP 获取

4、相关资源参考安信可官方网站：https://docs.ai-thinker.com/