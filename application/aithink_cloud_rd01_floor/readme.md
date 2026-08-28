# Ai-Think Cloud Rd01 Floor

### 一、功能描述

​		此demo是通过rd01雷达模块+爱星物联实现人体检测与场景联动的功能（目前是一对一）。雷达检测模块为主机，其它设备为从机（此demo为主机demo，从机demo可用aithing_cloud_soc_mcu），当主机检测到有人/无人状态变化会通过对应的主题发布状态，从机订阅到会改变开关状态；同时主机上的灯也会同步指示有人/无人状态。主机对应测试硬件为rd01_floor底板，从机可用WB2模块测试。

1支持按键功能

#### 1、按键功能

- 长按3s进入配网模式

#### 2、LED状态显示功能

- 进入配网状态：LED5闪烁
- 配网成功：LED5长亮

- 检测到有人：LED4长亮
- 检测到无人：LED4长灭

#### 3、场景联动

* 检测到无人/有人的状态变化：发布对应的状态（一对一）

### 二、编译

#### 1、menuconfig配置

* menuconfig配置指令

  ```
   ./build.sh bl602 aithink_cloud_rd01_floor menuconfig
  ```

* menuconfig功能配置

  * adt配置
    * 云端控制使能：adt -->Cloud config -->Aithing cloud -->wan network enable
    * 局域网控制使能： adt -->Cloud config -->Aithing cloud -->lan network enable
    * BLE配网使能: adt -->Cloud config -->Aithing cloud -->ble distribution network enable
    * AP配网使能：adt -->Cloud config -->Aithing cloud -->ap distribution network enable
    * ota使能：adt -->Cloud config -->Aithing cloud -->ota enable
  * application配置
    * LED GPIO配置：application-->led config
    * flash存储数据的起始地址配置(需根据不同分区表而配置，需注意不同平台以及不同flash大小上的分区表时，其起始地址是不同的)：application-->Data start address-->Flash rw start addr
    * flash 存储扇区大小配置：application-->Data start address-->flash region size
    * 芯片平台类型配置：application-->chip platform-->chip platform（此demo仅支持bl602）
    * 云端控制模块使能：application-->module config-->wan network enable
    * 局域网控制模块使能：application-->module config-->Lan network enable
    * BLE配网模块使能：application-->module config-->ble distribution network enable
    * AP配网模块使能：application-->module config-->ap distribution network enable
    * OTA模块使能：application-->module config-->ota module enable

#### 2、编译指令

```
 ./build.sh bl602 aithink_cloud_rd01_floor cn debug
```

#### 3、固件烧录

* [烧录工具下载链接](https://dev.bouffalolab.com/home)
* **注：分区表需根据实际情况调整地址，本demo分区表已提供（aithink_cloud_rd01_floor-->partitioned_table下）**

### 三、使用

#### 1、配网

​	主/从机 烧录三元组、下载app、配网等皆参考demo aithing_cloud_soc_mcu下readme中使用描述

#### 2、检测与联动

- 雷达检测到有人：LED4亮，同时发布从机开关开，从机订阅到然后开关开
- 雷达检测到无人：LED4灭，同时发布从机开关关，从机订阅到然后开关关

























































