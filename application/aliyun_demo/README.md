### 功能描述：

连接阿里平台例程


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
2.通过menuconfig配置连接路由的ssid和password，默认ssid为"myssid",password为"mypassword"
3.连网络成功后会连接阿里mqtt服务器并订阅相关主题
```
