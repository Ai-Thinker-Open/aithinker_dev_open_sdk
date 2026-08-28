# 📶 AIoT Wi-Fi STA + MQTT(MQTTS) 示例项目

本项目基于 AIIO 平台，演示如何使用 Wi-Fi 连接到指定路由器，并通过 MQTTS 协议与 MQTT 服务器进行通信，实现物联网数据的安全传输和消息收发功能。

---

## 🚀 功能概述

- 初始化 Wi-Fi 模块，配置为 **STA 模式** 并连接指定的热点。
- 成功连接后自动启动 MQTT 客户端，并连接至 `mqtt.eclipseprojects.io` 的 **MQTTS 安全服务**。
- 支持消息的发布、订阅和取消订阅。
- 支持 MQTT 事件回调，包括连接、接收数据、断开连接、错误等。
- 可根据配置轻松切换为 TCP / WS / WSS 等 MQTT 协议。
- 内嵌 TLS 证书，保障通信安全。

---

## 📁 项目结构

| 文件名 | 说明 |
|--------|------|
| `aiio_main.c` | 主入口，包含 Wi-Fi 和 MQTT 初始化逻辑 |
| `hello_h1.h`, `hello_h2.h` | 自定义头文件 |
| `aiio_adapter_include.h` | AIIO 平台接口适配封装 |
| `mqtt_eclipseprojects_io` | 公网 MQTT TLS 根证书 |
| `aiio_log_*` | EasyLogger 日志支持 |

---

## ⚙️ 主要配置项

请根据实际情况设置以下配置宏（通常在配置文件或构建系统中定义）：

| 宏名称 | 描述 |
|--------|------|
| `CONFIG_AXK_WIFI_STA_SSID` | 要连接的 Wi-Fi 名称 |
| `CONFIG_AXK_WIFI_STA_PASS` | Wi-Fi 密码 |
| `CONFIG_AXK_WIFI_STA_CHANNEL` | Wi-Fi 信道（可选） |
| `CONFIG_AXK_WIFI_SSID_HIDDEN_ENABLE` | 是否连接隐藏 Wi-Fi |
| `CONFIG_AXK_WIFI_AP_SSID/PASS/...` | SoftAP 配置（当前未使用） |
| `CONFIG_MAX_STA_CONN_LIMIT` | 最多可连接的 STA 设备数 |

MQTT 服务器 URI 可通过以下宏切换：

```c
#define BROKER_URI_TCP      "mqtt://mqtt.eclipseprojects.io"
#define BROKER_URI_SSL      "mqtts://mqtt.eclipseprojects.io:8883"
#define BROKER_URI_WS       "ws://mqtt.eclipseprojects.io:80/mqtt"
#define BROKER_URI_WSS      "wss://mqtt.eclipseprojects.io:443/mqtt"
#define BROKER_URI_SSL_PSK  "mqtts://mqttsserver"
