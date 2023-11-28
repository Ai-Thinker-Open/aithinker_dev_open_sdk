/**
 * @file   ln_at_parser.c
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-01-22     MurphyZhao   the first version
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "ln_at_parser.h"
#include "ln_at_cfg.h"

#ifndef LN_AT_CMD_END_MARK
#define LN_AT_CMD_END_MARK LN_AT_CMD_END_MARK_CRLF
#endif

#ifndef LN_AT_PARSER_BUILD_LIB
  #include "ln_at_adapt.h"
#else
  #define LN_AT_LOG_D(...)
  #define LN_AT_LOG_E(...)
#endif /* LN_AT_PARSER_BUILD_LIB */

#define LN_AT_PARSER_LOG_D(fmt, ...)   //LN_AT_LOG_D(fmt"\r\n", ##__VA_ARGS__)
#define LN_AT_PARSER_LOG_E(fmt, ...)   //LN_AT_LOG_E(fmt"\r\n", ##__VA_ARGS__)

#ifndef LN_AT_MAX_PARA_NUM
#define LN_AT_MAX_PARA_NUM 20
#endif

#define LN_AT_CMD_FILTER_SPACE

typedef enum ln_at_para_type
{
    LN_AT_PARA_TYPE_INIT    = 0,
    LN_AT_PARA_TYPE_STR     = 1,
    LN_AT_PARA_TYPE_OMIT    = 2,
    LN_AT_PARA_TYPE_UNKNOWN = 3,
} ln_at_para_type_t;

typedef struct _at_para
{
    ln_at_para_type_t para_type;

    char *para_start_pos; /* filter start char '"' and end char ", if param is string type. */
    char *para_end_pos;   /* filter start char '"' and end char ", if param is string type. */
} ln_at_para_t;

typedef struct 
{
    char *start_pos;
    char *end_pos;   /* (*end_pos == '\0') */
} ln_at_cmd_name_t;  /* AT+TEST=xxx, CMD NAME is `+TEST` */

typedef struct
{
    ln_at_cmd_name_t name;
    uint8_t cmd_type_flag; /* basic, exec, get, test, set */
    size_t total_para_num; /* Include omit params */
    ln_at_para_t para[LN_AT_MAX_PARA_NUM]; /* Cache AT command's params */
} ln_at_parser_t;

/**
 * Cache AT instruction and cache at data
*/
static ln_at_parser_t g_ln_at_cmd_context = {0};

#define LN_AT_CMD_MARK_PLUS      (1 << 2) // +
#define LN_AT_CMD_MARK_EQUAL     (1 << 1) // =
#define LN_AT_CMD_MARK_QUESTION  (1 << 0) // ?

#define LN_AT_CMD_MARK_BASIC (0x00)                                                                 // 000: AT/ATZ/ATE
#define LN_AT_CMD_MARK_EXEC  (LN_AT_CMD_MARK_PLUS)                                                  // 100: AT+TEST
#define LN_AT_CMD_MARK_GET   (LN_AT_CMD_MARK_PLUS | LN_AT_CMD_MARK_QUESTION)                        // 101: AT+TEST?
#define LN_AT_CMD_MARK_TEST  (LN_AT_CMD_MARK_PLUS | LN_AT_CMD_MARK_EQUAL | LN_AT_CMD_MARK_QUESTION) // 111: AT+TEST=?
#define LN_AT_CMD_MARK_SET   (LN_AT_CMD_MARK_PLUS | LN_AT_CMD_MARK_EQUAL)                           // 110: AT+TEST=<xxx>

#define LN_AT_CMD_TYPE_FLAG_CLR(cmd_type_flag) do {cmd_type_flag = 0;} while(0)

#define LN_AT_ADD_PLUS_MARK(cmd_type_flag) do {cmd_type_flag |= LN_AT_CMD_MARK_PLUS;} while(0)
#define LN_AT_CLR_PLUS_MARK(cmd_type_flag) do {cmd_type_flag &= (~LN_AT_CMD_MARK_PLUS);} while(0)

