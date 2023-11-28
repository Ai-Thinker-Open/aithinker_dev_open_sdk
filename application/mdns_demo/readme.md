# 功能概述

这个demo是用来演示mdns广播的例程

# 目录结构

├── src
│   ├── components
│   │   ├── mdns_test	//mdns相关功能
│   │   │   ├── mdns_test.c
│   │   │   └── mdns_test.h
│   │   └── wifi_connect	//wifi连接代码
│   │       ├── wifi_connect.c
│   │       └── wifi_connect.h
│   └── main.c
└── vendor						//编译脚本

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh <platform> mdns_demo menuconfig

   eg：./build.sh bl618 mdns_demo menuconfig

   修改模组连接的wifi的SSID和密码

   ![](.\doc\source\001.jpg)

   ![](.\doc\source\002.jpg)

3. 执行 ./build.sh <platform> mdns_demo cn debug 开始编译

   eg：./build.sh bl618 mdns_demo cn debug

# 使用方式

模组下载测试固件

手机安装“Service Browser”

模组上电重启会自动连接编译时指定的wifi

手机也连接到同一个wifi路由器，打开“Service Browser”就可以看到模组的广播内容

![](.\doc\source\003.jpg)

![](.\doc\source\004.jpg)