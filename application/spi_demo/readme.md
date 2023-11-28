### 功能描述：
支持驱动外设，通过UART、SPI协议操作Flash，对外设Flash进行擦、写、读


### demo目录结构

```
├── spi_demo							//demo
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

2. menuconfig配置参数：通过指令： ./build.sh 芯片型号 应用名称 menuconfig		 举例：./build.sh ln882h spi_demo menuconfig
进入配置界面，选择"application"——>"spi_demo"——>"AIIO_SPI_PIN_CFG",查看和配置引脚。选择"Exit"退出。

3. 编译：通过指令： ./build.sh 芯片型号 应用名称 国家编码 调试等级	举例：./build.sh ln882h spi_demo cn debug

4. 编译成功，生成名为："dev_project.bin"的固件，存在于out路径下，根据编译时的平台查找固件，例如：ln882h，则找该平台下对应demo的固件。

5. 固件烧录：不同平台的固件烧录工具不同，应选择正确平台的烧录工具进行固件烧录
```

### demo使用
```
1、模组与W25qxx模块按照menuconfig中的引脚配置来接线

2、通过串口调试助手查看log信息，波特率为：921600
```






