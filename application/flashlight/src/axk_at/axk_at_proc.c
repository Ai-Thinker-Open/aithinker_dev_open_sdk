#include "axk_at.h"
#include "axk_at_cmd.h"
#include "aiio_adapter_include.h"

#define AT_ECHO_ENABLE  CONFIG_AXK_AT_ECHO

#define AT_CMD_HEADER "AT"

at_cmd_ctrl g_at_ctrl = {
    .at_state = AT_IDLE,
    .send_len = 0,
    .trans_len = 0,
    .is_first_recv_data = AXK_TRUE,
    .is_first_over_data = AXK_TRUE,
    .is_recv_end_char_flag = 0,
    .specific_callback = NULL,
};

static void at_get_cmd_name(char *cmd_line, at_cmd_attr *cmd_parsed)
{
    uint16_t n = 0;

    for (uint16_t i = AT_CMD_MAX_LEN; i > 0; i--)
    {
        if ((*cmd_line == '\0') || (*cmd_line == '=') || (*cmd_line == '?'))
        {
            cmd_parsed->at_cmd_len = n;
            return;
        }
        else
        {
            cmd_parsed->cmd_name[n] = *cmd_line;
            cmd_line++;
            n++;
        }
    }
    cmd_parsed->at_cmd_len = -1;
    memset(cmd_parsed->cmd_name, 0, AT_CMD_MAX_LEN);
}

at_cmd_func *at_find_proc_func(const at_cmd_attr *cmd_parsed)
{
    at_cmd_func_list *cmd_list = at_get_list();
    uint32_t i;

    for (i = 0; i < AT_CMD_LIST_NUM; i++)
    {
        uint16_t j;

        for (j = 0; j < cmd_list->at_cmd_num[i]; j++)
        {
            at_cmd_func *cmd_func = (at_cmd_func *)((cmd_list->at_cmd_list[i] + j));
            if ((cmd_parsed->at_cmd_len == cmd_func->at_cmd_len) &&
                (strcmp(cmd_parsed->cmd_name, cmd_func->at_cmd_name) == 0))
            {
                return cmd_func;
            }
        }
    }

    return NULL;
}

static int at_cmd_excute(const at_cmd_func *cmd_func, at_cmd_attr *cmd_parsed)
{
    int ret;

    if (cmd_func == NULL || cmd_parsed == NULL)
    {
        return AT_ERR_FAILURE;
    }

    if (cmd_parsed->at_cmd_type == AT_CMD_TYPE_TEST)
    {
        if (cmd_func->at_test_cmd)
        {
            ret = cmd_func->at_test_cmd((int)cmd_parsed->at_param_cnt,
                                        (const char **)&cmd_parsed->param_array[0]);
        }
        else
        {
            aiio_log_e("COMMAND TYPE NOT SUPPORT!\r\n");
            ret = AT_ERR_FAILURE;
            aiio_log_e("[AT][PROC] err:%d\r\n", cmd_parsed->at_cmd_type);
        }
    }
    else if (cmd_parsed->at_cmd_type == AT_CMD_TYPE_QUERY)
    {
        if (cmd_func->at_query_cmd)
        {
            ret = cmd_func->at_query_cmd((int)cmd_parsed->at_param_cnt,
                                         (const char **)&cmd_parsed->param_array[0]);
        }
        else
        {
            aiio_log_e("COMMAND TYPE NOT SUPPORT!\r\n");
            ret = AT_ERR_FAILURE;
            aiio_log_e("[AT][PROC] err:%d\r\n", cmd_parsed->at_cmd_type);
        }
    }
    else if (cmd_parsed->at_cmd_type == AT_CMD_TYPE_SETUP)
    {
        if (cmd_func->at_setup_cmd)
        {
            ret = cmd_func->at_setup_cmd((int)cmd_parsed->at_param_cnt,
                                         (const char **)&cmd_parsed->param_array[0]);
        }
        else
        {
            aiio_log_e("COMMAND TYPE NOT SUPPORT!\r\n");
            ret = AT_ERR_FAILURE;
            aiio_log_e("[AT][PROC] err:%d\r\n", cmd_parsed->at_cmd_type);
        }
    }
    else if (cmd_parsed->at_cmd_type == AT_CMD_TYPE_EXE)
    {
        if (cmd_func->at_exe_cmd)
        {
            ret = cmd_func->at_exe_cmd((int)cmd_parsed->at_param_cnt,
                                       (const char **)&cmd_parsed->param_array[0]);
        }
        else
        {
            aiio_log_e("COMMAND TYPE NOT SUPPORT!\r\n");
            ret = AT_ERR_FAILURE;
            aiio_log_e("[AT][PROC] err:%d\r\n", cmd_parsed->at_cmd_type);
        }
    }
    else
    {
        aiio_log_e("COMMAND TYPE NOT SUPPORT!\r\n");
        ret = AT_ERR_FAILURE;
        aiio_log_e("[AT][PROC] err:%d\r\n", cmd_parsed->at_cmd_type);
    }

    return ret;
}

