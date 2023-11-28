# 功能概述

这个demo是用来演示使用NFC标签配网的demo，手机需要支持NFC

# 目录结构

├── doc								//帮助文档
├── readme.md
├── src								//源码
│   ├── components		//第三方组件
│   │   ├── cJSON
│   │   ├── local_control
│   │   ├── utils_aes
│   │   └── wifi_config
│   └── main.c					//demo
└── vendor						//编译脚本

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh <platform> nfcconfig menuconfig ,不用修改直接退出，生成默认config

   eg：./build.sh bl618 nfcconfig menuconfig

3. 执行 ./build.sh <platform> nfcconfig cn debug 开始编译

   eg：./build.sh bl618 nfcconfig cn debug

# 使用方式

## 材料准备

测试固件

串口工具

Android手机一部(需要支持NFC功能)

NFC标签一张

对应模组一个

NFC标签助手（手机市场搜索安装）

![](.\doc\source\001.png)

安信可配网助手(doc\003-Assistant-断开AP等待系统自动回连WiFi-20230315.apk)

![](.\doc\source\009.png)

## 测试环境搭建

串口配置 921600,8N1，连接模组串口

模组烧录固件

手机打开“NFC标签助手”

选择“写入”->“URL”

![](.\doc\source\002.png)

第一行选择“https://”

第二行填入数据：

 [www.ai-thinker.com/app/assistant?mac=b4:e8:42:c7:b8:42&product=1](https://www.ai-thinker.com/app/assistant?mac=98:d:51:25:92:90&product=1)

然后点击“添加到复合记录”（注意不能直接点确定，否则数据会互相覆盖）

注意中间的mac需要根据对应模组选择（MAC地址可以在模组启动信息中找到），否则会无法配网

![](.\doc\source\003.png)

如果提示格式错误可以选择“其它”->“存储区NDEF格式化”，格式化完成后重新写入

![](.\doc\source\004.png)

第一条数据写入完成后再次选择“写入”->“应用”

![](.\doc\source\005.png)

填入长沙博安通开发的配网APP名称“com.aithinker.assistant”->“添加到复合记录”

![](.\doc\source\006.png)

选择“写入”->“复合记录”

![](.\doc\source\007.png)

点击“写入”

![](.\doc\source\008.png)

此时将NFC标签靠近手机感应区就会开始写入标签

模组上电并复位（模组上电会自动进入配网模式）

手机靠近NFC标签(不用开启APP，在桌面就可以)就会拉起配网，点击“立即配网”

开始配网前确保如下条件以满足

需要手机开启NFC功能

如果设备之前已经在列表中了需要先删除后重新开始配网

手机连接需要配网的wifi（同时需要开启自动连接功能，这个一般默认是开的）

![](.\doc\source\010.png)

输入需要连接的网络的SSID和密码（手机也需要连接到当前网络，注意仅支持2.4G频段）

![](.\doc\source\011.png)

此时显示正在搜索设备

![](.\doc\source\012.png)

找到设备之后会提示是否连接到设备，此时点击“连接”

![](.\doc\source\013.png)

配网过程中会显示“正在配网中”

![](.\doc\source\014.png)

配网成功后结果如下

![](.\doc\source\015.png)

点击控制设备可以进入控制页面（当前模组没有做自动重连功能，复位后需要重新配网）

![](.\doc\source\016.png)

配网过程中模组会打印收到的SSID和密码

![](.\doc\source\017.png)