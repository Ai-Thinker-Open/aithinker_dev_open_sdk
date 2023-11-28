# LN882H SDK 简介

# 编译方式

## 1. Keil MDK

### 环境配置
参见`doc/LN882H Keil ARMCC开发环境搭建指导.pdf`文件搭建环境。

### 用户工程选择
用户工程在 `project/xxxx`，例如 `wifi_mcu_basic_example` 工程，打开子目录中的 Keil 工程，点击 Keil IDE
界面中的 **编译**、**下载**、**调试** 按钮即可开始对应的功能。

## 2. CMake+GCC

### 环境配置

参见 `doc/lightningsemi_sdk_cross_build_setup.pdf` 文件搭建环境。

### 用户工程选择
工程以 `CMakeLists.txt` 文件进行组织，顶层的 `CMakeLists.txt` 选择了用户工程，参见顶层 `CMakeLists.txt` 中如下指令：

```
################################################################################
#########################   NOTE: select user project here  ####################
################################################################################
set(USER_PROJECT  wifi_mcu_basic_example)
```

### 用户工程编译、下载

以脚本 `start_build.py` 开始编译、下载动作。用法如下：

```
    *****************************  usage  *****************************
    argv[1] -- build action, such as clean/config/build/rebuild/jflash.
    Example:
                python3   start_build.py   rebuild
    *******************************************************************
```

- 清除结果： `python3  start_build.py  clean`
- 配置工程： `python3  start_build.py  config`
- 编译工程： `python3  start_build.py  build`
- 清除、配置、编译三合一： `python3  start_build.py  rebuild`
- 烧录镜像文件： `python3  start_build.py  jflash`
