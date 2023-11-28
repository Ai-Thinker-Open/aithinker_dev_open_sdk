/**
 * @file   ln_at.c
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-28     MurphyZhao   the first version
 */

#include "ln_at.h"
#include "ln_at_transfer.h"
#include "ln_at_cmd.h"

int ln_at_init(void)
{
#if LN_AT_USING_SERVER
    if (ln_at_server_init(ln_transfer_create(), ln_at_cmd_table_create(), LN_AT_ENABLE_ECHO) != LN_AT_ERR_NONE)
    {
        LN_AT_LOG_E("OS_ThreadCreate at_task fail.\r\n");
        return -1;
    }
#endif /* LN_AT_USING_SERVER */

    ln_at_printf("\r\nready\r\n");
    return 0;
}
