# wifi_sta_ble_master



### demo功能概述

该demo实现wifi station模式与ble master模式共存


### demo编译

进入sdk根目录

执行 ./build.sh platform wifi_sta_ble_master menuconfig ,配置需要连接WiFi的ssid和password

eg：./build.sh bl602 wifi_sta_ble_master menuconfig

执行 ./build.sh platform wifi_sta_ble_master cn debug 开始编译

eg：./build.sh bl602 wifi_sta_ble_master cn debug



### demo使用

 1. 打开串口调试助手查看日志，查看模组是否按照预期连接上目标ap；

2. 需要使用两个模组，一个模组烧录本例程固件，另外一个模组烧录wifi_sta_ble_salve例程的固件，用串口查看两模组ble是否连接成功,如果连接成功主机每2秒会收到一包来自从机的消息。
 
