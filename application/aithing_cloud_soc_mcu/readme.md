# aithing_cloud_soc_mcu

### 功能概述

​	爱星物联公版单控demo。实现单模组配网、连云、数据上下行、OTA功能。

​	基于AiLink协议栈，支持BLE配网和AP配网、支持MQTT通信、支持UDP局域网通信、支持模组OTA。

​	通过menuconfig控制“CONFIG_CLOUD_CONTROL_LED_MODULE”宏切换两个公版产品，CONFIG_CLOUD_CONTROL_LED_MODULE不打开为香薰机，打开为智能灯。

### 编译

#### menuconfig配置

* menuconfig配置指令

  * BL602 menuconfig配置指令

    ```
     ./build.sh bl602 aithing_cloud_soc_mcu menuconfig
    ```

  * BL618 menuconfig配置指令

    ```
     ./build.sh bl618 aithing_cloud_soc_mcu menuconfig
    ```

  * BL616 menuconfig配置指令

    ```
     ./build.sh bl616 aithing_cloud_soc_mcu menuconfig
    ```
    
  * RTL8720 menuconfig配置指令

    ```
    ./build.sh rtl8720dx aithing_cloud_soc_mcu menuconfig
    ```
    
  * RTL8711 menuconfig配置指令

    ```
    ./build.sh rtl8711dx aithing_cloud_soc_mcu menuconfig
    ```

#### 编译指令

* BL602固件编译

  ```
   ./build.sh bl602 aithing_cloud_soc_mcu cn debug
  ```

* BL618固件编译

  ```
   ./build.sh bl618 aithing_cloud_soc_mcu cn debug
  ```

* BL616固件编译

  ```
  ./build.sh bl616 aithing_cloud_soc_mcu cn debug
  ```
  
* RTL8720固件编译

  ```
  ./build.sh rtl8720dx aithing_cloud_soc_mcu cn debug
  ```
  
* RTL8711固件编译

  ```
  ./build.sh rtl8711dx aithing_cloud_soc_mcu cn debug
  ```

### 使用

​	编译成功后，将固件烧录到设备上。按以下步骤使用设备。

* 三元组获取

  因为此demo的产品写死为公版平台安信可账号下的，所以三元组要由公版平台安信可账号下生成。

* 三元组数据写入设备

  使用AT指令写入三元组数据，AT指令格式如下：

  ```
  AT+KEY=设备id,用户名,密码	
  ```

* 配网

  * 下载安装APP:同样因为此demo的产品写死为公版平台安信可账号下的，因此需要搜索安装公版爱星物联APP。

  * 长按按键进入配网模式，app扫描到设备进行配网。

* 使用APP进行数据通信

  ​	可通过mqtt和局域网两种方式进行数据上下行。香薰机产品未做实际控制外设，智能灯产品可控制模块灯开关。

### 说明

​	此demo的产品写死为公版平台安信可账号下的，如需使用自己的产品，请修改代码，替换为自己所创建产品的产品信息以及物模型或者使用爱星物联通用透传demo----**aithink_cloud_general** ，使用MCU+soc方案。

























































