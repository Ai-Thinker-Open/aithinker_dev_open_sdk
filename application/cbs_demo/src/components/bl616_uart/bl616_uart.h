#ifndef __BL616_UART_H_
#define __BL616_UART_H_
#include "chip_include.h"
#ifdef __BL616_UART_C_
#define BL616_UART_EXT
#else
#define BL616_UART_EXT extern
#endif

#define FLASH_RW_START_ADDR 0x150000

BL616_UART_EXT void bl616_uart_init(void);
#endif
