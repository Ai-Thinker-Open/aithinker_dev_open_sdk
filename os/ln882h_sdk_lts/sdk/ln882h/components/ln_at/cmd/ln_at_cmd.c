/**
 * @file   ln_at_cmd.c
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-28     MurphyZhao   the first version
 */

#include "ln_at_cmd.h"

#include <stdint.h>

#if defined(__ICCARM__) || defined(__ICCRX__)         /* for IAR compiler */
#pragma section="ln_at_cmd_tbl"
#endif

static uint32_t         *ln_at_cmd_hash_tbl = NULL;
static ln_at_cmd_item_t *ln_at_cmd_table    = NULL;
static uint32_t          ln_at_cmd_num      = 0;

static ln_at_cmd_table_t g_at_cmd_tbl = {0};

/**
 * @brief ln_at_cmd_items_init
 * 
 * Method 2:
 *     Use ln_at_cmd_tbl section to hold AT command list.
 */
static int ln_at_cmd_items_init(void)
{
    /* initialize the utest commands table.*/
#if defined(__CC_ARM) || defined(__CLANG_ARM)       /* ARM C Compiler */
    extern const int ln_at_cmd_tbl$$Base;
    extern const int ln_at_cmd_tbl$$Limit;
    ln_at_cmd_table = (ln_at_cmd_item_t *)&ln_at_cmd_tbl$$Base;
    ln_at_cmd_num   = (ln_at_cmd_item_t *)&ln_at_cmd_tbl$$Limit - ln_at_cmd_table;
#elif defined (__ICCARM__) || defined(__ICCRX__)    /* for IAR Compiler */
    ln_at_cmd_table = (ln_at_cmd_item_t *)__section_begin("ln_at_cmd_tbl");
    ln_at_cmd_num   = (ln_at_cmd_item_t *)__section_end("ln_at_cmd_tbl") - ln_at_cmd_table;
#elif defined (__GNUC__)                            /* for GCC Compiler */
    extern const int __ln_at_cmd_tbl_start;
    extern const int __ln_at_cmd_tbl_end;
    ln_at_cmd_table = (ln_at_cmd_item_t *)&__ln_at_cmd_tbl_start;
    ln_at_cmd_num   = (ln_at_cmd_item_t *)&__ln_at_cmd_tbl_end - ln_at_cmd_table;
#endif

    ln_at_cmd_hash_tbl = LN_AT_CALLOC(1, sizeof(uint32_t) * ln_at_cmd_num);
    if (!ln_at_cmd_hash_tbl)
    {
        LN_AT_LOG_E("calloc failed\r\n");
        return -1;
    }

    for (uint32_t i = 0; i < ln_at_cmd_num; i++)
    {
        ln_at_cmd_hash_tbl[i] = ln_djb_hash(ln_at_cmd_table[i].name);
        LN_AT_LOG_E("[%s]:0x%08x - %d\r\n", 
            ln_at_cmd_table[i].name, ln_at_cmd_hash_tbl[i], ln_at_cmd_hash_tbl[i]);
    }

    return ln_at_cmd_num;
}

ln_at_cmd_table_t *ln_at_cmd_table_create(void)
{
    ln_at_cmd_table_t *tbl = &g_at_cmd_tbl;

    ln_at_cmd_items_init();
    tbl->flash_cmd_tbl      = ln_at_cmd_table;
    tbl->flash_cmd_hash_tbl = ln_at_cmd_hash_tbl;
    tbl->flash_cmd_num      = ln_at_cmd_num;

    return tbl;
}
