# 简介

这个demo是用来演示适用红外接收探头接收NEC红外码的demo

适用型号：BL602、BL616、BL618

# 环境搭建

参考主工程的readme，搭建开发环境后可以正常 demo 例程表示环境搭建完毕

# 编译

1. 进入sdk根目录

2. 执行 ./build.sh <platform> ir_nec_rx_demo menuconfig ,不用修改直接退出，生成默认config

   eg：./build.sh bl602 ir_nec_rx_demo menuconfig

3. 执行 ./build.sh <platform> ir_nec_rx_demo cn debug 开始编译

   eg：./build.sh bl602 ir_nec_rx_demo cn debug

# 测试

## 使用说明

使用红外接收传感器与模组接好线，使用NEC协议的红外对着红外接收传感器发射，即可在log中看到对应发射的码值

## 接线
模组<———————————>红外接收传感器

GND<———————————>GND

VCC<———————————>VCC

IO12<———————————>DATA

## log

当接收到NEC红外码，log会输出对应的码值
~~~
I | [0] [aiio_main] (main.c:51 aiio_main) IR NEC RX DEMO!
I | [191] [NEC_RX] (main.c:43 ir_nec_rx_task) receive bit: 32
I | [191] [NEC_RX] (main.c:44 ir_nec_rx_task) receive value: 0x00000000e916ff00
I | [1259] [NEC_RX] (main.c:43 ir_nec_rx_task) receive bit: 32
I | [1259] [NEC_RX] (main.c:44 ir_nec_rx_task) receive value: 0x00000000e916ff00
I | [2326] [NEC_RX] (main.c:43 ir_nec_rx_task) receive bit: 32
I | [2327] [NEC_RX] (main.c:44 ir_nec_rx_task) receive value: 0x00000000e916ff00
I | [3394] [NEC_RX] (main.c:43 ir_nec_rx_task) receive bit: 32
I | [3395] [NEC_RX] (main.c:44 ir_nec_rx_task) receive value: 0x00000000e916ff00
I | [4462] [NEC_RX] (main.c:43 ir_nec_rx_task) receive bit: 32
I | [4463] [NEC_RX] (main.c:44 ir_nec_rx_task) receive value: 0x00000000e916ff00
I | [5530] [NEC_RX] (main.c:43 ir_nec_rx_task) receive bit: 32
I | [5531] [NEC_RX] (main.c:44 ir_nec_rx_task) receive value: 0x00000000e916ff00
I | [6598] [NEC_RX] (main.c:43 ir_nec_rx_task) receive bit: 32
I | [6598] [NEC_RX] (main.c:44 ir_nec_rx_task) receive value: 0x00000000e916ff00
I | [7666] [NEC_RX] (main.c:43 ir_nec_rx_task) receive bit: 32
I | [7666] [NEC_RX] (main.c:44 ir_nec_rx_task) receive value: 0x00000000e916ff00
I | [8734] [NEC_RX] (main.c:43 ir_nec_rx_task) receive bit: 32
I | [8734] [NEC_RX] (main.c:44 ir_nec_rx_task) receive value: 0x00000000e916ff00
~~~