#define LN_AT_ADD_EQUAL_MARK(cmd_type_flag) do {cmd_type_flag |= LN_AT_CMD_MARK_EQUAL;} while(0)
#define LN_AT_CLR_EQUAL_MARK(cmd_type_flag) do {cmd_type_flag &= (~LN_AT_CMD_MARK_EQUAL);} while(0)

#define LN_AT_ADD_QUESTION_MARK(cmd_type_flag) do {cmd_type_flag |= LN_AT_CMD_MARK_QUESTION;} while(0)
#define LN_AT_CLR_QUESTION_MARK(cmd_type_flag) do {cmd_type_flag &= (~LN_AT_CMD_MARK_QUESTION);} while(0)

ln_at_parser_err_t ln_at_cmd_parse(char *intact_line, size_t len)
{
    size_t index = 0;
    size_t new_index = 0;

    char *start_pos      = NULL;
    char *start_pos_bak  = NULL;
    char *end_pos        = NULL;

    char *equals_pos     = NULL;
    char *para_start_pos = NULL;

    char ch;
    char pre_ch;
    bool true_quotation  = false; /* 0: not found true quotation mark, 1 found true quotation mark '"' */
    size_t quotation_num = 0;
    bool is_int_param    = false;
    size_t backslash_num = 0;
    bool is_backslash    = false;
    bool is_negative_number = false;

    ln_at_parser_err_t result = LN_AT_PSR_ERR_COMMON;
    ln_at_parser_t    *at_cmd = &g_ln_at_cmd_context;

    len = len - strlen(LN_AT_CMD_END_MARK);

    /* find CRLF */
    end_pos = strstr(intact_line, LN_AT_CMD_END_MARK);
    if (!(end_pos && (end_pos == (intact_line + len))))
    {
        result = LN_AT_PSR_ERR_NO_CRLF;
        LN_AT_PARSER_LOG_E("Not found CRLF");
        goto __exit;
    }

    LN_AT_PARSER_LOG_D("intact_line:0x%p, end_pos:0x%p, len:%d", intact_line, end_pos, len);

    /* find "AT" or "at" */
    start_pos     = strstr(intact_line, "AT");
    start_pos_bak = strstr(intact_line, "at");
    start_pos     = (start_pos ? start_pos : (start_pos_bak ? start_pos_bak : NULL));
    if(!start_pos)
    {
        result = LN_AT_PSR_ERR_NO_AT;
        LN_AT_PARSER_LOG_E("Not found AT");
        goto __exit;
    }

    memset(at_cmd, 0x0, sizeof(ln_at_parser_t));

    {
        #undef SPECIAL_CMD
        #define SPECIAL_CMD "+PVTCMD"
        char *pos = NULL;
        if ((pos = strstr(intact_line, SPECIAL_CMD)) != NULL)
        {
            /* find PVTCMD */
            at_cmd->name.start_pos = pos;
            at_cmd->name.end_pos   = pos + strlen(SPECIAL_CMD);
            *at_cmd->name.end_pos = '\0';
            at_cmd->total_para_num = 1;
            at_cmd->cmd_type_flag = LN_AT_CMD_MARK_SET;
            at_cmd->para[at_cmd->total_para_num - 1].para_start_pos = at_cmd->name.end_pos + 1;
            at_cmd->para[at_cmd->total_para_num - 1].para_end_pos   = end_pos;
            *end_pos = '\0';
            at_cmd->para[at_cmd->total_para_num - 1].para_type = LN_AT_PARA_TYPE_STR;
            result = LN_AT_PSR_ERR_NONE;
            goto __exit;
        }
    }


    /* Filters Spaces and checks for pairs of double quotes */
    LN_AT_PARSER_LOG_D("Before Filter data:%s", start_pos);

    {
        new_index      = 0;
        quotation_num  = 0;
        true_quotation = false;
        start_pos_bak  = start_pos;

        while(len--)
        {
            ch = *start_pos_bak++;
            // LN_AT_PARSER_LOG_D("start_pos_bak:%p, index:%d, ch:%c  flag:%d  prev ch:%c\r\n", 
            //         start_pos_bak, index, ch, true_quotation, *(start_pos_bak-2));

            if (ch != ' ')
            {
                if (ch == '"' && (*(start_pos_bak - 2) != '\\'))
                {
                    true_quotation = !true_quotation;
                    quotation_num++;
                }
                start_pos[index] = ch;
                index++;
                continue;
            }
            else
            {
                if (true_quotation)
                {
                    start_pos[index] = ch;
                    index++;
                    continue;
                }
                else
                {
                    continue;
                }
            }
        }

        if ((quotation_num % 2) != 0) /* Not a pair of quotation mark */
        {
            result = LN_AT_PSR_ERR_FORMAT;
            LN_AT_PARSER_LOG_E("AT format error!");
            goto __exit;
        }

        start_pos[index] = '\0';
        len = index;
    }
    LN_AT_PARSER_LOG_D("After Filter data:%s\r\n new len:%d", start_pos, len);

    /**
     * Effective length after filtering whitespace
    */
    end_pos = start_pos + len; /* [start_pos, end_pos) */

    /* parse cmd type and cmd name */
    LN_AT_CMD_TYPE_FLAG_CLR(at_cmd->cmd_type_flag);
    if (len < 2)
    {
        result = LN_AT_PSR_ERR_NO_AT;
        LN_AT_PARSER_LOG_E("Not found AT");
        goto __exit;
    }
    else if (len == 2) /* AT */
    {
        LN_AT_PARSER_LOG_D("AT cmd");
        at_cmd->name.start_pos = start_pos;
        at_cmd->name.end_pos   = &start_pos[strlen(start_pos)];
    }
    else
    {
        if (start_pos[2] != '+') /* AT+XXX, Then parse '='、'?' and params */
        {
            at_cmd->name.start_pos = start_pos;
            at_cmd->name.end_pos   = &start_pos[strlen(start_pos)];
        }
        else
        {
            LN_AT_ADD_PLUS_MARK(at_cmd->cmd_type_flag);

            equals_pos = strchr(start_pos, '=');
            if (equals_pos)
            {
                LN_AT_ADD_EQUAL_MARK(at_cmd->cmd_type_flag);

                at_cmd->name.start_pos = &start_pos[2];
                at_cmd->name.end_pos   = equals_pos;
            }
            else if (start_pos[strlen(start_pos) - 1] == '?') /* AT+TEST? */
            {
                LN_AT_ADD_QUESTION_MARK(at_cmd->cmd_type_flag);

                at_cmd->name.start_pos = &start_pos[2];
                at_cmd->name.end_pos   = &start_pos[strlen(start_pos) - 1];
            }
            else /* AT+TEST */
            {
                at_cmd->name.start_pos = &start_pos[2];
                at_cmd->name.end_pos   = &start_pos[strlen(start_pos)];
            }

            if (start_pos[strlen(start_pos) - 1] == '?' &&
                start_pos[strlen(start_pos) - 2] == '=') /* exclude error at cmd: AT+TEST=1,2,? */ /* AT+TEST=? */
            {
                LN_AT_ADD_QUESTION_MARK(at_cmd->cmd_type_flag);
            }
        }
    }
    LN_AT_PARSER_LOG_D("cmd_type_flag: %d\r\ncmd name:%.*s\r\n", 
        at_cmd->cmd_type_flag, at_cmd->name.end_pos - at_cmd->name.start_pos, at_cmd->name.start_pos);

    if (at_cmd->cmd_type_flag != LN_AT_CMD_MARK_SET) /* set */
    {
        at_cmd->name.start_pos[at_cmd->name.end_pos - at_cmd->name.start_pos] = '\0';

        result = LN_AT_PSR_ERR_NONE;
        LN_AT_PARSER_LOG_E("AT CMD Parse success!");
        goto __exit;
    }
    LN_AT_PARSER_LOG_D("CMD TYPE: SET");

    /* parse cmd args, parse escape character */
    para_start_pos = at_cmd->name.end_pos + 1;  /* skip '=' */
    LN_AT_PARSER_LOG_D("CMD para: %s", para_start_pos);

    LN_AT_PARSER_LOG_D("\r\n--------------------\r\n");

    LN_AT_PARSER_LOG_D("Start parse params...");

    start_pos_bak  = para_start_pos;
    true_quotation = false;
    is_int_param   = false;
    backslash_num  = 0;

    at_cmd->total_para_num = 1;
    quotation_num = 0;
    index = 0;

    if (*end_pos != '\0')
    {
        result = LN_AT_PSR_ERR_PARSER;
        LN_AT_PARSER_LOG_E("Parser error or at cmd format error!");
        goto __exit;
    }

    while(start_pos_bak <= end_pos)
    {
        if (at_cmd->total_para_num > LN_AT_MAX_PARA_NUM)
        {
            result = LN_AT_PSR_ERR_TOO_MANY_PARA;
            LN_AT_PARSER_LOG_E("Too many parameters!");
            goto __exit;
        }

        ch = *start_pos_bak++;
        index ++;
        // LN_AT_PARSER_LOG_D("index:%d, ch:%c  flag:%d  prev ch:%c  quotation_num:%d, para num:%d\r\n", 
        //         index, ch, true_quotation, *(start_pos_bak-2), quotation_num, at_cmd->total_para_num);

        if (ch == '"' && (*(start_pos_bak - 2) != '\\')) /* string param */
        {
            true_quotation = !true_quotation;
            quotation_num++;
            if (quotation_num % 2 != 0)
            {
                at_cmd->para[at_cmd->total_para_num - 1].para_start_pos = start_pos_bak;
                at_cmd->para[at_cmd->total_para_num - 1].para_type = LN_AT_PARA_TYPE_STR;

                LN_AT_PARSER_LOG_D("set string param start");
            }
            else
            {
                LN_AT_PARSER_LOG_D("set string param end");
                at_cmd->para[at_cmd->total_para_num - 1].para_end_pos = start_pos_bak - 1; /* [start,end) */
            }

            continue;
        }

        if (true_quotation)
        {
            continue;
        }
        else
        {
            LN_AT_PARSER_LOG_D("not in \"\"");

            if((ch >= '0' && ch <= '9')) /* int param */
            {
                if (!is_int_param)
                {
                    is_int_param = !is_int_param;
                    if (is_negative_number)
                    {
                        at_cmd->para[at_cmd->total_para_num - 1].para_start_pos = start_pos_bak - 2;
                    }
                    else
                    {
                        at_cmd->para[at_cmd->total_para_num - 1].para_start_pos = start_pos_bak - 1;
                    }

                    at_cmd->para[at_cmd->total_para_num - 1].para_type = LN_AT_PARA_TYPE_INIT;

                    LN_AT_PARSER_LOG_D("set int param start");
                }
            }
            else if (ch == '-')
            {
                is_negative_number = true;
            }
            else if (ch == '+')
            {
                is_negative_number = false;
            }
            else if (ch == ',' || ch == '\0')
            {
                if (is_int_param)
                {
                    is_int_param = !is_int_param;
                    is_negative_number = false;
                    at_cmd->para[at_cmd->total_para_num - 1].para_end_pos = start_pos_bak - 1; /* [start,end) */

                    LN_AT_PARSER_LOG_D("set int param end");
                }

                pre_ch = *(start_pos_bak - 2);

                if ((pre_ch != '"') && (!(pre_ch >= '0' && pre_ch <= '9')))
                {
                    LN_AT_PARSER_LOG_D("is omit para");
                    at_cmd->para[at_cmd->total_para_num - 1].para_type = LN_AT_PARA_TYPE_OMIT;
                }
                at_cmd->total_para_num++;

                if (ch == '\0')
                {
                    LN_AT_PARSER_LOG_D("Find end str \\0");
                }

                continue;
            }
            else /* error status */
            {
                result = LN_AT_PSR_ERR_FORMAT;
                LN_AT_PARSER_LOG_E("AT format error! ch:%c,%d", ch, ch);
                goto __exit;
            }
            continue;
        }
    }

    at_cmd->total_para_num--;

    LN_AT_PARSER_LOG_D("total param num:%d, quotation_num:%d", at_cmd->total_para_num, quotation_num);

    LN_AT_PARSER_LOG_D("\r\nBefore filter param list:");
    for (index = 0; index < at_cmd->total_para_num; index++)
    {
        if (at_cmd->para[index].para_type == LN_AT_PARA_TYPE_OMIT)
        {
            LN_AT_PARSER_LOG_D("<%02d> type:%s, value:default", index+1, "OMIT");
        }
        else
        {
            LN_AT_PARSER_LOG_D("<%02d> type:%s, value:%.*s", 
                index+1, (at_cmd->para[index].para_type == LN_AT_PARA_TYPE_INIT ? "INT":"STR"),
                at_cmd->para[index].para_end_pos - at_cmd->para[index].para_start_pos,
                at_cmd->para[index].para_start_pos);
        }
    }
    LN_AT_PARSER_LOG_D("\r\n++++++++++++++++++\r\n");

    LN_AT_PARSER_LOG_D("Parse every param:\r\n");

    for (index = 0; index < at_cmd->total_para_num; index++)
    {
        if (at_cmd->para[index].para_type == LN_AT_PARA_TYPE_STR)
        {
            /* remove escape character '\' */
            new_index     = 0;
            backslash_num = 0;
            is_backslash  = false;
            start_pos_bak = start_pos = at_cmd->para[index].para_start_pos;

            while(start_pos_bak < at_cmd->para[index].para_end_pos)
            {
                ch = *start_pos_bak++;
                if (ch == '\\')
                {
                    is_backslash = !is_backslash;
                    backslash_num++;
                }

                if (is_backslash)
                {
                    is_backslash = !is_backslash;

                    if (*start_pos_bak == '\\')
                    {
                        start_pos[new_index] = *start_pos_bak;
                        start_pos_bak++;
                        new_index++;
                        continue;
                    }
                    else if (*start_pos_bak == '"' || *start_pos_bak == ',') /* next char */
                    {
                        start_pos[new_index] = *start_pos_bak;
                        start_pos_bak++;
                        new_index++;
                        continue;
                    }
                }
                else if (ch == '"' || ch == ',')
                {
                    result = LN_AT_PSR_ERR_FORMAT;
                    LN_AT_PARSER_LOG_E("Unescaped quotes or commas");
                    goto __exit;
                }
                else
                {
                    start_pos[new_index] = ch;
                    new_index++;
                }
            }
            start_pos[new_index] = '\0';
        }
        else if (at_cmd->para[index].para_type == LN_AT_PARA_TYPE_INIT)
        {
            at_cmd->para[index].para_start_pos[at_cmd->para[index].para_end_pos - at_cmd->para[index].para_start_pos] = '\0';
        }
    }

    at_cmd->name.start_pos[at_cmd->name.end_pos - at_cmd->name.start_pos] = '\0';

    LN_AT_PARSER_LOG_D("\r\nAfter filter param list:");
    for (index = 0; index < at_cmd->total_para_num; index++)
    {
        if (at_cmd->para[index].para_type == LN_AT_PARA_TYPE_OMIT)
        {
            LN_AT_PARSER_LOG_D("<%02d> type:%s, value:default", index+1, "OMIT");
        }
        else
        {
            LN_AT_PARSER_LOG_D("<%02d> type:%s, value:%s", 
                index+1, (at_cmd->para[index].para_type == LN_AT_PARA_TYPE_INIT ? "INT":"STR"),
                at_cmd->para[index].para_start_pos);
        }
    }
    result = LN_AT_PSR_ERR_NONE;

__exit:
    LN_AT_PARSER_LOG_D("\r\n=====================\r\n");
    return result;
}

