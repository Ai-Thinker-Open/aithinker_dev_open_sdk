### 功能描述：
1、该demo是按键传感器例程
2、支持GPIO输入输出高低电平来控制按键传感器
3、通过menuconfig可配置输出引脚
4、采集按键控制状态，在串口打印对应的按键状态


### demo目录结构

```
├── button_demo							//demo
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

2. menuconfig配置参数：通过指令： ./build.sh 芯片型号 应用名称 menuconfig		 举例：./build.sh bl602 button_demo menuconfig
进入配置界面，选择"application"——>"button_demo"——>"AIIO_GPIO_CFG",即可查看和配置引脚信息。选择"Exit"退出。

3. 编译：通过指令： ./build.sh 芯片型号 应用名称 国家编码 调试等级	举例：./build.sh bl602 button_demo cn debug

4. 编译成功，生成名为："dev_project.bin"的固件，存在于out路径下，根据编译时的平台查找固件，例如：ln882h，则找该平台下对应demo的固件。

5. 固件烧录：不同平台的固件烧录工具不同，应选择正确平台的烧录工具进行固件烧录
```

### demo使用
```
1、根据menuconfig中的引脚配置信息，连接模组与按键传感器：GND->“-”、3V3->模块中间的引脚、IO4->“S”。

2、button_demo主要有3个功能：
    1)单按1次
    2)连续按2次
    3)长按

3、通过串口调试助手查看按键的状态，波特率为：921600
```










