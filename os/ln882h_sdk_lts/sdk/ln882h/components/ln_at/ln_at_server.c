/**
 * @file   ln_at_server.c
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-28     MurphyZhao   the first version
 */

#include "ln_at.h"
#include "ln_utils.h"
#include "ln_at_adapt.h"
#include "parser/ln_at_parser.h"
#include <ctype.h>          /* c stdlib file */
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#if LN_AT_USING_SERVER

#define SPECIAL_KEY_ESC              0x1B
#define SPECIAL_KEY_BACKSPACE        0x08
#define SPECIAL_KEY_DELETE           0x7F

#define BACKSPACE_CONTENT            "\b \b"
#define CR_CONTENT                   '\r'
#define LF_CONTENT                   '\n'
#define CRLF_CONTENT                 "\r\n"
#define BUSY_P_CONTENT               "\r\nbusy p...\r\n"
#define BUSY_S_CONTENT               "\r\nbusy s...\r\n"

#ifndef LN_AT_CMD_END_MARK
#define LN_AT_CMD_END_MARK LN_AT_CMD_END_MARK_CRLF
#endif

static ln_at_server_t g_at_server;
static char s_at_log_buff[LN_AT_PRINTF_BUF_SIZE] = {0,};

uint8_t at_server_busy_send_flag=0;

#ifdef LN_AT_USING_AT_DATA_TRANSMIT

#define LN_AT_DATA_TRANSMIT_TIMEOUT   (20)   /* uint: ms */
#define LN_AT_DATA_THREE_PLUS_TIMEOUT (1000) /* uint: ms */

#ifndef LN_AT_CLIENT_TO_SERVER_DATA_BUFF_SIZE
#define LN_AT_CLIENT_TO_SERVER_DATA_BUFF_SIZE (2048u)
#endif

#ifndef LN_AT_C_TO_S_PKT_MAX_SIZE
#define LN_AT_C_TO_S_PKT_MAX_SIZE (2048u)
#endif

#if (LN_AT_C_TO_S_PKT_MAX_SIZE > LN_AT_CLIENT_TO_SERVER_DATA_BUFF_SIZE)
#error "Error config of LN_AT_C_TO_S_PKT_MAX_SIZE"
#endif

typedef struct
{
    int              link_id;
    uint32_t         need_data_len; /* link task set the need_data_len */
    send_notify_cb_t parser_notify_link_cb;
} ln_at_curr_link_info_t;

static ln_at_curr_link_info_t s_at_curr_link_info = {0};
static char s_at_client_to_server_data_buff[LN_AT_CLIENT_TO_SERVER_DATA_BUFF_SIZE];

static void *s_transmit_timer          = NULL;
static bool  s_is_first_in_passthrough = true;
static bool  s_received_3plus          = false;
static bool  s_is_true_3plus           = false;

#define AT_CURR_LINK_INFO_RESET()                                           \
    do {                                                                    \
        memset(&s_at_curr_link_info, 0x0, sizeof(ln_at_curr_link_info_t));  \
    } while(0)

#endif /* LN_AT_USING_AT_DATA_TRANSMIT */

static void *g_ln_at_parser_sem = NULL;
static void ln_at_parser_task(void *arg);
static void ln_at_parser_notify(void);

#define LN_AT_PRINT_LOCK()                                              \
    do {                                                                \
        ln_at_sem_wait(g_at_server.at_print_lock, LN_AT_WAIT_FOREVER);  \
    } while(0)

#define LN_AT_PRINT_UNLOCK()                           \
    do {                                               \
        ln_at_sem_release(g_at_server.at_print_lock);  \
    } while(0)

ln_at_server_t *ln_at_server_get(void)
{
    return &g_at_server;
}

#ifdef LN_AT_USING_AT_DATA_TRANSMIT
void ln_at_server_recv_len_get(ln_at_c2s_data_t *data)
{
    ln_at_server_t *server = ln_at_server_get();
    ln_at_lock();
    data->len = server->index;
    data->data = s_at_client_to_server_data_buff;
    ln_at_unlock();
}
#endif

