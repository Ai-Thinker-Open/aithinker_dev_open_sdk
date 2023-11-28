## 概述

LLSync SDK 提供了 LLSync 协议接入方案，SDK 内实现了 LLSync 协议用于和 APP(网关设备)进行通信，打通了 `BLE设备-APP(网关设备)-物联网开发平台` 的数据链路，支持开发者快速接入 BLE 设备到物联网开发平台。开发者接入 LLSync SDK需要做的工作有：

1. 添加 LLSync Service到 BLE 协议栈中
2. 通过脚本将数据模版转换为C代码，添加相应的数据处理

LLSync SDK 封装了协议实现细节和数据传输过程，让开发者可以聚焦在数据处理上，以达到快速开发的目的。


## 软件架构

LLSync SDK 结构框图:

![LLSync结构框图](https://main.qcloudimg.com/raw/9fabb2c222ae40d6a93641b745a327bd.png)

SDK 分三层设计，从上至下分别为应用层、LLSync核心层、HAL移植层。

* 应用层：LLSync SDK 生成了数据模版的模版文件，用户需要根据需求做具体实现。
* LLSync 核心组件：实现了 BLE 设备和App (网关设备)之间的通信协议，身份认证，数据解析等功能，用户一般无需改动即可使用。
* HAL 移植层：主要是适配 BLE 协议栈，用户需要进行移植和适配。

## 目录结构

```c
qcloud_iot_explorer_ble
  ├─config                            # SDK 配置文件
  ├─docs                              # 文档
  ├─inc                               # 头文件
  ├─scripts                           # 脚本
  │   ├─interpret_json_dt             # 数据模版转换
  └─src                               # LLSync源码
      ├─core                          # 核心代码
      ├─internal_inc                  # 内部头文件
      └─utils                         # 工具代码
```

## 移植指引

`LLSync`提供了三种功能：

* `BLE`通信功能，支持`腾讯连连小程序`通过`BLE`控制`BLE`设备。
* `BLE`配网能力，支持`腾讯连连小程序`通过`BLE`给`WIFI-BLE`设备配置网络。
* 双路通信能力，支持`腾讯连连小程序`通过`BLE`或`Wi-Fi`控制`WIFI-BLE`设备。

请参见 [移植指引](./docs/LLSync%20SDK%20接入指引.md)
