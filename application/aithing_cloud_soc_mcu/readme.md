# Ai-Think Cloud

### 功能概述

#### 可支持的功能

* 支持按键功能
  * 可支持轻触短按和长按功能（短按控制LED翻转，长按控制设备进入配网模式）
  * 需通过menuconfig配置GPIO方可实现功能
  * 默认配置GPIO为IO0
* 支持LED状态显示功能
  * 需通过menuconfig配置GPIO方可实现功能
  * 可支持按键控制灯状态翻转。
  * LED状态显示分别为：
    * 设备启动运行状态：红灯、蓝灯和绿灯同时闪烁
    * 设备已进入配网状态：红灯和绿灯同时闪烁
    * 设备退出配网状态：红灯、蓝灯和绿灯同时闪烁
    * 设备配网失败状态：红灯闪烁
    * 设备已连接WiFi状态：绿灯常亮
    * 设备已断开WiFi状态：绿灯闪烁
    * 设备已连接云端：蓝灯常亮
    * 设备已断开云端：蓝灯闪烁
  * 默认配置为红灯、蓝灯和绿灯的IO为IO0
* 支持mqtt通信的云端控制功能
  * 需通过menuconfig配置开启云端控制功能
  * 默认配置为开启状态
* 支持udp通信的局域网控制功能
  * 需通过menuconfig配置开启局域网控制功能
  * 默认配置为未开启状态
* 支持AP配网功能
  * 需通过menuconfig配置开启AP配网功能
  * 默认配置为开启状态
* 支持BLE配网功能
  * 需通过menuconfig配置开启BLE配网功能
  * 默认配置为开启状态
* 支持OTA功能
  * 需通过menuconfig配置开启OTA功能
  * 默认配置为开启状态
* 支持AT指令功能
  * 现仅支持AT和AT+KEY这两个指令

### 目录结构

```
描述清除demo目录下文件夹以及相关文件的功能
```



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

* menuconfig功能配置
  * adt配置
    * 云端控制使能：adt -->Cloud config -->Aithing cloud -->wan network enable
    * 局域网控制使能： adt -->Cloud config -->Aithing cloud -->lan network enable
    * BLE配网使能: adt -->Cloud config -->Aithing cloud -->ble distribution network enable
    * AP配网使能：adt -->Cloud config -->Aithing cloud -->ap distribution network enable
    * ota使能：adt -->Cloud config -->Aithing cloud -->ota enable
  * application配置
    * 按键GPIO配置：application-->button config
    * LED GPIO配置：application-->led config
    * flash存储数据的起始地址配置：application-->Data start address-->Flash rw start addr
    * flash 存储扇区大小配置：application-->Data start address-->flash region size
    * 芯片平台类型配置：application-->chip platform-->chip platform
    * 云端控制模块使能：application-->module config-->wan network enable
    * 局域网控制模块使能：application-->module config-->Lan network enable
    * BLE配网模块使能：application-->module config-->ble distribution network enable
    * AP配网模块使能：application-->module config-->ap distribution network enable
    * OTA模块使能：application-->module config-->ota module enable

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

#### 固件烧录

* [bl602烧录指导]([Ai-WB2系列的固件烧录指导_ai-wb2-12f_安信可科技的博客-CSDN博客](https://blog.csdn.net/Boantong_/article/details/125781602?spm=1001.2014.3001.5501))
* [bl616&bl618烧录指导]([Ai-M61/62系列的固件烧录指导_安信可科技的博客-CSDN博客](https://blog.csdn.net/Boantong_/article/details/131245472?spm=1001.2014.3001.5501))

### 使用

将demo编译生成固件后，可将固件烧录到设备上。后面可按以下步骤使用设备。

* 三元组数据写入设备

  * 使用AT指令写入三元组数据，AT指令格式如下

    ```
    AT+KEY=设备id,用户名,密码	
    ```

  * 三元组数据获取

    * 打开[爱星云开放平台](https://open.iot-aithings.com/product/firmware/index)

    * 生成三元组数据

      * 首先：产品开发菜单，选择设备管理子菜单；

      * 其次： 点击生成三元组，检查本账号允许导入的最大数量，是否还有剩余，根据序列号模板，导入对应序列号，选择产品，输入批次；

      * 然后：点击确认，将生产对应数量的三元组，并且将在设备列表中查看

        ![image-20230829171156943](doc\imagic\image-20230829171156943.png)

        ![image-20230829171502140](doc\imagic\image-20230829171502140.png)

    * 导出三元组数据

      * 在设备管理页面上点击导出三元组

      * 根据产品名称、批次号、设备序列号或是是否激活选项进行导出对应的三元组数据

        ![image-20230829171820233](doc\imagic\image-20230829171820233.png)

        ![image-20230829171834524](doc\imagic\image-20230829171834524.png)

* 使用APP对设备进行配网

  * APP安装

    * 当手机型号是IOS、小米、华为、OPPO或是三星，这些可自动到应用商店上下载APP（但这些APP版本号相对比较落后）

    * 可扫描以下二维码进行下载APP安装

      ![image-20230829173426918](doc\imagic\image-20230829173426918.png)

  * 通过APP对设备进行配网

    * APP在对设备进行配网时，APP优先会选择BLE配网。
    * 若是需用APP选择AP配网时，则可将手机上蓝牙关闭后；当APP重新扫描到设备时，则可以进行AP配网了。

* 使用APP控制设备状态

  * 通过局域网控制设备

    APP和设备处于同一局域网时，APP通过设备的mDNS复位发现设备；并通过udp通信下发命令数据而控制设备状态。

  * 通过云端控制设备

    当设备通过mqtt连接云端后，APP可通过mqtt通信下发命令数据而控制色好吧状态。

* 使用APP对设备进行固件升级

  1. 需在爱星云的云管平台上的固件管理页面上，选择指定模组类型，并将ota固件以及固件包上传云平台

     ![image-20230829174631783](doc\imagic\image-20230829174631783.png)

     ![image-20230829174845627](doc\imagic\image-20230829174845627.png)

     

  2. 需在爱星云的云管平台上的芯片模组管理页面上，选择指定模组，并在指定模组页面内选定版本号

     ![image-20230829175138784](doc\imagic\image-20230829175138784.png)

     ![image-20230829175253325](doc\imagic\image-20230829175253325.png)

  3. 需在爱星云开放平台上的产品开发选项的固件OTA页面上，选择指定产品类型，而后则可新增OTA

     ![image-20230829175408768](doc\imagic\image-20230829175408768.png)

### 注意

* 在menuconfig配置flash存储数据的起始地址时，需根据不同分区表而配置，需注意不同平台以及不同flash大小上的分区表时，其起始地址是不同的。
* 在现阶段上编译该demo时，并不能生成ota文件。若是需生成ota文件时，可将以下仓库上切换zlm分支后，编译该demo便可生成ota文件。
  * bl602_sdk_lts 仓库
  * aithinker_dev_sdk 仓库































