size_t ln_at_vprintf(const char *format, va_list args)
{
    uint32_t len = 0;
    int      ret = 0;

    ln_at_server_t   *server   = &g_at_server;
    ln_at_transfer_t *transfer = server->transfer;

    len = vsnprintf(s_at_log_buff, sizeof(s_at_log_buff), format, args);
    ret = (transfer->write)((const void *)s_at_log_buff, len);
    return ret;
}

void ln_at_printf(const char *format, ...)
{
    va_list args;

    LN_AT_PRINT_LOCK();
    va_start(args, format);

    ln_at_vprintf(format, args);

    va_end(args);
    LN_AT_PRINT_UNLOCK();
}

void ln_at_hex_print(uint8_t *buf, size_t size, const char *format, ...)
{
    va_list args;
    ln_at_server_t   *server   = &g_at_server;
    ln_at_transfer_t *transfer = server->transfer;

    LN_AT_PRINT_LOCK();
    va_start(args, format);
    ln_at_vprintf(format, args);
    va_end(args);

    (transfer->write)((const void *)buf, size);
    (transfer->write)("\r\n", 2);
    LN_AT_PRINT_UNLOCK();
}

int ln_at_cmd_args_get(const char *args_string, const char *expr, ...)
{
    va_list args;
    int num = 0;

    if (!args_string || !expr)
        return -1;

    va_start(args, expr);
    num = vsscanf(args_string, expr, args);
    va_end(args);

    return num;
}

/**
 * @brief hash type:Daniel J. Bernstein hash algorithm.
*/
uint32_t ln_djb_hash(const char *str)
{
    uint32_t hash = 5381;
    int c = *str;

    while(c)
    {
        hash = ((hash << 5) + hash) + (tolower(c)); /* hash * 33 + c */
        str++;
        c = *str;
    }
    return hash;
}

#ifdef LN_AT_USING_AT_DATA_TRANSMIT
static void ln_at_transmit_timer_cb(void *arg)
{
    ln_at_server_t         *server    = &g_at_server;
    ln_at_curr_link_info_t *link_info = &s_at_curr_link_info;

    if (s_received_3plus)
    {
        LN_AT_PARSER_BUSY_SET();
        server->curr_mode = LN_AT_WORK_MODE_CMD;
        server->index = 0;
        s_is_first_in_passthrough = true;
        LN_AT_PARSER_BUSY_CLR();
    }
    else
    {
        if (link_info->parser_notify_link_cb)
        {
            /* set send busy */
            server->is_send_busy = true;
            link_info->parser_notify_link_cb(s_at_curr_link_info.link_id);
        }
        else
        {
            server->index = 0;
        }
        s_is_first_in_passthrough = true;
    }
}
#endif /* LN_AT_USING_AT_DATA_TRANSMIT */

