/**
 * @file   ln_at_cfg.h
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2021 LightningSemi Technology Co., Ltd. All rights reserved.
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-28     MurphyZhao   the first version
 */

#ifndef __LN_AT_CFG_H__
#define __LN_AT_CFG_H__

//#define LN_AT_DEBUG_ENABLE

#define LN_AT_USING_SERVER      (1)
#define LN_AT_ENABLE_ECHO       (1) /* enable */

#define LN_AT_MAX_LINE_SIZE     (512)
#define LN_AT_PRINTF_BUF_SIZE   (2048)

#ifndef LN_AT_TRANSFER_TASK_STACK_SIZE
#define LN_AT_TRANSFER_TASK_STACK_SIZE   (20*1024)
#endif

#ifndef LN_AT_TRANSFER_TASK_PRI
#define LN_AT_TRANSFER_TASK_PRI (3) /* OS_PRIORITY_NORMAL */
#endif

#define LN_AT_WAIT_FOREVER      (0xffffffffU)

#define LN_AT_CMD_END_MARK_CR   ("\r")
#define LN_AT_CMD_END_MARK_LF   ("\n")
#define LN_AT_CMD_END_MARK_CRLF ("\r\n")

// #define LN_AT_USING_AT_DATA_TRANSMIT

#endif /* __LN_AT_CFG_H__ */
