/**
 * @file     bootram_port.h
 * @author   BSP Team
 * @brief    This file contains all of bootram port functions prototypes.
 * @version  0.0.0.1
 * @date     2020-12-29
 *
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 *
 */

#ifndef __BOOT_PORT_H
#define __BOOT_PORT_H

#include "proj_config.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int           bootram_flash_read(uint32_t offset, uint32_t len, void* buf);
int           bootram_flash_write(uint32_t offset, uint32_t len, const void* buf);
int           bootram_flash_erase(uint32_t offset, uint32_t len);
int           bootram_flash_chiperase(void);
void          bootram_serial_init(void);
size_t        bootram_serial_read(void* buf, size_t size);
size_t        bootram_serial_write(const void* buf, size_t size);
int           bootram_serial_flush(void);
unsigned char bootram_serial_setbaudrate(uint32_t baudrate);
void          bootram_user_reboot(void);
int           bootram_flash_info(void);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // __BOOT_PORT_H