ln_at_err_t ln_at_server_init(ln_at_transfer_t *transfer, ln_at_cmd_table_t *table, uint8_t echo)
{
    int ret = 0;
    ln_at_err_t rst = LN_AT_ERR_NONE;

    ln_at_server_t *server = &g_at_server;
    LN_AT_MEMSET(server, 0x0, sizeof(ln_at_server_t));

    if (transfer)
        server->transfer = transfer;
    else
        return LN_AT_ERR_PARAM;

    if (table)
        server->tbl = table;
    else
        return LN_AT_ERR_PARAM;

    server->at_lock = ln_at_lock_create();
    if (server->at_lock == NULL)
    {
        rst = LN_AT_ERR_SYS;
        goto __exit;
    }

    server->at_print_lock = ln_at_sem_create(1, 1);
    if (server->at_print_lock == NULL)
    {
        rst = LN_AT_ERR_SYS;
        goto __exit;
    }

    server->echo = echo;

    if (transfer->init)
    {
        ret = (transfer->init)();
        if (ret != 0)
        {
            rst = LN_AT_ERR_COMMON;
            goto __exit;
        }
    }

#ifdef LN_AT_USING_AT_DATA_TRANSMIT

    AT_CURR_LINK_INFO_RESET();
    if (NULL == (s_transmit_timer = ln_at_timer_create(LN_AT_TIMER_ONCE,
            ln_at_transmit_timer_cb, &s_at_curr_link_info, LN_AT_DATA_TRANSMIT_TIMEOUT)))
    {
        rst = LN_AT_ERR_SYS;
        goto __exit;
    }
#endif /* LN_AT_USING_AT_DATA_TRANSMIT */

    if (NULL == (g_ln_at_parser_sem = ln_at_sem_create(0,2)))
    {
        rst = LN_AT_ERR_SYS;
        goto __exit;
    }

    if(ln_at_task_create("AT_Parser",
                (void *)ln_at_parser_task,
                NULL,
                LN_AT_TRANSFER_TASK_PRI,
                LN_AT_TRANSFER_TASK_STACK_SIZE) == NULL)
    {
        rst = LN_AT_ERR_SYS;
        goto __exit;
    }

    return LN_AT_ERR_NONE;

__exit:
    if (server->at_lock)
    {
        ln_at_lock_delete();
    }

    if (server->at_print_lock)
    {
        ln_at_sem_delete(server->at_print_lock);
    }

#ifdef LN_AT_USING_AT_DATA_TRANSMIT
    if (s_transmit_timer)
    {
        ln_at_timer_delete(s_transmit_timer);
        s_transmit_timer = NULL;
    }
#endif /* LN_AT_USING_AT_DATA_TRANSMIT */

    if (g_ln_at_parser_sem)
    {
        ln_at_sem_delete(g_ln_at_parser_sem);
        g_ln_at_parser_sem = NULL;
    }
    return rst;
}

ln_at_cmd_table_t *ln_at_cmd_table_get(void)
{
    ln_at_server_t *server = &g_at_server;
    return server->tbl;
}

static ln_at_cmd_item_t *ln_at_query_cmd(ln_at_server_t *server, uint32_t hash_code)
{
    ln_at_cmd_item_t *item;
    uint32_t index = 0;

    if (!server->tbl->flash_cmd_tbl || !server->tbl->flash_cmd_hash_tbl || server->tbl->flash_cmd_num == 0)
    {
        goto __exit;
    }

    for (index = 0; index < server->tbl->flash_cmd_num; index++)
    {
        if(server->tbl->flash_cmd_hash_tbl[index] == hash_code)
        {
            item = &server->tbl->flash_cmd_tbl[index];
            return item;
        }
    }

__exit:
    LN_AT_LOG_E("Not found at cmd!\r\n");
    return NULL;
}

static ln_at_err_t ln_at_cmd_do(ln_at_server_t *server, ln_at_parser_rst_t *parser_rst)
{
    ln_at_err_t result = LN_AT_ERR_COMMON;
    uint32_t hash_code = 0;
    ln_at_cmd_item_t *item;

    if (parser_rst->type != LN_AT_PSR_CMD_TYPE_BASIC)
    {
        hash_code = ln_djb_hash(parser_rst->name + sizeof(char));
    }
    else
    {
        hash_code = ln_djb_hash(parser_rst->name);
    }

    if ((item = ln_at_query_cmd(server, hash_code)) == NULL)
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        ln_at_printf("\r\nNot found <%s> command!\r\n",
                parser_rst->name + sizeof(char));
        return LN_AT_ERR_NOT_FIND;
    }

    switch(parser_rst->type)
    {
        case LN_AT_PSR_CMD_TYPE_GET:
            if(!item->get)
            {
                ln_at_printf(LN_AT_RET_ERR_STR);
            }
            else if((item->get)(parser_rst->name) == LN_AT_ERR_NONE)
            {
                result = LN_AT_ERR_NONE;
            }
            break;
        case LN_AT_PSR_CMD_TYPE_SET:
            if(!item->set)
            {
                ln_at_printf(LN_AT_RET_ERR_STR);
            }
            else if((item->set)(parser_rst->para_num, parser_rst->name) == LN_AT_ERR_NONE)
            {
                result = LN_AT_ERR_NONE;
            }
            break;
        case LN_AT_PSR_CMD_TYPE_TEST:
            if(!item->test)
            {
                ln_at_printf(LN_AT_RET_ERR_STR);
            }
            else if((item->test)(parser_rst->name) == LN_AT_ERR_NONE)
            {
                result = LN_AT_ERR_NONE;
            }
            break;
        case LN_AT_PSR_CMD_TYPE_BASIC:
        case LN_AT_PSR_CMD_TYPE_EXEC:
            if(!item->exec)
            {
                ln_at_printf(LN_AT_RET_ERR_STR);
            }
            else if((item->exec)(parser_rst->name) == LN_AT_ERR_NONE)
            {
                result = LN_AT_ERR_NONE;
            }
            break;
        default:
            ln_at_printf(LN_AT_RET_ERR_STR);
            result = LN_AT_ERR_COMMON;
    }

    return result;
}

