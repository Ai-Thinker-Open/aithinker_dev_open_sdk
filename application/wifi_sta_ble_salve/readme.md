# wifi_sta_ble_salve



### demo功能概述

该demo实现wifi station模式与ble salve模式共存


### demo编译

进入sdk根目录

执行 ./build.sh platform wifi_sta_ble_salve menuconfig ,配置需要连接WiFi的ssid和password

eg：./build.sh bl602 wifi_sta_ble_salve menuconfig

执行 ./build.sh platform wifi_sta_ble_salve cn debug 开始编译

eg：./build.sh bl602 wifi_sta_ble_salve cn debug



### demo使用

 1. 打开串口调试助手查看日志，查看模组是否按照预期连接上目标ap；

 2. 通过ble调试助手（如nRF Connect）扫描和连接模组ble，蓝牙广播名为AXK。
 
