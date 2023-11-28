/**
 * @file   ln_at.h
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-28     MurphyZhao   the first version
 */

#ifndef __LN_AT_H__
#define __LN_AT_H__

#include "ln_at_cfg.h"
#include "ln_at_adapt.h"
#include "parser/ln_at_parser.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define LN_AT_RET_OK_STR   ("\r\nOK\r\n")
#define LN_AT_RET_ERR_STR  ("\r\nERROR\r\n")
#define LN_AT_RET_FAIL_STR ("\r\nFAIL\r\n")

/* lightning at parser version information */
#define LN_AT_VERSION     1L  /**< major version number  */
#define LN_AT_SUBVERSION  0L  /**< minor version number  */
#define LN_AT_REVISION    0L  /**< revise version number */

/**
 * @LN_AT_ERR_OMIT:
 *          If you use an ellipsis parameter in a command
 *          that does not support ellipsis, report this error.
*/
typedef enum ln_at_err
{
    LN_AT_ERR_NONE        = 0,   /* LN_AT_ERR_NONE */
    LN_AT_ERR_COMMON      = 1,   /* LN_AT_ERR_COMMON */
    LN_AT_ERR_PARAM       = 2,   /* LN_AT_ERR_PARAM */
    LN_AT_ERR_SYS         = 3,   /* LN_AT_ERR_SYS */
    LN_AT_ERR_PARSE       = 4,
    LN_AT_ERR_FORMAT      = 5,
    LN_AT_ERR_OMIT        = 6,
    LN_AT_ERR_NOT_FIND    = 7,
    LN_AT_ERR_NOT_SUPPORT
} ln_at_err_t;

typedef struct
{
    const char *name;     /* at command string */
    uint32_t    hash;

    ln_at_err_t (*get)(const char *name);
    ln_at_err_t (*set)(uint8_t para_num, const char *name);
    ln_at_err_t (*test)(const char *name);
    ln_at_err_t (*exec)(const char *name);
} ln_at_cmd_item_t;

/**
 * eg:
 * LN_AT_CMD_ITEM_DEF("USR_CMD_NAME", ln_usr_cmd_get, ln_usr_cmd_set, ln_usr_cmd_test, ln_usr_cmd_exec)
 *
 * Note: "USR_CMD_NAME" require quotes!!!
*/
#define LN_AT_CMD_ITEM_DEF(name, get, set, test, exec)              \
                          {name, 0, get, set, test, exec},

/**
 * eg:
 * LN_AT_CMD_REG(USR_CMD_NAME, ln_usr_cmd_get, ln_usr_cmd_set, ln_usr_cmd_test, ln_usr_cmd_exec)
 *
 * Note: USR_CMD_NAME does not require quotes!!!
*/
#define LN_AT_CMD_REG(name, get, set, test, exec)                   \
    LN_AT_USED static const ln_at_cmd_item_t _ln_at_cmd_item_##name \
    LN_AT_SECTION("ln_at_cmd_tbl") =                                \
    {                                                               \
        #name,                                                      \
        0,                                                          \
        get,                                                        \
        set,                                                        \
        test,                                                       \
        exec                                                        \
    }

typedef struct _ln_at_transfer
{
    int  (*init)(void);   /* init */
    int  (*deinit)(void); /* deinit */
    int  (*putc)(char c); /* putc */
    char (*getc)(void);   /* getc, In this func, will wait until getting a new char */
    int  (*write)(const char *buff, size_t size);

    void     *sem;
    void     *thread;
    uint32_t  status; /* 0:not init; 1:init */
} ln_at_transfer_t;

typedef struct
{
    ln_at_cmd_item_t *flash_cmd_tbl;       /* at command table, cmd is saved in flash */
    uint32_t         *flash_cmd_hash_tbl;  /* The hash table corresponding to the command in order */
    uint32_t          flash_cmd_num;       /* total at command number that saved in flash */
} ln_at_cmd_table_t;

typedef enum
{
    LN_AT_STATUS_IDLE = 0,
    LN_AT_STATUS_BUSY = 1,
} ln_at_status_t;

typedef enum
{
    LN_AT_WORK_MODE_CMD              = 0, /* default */
    LN_AT_WORK_MODE_DATA_NORMAL      = 1,
    LN_AT_WORK_MODE_DATA_PASSTHROUGH = 2,
} ln_at_curr_mode_t;

