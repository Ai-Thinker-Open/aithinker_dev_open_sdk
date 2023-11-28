/**
 * @file   ln_at_cmd_bt.c
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 *
 */

#include "utils/debug/log.h"
#include "utils/ln_psk_calc.h"
#include "utils/ln_list.h"
#include "utils/reboot_trace/reboot_trace.h"
#include "hal/hal_uart.h"
#include <stdlib.h>
#include <string.h>

#include "ln_at.h"
#include "gap.h"
#include "ln_nvds.h"
#include "base_addr_ln882h.h"
#include "ble_port.h"
#include "ln882h.h"

#include "ln_ble_app_kv.h"
#include "ln_ble_connection_manager.h"
#include "ln_ble_scan.h"
#include "ln_ble_advertising.h"
#include "ln_ble_test.h"

#include "ln_ble_trans_server.h"
#include "ln_ble_trans_client.h"

#include "utils/debug/log.h"


static uint32_t g_uart_baudrate = 115200;
static bool g_is_default = false;
static uint8_t g_para_index = 1;
static char *g_para_str = NULL;
static int g_para_int = 0;

static void ln_at_bt_uart_param_reset(void)
{
    g_is_default = true;
    g_para_index = 1;
    g_para_str = NULL;
    g_para_int = -1;
}

static int str2addr(char *src, bd_addr_t *dst_addr)
{
    char tmp[GAP_BD_ADDR_LEN*2];
    int i = 0;
    /* string to int array*/
    for(i=0; i<GAP_BD_ADDR_LEN*2; i++)
    {
        if('0'<=src[i] && '9'>=src[i])
            tmp[i] = src[i]-'0';
        else if('a'<=src[i] && 'f'>=src[i])
            tmp[i] = src[i]-'a'+10;
        else if('A'<=src[i] && 'F'>=src[i])
            tmp[i] = src[i]-'A'+10;
        else
            return -1;
    }
    /* array to address type*/
    for(i=0; i<GAP_BD_ADDR_LEN; i++)
    {
        dst_addr->addr[i] = tmp[2*i]<<4 | tmp[2*i+1];
    }
    /*reversal addr*/
    for(i=0; i<(GAP_BD_ADDR_LEN/2); i++)
    {
        uint8_t ch = dst_addr->addr[i];
        dst_addr->addr[i] = dst_addr->addr[GAP_BD_ADDR_LEN-i-1];
        dst_addr->addr[GAP_BD_ADDR_LEN-i-1] = ch;
    }

    return 0;
}

int stoi(char *src, uint16_t len)
{
    int sum = 0;
    for(int i=0;i<len;i++)
    {
        if(src[i]<'0' || src[i]>'9')
            return -1;
        sum = sum*10+(src[i]-'0');
    }
    return sum;
}

/**
 ****************************************************************************************
 * @brief ble test tx start.
 *
 * @Format AT+LETSTTX=<tx_ch>,<phy>,<tx_len>,<pkt>\r\n
 *
 * @param[tx_ch] phy channel index, Range 0-39.
 * @param[phy] (@see enum gap_phy_val).
 * @param[tx_len] Length in bytes of payload data in each packet, range 0x00-0xFF.
 * @param[pkt] (@see enum gap_pkt_pld_type).
 *
 * @example AT+LETSTTX=0,1,16,2\r\n
 ****************************************************************************************
 */