static int at_func_process(char *out_cmd_line, at_cmd_attr *cmd_parsed)
{
    int ret;
    at_cmd_func *cmd_func = NULL;
    at_get_cmd_name(out_cmd_line, cmd_parsed);
    if (cmd_parsed->at_cmd_len != (-1))
    {
        cmd_func = at_find_proc_func(cmd_parsed);
    }

    if (cmd_func != NULL)
    {
        ret = axk_at_cmd_parse(out_cmd_line, cmd_parsed);
        if (ret != AT_ERR_SUCCESS)
        {
            aiio_log_e("%s line: %d PARSE CMD FAIL!\r\n", __FUNCTION__, __LINE__);
            return ret;
        }

        ret = at_cmd_excute(cmd_func, cmd_parsed);
    }
    else
    {
        ret = AT_ERR_FAILURE;
        aiio_log_e("%s line: %d COMMAND NOT SUPPORT!\r\n", __FUNCTION__, __LINE__);
    }

    return ret;
}

static int at_cmd_process(const char *at_cmd_line)
{
    uint32_t at_cmd_line_len;
    char *out_cmd_line = NULL;
    at_cmd_attr cmd_parsed = {0};
    int ret;

    if (g_at_cmd_config.echo)
        AT_ENTER;

// #if AT_ECHO_ENABLE
//     AT_ENTER;
// #endif
    if (at_cmd_line == NULL)
    {
        aiio_log_e("INVALID NULL CMD!\r\n");
        AT_RESPONSE_ERROR;
        return AT_ERR_FAILURE;
    }

    at_cmd_line_len = (uint32_t)strlen(at_cmd_line) + 1;
    out_cmd_line = (char *)malloc(at_cmd_line_len);
    if (out_cmd_line == NULL)
    {
        aiio_log_e("%s line%d NO ENOUGH MEMORY!\r\n", __FUNCTION__, __LINE__);
        AT_RESPONSE_ERROR;
        return AT_ERR_FAILURE;
    }
    memset(out_cmd_line, 0, at_cmd_line_len);

    strncpy(out_cmd_line, at_cmd_line, at_cmd_line_len - 1);

    out_cmd_line[at_cmd_line_len - 1] = '\0';
    ret = at_func_process(out_cmd_line, &cmd_parsed);

    if (ret != AT_ERR_SUCCESS && ret != AT_ERR_RECVING && ret != AT_ERR_BUSY && ret != AT_ERR_TRANSPARENT)
    {
        aiio_log_e("err at %s line: %d\r\n", __FUNCTION__, __LINE__);
        if(ret==AT_ERR_FAILURE_WITH_ENTER)
            AT_RESPONSE_ERROR_WITH_ENTER;
        else
            AT_RESPONSE_ERROR;

    }
    else if (ret == AT_ERR_RECVING)
    {
        g_at_ctrl.at_state = AT_DATA_RECVING;
    }
    else if(ret==AT_ERR_BUSY)
    {
        g_at_ctrl.at_state = AT_CMD_PROCESS;
    }
    else if(ret==AT_ERR_TRANSPARENT)
    {
        g_at_ctrl.at_state = AT_TRANSPARENT;
    }
    else
    {
        g_at_ctrl.at_state=AT_IDLE;
    }

    free(out_cmd_line);
    out_cmd_line = NULL;

    return ret;
}

void axk_at_cmd_execute(char *buf)
{
    uint32_t ret;

    if (g_at_cmd_config.echo)
        axk_at_printf("%s", buf);

// #if AT_ECHO_ENABLE
//     axk_at_printf("%s", buf);
// #endif

    if (memcmp(buf, AT_CMD_HEADER, strlen(AT_CMD_HEADER)) == 0)
    {

        if (g_at_ctrl.at_state == AT_CMD_PROCESS)
        {
            axk_at_printf("\r\nbusy p...\r\n");
        }
        else
        {
            g_at_ctrl.at_state = AT_CMD_PROCESS;
            char *at_buf = buf + strlen(AT_CMD_HEADER);
            ret = at_cmd_process(at_buf);
            if ((ret != AT_ERR_SUCCESS) && (ret != AT_ERR_RECVING) && (ret != AT_ERR_BUSY) && (ret != AT_ERR_TRANSPARENT))
            {
                g_at_ctrl.at_state = AT_IDLE;
            }
        }
    }
    else
    {
        AT_ENTER;
        AT_RESPONSE_ERROR;
        g_at_ctrl.at_state = AT_IDLE;
    }
}

int at_port_enter_specific(at_port_specific_callback_t callback)
{
    if (g_at_ctrl.specific_callback == NULL) {
        g_at_ctrl.specific_callback = callback;
    } else {
        return -1;
    }

    return 0;
}

int at_port_is_specific_mode(void)
{
    if (g_at_ctrl.specific_callback == NULL) {
        return 0;
    } else {
        return 1;
    }
}

void at_port_exit_specific()
{
    g_at_ctrl.specific_callback = NULL;
}

// void at_port_exit_passthrough()
// {
//     g_at_ctrl.at_state = AT_IDLE;
//     at_port_exit_specific();

//     if (g_at_cmd_config.sysmsg.bit.bit0) {
//         axk_at_printf("\r\n+QUITT\r\n");
//     }
// }

// int at_check_print_urc(void)
// {
//     if (g_at_ctrl.at_state == AT_TRANSPARENT) {
//         if (g_at_cmd_config.sysmsg.bit.bit2 == 0) {
//             return 0;
//         }
//     }

//     return 1;
// }


