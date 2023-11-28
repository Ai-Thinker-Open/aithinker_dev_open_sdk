# wifi_ap_ble_salve



### demo功能概述

该demo实现wifi ap模式与ble salve模式共存


### demo编译

进入sdk根目录

执行 ./build.sh platform wifi_ap_ble_salve menuconfig ,配置参数，不修改则生成默认config

eg：./build.sh bl602 wifi_ap_ble_salve menuconfig

执行 ./build.sh platform wifi_ap_ble_salve cn debug 开始编译

eg：./build.sh bl602 wifi_ap_ble_salve cn debug



### demo使用

1. 通过手机查看模组热点是否开启成功，ap名默认为test_ap，密码是12345678；

2. 通过ble调试助手（如nRF Connect）扫描和连接模组ble，蓝牙广播名为AXK。







