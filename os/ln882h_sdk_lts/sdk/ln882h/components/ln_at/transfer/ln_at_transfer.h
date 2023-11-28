/**
 * @file   ln_at_transfer.h
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-28     MurphyZhao   the first version
 */

#ifndef __LN_AT_TRANSFER_H__
#define __LN_AT_TRANSFER_H__

#include "ln_at.h"
#include "serial/serial.h"

//sys at uart serial need
typedef struct
{
    Serial_t *serial_hdl;
    ln_at_transfer_t *trans_hdl;
    
    void (*rx_cb)(void);
}at_sys_serial_handle_t;

extern at_sys_serial_handle_t *at_sys_serial_get_handle(void);

ln_at_transfer_t *ln_transfer_create(void);
int ln_at_write(const char *buf, size_t size);

extern at_sys_serial_handle_t g_sys_handle[1];

#endif /* __LN_AT_TRANSFER_H__ */