ln_at_parser_err_t ln_at_parser_get_result(ln_at_parser_rst_t *rst)
{
    ln_at_parser_t *at_cmd = &g_ln_at_cmd_context;
    if (rst == NULL) {
        return LN_AT_PSR_ERR_PARAM;
    }

    rst->name = at_cmd->name.start_pos;

    switch (at_cmd->cmd_type_flag)
    {
        case LN_AT_CMD_MARK_BASIC:
            rst->type = LN_AT_PSR_CMD_TYPE_BASIC;
            break;
        case LN_AT_CMD_MARK_EXEC:
            rst->type = LN_AT_PSR_CMD_TYPE_EXEC;
            break;
        case LN_AT_CMD_MARK_GET:
            rst->type = LN_AT_PSR_CMD_TYPE_GET;
            break;
        case LN_AT_CMD_MARK_TEST:
            rst->type = LN_AT_PSR_CMD_TYPE_TEST;
            break;
        case LN_AT_CMD_MARK_SET:
            rst->type = LN_AT_PSR_CMD_TYPE_SET;
            break;
        
        default:
            break;
    }

    rst->para_num = at_cmd->total_para_num;
    return LN_AT_PSR_ERR_NONE;
}

ln_at_parser_err_t ln_at_parser_get_int_param(uint8_t param_index, bool *is_default, int *value)
{
    ln_at_para_type_t type;
    ln_at_parser_t *at_cmd = &g_ln_at_cmd_context;
    if (!value || !is_default || (param_index < 1 || param_index > LN_AT_MAX_PARA_NUM))
    {
        return LN_AT_PSR_ERR_PARAM;
    }

    *is_default = false;
    type = at_cmd->para[param_index - 1].para_type;

    if (type == LN_AT_PARA_TYPE_OMIT)
    {
        *value = 0;
        *is_default = true;
    }
    else if (type == LN_AT_PARA_TYPE_INIT)
    {
        *value = atoi(at_cmd->para[param_index - 1].para_start_pos);
    }
    else
    {
        return LN_AT_PSR_ERR_COMMON;
    }
    return LN_AT_PSR_ERR_NONE;    
}