#ifdef LN_AT_USING_AT_DATA_TRANSMIT
/**
 * called by:
 * AT+CIPSEND
 * AT+CIPSEND=[link id,]<len>
 * AT+CIPSENDEX
 * AT+MQTTPUBRAW
*/
void ln_at_curr_link_info_update(int link_id, send_notify_cb_t fnp_notify_cb, ln_at_curr_mode_t mode, uint32_t need_data_len)
{
    /* LOCK */
    ln_at_lock();
    ln_at_server_t *server = ln_at_server_get();
    server->curr_mode      = mode;

    s_at_curr_link_info.parser_notify_link_cb = fnp_notify_cb;
    s_at_curr_link_info.need_data_len         = need_data_len;
    s_at_curr_link_info.link_id               = link_id;
    ln_at_unlock();
    /* UNLOCK */
}

void ln_at_handle_data_mode(ln_at_curr_mode_t curr_mode, char ch)
{
    ln_at_server_t *server = &g_at_server;
    ln_at_curr_link_info_t *link_info = &s_at_curr_link_info;
    static char curr_ch = 0, prev_ch = 0, prev_prev_ch = 0;

    s_at_client_to_server_data_buff[server->index++] = ch;
    prev_prev_ch = prev_ch;
    prev_ch      = curr_ch;
    curr_ch      = ch;

    if (curr_mode == LN_AT_WORK_MODE_DATA_NORMAL)
    {
        LN_AT_LOG_E("\r\nData mode: Normal\r\n");
        if (server->index >= link_info->need_data_len)
        {
            LOG(LOG_LVL_ERROR, "Recv OK! len:%d; need len:%d\r\n",
                server->index, link_info->need_data_len);
            if (link_info->parser_notify_link_cb)
            {
                link_info->parser_notify_link_cb(s_at_curr_link_info.link_id);
            }
            else
            {
                server->index = 0;
            }

            // server->index = 0; // should be clear when data send success
            server->curr_mode = LN_AT_WORK_MODE_CMD;
        }
    }
    else if (curr_mode == LN_AT_WORK_MODE_DATA_PASSTHROUGH)
    {
        LN_AT_LOG_E("\r\nData mode: PassThrough\r\n");

        if (s_is_first_in_passthrough)
        {
            s_is_first_in_passthrough = false;
            ln_at_timer_start(s_transmit_timer);
        }

        if (ch == '+' && prev_ch == '+' && prev_prev_ch == '+' && server->index == 3)
        {
            s_received_3plus = true;
            ln_at_timer_update_period(s_transmit_timer, LN_AT_DATA_THREE_PLUS_TIMEOUT);
        }
        else if (!s_is_true_3plus)
        {
            s_received_3plus = false;
            ln_at_timer_update_period(s_transmit_timer, LN_AT_DATA_TRANSMIT_TIMEOUT);
        }

        if (server->index >= LN_AT_C_TO_S_PKT_MAX_SIZE)
        {
            LN_AT_LOG_E("\r\nAT UART has received one packet!len:%d\r\n",
                server->index);
            if (link_info->parser_notify_link_cb)
            {
                link_info->parser_notify_link_cb(s_at_curr_link_info.link_id);
            }
            else
            {
                server->index = 0;
            }
            // server->index = 0; // should be clear when data send success
        }
    }
}
#endif /* LN_AT_USING_AT_DATA_TRANSMIT */

