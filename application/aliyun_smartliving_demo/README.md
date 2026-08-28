### 功能描述：
1、连接阿里飞燕平台例程
2、连接成功后可发布订阅数据


### demo目录结构

```
├── buzzer_demo							//demo
│   ├── src                             //源码
|   |   ├──compoents
|   |   └──main.c                       //demo代码例程
│   ├── vender	                        //每个平台的参数配置
|   |	├──bl602
|   |   ├──bl616
|   |   ├──bl618
|   |   ├──esp8266
|   |   ├──hi3861
|   |   ├──ln882h
|   |   ├──rtl8720cf
|   |   └──w800				
│   └── readme.md                       //readme说明
```


### demo编译

```
1、进入SDK目录，在SDK目录下执行指令

2. menuconfig配置参数：通过指令： ./build.sh 芯片型号 应用名称 menuconfig		 举例：./build.sh bl602 tcp_client_demo menuconfig
进入配置界面，选择"application"——>"buzzer_demo"——>"AIIO_GPIO_CFG",即可查看和配置引脚信息。选择"Exit"退出。

3. 编译：通过指令： ./build.sh 芯片型号 应用名称 国家编码 调试等级	举例：./build.sh bl602 button_demo cn debug

4. 编译成功，生成名为："dev_project.bin"的固件，存在于out路径下，根据编译时的平台查找固件，例如：ln882h，则找该平台下对应demo的固件。

5. 固件烧录：不同平台的固件烧录工具不同，应选择正确平台的烧录工具进行固件烧录
```

### demo使用
```
1.串口：uart0 波特率：921600
2.烧录三元组（可自己账号下创建产品与调试设备注意：设备创建的devicename要为小写的模块mac地址，必须对应，否则无法配网）
烧录指令：AT+KEY=Product Key,Product Secret,DeviceName,DeviceSecret,Product ID
例：AT+KEY=a1pdavA7eaS,jbZnJViCLECzmYHM,7cb94c1dc676,8b876b9df1e88512d1a6ef6de4e4fd5b,21567997
3.进入配网
设备启动运行状态：红灯、蓝灯和绿灯同时闪烁
设备已进入配网状态：红灯和绿灯同时闪烁
设备已连接云端：蓝灯常亮
按键重新进入配网：长按button按键至红灯和绿灯同时闪烁
4. 配网app：云智能
5. Demo功能：仅开灯/关灯控制
```