ln_at_parser_err_t ln_at_parser_get_str_param(uint8_t param_index, bool *is_default, char **pp_str)
{
    ln_at_para_type_t type;
    ln_at_parser_t *at_cmd = &g_ln_at_cmd_context;
    if (!pp_str || !is_default || (param_index < 1 || param_index > LN_AT_MAX_PARA_NUM))
    {
        return LN_AT_PSR_ERR_PARAM;
    }

    *is_default = false;

    type = at_cmd->para[param_index - 1].para_type;

    if (type == LN_AT_PARA_TYPE_OMIT)
    {
        *pp_str = NULL;
        *is_default = true;
    }
    else if (type == LN_AT_PARA_TYPE_STR)
    {
        *pp_str = at_cmd->para[param_index - 1].para_start_pos;
    }
    else
    {
        return LN_AT_PSR_ERR_COMMON;
    }
    return LN_AT_PSR_ERR_NONE;
}

ln_at_parser_err_t ln_at_param_dump(void)
{
    int num = 0;
    const char *type_str[4] = {
        "int",
        "string",
        "omit",
        "unknown"
    };

    const char *value = NULL;
    ln_at_parser_t *at_cmd = &g_ln_at_cmd_context;

    num = at_cmd->total_para_num;

    LN_AT_LOG_I("======================================\r\n");
    LN_AT_LOG_I("index type     value\r\n");
    for (int i = 0; i < num; i++)
    {
        if (at_cmd->para[i].para_type > LN_AT_PARA_TYPE_UNKNOWN)
        {
            continue;
        }

        if (at_cmd->para[i].para_type == LN_AT_PARA_TYPE_OMIT || 
            at_cmd->para[i].para_type == LN_AT_PARA_TYPE_UNKNOWN) {
            value = " ";
        } else {
            value = (const char *)at_cmd->para[i].para_start_pos;
        }

        LN_AT_LOG_I("[%2d] %-8.8s %s\r\n",
            i,
            type_str[at_cmd->para[i].para_type],
            value);
    }
    LN_AT_LOG_I("======================================\r\n");
}