typedef struct
{
    char    *data;
    uint32_t len;
} ln_at_c2s_data_t; /* host mcu --> at module */

struct _ln_at_server_handle
{
    void *at_thr;
    void *at_lock;      /* mutex or semphore */
    void *at_print_lock;

    ln_at_transfer_t  *transfer;
    ln_at_cmd_table_t *tbl;
    ln_at_curr_mode_t  curr_mode;
    ln_at_status_t     status;

    uint32_t index;
    uint32_t echo:8;
    bool     is_send_busy;

    char in_buff[LN_AT_MAX_LINE_SIZE];
};
typedef struct _ln_at_server_handle ln_at_server_t;

#define LN_AT_PARSER_BUSY_SET()                      \
    do                                               \
    {                                                \
        ln_at_server_t *server = ln_at_server_get(); \
        ln_at_lock();                                \
        server->status = LN_AT_STATUS_BUSY;          \
        ln_at_unlock();                              \
    } while(0)

#define LN_AT_PARSER_BUSY_CLR()                      \
    do                                               \
    {                                                \
        ln_at_server_t *server = ln_at_server_get(); \
        ln_at_lock();                                \
        server->status = LN_AT_STATUS_IDLE;          \
        ln_at_unlock();                              \
    } while(0)

#define LN_AT_CURR_MODE_SET(mode)                    \
    do {                                             \
        ln_at_server_t *server = ln_at_server_get(); \
        ln_at_lock();                                \
        server->curr_mode = mode;                    \
        ln_at_unlock();                              \
    } while(0)

#define LN_AT_RECV_LEN_GET() ({                      \
    uint32_t len = 0;                                \
    do {                                             \
        ln_at_server_t *server = ln_at_server_get(); \
        ln_at_lock();                                \
        len = server->index;                         \
        ln_at_unlock();                              \
    } while(0);                                      \
    len;                                             \
    })

#define LN_AT_RESET_PARSER_ENV()                               \
    do                                                         \
    {                                                          \
        ln_at_server_t *server = ln_at_server_get();           \
        ln_at_lock();                                          \
        server->index = 0;                                     \
        memset(server->in_buff, 0x0, sizeof(server->in_buff)); \
        ln_at_unlock();                                        \
    } while(0)

#define LN_AT_PARA_NUM_CHECK(real_num, in_num)  \
    do {                                        \
        if (real_num != in_num)                 \
        {                                       \
            return LN_AT_ERR_PARAM;             \
        }                                       \
    } while (0)

#define LN_AT_PARA_POINTER_CHECK(ptr)           \
    do {                                        \
        if (!ptr)                               \
        {                                       \
            return LN_AT_ERR_PARAM;             \
        }                                       \
    } while (0)

#if LN_AT_USING_SERVER

/**
 * 0:success; others:failed
*/
typedef int (*send_notify_cb_t)(int link_id);

int                 ln_at_init(void);
ln_at_server_t     *ln_at_server_get(void);
ln_at_err_t         ln_at_server_init(ln_at_transfer_t *transfer, ln_at_cmd_table_t *table, uint8_t echo);
ln_at_err_t         ln_at_cmd_regist(ln_at_cmd_item_t *item); /* Register new commands dynamically */
ln_at_err_t         ln_at_cmd_table_set(ln_at_cmd_table_t *table);
ln_at_cmd_table_t  *ln_at_cmd_table_get(void);
uint32_t            ln_djb_hash(const char *str);

/**
 * @param en: 0-disable; 1-enable
*/
void    ln_at_echo_enable(uint8_t en);
void    ln_at_printf(const char *format, ...);
size_t  ln_at_vprintf(const char *format, va_list args);
void    ln_at_hex_print(uint8_t *buf, size_t size, const char *format, ...);
int     ln_at_cmd_args_get(const char *args_string, const char *expr, ...);
void    ln_at_preparser(char ch);
void    ln_at_server_recv_len_get(ln_at_c2s_data_t *data);
void    ln_at_curr_link_info_update(int link_id, send_notify_cb_t fnp_notify_cb, ln_at_curr_mode_t mode, uint32_t need_data_len);
#endif

#endif /* __LN_AT_H__ */
