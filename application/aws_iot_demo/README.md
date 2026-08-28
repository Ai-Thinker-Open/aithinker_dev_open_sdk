1. # 简介

   该demo演示连接亚马逊AWS IOT、订阅、发布。需要连接手机开启的热点或者路由器WiFi，频段在2.4G。热点名称或者路由wifi名称默认为：ZLM_24G，密码：12345678。

   1、支持STA模式可配置wifi名称、密码、信道等参数

   # 环境搭建

   参考主工程的readme，搭建开发环境后可以正常编译 demo 例程表示环境搭建完毕

   # 配置

   1. bl616

      ```
      ./build.sh bl616 aws_iot_demo menuconfig
      ```

      

   2. bl618

      ```
      ./build.sh bl618 aws_iot_demo menuconfig
      ```

      

   # 编译

   1. bl616

      ```
      ./build.sh bl616 aws_iot_demo cn debug
      ```

      

   2. bl618

      ```
      ./build.sh bl618 aws_iot_demo cn debug
      ```

      

   # 测试

   1. 烧录固件

   2. 查看log，设备连接到路由获取到IP之后，会进行mqtt连接，连接成功会订阅"/topic/qos0"主题，订阅成功会在"/topic/qos1"主题发布消息"subscribe /topic/qos0 ok!".

   3. 订阅到"/topic/qos0"主题的数据，会在"/topic/qos1"主题回复"welcome aws iot!"