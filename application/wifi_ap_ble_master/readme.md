# wifi_ap_ble_master



### demo功能概述

该demo实现wifi ap模式与ble master模式共存


### demo编译

进入sdk根目录

执行 ./build.sh platform wifi_ap_ble_master menuconfig ,配置参数，不修改则生成默认config

eg：./build.sh bl602 wifi_ap_ble_master menuconfig

执行 ./build.sh platform wifi_ap_ble_master cn debug 开始编译

eg：./build.sh bl602 wifi_ap_ble_master cn debug



### demo使用

1. 通过手机查看模组热点是否开启成功，ap名默认为test_ap，密码是12345678；

2. 需要使用两个模组，一个模组烧录本例程固件，另外一个模组烧录wifi_ap_ble_salve例程的固件，用串口查看两模组ble是否连接成功,如果连接成功主机每2秒会收到一包来自从机的消息。