ln_at_err_t ln_at_set_tx_test_start(uint8_t para_num, const char *name)
{
    app_le_test_tx_start_cmd(0x00, GAP_PHY_1MBPS, 0x10, GAP_PKT_PLD_REPEATED_10101010);

    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief ble test rx start.
 *
 * @Format AT+LETSTRX=<rx_ch>,<phy>,<mod_index>\r\n
 *
 * @param[rx_ch] channel index ,Range 0-39.
 * @param[phy] (@see enum gap_phy_val).
 * @param[mod_index] (@see enum gap_phy_val).
 *
 * @example AT+LETSTRX=0,1,0\r\n
 ****************************************************************************************
 */
ln_at_err_t ln_at_set_rx_test_start(uint8_t para_num, const char *name)
{
    app_le_test_rx_start_cmd(0x00, GAP_PHY_1MBPS, GAP_MODULATION_STANDARD);

    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief ble test stop.
 *
 * @Format AT+LETSTSTOP\r\n
 *
 * @param null
 *
 * @example AT+LETSTSTOP\r\n
 ****************************************************************************************
 */
ln_at_err_t ln_at_exec_bt_test_stop(const char *name)
{
    app_le_test_stop_cmd();

    ln_at_printf(LN_AT_RET_OK_STR);

    ln_chip_reboot();

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief get ble ble firmware version.
 *
 * @Format AT+LEVER?\r\n
 *
 * @param null.
 *
 * @example AT+LEVER?\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_get_version(const char *param)
{
    ln_at_printf(LN882H_SDK_VERSION_STRING);
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief set uart baudrate.
 *
 * @Format AT+LEBAUDRATE=<baudrate>\r\n
 *
 * @param[baudrate] uart baudrate
 *
 * @example AT+LEBAUDRATE="115200"\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_set_baudrate(uint8_t para_num, const char *param)
{
    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_str_param(g_para_index, &g_is_default, &g_para_str))
    {
        return LN_AT_ERR_PARAM;
    }
    ln_at_printf(LN_AT_RET_OK_STR);

    g_uart_baudrate = atoi(g_para_str);
    hal_uart_baudrate_set(UART1_BASE, g_uart_baudrate);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief get uart baudrate.
 *
 * @Format AT+LEBAUDRATE?\r\n
 *
 * @param null
 *
 * @example AT+LEBAUDRATE?\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_get_baudrate(const char *param)
{
    ln_at_printf("%s:%d\r\n", param, g_uart_baudrate);
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief soft reboot.
 *
 * @Format AT+LERST\r\n
 *
 * @param null
 *
 * @example AT+LERST\r\n
 ****************************************************************************************
 */
ln_at_err_t ln_at_exec_bt_soft_reset(const char *name)
{
    ln_at_printf(LN_AT_RET_OK_STR);

    ln_chip_reboot();

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief factory reset.
 *
 * @Format AT+LEFTYRST\r\n
 *
 * @param null
 *
 * @example AT+LEFTYRST\r\n
 ****************************************************************************************
 */
ln_at_err_t ln_at_exec_bt_soft_factory_reset(const char *name)
{
    ln_kv_ble_factory_reset();
    ln_at_printf(LN_AT_RET_OK_STR);
    ln_chip_reboot();

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief set ble role.
 *
 * @Format AT+LEROLE=<role>\r\n
 *
 * @param[role] (@see enum ble_role_type_t).
 *
 * @example AT+LEROLE=1\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_set_bt_role(uint8_t para_num, const char *param)
{
    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_int_param(g_para_index, &g_is_default, &g_para_int))
    {
        return LN_AT_ERR_PARAM;
    }
    if(g_para_int & BLE_ROLE_ALL)
    {
        ln_kv_ble_usr_data_get()->ble_role = g_para_int;
        ln_kv_ble_usr_data_store();
        ln_chip_reboot();
    }

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief get ble role.
 *
 * @Format AT+LEROLE?\r\n
 *
 * @param null.
 *
 * @example AT+LEROLE?\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_get_ble_role(const char *param)
{
    uint8_t le_role = ln_kv_ble_usr_data_get()->ble_role;

    ln_at_printf("%s:%d\r\n",param, le_role);
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief set ble security enable.
 *
 * @Format AT+LESEC=<enable>\r\n
 *
 * @param[enable] enable data encrypt, 0 is enable,1 is disable.
 *
 * @example AT+LESEC=1\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_set_security(uint8_t para_num, const char *param)
{
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief get ble paired device list.
 *
 * @Format AT+LEPDL?\r\n
 *
 * @param null.
 *
 * @example AT+LEPDL?\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_get_pair_dev_list(const char *param)
{
    kv_usr_data_t *nvds = ln_kv_ble_usr_data_get();
    int i = 0, j = 0;
    uint8_t null_addr[GAP_BD_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    for(i=0; i<MAX_DEV_BOND_MUN; i++)
    {
        if(nvds->pair_list_dev[i].mru >= 1 && nvds->pair_list_dev[i].mru <= MAX_DEV_BOND_MUN
                && memcmp(nvds->pair_list_dev[i].addr.addr, null_addr, GAP_BD_ADDR_LEN))
        {
            ln_at_printf("%s:", param);
            for(j=0;j<GAP_BD_ADDR_LEN;j++)
                ln_at_printf("%02x",nvds->pair_list_dev[i].addr.addr[GAP_BD_ADDR_LEN-j]);
            ln_at_printf(",");
            for(j=0;j<GAP_KEY_LEN;j++)
                ln_at_printf("%02x",nvds->pair_list_dev[i].ltk.ltk[j]);
            ln_at_printf("\r\n");
        }
    }
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief set ble local advertising name.
 *
 * @Format AT+LENAME=<name>\r\n
 *
 * @param[name] string len max see BLE_DEV_NAME_MAX_LEN.
 *
 * @example AT+LENAME="ln882h"\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_set_bt_name(uint8_t para_num, const char *param)
{
    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_str_param(g_para_index, &g_is_default, &g_para_str))
    {
        return LN_AT_ERR_PARAM;
    }

    if(g_para_str)
        ln_kv_ble_name_store((const uint8_t *)g_para_str, strlen(g_para_str));
    //ln_at_printf("%s:%s\r\n", param, g_para_str);

    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief get ble local advertising name.
 *
 * @Format AT+LENAME?\r\n
 *
 * @param null.
 *
 * @example AT+LENAME?\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_get_bt_name(const char *param)
{
    char data[64];
    uint8_t *name;
    uint16_t name_len;

    name = ln_kv_ble_dev_name_get(&name_len);
    memcpy(data, name, name_len);
    data[name_len] = '\0';
    ln_at_printf("%s:%s\r\n", param, data);

    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief set ble local public static address.
 *
 * @Format AT+LEADDR=<addr>\r\n
 *
 * @param[addr] A string of 12 characters(0-9,a-f,A-F).
 *
 * @example AT+LEADDR="00ff00123456"\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_set_bt_addr(uint8_t para_num, const char *param)
{
    ln_bd_addr_t addr;

    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_str_param(g_para_index, &g_is_default, &g_para_str))
    {
        return LN_AT_ERR_PARAM;
    }
    if((GAP_BD_ADDR_LEN*2 != strlen(g_para_str)) || str2addr(g_para_str, (bd_addr_t *)&addr))
    {
        return LN_AT_ERR_PARAM;
    }
    ln_at_printf("%s:%02x%02x%02x%02x%02x%02x\r\n",param,addr.addr[5],addr.addr[4],addr.addr[3],addr.addr[2],addr.addr[1],addr.addr[0]);
    ln_kv_ble_addr_store(addr);

    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief get ble local public static address.
 *
 * @Format AT+LEADDR?\r\n
 *
 * @param null.
 *
 * @example AT+LEADDR?\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_get_bt_addr(const char *param)
{
    ln_bd_addr_t *addr = ln_kv_ble_pub_addr_get();
    ln_at_printf("%s:%02x%02x%02x%02x%02x%02x\r\n",param, addr->addr[5], addr->addr[4], addr->addr[3], addr->addr[2], addr->addr[1], addr->addr[0]);

    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief create ble connection link by address.
 *
 * @Format AT+LECONN=<addr>\r\n
 *
 * @param[addr] remote device address.
 *
 * @example AT+LECONN="00ff00123456"\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_bt_connect_req(uint8_t para_num, const char *name)
{
    bd_addr_t addr;

    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_str_param(g_para_index, &g_is_default, &g_para_str))
    {
        return LN_AT_ERR_PARAM;
    }
    if(str2addr(g_para_str, &addr))
    {
        return LN_AT_ERR_PARAM;
    }
    //ln_at_printf("%02x%02x%02x%02x%02x%02x\r\n",addr.addr[0],addr.addr[1],addr.addr[2],addr.addr[3],addr.addr[4],addr.addr[5]);
    ln_at_printf(LN_AT_RET_OK_STR);

    ln_ble_conn_req((ln_bd_addr_t *)&addr);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief get connect link status.
 *
 * @Format AT+LECONN?\r\n
 *
 * @param null.
 *
 * @example AT+LECONN?\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_get_conn_status(const char *param)
{
    ble_dev_conn_manager_t *con_mgr = le_conn_mgr_info_get();
    if(!con_mgr->conn_num)
    {
        ln_at_printf("%s:0",param);
        return LN_AT_ERR_NONE;
    }
    for(int i=0; i<BLE_CONN_DEV_NUM_MAX; i++)
    {
        le_dev_con_env_t *con_env = &con_mgr->con_env[i];
        if(APP_CONN_INVALID_HANDLE != con_env->conn_handle)
        {
            ln_at_printf("%s:%d,%02x%02x%02x%02x%02x%02x\r\n",param, i,
                            con_env->dev_remote.addr.addr[5],
                            con_env->dev_remote.addr.addr[4],
                            con_env->dev_remote.addr.addr[3],
                            con_env->dev_remote.addr.addr[2],
                            con_env->dev_remote.addr.addr[1],
                            con_env->dev_remote.addr.addr[0]);
        }
    }

    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}


/**
 ****************************************************************************************
 * @brief disconnect a link by connection index.
 *
 * @Format AT+LEDISC=<conn_idx>\r\n
 *
 * @param[conn_idx] connection index, value is 0 for only one ble link
 *
 * @example AT+LEDISC=0\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_bt_disconnect_req(uint8_t para_num, const char *name)
{
    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_int_param(g_para_index, &g_is_default, &g_para_int))
    {
        return LN_AT_ERR_PARAM;
    }
    if(g_para_int < BLE_CONN_DEV_NUM_MAX)
        ln_ble_disc_req(g_para_int);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief disconnect all link.
 *
 * @Format AT+LEDISC\r\n
 *
 * @param null
 *
 * @example AT+LEDISC\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_bt_disconnect_all(const char *name)
{
    ble_dev_conn_manager_t *con_mgr =  le_conn_mgr_info_get();
    for(int i=0; i<BLE_CONN_DEV_NUM_MAX;i++)
    {
        if(con_mgr->con_env[i].state > LE_CONN_STATE_DISCONNECTED)
            ln_ble_disc_req(i);
    }

    return LN_AT_ERR_NONE;
}


/**
 ****************************************************************************************
 * @brief set ble tx power.
 *
 * @Format AT+LETXPOWER=<tx_power>\r\n
 *
 * @param[tx_power] string value(dBm).
 *
 * @example AT+LETXPOWER="-2"\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_set_tx_power(uint8_t para_num, const char *param)
{
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief get ble tx power.
 *
 * @Format AT+LETXPWR?\r\n
 *
 * @param null.
 *
 * @example AT+LETXPWR?\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_get_tx_power(const char *param)
{
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief get connection paramter.
 *
 * @Format AT+LECONPARAM?\r\n
 *
 * @param null.
 *
 * @example AT+LECONPARAM?\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_get_conn_param(const char *param)
{
    ln_ble_conn_param_t * con_param = &le_conn_mgr_info_get()->con_param;

    ln_at_printf("%s:%d,%d,%d,%d\r\n", param, con_param->intv_min,
                                            con_param->intv_max,
                                            con_param->latency,
                                            con_param->time_out);

    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/* --------------------client interface -----------------------*/

/**
 ****************************************************************************************
 * @brief set ble white list.
 *
 * @Format AT+LEWHTLIST=<operate>,<addr_type>,<addr>\r\n
 *
 * @param[operate] value range 0-1, 0 is delete, 1 is add.
 * @param[addr_type] value range 0-1, 0 is delete, 1 is add.
 * @param[addr] A string of 12 characters(0-9,a-f,A-F).
 *
 * @example AT+LEWHTLIST=1,0,"00ff00123456"\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_set_white_list(uint8_t para_num, const char *param)
{
    kv_usr_data_t *nvds = ln_kv_ble_usr_data_get();
    bt_white_list_t tmp_white_list;
    bt_white_list_t null_white_list = {0};
    uint8_t operate = 0;

    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_int_param(g_para_index++, &g_is_default, &operate))
    {
        return LN_AT_ERR_PARAM;
    }
    if(ln_at_parser_get_int_param(g_para_index++, &g_is_default, &tmp_white_list.addr_type))
    {
        return LN_AT_ERR_PARAM;
    }
    if(ln_at_parser_get_str_param(g_para_index, &g_is_default, &g_para_str))
    {
        return LN_AT_ERR_PARAM;
    }
    if((GAP_BD_ADDR_LEN*2 != strlen(g_para_str)) || str2addr(g_para_str, (bd_addr_t *)&tmp_white_list.addr))
    {
        return LN_AT_ERR_PARAM;
    }
    //find white list is exist
    for(int i=0;i<MAX_DEV_WHITE_LIST_MUN;i++)
    {
        if(!memcmp(&nvds->white_list[i], &tmp_white_list, sizeof(bt_white_list_t)))
        {
            if(!operate)
                memset(&nvds->white_list[i], 0, sizeof(bt_white_list_t));
            return LN_AT_ERR_NONE;
        }
    }
    if(operate)//insert
    {
        for(int j=0; j<MAX_DEV_WHITE_LIST_MUN; j++)
        {
            if(!memcmp(&nvds->white_list[j], &null_white_list, sizeof(bt_white_list_t)))
            {
                memcpy(&nvds->white_list[j], &tmp_white_list, sizeof(bt_white_list_t));
                break;
            }
        }
    }

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief get ble white list.
 *
 * @Format AT+LEWHTLIST?\r\n
 *
 * @param null.
 *
 * @example AT+LEWHTLIST?\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_get_white_list(const char *param)
{
    kv_usr_data_t *nvds = ln_kv_ble_usr_data_get();
    uint8_t null_addr[GAP_BD_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int i = 0, j = 0;
    for(i=0; i<MAX_DEV_WHITE_LIST_MUN; i++)
    {
        ln_at_printf("%s:", param);
        if(memcmp(nvds->white_list[i].addr.addr, null_addr, sizeof(GAP_BD_ADDR_LEN)))
        {
            for(j=0;j<GAP_BD_ADDR_LEN;j++)
                ln_at_printf("%02x",nvds->white_list[i].addr.addr[GAP_BD_ADDR_LEN-j]);
        }
        ln_at_printf("\r\n");
    }

    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief clear all ble white list.
 *
 * @Format AT+LEWHTLIST\r\n
 *
 * @param null.
 *
 * @example AT+LEWHTLIST\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_clear_white_list(const char *param)
{
    kv_usr_data_t *nvds = ln_kv_ble_usr_data_get();
    memset(nvds->white_list, 0, MAX_DEV_WHITE_LIST_MUN*sizeof(bt_white_list_t));

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief set ble filter.
 *
 * @Format AT+LEFLTR=<operate>,<addr>\r\n
 *
 * @param[operate] value range 0-1, 0 is delete, 1 is add.
 * @param[addr] A string of 12 characters(0-9,a-f,A-F).
 *
 * @example AT+LEFLTR=1,"00ff00123456"\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_set_filter(uint8_t para_num, const char *param)
{
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief get ble role.
 *
 * @Format AT+LEFLTR?\r\n
 *
 * @param null.
 *
 * @example AT+LEFLTR?\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_get_filter(const char *param)
{
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief set ble scan enable.
 *
 * @Format AT+LESCAN=<enable>\r\n
 *
 * @param[enable] value range 0-1, 0 is disable, 1 is enable.
 *
 * @example AT+LESCAN=1\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_set_scan_enable(uint8_t para_num, const char *param)
{
    uint8_t le_role = ln_kv_ble_usr_data_get()->ble_role;

    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_int_param(g_para_index, &g_is_default, &g_para_int))
    {
        return LN_AT_ERR_PARAM;
    }

    if(le_role & BLE_ROLE_CENTRAL)
    {
        if(g_para_int)
            ln_ble_scan_start(&le_scan_mgr_info_get()->scan_param);
        else
            ln_ble_scan_stop();

        ln_at_printf(LN_AT_RET_OK_STR);
    }

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief get scan state.
 *
 * @Format AT+LESCAN?\r\n
 *
 * @param null.
 *
 * @example AT+LESCAN?\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_get_scan_sta(const char *param)
{
    uint8_t le_role = ln_kv_ble_usr_data_get()->ble_role;

    if(le_role & BLE_ROLE_CENTRAL)
    {
        ln_at_printf("%s:%d\r\n", param, le_scan_state_get());
    }

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief client read request to server device.
 *
 * @Format AT+LERDREQ=<conn_idx>,<handle>\r\n
 *
 * @param[conn_idx] connection index, value is 0 for only one ble link.
 * @param[handle] attribute handle
 *
 * @example AT+LERDREQ=0,17\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_gatt_read_req(uint8_t para_num, const char *name)
{
#if(BLE_DATA_TRANS_CLIENT)
    char *str_offset = NULL;
    int i = 0;
    int conn_index = 0;
    int conn_hdl= 0;
    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_str_param(g_para_index, &g_is_default, &g_para_str))
    {
        return LN_AT_ERR_PARAM;
    }
    str_offset = (char *)g_para_str;
    for(i=0;i<strlen(g_para_str);i++)
    {
        if(',' == str_offset[i])
            break;
    }
    conn_index = stoi(str_offset, i);
    str_offset  = str_offset+i+1;
    for(i=0;i<strlen(str_offset);i++)
    {
        if(',' == str_offset[i])
            break;
    }
    conn_hdl = stoi(str_offset, i);
    str_offset  = str_offset+i+1;


    if(conn_index >= 0 && conn_hdl >= 0)
    {
        ln_ble_trans_cli_read(conn_index, conn_hdl, 0);
    }
#endif
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief client write request to server device.
 *
 * @Format AT+LEWRREQ=<conn_idx>,<handle>,<data>\r\n
 *
 * @param[conn_idx] connection index, value is 0 for only one ble link.
 * @param[handle] attribute handle
 * @param[data] aiisc format data.
 *
 * @example AT+LEWRREQ=0,17,123456\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_gatt_write_req(uint8_t para_num, const char *name)
{
 #if (BLE_DATA_TRANS_CLIENT)
    char *str_offset = NULL;
    int i = 0;
    int conn_index = 0;
    int conn_hdl= 0;
    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_str_param(g_para_index, &g_is_default, &g_para_str))
    {
        return LN_AT_ERR_PARAM;
    }
    str_offset = (char *)g_para_str;
    for(i=0;i<strlen(g_para_str);i++)
    {
        if(',' == str_offset[i])
            break;
    }
    conn_index = stoi(str_offset, i);
    str_offset  = str_offset+i+1;
    for(i=0;i<strlen(str_offset);i++)
    {
        if(',' == str_offset[i])
            break;
    }
    conn_hdl = stoi(str_offset, i);
    str_offset  = str_offset+i+1;

    if(conn_index >= 0 && conn_hdl >= 0)
    {
        ln_trans_cli_send_t send_info;
        send_info.conn_idx = conn_index;
        send_info.hdl = conn_hdl;
        send_info.len = strlen(str_offset);
        send_info.data = (uint8_t *)str_offset;
        ln_ble_trans_cli_write_req(&send_info);
    }
#endif

    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief client write command to server device.
 *
 * @Format AT+LEWRCMD=<conn_idx>,<handle>,<data>\r\n
 *
 * @param[conn_idx] connection index, value is 0 for only one ble link.
 * @param[handle] attribute handle
 * @param[data] aiisc format data.
 *
 * @example AT+LEWRCMD=0,17,123456\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_gatt_write_cmd(uint8_t para_num, const char *name)
{
#if (BLE_DATA_TRANS_CLIENT)
    char *str_offset = NULL;
    int i = 0;
    int conn_index = 0;
    int conn_hdl= 0;
    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_str_param(g_para_index, &g_is_default, &g_para_str))
    {
        return LN_AT_ERR_PARAM;
    }
    str_offset = (char *)g_para_str;
    for(i=0;i<strlen(g_para_str);i++)
    {
        if(',' == str_offset[i])
            break;
    }
    conn_index = stoi(str_offset, i);
    str_offset  = str_offset+i+1;
    for(i=0;i<strlen(str_offset);i++)
    {
        if(',' == str_offset[i])
            break;
    }
    conn_hdl = stoi(str_offset, i);
    str_offset  = str_offset+i+1;

    if(conn_index >= 0 && conn_hdl >= 0)
    {
        ln_trans_cli_send_t send_info;
        send_info.conn_idx = conn_index;
        send_info.hdl = conn_hdl;
        send_info.len = strlen(str_offset);
        send_info.data = (uint8_t *)str_offset;
        ln_ble_trans_cli_write_cmd(&send_info);
    }
#endif
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/* --------------------server interface -----------------------*/

/**
 ****************************************************************************************
 * @brief server send notify to client device.
 *
 * @Format AT+LENTF=<conn_idx>,<handle>,<data>\r\n
 *
 * @param[conn_idx] connection index, value is 0 for only one ble link.
 * @param[handle] attribute handle
 * @param[data] aiisc format data.
 *
 * @example AT+LENTF=0,17,123456\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_gatt_notify(uint8_t para_num, const char *name)
{
    char *str_offset = NULL;
    int i = 0;
    int conn_index = 0;
    int conn_hdl= 0;
    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_str_param(g_para_index, &g_is_default, &g_para_str))
    {
        return LN_AT_ERR_PARAM;
    }
    str_offset = (char *)g_para_str;
    for(i=0;i<strlen(g_para_str);i++)
    {
        if(',' == str_offset[i])
            break;
    }
    conn_index = stoi(str_offset, i);
    str_offset  = str_offset+i+1;
    for(i=0;i<strlen(str_offset);i++)
    {
        if(',' == str_offset[i])
            break;
    }
    conn_hdl = stoi(str_offset, i);
    str_offset  = str_offset+i+1;

#if(BLE_DATA_TRANS_SERVER)
    if(conn_index >= 0 && conn_hdl >= 0)
    {
        ln_trans_svr_send_t send_info;
        send_info.conn_idx = conn_index;
        send_info.hdl = conn_hdl;
        send_info.len = strlen(str_offset);
        send_info.data = (uint8_t *)str_offset;
        ln_ble_trans_svr_ntf(&send_info);
    }
#endif

    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief server send indication to client device.
 *
 * @Format AT+LEIND=<conn_idx>,<handle>,<data>\r\n
 *
 * @param[conn_idx] connection index, value is 0 for only one ble link.
 * @param[handle] attribute handle
 * @param[data] aiisc format data.
 *
 * @example AT+LEIND=0,17,123456\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_gatt_indication(uint8_t para_num, const char *name)
{
    char *str_offset = NULL;
    int i = 0;
    int conn_index = 0;
    int conn_hdl= 0;
    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_str_param(g_para_index, &g_is_default, &g_para_str))
    {
        return LN_AT_ERR_PARAM;
    }
    str_offset = (char *)g_para_str;
    for(i=0;i<strlen(g_para_str);i++)
    {
        if(',' == str_offset[i])
            break;
    }
    conn_index = stoi(str_offset, i);
    str_offset  = str_offset+i+1;
    for(i=0;i<strlen(str_offset);i++)
    {
        if(',' == str_offset[i])
            break;
    }
    conn_hdl = stoi(str_offset, i);
    str_offset  = str_offset+i+1;

#if (BLE_DATA_TRANS_SERVER)
    if(conn_index >= 0 && conn_hdl >= 0)
    {
        ln_trans_svr_send_t send_info;
        send_info.conn_idx = conn_index;
        send_info.hdl = conn_hdl;
        send_info.len = strlen(str_offset);
        send_info.data = (uint8_t *)str_offset;

        ln_ble_trans_svr_ind(&send_info);
        }
#endif
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief set ble advertising enable.
 *
 * @Format AT+LEADV=<enable>\r\n
 *
 * @param[enable] value range 0-1, 0 is disable, 1 is enable.
 *
 * @example AT+LEADV=1\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_set_adv_enable(uint8_t para_num, const char *param)
{
    uint8_t le_role = ln_kv_ble_usr_data_get()->ble_role;
    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_int_param(g_para_index, &g_is_default, &g_para_int))
    {
        return LN_AT_ERR_PARAM;
    }

    if(le_role & BLE_ROLE_CENTRAL)
    {
        if(g_para_int)
            ln_ble_adv_start();
        else
            ln_ble_adv_stop();

        ln_at_printf(LN_AT_RET_OK_STR);
    }

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief get advertising state.
 *
 * @Format AT+LEADV?\r\n
 *
 * @param null.
 *
 * @example AT+LEADV?\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_get_adv_sta(const char *param)
{
    uint8_t le_role = ln_kv_ble_usr_data_get()->ble_role;
    if(le_role & BLE_ROLE_PERIPHERAL)
    {
        ln_at_printf("%s:%d\r\n", param, le_adv_state_get());

        ln_at_printf(LN_AT_RET_OK_STR);
    }

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief set ble advertising data.
 *
 * @Format AT+LEADVDATA=<data>\r\n
 *
 * @param[data] aiisc format data.
 *
 * @example AT+LEADVDATA="1235433"\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_set_adv_data(uint8_t para_num, const char *param)
{
    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_str_param(g_para_index, &g_is_default, &g_para_str))
    {
        return LN_AT_ERR_PARAM;
    }
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 ****************************************************************************************
 * @brief set connection paramter.
 *
 * @Format AT+LECONPARAM=<intv_min>,<intv_max>,<latency>,<time_out>\r\n
 *
 * @param[intv_min] Connection interval minimum, range: 6~0x0c80, time = N*1.25ms (7.5ms--4s).
 * @param[intv_max] Connection interval maximum, range: 6~0x0c80, time = N*1.25ms (7.5ms--4s).
 * @param[latency] Latency, range: 0~0x01f3.
 * @param[time_out] Supervision timeout, range:0~0x0c80, time = N*10ms (100ms--32s).
 *
 * @example AT+LECONPARAM=50,60,5,3000\r\n
 ****************************************************************************************
 */
static ln_at_err_t ln_at_set_conn_param(uint8_t para_num, const char *param)
{
    ln_ble_conn_param_t conn_param;

    ln_at_bt_uart_param_reset();
    if(ln_at_parser_get_int_param(g_para_index++, &g_is_default, &conn_param.intv_min))
    {
        return LN_AT_ERR_PARAM;
    }
    if(ln_at_parser_get_int_param(g_para_index++, &g_is_default, &conn_param.intv_max))
    {
        return LN_AT_ERR_PARAM;
    }
    if(ln_at_parser_get_int_param(g_para_index++, &g_is_default, &conn_param.latency))
    {
        return LN_AT_ERR_PARAM;
    }
    if(ln_at_parser_get_int_param(g_para_index++, &g_is_default, &conn_param.time_out))
    {
        return LN_AT_ERR_PARAM;
    }

    for(int i=0; i<BLE_CONN_DEV_NUM_MAX;i++)
    {
        le_dev_con_env_t *con_env =  &le_conn_mgr_info_get()->con_env[i];
        if(LE_CONN_STATE_CONNECTED == con_env->state)
            ln_ble_conn_param_update(i, &conn_param);
    }

    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

/**
 * name, get, set, test, exec
*/
/*ble test command group*/
LN_AT_CMD_REG(LETSTTX,          NULL,                   ln_at_set_tx_test_start,NULL,       NULL);
LN_AT_CMD_REG(LETSTRX,          NULL,                   ln_at_set_rx_test_start,NULL,       NULL);
LN_AT_CMD_REG(LETSTSTOP,        NULL,                   NULL,                   NULL,       ln_at_exec_bt_test_stop);

/*ble general command group*/
LN_AT_CMD_REG(LEVER,            ln_at_get_version,      NULL,                   NULL,       NULL);
LN_AT_CMD_REG(LEBAUDRATE,       ln_at_get_baudrate,     ln_at_set_baudrate,     NULL,       NULL);
LN_AT_CMD_REG(LERST,            NULL,                   NULL,                   NULL,       ln_at_exec_bt_soft_reset);
LN_AT_CMD_REG(LEFTYRST,         NULL,                   NULL,                   NULL,       ln_at_exec_bt_soft_factory_reset);
LN_AT_CMD_REG(LEROLE,           ln_at_get_ble_role,     ln_at_set_bt_role,      NULL,       NULL);
LN_AT_CMD_REG(LESEC,            NULL,                   ln_at_set_security,     NULL,       NULL);
LN_AT_CMD_REG(LEPDL,            ln_at_get_pair_dev_list,NULL,                   NULL,       NULL);
LN_AT_CMD_REG(LENAME,           ln_at_get_bt_name,      ln_at_set_bt_name,      NULL,       NULL);
LN_AT_CMD_REG(LEADDR,           ln_at_get_bt_addr,      ln_at_set_bt_addr,      NULL,       NULL);
LN_AT_CMD_REG(LECONN,           ln_at_get_conn_status,  ln_at_bt_connect_req,   NULL,       NULL);
LN_AT_CMD_REG(LEDISC,           NULL,                   ln_at_bt_disconnect_req,NULL,       ln_at_bt_disconnect_all);
LN_AT_CMD_REG(LETXPWR,          ln_at_get_tx_power,     ln_at_set_tx_power,     NULL,       NULL);
LN_AT_CMD_REG(LECONPARAM,       ln_at_get_conn_param,   ln_at_set_conn_param,   NULL,       NULL);

/*ble master command group*/
LN_AT_CMD_REG(LEWHTLIST,        ln_at_get_white_list,   ln_at_set_white_list,   NULL,       ln_at_clear_white_list);
LN_AT_CMD_REG(LEFLTR,           ln_at_get_filter,       ln_at_set_filter,       NULL,       NULL);
LN_AT_CMD_REG(LESCAN,           ln_at_get_scan_sta,     ln_at_set_scan_enable,  NULL,       NULL);
LN_AT_CMD_REG(LERDREQ,          NULL,                   ln_at_gatt_read_req,    NULL,       NULL);
LN_AT_CMD_REG(LEWRREQ,          NULL,                   ln_at_gatt_write_req,   NULL,       NULL);
LN_AT_CMD_REG(LEWRCMD,          NULL,                   ln_at_gatt_write_cmd,   NULL,       NULL);

/*ble slave command group*/
LN_AT_CMD_REG(LENTF,            NULL,                   ln_at_gatt_notify,      NULL,       NULL);
LN_AT_CMD_REG(LEIND,            NULL,                   ln_at_gatt_indication,  NULL,       NULL);
LN_AT_CMD_REG(LEADV,            ln_at_get_adv_sta,      ln_at_set_adv_enable,   NULL,       NULL);
LN_AT_CMD_REG(LEADVDATA,        NULL,                   ln_at_set_adv_data,     NULL,       NULL);

