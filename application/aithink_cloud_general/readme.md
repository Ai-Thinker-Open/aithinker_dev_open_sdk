# aithink_cloud_general

### 一、功能描述

​	爱星物联通用透传demo。采用MCU+SOC的方式，MCU负责控制外设功能，而SOC模组则是负责数据通信。

​	基于AiLink协议栈，支持BLE配网和AP配网、支持MQTT通信、支持UDP局域网通信、支持AT指令与串口协议、支持模组OTA与MCU OTA。

### 二、编译

#### 1、menuconfig配置

* menuconfig配置指令

  * BL602 menuconfig配置指令

    ```
     ./build.sh bl602 aithink_cloud_general menuconfig
    ```

  * BL618 menuconfig配置指令

    ```
     ./build.sh bl618 aithink_cloud_general menuconfig
    ```

  * BL616 menuconfig配置指令

    ```
    ./build.sh bl616 aithink_cloud_general menuconfig
    ```
    
  * RTL8720 menuconfig配置指令

    ```
    ./build.sh rtl8720dx aithink_cloud_general menuconfig
    ```
    
  * RTL8711 menuconfig配置指令

    ```
    ./build.sh rtl8711dx aithink_cloud_general menuconfig
    ```

#### 2、编译指令

* BL602固件编译

  ```
   ./build.sh bl602 aithink_cloud_general cn debug
  ```

* BL618固件编译

  ```
   ./build.sh bl618 aithink_cloud_general cn debug
  ```

* BL616固件编译

  ```
  ./build.sh bl616 aithink_cloud_general cn debug
  ```
  
* RTL8720固件编译

  ```
  ./build.sh rtl8720dx aithink_cloud_general cn debug
  ```
  
* RTL8711固件编译

  ```
   ./build.sh rtl8711dx aithink_cloud_general cn debug
  ```

### 三、使用	

​	将demo编译生成固件后，可将固件烧录到设备上。后面可按以下步骤使用设备。**（注：此demo产品信息为上电后从MCU获取的，因此不限产品，可根据自己的需求创建自己的产品。只要MCU SDK、烧录的三元组、构建的app在同一平台账号下，此demo都适用）**

* 串口说明

  ​	log口输出模组log，透传口为与MCU进行透传协议交互以及AT指令口。默认log口波特率为921600，透传口波特率为9600，默认IO口见下表，波特率和IO口都可以通过menuconfig进行配置。

  | 芯片型号 | UART0 (log 口)   | UART1 (透传口)   |
  | -------- | ---------------- | ---------------- |
  | BL602    | TX:IO3, RX:IO4   | TX:IO16, RX:IO7  |
  | BL616    | TX:IO20, RX:IO3  | TX:IO21, RX:IO22 |
  | BL618    | TX:IO18, RX:IO19 | TX:IO21, RX:IO22 |
  | RTL8720  | TX:IO7, RX:IO8   | TX:IO1, RX:IO2   |
  | RTL8711  | TX:IO5, RX:IO4   | TX:IO31, RX:IO30 |

* 开发测试应用

  ​	开发前准备→产品开发→APP开发。

  详细操作及更详细的说明见**爱星物联开发者帮助文档中心（https://doc.iot-aithings.com）**

  









