#include "axk_at_cmd.h"
#include "axk_at.h"
#include "axk_uart_init.h"
#include "axk_device_info.h"

#include "aiio_adapter_include.h"

#include "axk_mqtt.h"
#include "axk_at_network.h"
#include "axk_nvs_info.h"
#include "axk_blufi.h"
#include "axk_at_basic.h"
#include "axk_sleep.h"
#include "at_http.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

static at_cmd_func_list sg_at_cmd_list = {0};
at_cmd_config_t g_at_cmd_config = {
    .echo = 0,
};

const at_cmd_func g_at_general_func_tbl[] = {
    {"E0", 2, NULL, NULL, NULL, at_exe_echo_off_cmd},
    {"E1", 2, NULL, NULL, NULL, at_exe_echo_on_cmd},
    // MQTT
    {"+MQTTPUBRAW", 11, NULL, NULL, at_setup_mqtt_rawpub_cmd, NULL},
    {"+MQTTPUB", 8, NULL, NULL, axk_setup_mqtt_pub_cmd, NULL},
    {"+MQTTSUB", 8, NULL, NULL, axk_setup_mqtt_sub_cmd, NULL},
    {"+WEBWIFICFG", 11, NULL, NULL, at_setup_webcfg_cmd, NULL},
    {"+APPWIFICFG", 11, NULL, NULL, at_setup_appcfg_cmd, NULL},
    {"+RESTORE", 8, NULL, NULL, NULL, at_exe_restore_cmd},
    {"+BLUFI", 6, NULL, NULL, at_setup_blufi_cmd, NULL},
    {"+PRODUCTVER", 11, NULL, at_query_fw_version_cmd, NULL, NULL},     //查询版本号
    {"+PRODUCTMODEL", 13, NULL, at_query_productCode_cmd, at_setup_productCode_cmd, NULL},  //查询productCode
    {"+DEVICECODE", 11, NULL, at_query_deviceCode_cmd, NULL, NULL},     //查询deviceCode
    {"+SLEEP", 6, NULL, NULL, NULL, at_exe_sleep_cmd},
    {"+RSSI", 5, NULL, at_query_rssi_cmd, NULL, NULL},
    {"+MCUVER", 7, NULL, at_query_mcu_version_cmd, at_setup_mcu_version_cmd, NULL},
    {"+AUTOSLEEP", 10, NULL, NULL, at_setup_auto_sleep_cmd, NULL},
    {"+STAMAC", 7, NULL, at_query_sta_mac_cmd, NULL, NULL},
    //http
    {"+HTTPCLIENT", 11, NULL, NULL, at_setup_http_client_cmd, NULL},
};

#define AT_GENERAL_FUNC_NUM (sizeof(g_at_general_func_tbl) / sizeof(g_at_general_func_tbl[0]))

void axk_at_printf(char *fmt, ...)
{
    char string[AT_CMD_MAX_LEN] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(string, AT_CMD_MAX_LEN, fmt, ap);
    va_end(ap);

    g_uart.send_data((uint8_t *)string, strlen(string));
}

at_cmd_func_list *at_get_list(void)
{
    return (at_cmd_func_list *)&sg_at_cmd_list;
}

