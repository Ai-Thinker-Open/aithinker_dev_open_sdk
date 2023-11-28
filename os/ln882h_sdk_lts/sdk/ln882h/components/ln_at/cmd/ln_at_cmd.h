/**
 * @file   ln_at_cmd.h
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-28     MurphyZhao   the first version
 */

#ifndef __LN_AT_CMD_H__
#define __LN_AT_CMD_H__

#include "ln_at.h"

typedef enum
{
    LN_AT_CMD_TYPE_NONE,
    LN_AT_CMD_TYPE_CUR,
    LN_AT_CMD_TYPE_DEF,
} ln_at_cmd_type_e;

ln_at_cmd_table_t *ln_at_cmd_table_create(void);

#endif /* __LN_AT_CMD_H__ */
