
# 简介

该demo是mqtts客户端功能。需要连接手机开启的热点或者路由器WiFi，频段在2.4G。热点名称或者路由wifi名称默认为：myssid，密码：mypassword。

1、支持STA模式可配置wifi名称、密码、信道等参数
2、可配置连接MQTT的Url、Port、username、password、clientID等参数
3、默认连接URL为SSL加密方式

# 环境搭建

参考主工程的readme，搭建开发环境后可以正常 demo 例程表示环境搭建完毕

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh <platform> mqtts_demo menuconfig ,不用修改直接退出，生成默认config （也可以自主修改连接参数）

   eg：./build.sh bl602 mqtts_demo menuconfig

3. 执行 ./build.sh <platform> mqtts_demo cn debug 开始编译

   eg：./build.sh bl602 mqtts_demo cn debug

# 测试

1. 烧录固件

2. 查看log，设备连接到路由获取到IP之后，会进行mqtt连接，然后mqtt进行subscribe、unsubscribe、publish等操作