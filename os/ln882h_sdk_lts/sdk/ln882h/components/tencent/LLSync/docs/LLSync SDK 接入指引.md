## 概述

本文档介绍如何将 LLSync SDK 移植到目标硬件平台。LLSync SDK 采用模块化设计，分离 LLSync 核心组件与硬件抽象层，在进行跨平台移植时，一般只需要用户对硬件抽象层进行适配即可。

## SDK 获取

SDK 使用 Github 托管，可访问 Github 下载最新版本设备端[LLSync SDK](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded)。

## 接入指引

`LLSync SDK`现已支持标准蓝牙功能、辅助配网功能和双路通信功能。

1. `BLE`通信功能，支持`腾讯连连小程序`通过`BLE`控制`BLE`设备。
2. `BLE`配网能力，支持`腾讯连连小程序`通过`BLE`给`Wi-Fi+BLE`设备配置网络。
3. 双路通信能力，支持`腾讯连连小程序`通过`BLE`或`Wi-Fi`控制`Wi-Fi+BLE`设备。

您可以根据需求选择使用`LLSync SDK`的不同能力，请参见[标准蓝牙功能详细接入指引](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/docs/LLSync%20SDK%E6%A0%87%E5%87%86%E8%93%9D%E7%89%99%E5%8A%9F%E8%83%BD%E6%8E%A5%E5%85%A5%E6%8C%87%E5%BC%95.md)、[辅助配网功能详细接入指引](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/docs/LLSync%20SDK%E8%BE%85%E5%8A%A9%E9%85%8D%E7%BD%91%E5%8A%9F%E8%83%BD%E6%8E%A5%E5%85%A5%E6%8C%87%E5%BC%95.md)和[双路通信功能详细接入指引](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/docs/LLSync%20SDK%E5%8F%8C%E8%B7%AF%E9%80%9A%E4%BF%A1%E5%8A%9F%E8%83%BD%E6%8E%A5%E5%85%A5%E6%8C%87%E5%BC%95.md)。

我们也提供了对应的[示例程序](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded-demo)供您参考。

## LLSync 协议

请参见 [LLSync协议说明](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/docs/LLSync%E8%93%9D%E7%89%99%E8%AE%BE%E5%A4%87%E6%8E%A5%E5%85%A5%E5%8D%8F%E8%AE%AE.pdf)。

## SDK使用参考

请参见 [LLSync SDK使用参考](https://cloud.tencent.com/document/product/1081/48399)。

