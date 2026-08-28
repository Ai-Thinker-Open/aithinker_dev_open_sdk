#ifndef __CHIP_INCLUDE_H_
#define __CHIP_INCLUDE_H_

#include "chip_types.h"

// 系统初始化头文件
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "mem.h"
#include "semphr.h"
#include "ring_buffer.h"
#include "bflb_mtimer.h"
#include "board.h"
#include "shell.h"
#include "bflb_irq.h"
#include "bflb_uart.h"


//wifi 头文件
#include <lwip/tcpip.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

#include "bl_fw_api.h"
#include "wifi_mgmr_ext.h"
#include "wifi_mgmr.h"

#include "bl616_glb.h"
#include "bflb_gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Flash 管理
#include "easyflash.h"
#include "bflb_flash.h"

#include "aiio_adapter_include.h"
#endif
