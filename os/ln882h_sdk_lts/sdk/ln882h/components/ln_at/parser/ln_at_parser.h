/**
 * @file   ln_at_parser.h
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-01-28     MurphyZhao   the first version
 */

#ifndef __LN_AT_PARSER_H__
#define __LN_AT_PARSER_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// #define LN_AT_PARSER_BUILD_LIB

typedef enum
{
    LN_AT_PSR_ERR_NONE          = 0,
    LN_AT_PSR_ERR_COMMON        = 1,
    LN_AT_PSR_ERR_NO_CRLF       = 2,
    LN_AT_PSR_ERR_NO_AT         = 3, /* Not found "AT" string in AT instruction */
    LN_AT_PSR_ERR_FORMAT        = 4, /* The format of the AT instruction is wrong */
    LN_AT_PSR_ERR_TOO_MANY_PARA = 5, /* The AT instruction has too many parameters */
    LN_AT_PSR_ERR_PARSER        = 6, /* Parser error */
    LN_AT_PSR_ERR_PARAM         = 7,
    LN_AT_PSR_ERR_NOT_SUPPORT
} ln_at_parser_err_t;

typedef enum
{
    LN_AT_PSR_CMD_TYPE_BASIC = 0,
    LN_AT_PSR_CMD_TYPE_EXEC  = 1,
    LN_AT_PSR_CMD_TYPE_GET   = 2,
    LN_AT_PSR_CMD_TYPE_TEST  = 3,
    LN_AT_PSR_CMD_TYPE_SET   = 4,
} ln_at_parser_cmd_type_t;

typedef struct
{
    char                    *name; /* cmd name */
    ln_at_parser_cmd_type_t  type;
    uint8_t                  para_num;
} ln_at_parser_rst_t;

ln_at_parser_err_t ln_at_cmd_parse(char *intact_line, size_t len);
ln_at_parser_err_t ln_at_parser_get_result(ln_at_parser_rst_t *rst);

ln_at_parser_err_t ln_at_parser_get_int_param(uint8_t param_index, bool *is_default, int *value);
ln_at_parser_err_t ln_at_parser_get_str_param(uint8_t param_index, bool *is_default, char **pp_str);
ln_at_parser_err_t ln_at_param_dump(void);

#endif /* __LN_AT_PARSER_H__ */
