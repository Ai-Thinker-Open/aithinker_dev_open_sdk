/**
 * @file     bootram_mode.h
 * @author   BSP Team
 * @brief
 * @version  0.0.0.1
 * @date     2020-12-31
 *
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 *
 */
#ifndef __BOOTRAM_MODE_H__
#define __BOOTRAM_MODE_H__

#include "proj_config.h"
#include "ramcode_port.h"
#include "ctype.h"
#include "ymodem.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef MIN
#  define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#  define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

/* Number of elements in an array */
#define NELEMENTS(array) (sizeof(array) / sizeof(array[0]))
#define OK               (0)
#define FAIL             (-1)

#define CMD_MAXARGS      4
#define CMD_PBSIZE       64

typedef enum {
    BOOTRAM_MODE_INIT           = 1,
    BOOTRAM_MODE_CMD            = 2,
    BOOTRAM_MODE_CMD_2_DOWNLOAD = 3,
    BOOTRAM_MODE_DOWNLOAD       = 4,
    BOOTRAM_MODE_DOWNLOAD_2_CMD = 5,
} fsm_status_t;

typedef struct
{
    fsm_status_t bootram_cur_mode;
} bootram_fsm_t;

typedef struct
{
    uint8_t  file_count;          // how many files have been download.
    uint32_t start_addr;          // download start address (initial value for flash_write_offset).
    uint32_t flash_write_offset;  // changes after write a block to flash.

    uint8_t* cache_buffer_arr;  // save ymodem block here before write to flash.
    uint32_t cache_buffer_pos;  // offset for a block placed in cache_buffer_arr.

    uint8_t* cache_tailing_buf;  // 如果多次从 ymodem block 拷贝数据到 cache_buffer_arr
                                 // 超出了数组长度，就把尾巴临时拷贝到这个数据
    uint32_t cache_tailing_pos;  // 上面 cache_tailing_buf 中数据长度

#if (defined(FLASH_VERIFY_ENABLE) && (1 == FLASH_VERIFY_ENABLE))
    uint8_t* verify_buffer_arr;  // read from flash.
#endif
} upgrade_ctrl_t;

typedef struct
{
    int  index;  // how many items in `cmd_line`
    char cmd_line[CMD_PBSIZE];
} bootram_cmd_ctrl_t;

typedef struct
{
    ymodem_t       ymodem_ctrl;
    upgrade_ctrl_t upgrade_ctrl;
} bootram_file_ctrl_t;

#define bootram_console_printf(                                                                    \
    format,                                                                                        \
    ...)  //__wrap_sprintf((stdio_write_fn)bootram_console_stdio_write, format, ##__VA_ARGS__)

bootram_fsm_t*       bootram_fsm_get_handle(void);
bootram_cmd_ctrl_t*  bootram_cmd_get_handle(void);
bootram_file_ctrl_t* bootram_file_get_handle(void);

void bootram_ctrl_init(void);
void bootram_ctrl_loop(void);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  /* __BOOTRAM_MODE_H__ */
