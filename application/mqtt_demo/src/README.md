# 📡 AIIO Wi-Fi STA + MQTT 示例工程

本示例演示了如何在 AIIO 平台上使用 **Wi-Fi STA 模式连接路由器**，并在成功获取 IP 后，通过 **MQTT 协议**连接至公网 MQTT Broker（如 `mqtt.eclipseprojects.io`），实现发布/订阅功能。

## 🌐 项目功能概述

- 初始化 Wi-Fi，配置为 STA（Station）模式。
- 自动连接配置的 Wi-Fi 热点（SSID/PASSWORD）。
- 成功连接并获取 IP 后，建立 MQTT 连接。
- 通过 MQTT 实现订阅/取消订阅/发布消息。
- 支持 MQTT over TCP 和 SSL/TLS。
- 注册 Wi-Fi 和 MQTT 事件回调，追踪状态。

## 🧱 项目依赖

本项目基于 AIIO 平台，依赖以下模块：

- `aiio_wifi`：Wi-Fi 接入模块
- `aiio_mqtt`：MQTT 客户端模块
- `aiio_os`：操作系统线程与延时支持
- `aiio_log`：日志模块
- `EasyLogger`：日志打印示例
- 自定义头文件：`hello_h1.h`, `hello_h2.h`
- 网络证书支持（用于 MQTT SSL）

## ⚙️ 配置项说明

可通过 `CONFIG_...` 宏定义设置 Wi-Fi 与 MQTT 参数：

| 宏定义 | 说明 |
|--------|------|
| `CONFIG_AXK_WIFI_STA_SSID` | 要连接的 Wi-Fi 名称 |
| `CONFIG_AXK_WIFI_STA_PASS` | Wi-Fi 密码 |
| `CONFIG_AXK_WIFI_STA_CHANNEL` | 连接的通道（可选） |
| `CONFIG_AXK_WIFI_SSID_HIDDEN_ENABLE` | 是否连接隐藏 SSID |
| `CONFIG_AXK_WIFI_AP_SSID/PASS/...` | SoftAP 模式（本示例未使用） |
| `CONFIG_MAX_STA_CONN_LIMIT` | AP 模式最大连接数（默认无效） |

> MQTT 服务器地址通过宏定义 `BROKER_URI_TCP` 等提供，默认使用 TCP 接入 `mqtt://mqtt.eclipseprojects.io`。

## 🔐 MQTT SSL 证书

如果启用 SSL/TLS 连接（即 `mqtts://...`），需使用内置的 CA 根证书：

```c
const char mqtt_eclipseprojects_io[] = "-----BEGIN CERTIFICATE-----\r\n ... \r\n-----END CERTIFICATE-----\r\n";