static int check_cmd_tbl(const at_cmd_func *cmd_tbl, uint16_t cmd_num)
{
    uint16_t i;
    uint16_t j;

    for (i = 0; i < cmd_num; i++)
    {
        if (cmd_tbl[i].at_cmd_len != (int8_t)strlen(cmd_tbl[i].at_cmd_name))
        {
            aiio_log_e("cmd %s len error\r\n", cmd_tbl[i].at_cmd_name);
            return AT_INVALID_PARAMETER;
        }

        for (j = 0; j < cmd_num; j++)
        {
            if (i == j)
            {
                continue;
            }

            if (((cmd_tbl[j].at_cmd_len == cmd_tbl[i].at_cmd_len) &&
                 (strcmp(cmd_tbl[j].at_cmd_name, cmd_tbl[i].at_cmd_name) == 0)) ||
                ((cmd_tbl[j].at_test_cmd != NULL) && (cmd_tbl[j].at_test_cmd == cmd_tbl[i].at_test_cmd)) ||
                ((cmd_tbl[j].at_query_cmd != NULL) && (cmd_tbl[j].at_query_cmd == cmd_tbl[i].at_query_cmd)) ||
                ((cmd_tbl[j].at_setup_cmd != NULL) && (cmd_tbl[j].at_setup_cmd == cmd_tbl[i].at_setup_cmd)) ||
                ((cmd_tbl[j].at_exe_cmd != NULL) && (cmd_tbl[j].at_exe_cmd == cmd_tbl[i].at_exe_cmd)))
            {
                aiio_log_e("2cmd %s error\r\n", cmd_tbl[j].at_cmd_name);
                return AT_ERR_NAME_OR_FUNC_REPEAT_REGISTERED3;
            }
        }
    }

    return AT_ERR_SUCCESS;
}

static int check_name_and_callback(const at_cmd_func_list *ctx, uint8_t tbl_index, const at_cmd_func *cmd_tbl,
                                   uint16_t cmd_num)
{
    int ret = AT_ERR_SUCCESS;
    uint16_t i;
    uint16_t j;

    for (i = 0; i < ctx->at_cmd_num[tbl_index]; i++)
    {
        const at_cmd_func *cmd_func = (at_cmd_func *)((ctx->at_cmd_list[tbl_index] + i));

        for (j = 0; j < cmd_num; j++)
        {
            if (((cmd_func->at_cmd_len == cmd_tbl[j].at_cmd_len) &&
                 (strcmp(cmd_func->at_cmd_name, cmd_tbl[j].at_cmd_name) == 0)) ||
                ((cmd_tbl[j].at_test_cmd != NULL) && (cmd_func->at_test_cmd == cmd_tbl[j].at_test_cmd)) ||
                ((cmd_tbl[j].at_query_cmd != NULL) && (cmd_func->at_query_cmd == cmd_tbl[j].at_query_cmd)) ||
                ((cmd_tbl[j].at_setup_cmd != NULL) && (cmd_func->at_setup_cmd == cmd_tbl[j].at_setup_cmd)) ||
                ((cmd_tbl[j].at_exe_cmd != NULL) && (cmd_func->at_exe_cmd == cmd_tbl[j].at_exe_cmd)))
            {
                aiio_log_e("1cmd %s error\r\n", cmd_tbl[j].at_cmd_name);
                return AT_ERR_NAME_OR_FUNC_REPEAT_REGISTERED3;
            }
        }
    }

    return ret;
}

int axk_at_register_cmd(const at_cmd_func *cmd_tbl, uint16_t cmd_num)
{
    int ret = AT_ERR_FAILURE;
    uint8_t i;

    if (cmd_tbl == NULL || cmd_num == 0)
    {
        return AT_ERR_FAILURE;
    }

    ret = check_cmd_tbl(cmd_tbl, cmd_num);
    if (ret != AT_ERR_SUCCESS)
    {
        return ret;
    }

    at_cmd_func_list *cmd_list = at_get_list();
    for (i = 0; i < AT_CMD_LIST_NUM; i++)
    {
        if ((cmd_list->at_cmd_list[i] == NULL) || (cmd_list->at_cmd_num[i] == 0))
        {
            cmd_list->at_cmd_list[i] = cmd_tbl;
            cmd_list->at_cmd_num[i] = cmd_num;
            ret = AT_ERR_SUCCESS;
            break;
        }

        ret = check_name_and_callback(cmd_list, i, cmd_tbl, cmd_num);
        if (ret != AT_ERR_SUCCESS)
        {
            break;
        }
    }

    return ret;
}

void axk_at_esp_cmd_register(void)
{
    int ret = axk_at_register_cmd(g_at_general_func_tbl, AT_GENERAL_FUNC_NUM);
    if (ret != AT_ERR_SUCCESS)
    {
        aiio_log_e("[AT] register general cmd fail err:%d\r\n", ret);
    }
}