void ln_at_preparser(char ch)
{
    static char prev_ch = 0;
    ln_at_status_t status;
#ifdef LN_AT_USING_AT_DATA_TRANSMIT
    ln_at_curr_mode_t curr_mode;
#endif

    ln_at_server_t *_server = &g_at_server;
    ln_at_transfer_t *transfer = _server->transfer;

    if (!transfer || !transfer->getc || !transfer->write)
    {
        LN_AT_LOG_E("[F:%S L:%d] Error param! Check to see if 'ln_at_server_init' is called before here. \r\n", __func__, __LINE__);
        return;
    }
    LN_AT_LOG_D("char:0x%02x-%c\r\n", ch, ch);

    ln_at_lock();
    status = _server->status;
    ln_at_unlock();

    if (status == LN_AT_STATUS_BUSY) /* BUSY */
    {
			//add 2023.02.08 add busy flag
			if(0==at_server_busy_send_flag)
			{
        transfer->write(BUSY_P_CONTENT, strlen(BUSY_P_CONTENT));
				at_server_busy_send_flag=1;
        return;
			}
    }

#ifdef LN_AT_USING_AT_DATA_TRANSMIT
    ln_at_lock();
    curr_mode = _server->curr_mode;
    ln_at_unlock();

    if (curr_mode != LN_AT_WORK_MODE_CMD)
    {
        ln_at_handle_data_mode(curr_mode, ch);
        return;
    }
#endif /* LN_AT_USING_AT_DATA_TRANSMIT */

//    LN_AT_LOG_E("CMD MODE\r\n");

    if (isprint(ch))        /* normal character(0x20-0x7e), Not include '\0' */
    {
        if (_server->echo)
        {
            transfer->putc(ch);
        }
        _server->in_buff[_server->index++] = ch;
    }
    else if (ch == SPECIAL_KEY_ESC)
    {
        _server->index = 0;
        LN_AT_MEMSET(_server->in_buff, 0x0, LN_AT_MAX_LINE_SIZE);
        transfer->write(CRLF_CONTENT, strlen(CRLF_CONTENT));
        return;
    }
    else if (ch == SPECIAL_KEY_BACKSPACE || ch == SPECIAL_KEY_DELETE)
    {
        if (_server->index > 0)
        {
            _server->index --;
            _server->in_buff[_server->index] = 0;
            if (_server->echo)
                transfer->write(BACKSPACE_CONTENT, strlen(BACKSPACE_CONTENT));
        }
        return;
    }
    else if (ch == CR_CONTENT || ch == LF_CONTENT)
    {
        if (_server->echo && prev_ch != CR_CONTENT) {
            transfer->write(CRLF_CONTENT, strlen(CRLF_CONTENT));
        }

        _server->in_buff[_server->index++] = ch;

        if(_server->index > 2 && NULL != strstr(_server->in_buff, LN_AT_CMD_END_MARK))
        {
            LN_AT_PARSER_BUSY_SET();
            ln_at_parser_notify();
        }
    }

    prev_ch = ch;

    if (_server->index >= LN_AT_MAX_LINE_SIZE)
    {
        prev_ch = 0;
        LN_AT_RESET_PARSER_ENV();
    }
}

static void ln_at_parser_notify(void)
{
    ln_at_sem_release(g_ln_at_parser_sem);
}

static void ln_at_parser_task(void *arg)
{
    LN_UNUSED(arg);

    ln_at_server_t *_server = &g_at_server;
    ln_at_parser_rst_t parser_rst;
    while(1)
    {
        if (0 == ln_at_sem_wait(g_ln_at_parser_sem, LN_AT_WAIT_FOREVER))
        {
            ln_at_cmd_parse(_server->in_buff, _server->index); /* "ATxxx\r\n" */

            ln_at_parser_get_result(&parser_rst);
            ln_at_cmd_do(_server, &parser_rst);

            LN_AT_RESET_PARSER_ENV();
            LN_AT_PARSER_BUSY_CLR();
						at_server_busy_send_flag=0;
        }
    }
}

/**
 * @param en: 0-disable; 1-enable
*/
void ln_at_echo_enable(uint8_t en)
{
    ln_at_server_t *server = &g_at_server;
    server->echo = en;
}

#endif /* #if LN_AT_USING_SERVER */
