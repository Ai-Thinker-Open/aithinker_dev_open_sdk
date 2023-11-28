### 功能描述：
1、该demo是双色灯传感器例程
2、支持GPIO输入输出高低电平来控制双色灯传感器
3、通过menuconfig可配置不同颜色的引脚控制


### demo目录结构

```
├── doubulelight_demo							//demo
│   ├── src                                     //源码
|   |   ├──compoents
|   |   └──main.c                               //demo代码例程
│   ├── vender	                                //每个平台的参数配置
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

2. menuconfig配置参数：通过指令： ./build.sh 芯片型号 应用名称 menuconfig		 举例：./build.sh bl602 doubulelight_demo menuconfig
进入配置界面，选择"application"——>"doubulelight_demo"——>"AIIO_GPIO_CFG",即可查看和配置引脚信息。选择"Exit"退出。

3. 编译：通过指令： ./build.sh 芯片型号 应用名称 国家编码 调试等级	举例：./build.sh bl602 doubulelight_demo cn debug

4. 编译成功，生成名为："dev_project.bin"的固件，存在于out路径下，根据编译时的平台查找固件，例如：ln882h，则找该平台下对应demo的固件。

5. 固件烧录：不同平台的固件烧录工具不同，应选择正确平台的烧录工具进行固件烧录
```

### demo使用
```
1、根据menuconfig中的引脚配置信息，连接模组与双色灯传感器,例如：GND->“-”、IO11->“S”、IO4->模块中间的引脚。

2、复位模组

3、查看双色灯传感器的亮灯颜色
```









