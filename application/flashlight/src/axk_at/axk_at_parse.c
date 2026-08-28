#include "axk_at.h"

#include "aiio_adapter_include.h"

#include <string.h>

int at_param_shift(const char *cmd_in, char *cmd_out, uint32_t size)
{
    char *output = (char *)NULL;
    char *out_bak = (char *)NULL;
    uint32_t len = size;

    if ((cmd_in == NULL) || (cmd_out == NULL))
    {
        aiio_log_e("cmd_in and cmd_out = NULL in %s[%d]\r\n", __FUNCTION__, __LINE__);
        return AT_ERR_FAILURE;
    }

    output = (char *)malloc(len);
    if (output == NULL)
    {
        aiio_log_e("malloc failure in %s[%d]\r\n", __FUNCTION__, __LINE__);
        return AT_ERR_FAILURE;
    }
    /* Backup the 'output' start address */
    out_bak = output;
    /* Scan each charactor in 'cmd_in',and squeeze the overmuch space and ignore invaild charactor */
    for (; *cmd_in != '\0'; cmd_in++)
    {
        if ((*cmd_in == '\\') && ((*(cmd_in + 1) == '\"') || (*(cmd_in + 1) == ',')))
        {
            continue;
        }
        *output = *cmd_in;
        output++;
    }
    *output = '\0';
    /* Restore the 'pscOutput' start address */
    output = out_bak;
    len = strlen(output);

    /* Copy out the buffer which is been operated already */
    strncpy(cmd_out, output, len);

    cmd_out[len] = '\0';

    free(out_bak);

    return AT_ERR_SUCCESS;
}

int cmd_parse_para_get(uint32_t *value, char *para_token_str)
{
    if ((para_token_str == NULL) || (value == NULL))
    {
        return AT_ERR_FAILURE;
    }
    uint32_t ret;
    uint32_t value_in_len;
    char *value_in = NULL;

    value_in_len = strlen(para_token_str) + 1;
    value_in = (char *)malloc(value_in_len);
    if (value_in == NULL)
    {
        aiio_log_e("%s,%d hi_malloc failed!\r\n", __FUNCTION__, __LINE__);
        return AT_ERR_FAILURE;
    }

    memset(value_in, 0, value_in_len);

    strncpy(value_in, para_token_str, strlen(para_token_str));

    ret = at_param_shift(value_in, para_token_str, value_in_len);
    if (ret != AT_ERR_SUCCESS)
    {
        aiio_log_e("%s,%d at_param_shift failed, err:%d!\r\n", __FUNCTION__, __LINE__, ret);
        free(value_in);
        return AT_ERR_FAILURE;
    }
    *value = (uint32_t)(uintptr_t)para_token_str;

    free(value_in);
    return AT_ERR_SUCCESS;
}

int cmd_parse_one_token(at_cmd_attr *cmd_parsed, uint32_t index, char *token)
{
    int ret = AT_ERR_SUCCESS;

    if (cmd_parsed == NULL)
    {
        return AT_ERR_FAILURE;
    }

    if (index == 0)
    {
        if (cmd_parsed->at_cmd_type != AT_CMD_TYPE_SETUP)
        {
            return ret;
        }
    }

    if (index >= AT_CMD_MAX_PARAS)
    {
        return AT_ERR_FAILURE;
    }

    if (token[0] == '\0')
    {
        uint32_t len = cmd_parsed->at_param_cnt;
        cmd_parsed->param_array[len] = token;
        cmd_parsed->at_param_cnt++;
        return ret;
    }

    if (cmd_parsed->at_param_cnt < AT_CMD_MAX_PARAS)
    {
        uint32_t len = cmd_parsed->at_param_cnt;
        ret = cmd_parse_para_get(&(cmd_parsed->param_array[len]), token);
        if (ret != AT_ERR_SUCCESS)
        {
            return ret;
        }
        cmd_parsed->at_param_cnt++;
    }

    return ret;
}

int at_cmd_token_split(char *cmd, char split, at_cmd_attr *cmd_parsed)
{
    enum
    {
        STAT_INIT,
        STAT_TOKEN_IN,
        STAT_TOKEN_OUT
    } state = STAT_INIT;

    char *token = NULL;
    char *p = NULL;
    uint32_t count = 0;
    int ret = AT_ERR_SUCCESS;

    if (cmd == NULL)
    {
        return AT_ERR_FAILURE;
    }

    token = cmd;

    for (p = cmd; (*p != '\0') && (ret == AT_ERR_SUCCESS); p++)
    {
        if (state == STAT_TOKEN_OUT)
        {
            token = p;
            state = STAT_TOKEN_IN;
        }

        if (state == STAT_INIT || state == STAT_TOKEN_IN)
        {
            if ((*p == split) && (*(p - 1) != '\\'))
            {
                *p = '\0';
                ret = cmd_parse_one_token(cmd_parsed, count++, token);
                state = STAT_TOKEN_OUT;
            }
        }
    }

    if (*(p - 1) == '\0')
    {
        token = p;
    }
    if ((ret == AT_ERR_SUCCESS) || (state == STAT_INIT))
    {
        ret = cmd_parse_one_token(cmd_parsed, count, token);
    }

    return ret;
}

int at_para_parse(char *cmd_line, at_cmd_attr *cmd_parsed)
{
    if ((cmd_line == NULL) || (cmd_parsed == NULL) || (strlen(cmd_line) == 0))
    {
        return AT_ERR_FAILURE;
    }

    return at_cmd_token_split(cmd_line, ',', cmd_parsed);
}

int axk_at_cmd_parse(char *cmd_line, at_cmd_attr *cmd_parsed)
{
    int ret;
    if (cmd_line == NULL || cmd_parsed == NULL)
    {
        return AT_ERR_FAILURE;
    }

    cmd_line += cmd_parsed->at_cmd_len;
    aiio_log_e("at_cmd_parse line %d;cmd_line: %s\r\n", __LINE__, cmd_line);
    if (*cmd_line == '\0')
    {
        cmd_parsed->at_cmd_type = AT_CMD_TYPE_EXE;
    }
    else if (*cmd_line == '?' && (cmd_line[1] == '\0'))
    {
        cmd_parsed->at_cmd_type = AT_CMD_TYPE_QUERY;
    }
    else if ((*cmd_line == '=') && (cmd_line[1] == '?') && (cmd_line[2] == '\0'))
    { /* 2: the third character */
        cmd_parsed->at_cmd_type = AT_CMD_TYPE_TEST;
    }
    else if ((*cmd_line == '=') && (cmd_line[1] != '?'))
    {
        cmd_parsed->at_cmd_type = AT_CMD_TYPE_SETUP;

        if (cmd_line[1] != '\0')
        {
            ret = at_para_parse(cmd_line + 1, cmd_parsed);
            if (ret != AT_ERR_SUCCESS)
            {
                return ret;
            }
        }
    }
    return AT_ERR_SUCCESS;
}