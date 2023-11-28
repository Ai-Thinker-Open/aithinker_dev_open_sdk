/**
 * @file   ln_at_cmd_wifi.h
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-28     MurphyZhao   the first version
 */

#include "utils/debug/log.h"
#include "utils/ln_psk_calc.h"
#include "utils/system_parameter.h"
#include "utils/ln_list.h"
#include "utils/ln_misc.h"
#include <stdlib.h>
#include <string.h>

#include <lwip/sockets.h>
#include "lwip/inet.h"
#include "wifi_manager.h"

#include "dhcpd_api.h"
#include "dhcpd.h"
#include "wifi.h"
#include "wifi_port.h"
#include "dhcpd.h"

#include "ln_at.h"
#include "ln_nvds.h"
#include "ln_at_cmd.h"

#include "wifi_driver_port.h"

#define LN_AT_DEFAULT_MAX_CONN  (3U)

typedef enum
{
    LN_AT_ECN_OPEN          = 0,
    LN_AT_ECN_WPA_PSK       = 2,
    LN_AT_ECN_WPA2_PSK      = 3,
    LN_AT_ECN_WPA_WPA2_PSK  = 4,
    LN_AT_ECN_MAX,
} ln_at_ecn_type_t;


typedef enum
{
    LN_AT_STORE_M_N_W_F = 0, /* Not write command config to flash */
    LN_AT_STORE_M_W_F   = 1, /* Write command config to flash */
} ln_at_store_mode_e;

typedef struct
{
    wifi_mode_t     mode;
} ln_at_ctrl_t;

static ln_at_ctrl_t g_ln_at_ctrl =
{
    .mode = WIFI_MODE_MAX,
};

static ln_at_ctrl_t *g_ln_at_ctrl_p = &g_ln_at_ctrl;
static void * sem_scan = NULL;
static ln_at_store_mode_e g_at_store_mode = LN_AT_STORE_M_W_F;

//static void ln_at_cache_wifi_mode_set(wifi_mode_t mode);
static ln_at_err_t ln_at_set_wifi_mode_def(uint8_t para_num, const char *name);

static int str_del_space_check_hex(char *s1)
{
    char *s2;
    s2 = s1;
    while (*s1) {
        if (*s1 == ' ') {
            s1++;
        } else if(!isxdigit(*s1) && (*s1 != ':')) {
            return -1;
        }else {
            *s2 = *s1;
            s1++;
            s2++;
        }
    }
    *s2 = '\0';
    
    return 0;
}

static int strmac2mac(char * str, uint8_t *mac)//"00:50:66:77:88:99"
{
    int16_t len = 0;
    char * pos_start = str;
    char value_ch[3];
    uint8_t bt_mac_str[6] = {0,};
    const char *hex_str = "0123456789abcdefABCDEF";
    
    if (pos_start == NULL) {
        return -1;
    }
    
    if (0 != str_del_space_check_hex(pos_start)) {
        return -1;
    }
    
    for (uint32_t idx = 0; idx < 6; idx++)
    {
        if(pos_start > 0) 
        {
            if (*pos_start == ':') {
                return -1;
            }
            pos_start = strpbrk(pos_start, hex_str);
            len = strspn(pos_start, hex_str);

            memset(value_ch, '\0', sizeof(value_ch));
            if (len == 1){
                memcpy(value_ch, pos_start, len);
                bt_mac_str[idx] = ln_char2hex(value_ch[0]);
            } else if (len == 2) {
                memcpy(value_ch, pos_start, len);
                bt_mac_str[idx] =  ln_char2hex(value_ch[0]) << 4;
                bt_mac_str[idx] += ln_char2hex(value_ch[1]);
            } else {
                return -1;
            }

            pos_start = strchr(pos_start,':'); //find next
            pos_start += sizeof(char);
        }
        else
        {
            return -1;
        }
    }
    
    memcpy(mac, bt_mac_str, 6);
    return 0;
}


wifi_mode_t ln_get_wifi_mode_from_at(void)
{
    return g_ln_at_ctrl_p->mode;
}

void ln_set_wifi_mode_from_at(wifi_mode_t _mode)
{
    g_ln_at_ctrl_p->mode = _mode;
}

// private command
// char ln_at_set_pvtcmd(char *str)
static ln_at_err_t ln_at_set_pvtcmd(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);

    bool is_default = false;
    char *pvtcmd = NULL;
    // uint8_t para_index = 1;
    if (para_num != 1)
    {
        goto err;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(1, &is_default, &pvtcmd))
    {
        goto err;
    }

    if (is_default || !pvtcmd)
    {
        goto err;
    }

    LOG(LOG_LVL_INFO, "Raw pvtcmd:%s;len:%d\r\n", pvtcmd, strlen(pvtcmd));

    if(0 != wifi_private_command(pvtcmd)) {
        LOG(LOG_LVL_ERROR, "pvt cmd parse failed!\r\n");
        goto err;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

/**
 * need to be called by user when power-on
*/
//void ln_at_cache_wifi_mode_set(wifi_mode_t mode)
//{
//    g_ln_at_ctrl_p->mode = mode;
//}

/**
 * default: get from RAM
 *
*/
static ln_at_err_t _ln_at_get_wifi_mode(ln_at_cmd_type_e type, const char *name)
{
    int temp;
    wifi_mode_t mode;

    if (type == LN_AT_CMD_TYPE_DEF)
    {
        if (0 != sysparam_poweron_wifi_mode_get(&mode))
        {
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_COMMON;
        }
    }
    else
    {
        if (g_ln_at_ctrl_p->mode == WIFI_MODE_MAX)
        {
            mode = g_ln_at_ctrl_p->mode = wifi_current_mode_get();
        }
        else
        {
            mode = g_ln_at_ctrl_p->mode;
        }
        LOG(LOG_LVL_INFO, "current work mode:%d\r\n", mode);
    }

    switch(mode)
    {
    case WIFI_MODE_STATION:
    case WIFI_MODE_AP:
    case WIFI_MODE_OFF:
        temp = LN2ESP_WIFI_MODE(mode);
        break;
    default:
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_NOT_SUPPORT;
    }

    ln_at_printf("%s:%d\r\n", name, (int)temp);
    ln_at_printf(LN_AT_RET_OK_STR);

    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_at_get_wifi_mode(const char *name)
{
    return _ln_at_get_wifi_mode(LN_AT_CMD_TYPE_NONE, name);
}

/**
 * get from RAM
*/
static ln_at_err_t ln_at_get_wifi_mode_current(const char *name)
{
    return _ln_at_get_wifi_mode(LN_AT_CMD_TYPE_CUR, name);
}

/**
 * get from flash cfg area
*/
static ln_at_err_t ln_at_get_wifi_mode_def(const char *name)
{
    return _ln_at_get_wifi_mode(LN_AT_CMD_TYPE_DEF, name);
}

/**
 * @brief _ln_at_wifi_mode_parse
 *     AT+CWMODE=<mode>[,<auto_connect>]
 * @param para_num
 * @param out_data
 * @return ln_at_err_t
 */
static ln_at_err_t _ln_at_wifi_mode_parse(uint8_t para_num, wifi_mode_t *out_data)
{
    wifi_mode_t mode;
    int temp = 0;
    bool is_default = false;

    if (para_num != 1 && para_num != 2)
    {
        return LN_AT_ERR_PARAM;
    }
    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(1, &is_default, &temp))
    {
        return LN_AT_ERR_PARSE;
    }

    LOG(LOG_LVL_ERROR, "is_default:%d; value:%d\r\n", is_default, temp);

    if (is_default)
    {
        return LN_AT_ERR_FORMAT;
    }

    if (temp > 3 || temp < 0)
    {
        LOG(LOG_LVL_ERROR, "Input param error! param num:%d, first param:%d\r\n", para_num, temp);
        return LN_AT_ERR_PARAM;
    }

    switch(temp)
    {
    case ESP_WIFI_TURNOFF:
    case ESP_WIFI_STA:
    case ESP_WIFI_AP:
        mode = ESP2LN_WIFI_MODE(temp);
        break;
    default:
        LOG(LOG_LVL_ERROR, "Not Support AT+STA mode! And not support switch off rf!");
        return LN_AT_ERR_NOT_SUPPORT;
    }

    *out_data = mode;
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_at_set_wifi_mode(uint8_t para_num, const char *name)
{
    return ln_at_set_wifi_mode_def(para_num, name);
}

/**
 * set to RAM
*/
static ln_at_err_t ln_at_set_wifi_mode_current(uint8_t para_num, const char *name)
{
    return ln_at_set_wifi_mode_def(para_num, name);
}

/**
 * @brief ln_at_set_wifi_mode_def, Restart to take effect
 *     mode:
 *     0: No wifi, switch off wifi rf
 *     1: station mode
 *     2: ap mode
 *     3: ap+station mode, not support
 * @param para_num
 * @param name
 * @return ln_at_err_t
 */
static ln_at_err_t ln_at_set_wifi_mode_def(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);

    ln_at_err_t ret = LN_AT_ERR_NONE;
    wifi_mode_t mode = WIFI_MODE_STATION;
    wifi_mode_t curr_mode;

	int temp = 1;
    bool is_default = false;
    wifi_scan_cfg_t scan_cfg = {0};
    LN_WIFI_DRIVER_ADAPTER* p = ln_get_wifi_drv_handle();
    wifi_sta_connect_t *conn = &p->conn;
	bool originReConn = p->reConn;

    if (LN_AT_ERR_NONE != (ret = _ln_at_wifi_mode_parse(para_num, &mode)))
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return ret;
    }

	//reconn check
	if (mode == WIFI_MODE_STATION)
	{
		if (para_num == 1)
		{
			p->reConn = true;
		}
		else
		{
			if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(2, &is_default, &temp))
			{
				goto __exit;
			}
			else
			{
				if (temp != 0 && temp != 1)
				{
					goto __exit;
				}
				p->reConn = temp;
			}
			Log_i("is_default:%d, reConn:%d", is_default, temp);
		}
	}

	//if same mode
    curr_mode = g_ln_at_ctrl_p->mode;
    if (curr_mode == mode && originReConn == p->reConn)
    {
        g_ln_at_ctrl_p->mode = curr_mode;
        LOG(LOG_LVL_INFO, "No need to switch mode\r\n");
        goto __ok;
    }

	//change mode
    p->stop();
    if (curr_mode == WIFI_MODE_STATION)
    {
        netdev_set_state(NETIF_IDX_STA, NETDEV_DOWN);
    }
    else if (curr_mode == WIFI_MODE_AP)
    {
        netdev_set_state(NETIF_IDX_AP, NETDEV_DOWN);
    }

    if (mode == WIFI_MODE_STATION)
    {	
        if (p->reConn && (g_at_store_mode != LN_AT_STORE_M_N_W_F))
        {
            if (SYSPARAM_ERR_NONE != sysparam_sta_scan_cfg_get(&scan_cfg))
            {
                goto __exit;
            }

            if (SYSPARAM_ERR_NONE != sysparam_sta_conn_cfg_get(conn))
            {
                goto __exit;
            }

            p->sta_connect();
        }
    }
		else if(mode == WIFI_MODE_AP)
		{
			p->start_softap();
		}
    else if (mode == WIFI_MODE_OFF) {
        Log_i("turn off wifi");
    }

    g_ln_at_ctrl_p->mode = mode;

    if (g_at_store_mode != LN_AT_STORE_M_N_W_F)
    {
        if (SYSPARAM_ERR_NONE != sysparam_poweron_wifi_mode_update(mode))
        {
            goto __exit;
        }
    }

__ok:
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ret = LN_AT_ERR_COMMON;
    ln_at_printf(LN_AT_RET_ERR_STR);
    return ret;
}

/**
 * @brief ln_at_help_wifi_mode
 *     mode:
 *     0: No wifi, switch off wifi rf
 *     1: station mode
 *     2: ap mode
 *     3: ap+station mode, not support
 * @param name
 * @return ln_at_err_t
 */
static ln_at_err_t ln_at_help_wifi_mode(const char *name)
{
    ln_at_printf("%s:(0-3)\r\n", name);
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_at_help_wifi_mode_current(const char *name)
{
    return ln_at_help_wifi_mode(name);
}

static ln_at_err_t ln_at_help_wifi_mode_def(const char *name)
{
    return ln_at_help_wifi_mode(name);
}

/**
 * AT+CWJAP
 * AT+CWJAP_CUR
 * AT+CWJAP_DEF
 *
 * GET:
 *     AT+CWJAP?  -->
 *       +CWJAP:<ssid>,<bssid>,<channel>,<rssi>,<pci_en>,<reconn_interval>,<listen_interval>,<scan_mode>,<pmf>
 *       OK
 * SET:
 *     AT+CWJAP=[<ssid>],[<pwd>][,<bssid>][,<pci_en>][,<reconn_interval>][,<listen_interval>][,<scan_mode>][,<jap_timeout>][,<pmf>]
 *
 *       WIFI CONNECTED
 *       WIFI GOT IP
 *
 *       OK
 *       [WIFI GOT IPv6 LL]
 *       [WIFI GOT IPv6 GL]
 *   or
 *       +CWJAP:<error code>
 *       ERROR
 * EXEC: Connect to the AP in the last Wi-Fi configuration
 *     AT+CWJAP
 * As station mode to join specific ap.
 *
 * If at cache mode is equal to flash cache mode, means that wifi is in this mode in the current;
 * If at cache mode is not equal to flash cache mode, means that wifi need to be switch to at cache mode.
 *
 * if at cache mode and flash cache mode both are not sta mode, return error.
 * if at cache mode is ap, and flash cache mode is sta or ap, return error.
*/

static ln_at_err_t _ln_at_sta_get_link_ap_info(const char *name)
{
    wifi_mode_t mode;
    wifi_sta_status_t sta_status = WIFI_STA_STATUS_STARTUP;
    wifi_scan_cfg_t   scan_cfg   = {0,};

    int8_t         rssi  = 0;
    const char    *ssid  = NULL;
    const uint8_t *bssid = NULL;
			
		LN_WIFI_DRIVER_ADAPTER* pWifi = ln_get_wifi_drv_handle();

    if (!name)
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_PARAM;
    }

    if (g_ln_at_ctrl_p->mode == WIFI_MODE_MAX)
    {
        mode = g_ln_at_ctrl_p->mode = wifi_current_mode_get();
    }
    else
    {
        mode = g_ln_at_ctrl_p->mode;
    }

    if (mode != WIFI_MODE_STATION)
    {
        ln_at_printf(LN_AT_RET_OK_STR);
        return LN_AT_ERR_NONE;     /* Future adaptation 8266 */
    }

    if (WIFI_ERR_NONE != wifi_get_sta_conn_info(&ssid, &bssid) || \
            WIFI_ERR_NONE != wifi_get_sta_status(&sta_status))
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_COMMON;
    }

    /* Check that the STA is connected to the AP */
    if (sta_status != WIFI_STA_STATUS_CONNECTED) /* sta is not connected */
    {
        ln_at_printf(LN_AT_RET_OK_STR);
        return LN_AT_ERR_NONE;     /* Future adaptation 8266 */
    }

    if (!ssid || !bssid)
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_COMMON;
    }

    if (WIFI_ERR_NONE != wifi_get_sta_scan_cfg(&scan_cfg) || \
            WIFI_ERR_NONE != wifi_sta_get_rssi(&rssi))
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_COMMON;
    }

    ln_at_printf("%s:\"%s\",\"%02x:%02x:%02x:%02x:%02x:%02x\",%d,%d,%d,%d,%d,%d,%d\r\n", name, ssid,
                 bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],
                 scan_cfg.channel, (int8_t)(rssi), pWifi->sta.pci_en, pWifi->sta.reconn_interval, pWifi->sta.listen_interval, pWifi->sta.scan_mode, pWifi->sta.pmf);
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

/**
 * default: get from RAM
 *
*/
static ln_at_err_t ln_at_get_wifi_join_ap(const char *name)
{
    return _ln_at_sta_get_link_ap_info(name);
}

/**
 * get from RAM
*/
static ln_at_err_t ln_at_get_wifi_join_ap_current(const char *name)
{
    return _ln_at_sta_get_link_ap_info(name);
}

/**
 * get from flash cfg area
*/
static ln_at_err_t ln_at_get_wifi_join_ap_def(const char *name)
{
    return _ln_at_sta_get_link_ap_info(name);
}

/**
 * AT+CWJAP=<ssid>,<pwd>[,<bssid>]
 * AT+CWJAP=[<ssid>],[<pwd>][,<bssid>][,<pci_en>][,<reconn_interval>][,<listen_interval>][,<scan_mode>][,<jap_timeout>][,<pmf>]
 * WIFI DISCONNECT
 * WIFI CONNECTED
 * WIFI GOT IP
 *
 * OK
*/
static ln_at_err_t _ln_at_parse_cwjap(uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;

    wifi_mode_t mode;
    sta_ps_mode_t ps_mode;
    wifi_scan_cfg_t scan_cfg = {0};
    wifi_sta_connect_t conn = {0};
    uint8_t psk_value[40] = {0x0};
    uint8_t mac_addr[MAC_ADDRESS_LEN] = {0};

    char *ssid_p = NULL;
    char *pwd_p = NULL;
    char *bssid_p = NULL;
		
		int pci_en = 0;
		int scan_mode = 0;
		int pmf = 0;
		int listen_interval = 3;
		int reconn_interval = 1;
		int jap_timeout = 15;

    uint8_t bssid_hex[BSSID_LEN];

    LN_WIFI_DRIVER_ADAPTER* pWifi = ln_get_wifi_drv_handle();

    if (!name || (para_num > 9))
    {
        return LN_AT_ERR_PARAM;
    }

    if (g_ln_at_ctrl_p->mode == WIFI_MODE_MAX)
    {
        mode = g_ln_at_ctrl_p->mode = wifi_current_mode_get();
    }
    else
    {
        mode = g_ln_at_ctrl_p->mode;
    }

    if (mode != WIFI_MODE_STATION)
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_NONE;     /* Future adaptation 8266 */
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &ssid_p))
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_PARSE;
    }

    if (is_default || !ssid_p)
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_FORMAT;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pwd_p))
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_PARSE;
    }

    if (is_default || !pwd_p)
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_FORMAT;
    }

    if (para_num > 3)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &bssid_p))
        {
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_PARSE;
        }

        if (is_default || !bssid_p)
        {
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_FORMAT;
        }
    }
		
		//add default para -- pci_en , reconn_interval , listen_interval , scan_mode , jap_timeout , pmf
		if(para_num > 4)
		{
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &pci_en))
        {
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_PARSE;
        }
				if(pci_en != 0 && pci_en != 1)
				{
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_PARSE;
				}
		}
		if(is_default)
		{
			pci_en = 0;
		}			

		//add default para -- pci_en , reconn_interval , listen_interval , scan_mode , jap_timeout , pmf
		if(para_num > 5)
		{
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &reconn_interval))
        {
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_PARSE;
        }
				if(reconn_interval < 0 || reconn_interval > 7200)
				{
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_PARSE;
				}
		}
		if(is_default)
		{
			reconn_interval = 1;
		}			

		//add default para -- pci_en , reconn_interval , listen_interval , scan_mode , jap_timeout , pmf
		if(para_num > 6)
		{
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &listen_interval))
        {
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_PARSE;
        }
				if(listen_interval < 1 || listen_interval > 100)
				{
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_PARSE;
				}		
		}
		if(is_default)
		{
			listen_interval = 3;
		}			

		//add default para -- pci_en , reconn_interval , listen_interval , scan_mode , jap_timeout , pmf
		if(para_num > 7)
		{
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &scan_mode))
        {
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_PARSE;
        }
				if(scan_mode != 0 && scan_mode != 1)
				{
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_PARSE;
				}
		}
		if(is_default)
		{
			scan_mode = 0;
		}			

		//add default para -- pci_en , reconn_interval , listen_interval , scan_mode , jap_timeout , pmf
		if(para_num > 8)
		{
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &jap_timeout))
        {
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_PARSE;
        }
				if(jap_timeout < 3 || jap_timeout > 600)
				{
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_PARSE;
				}						
		}
		if(is_default)
		{
			jap_timeout = 15;
		}			

		//add default para -- pci_en , reconn_interval , listen_interval , scan_mode , jap_timeout , pmf
		if(para_num == 9)
		{
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &pmf))
        {
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_PARSE;
        }
				if(pmf != 0 && pmf != 1 && pmf !=2)
				{
            ln_at_printf(LN_AT_RET_ERR_STR);
            return LN_AT_ERR_PARSE;
				}
		}
		if(is_default)
		{
			pmf = 0;
		}			

		
		pWifi->sta.pci_en = (uint8_t)pci_en;
		pWifi->sta.reconn_interval = (uint16_t)reconn_interval;
		pWifi->sta.listen_interval = (uint8_t)listen_interval;
		pWifi->sta.scan_mode = (uint8_t)scan_mode;
		pWifi->sta.jap_timeout = (uint16_t)jap_timeout;
		pWifi->sta.pmf = (uint8_t)pmf;
		

    LOG(LOG_LVL_INFO, "AT CWJAP:[ssid:%s; pwd:%s; bssid:%s]\r\n",
        ssid_p, pwd_p, bssid_p == NULL ? "NULL" : bssid_p);

    if (bssid_p != NULL)
    {
        wlib_mac_str2hex((const uint8_t *)bssid_p, (uint8_t *)bssid_hex);
    }
	
    conn.ssid = ssid_p;
    conn.pwd = pwd_p;
    conn.bssid = ((bssid_p != NULL) ? bssid_hex : NULL);
    conn.psk_value = NULL;

	strcpy(pWifi->ssid_buf(), conn.ssid);
	strcpy(pWifi->pwd_buf(), conn.pwd);
	if (conn.bssid != NULL)
	{
		memcpy(pWifi->bssid_buf(), conn.bssid, BSSID_LEN);
	}
	else
	{
		memset(pWifi->bssid_buf(), 0, BSSID_LEN);
	}
	pWifi->sta_connect();

    if (g_at_store_mode != LN_AT_STORE_M_N_W_F)
    {
        /* save scan config and connect config into flash */
        if (0 != sysparam_sta_conn_cfg_update(&conn))
        {
            goto __exit;
        }
    }

    if (LN_OK != pWifi->wait_for_ip(20000))
    {
        goto __exit;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
__exit:


		ln_at_printf("%s:%d\r\n", name, 4); /* 4: connect fail */
		ln_at_printf(LN_AT_RET_ERR_STR);
		return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_set_wifi_join_ap(uint8_t para_num, const char *name)
{
    return _ln_at_parse_cwjap(para_num, name);
}

static ln_at_err_t ln_at_set_wifi_join_ap_current(uint8_t para_num, const char *name)
{
    return _ln_at_parse_cwjap(para_num, name);
}

static ln_at_err_t ln_at_set_wifi_join_ap_def(uint8_t para_num, const char *name)
{
    return _ln_at_parse_cwjap(para_num, name);
}

static ln_at_err_t ln_at_help_wifi_join_ap(const char *name)
{
    ln_at_printf("%s=<ssid>,<pwd>[,<bssid>]\r\n", name);
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_at_help_wifi_join_ap_current(const char *name)
{
    return ln_at_help_wifi_join_ap(name);
}

static ln_at_err_t ln_at_help_wifi_join_ap_def(const char *name)
{
    return ln_at_help_wifi_join_ap(name);
}

/**
 * AT+CWQAP
 *
 * Disable auto connect, then disconnect.
 *
 * This AT CMD can also be used in ap mode.
*/
static ln_at_err_t ln_at_exec_sta_disconn_ap(const char *name)
{
    LN_UNUSED(name);

    if (wifi_sta_disconnect() != 0)
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_COMMON;
    }

    OS_MsDelay(50);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

#if 0
/**
 * AT+CWLAPOPT=<sort_enable>,<mask>
*/
static ln_at_err_t ln_at_set_scan_rst_list_opt(uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;

    int sort_en = 0;
    int mask = 0;
    sort_rule_t rule = RSSI_SORT;

    if (para_num != 2)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &sort_en))
    {
        goto __exit;
    }
    if (is_default)
    {
        sort_en = 1;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &mask))
    {
        goto __exit;
    }
    if (is_default)
    {
        mask = 2047;
    }

    LOG(LOG_LVL_ERROR, "sort en:%d; mask:%d\r\n", sort_en, mask);

    if (sort_en != 0 && sort_en != 1)
    {
        goto __exit;
    }

    if (mask > 0x7FF)
    {
        goto __exit;
    }

    if (sort_en) {
        rule = RSSI_SORT;
    } else {
        rule = NORMAL_SORT;
    }

    wifi_manager_set_ap_list_sort_rule(rule);
    sysparam_ap_list_info_mask_update(mask);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

/**
 * AT+CWLAP
 *
 * AT+CWLAP=<ssid>[,<mac>,<channel>,<scan_type>,<scan_time_min>,<scan_time_max>]
*/
static ln_at_err_t ln_at_set_scan_opt(uint8_t para_num, const char *name)
{
    int ret = 0;
    bool is_default = false;
    uint8_t para_index = 1;

    /* Four bytes of space are reserved for possible characters \r\n */
    char *ssid_p = NULL;
    char *bssid_p = NULL;
    uint8_t bssid_hex[BSSID_LEN];

    int channel = 6;
    int scan_type = 0;
    int scan_time_min = 100;
    int scan_time_max = 1500;

    LN_AT_MEMSET(bssid_hex, 0X0, sizeof(bssid_hex));

    if (!(para_num == 1 || para_num == 6))
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &ssid_p))
    {
        goto __exit;
    }

    if (is_default || !ssid_p)
    {
        goto __exit;
    }

    if (para_num >= 2)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &bssid_p))
        {
            goto __exit;
        }
    }

    if (para_num >= 3)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &channel))
        {
            goto __exit;
        }
    }

    if (para_num >= 4)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &scan_type))
        {
            goto __exit;
        }
    }

    if (para_num >= 5)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &scan_time_min))
        {
            goto __exit;
        }
    }

    if (para_num >= 6)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &scan_time_max))
        {
            goto __exit;
        }
    }


    LOG(LOG_LVL_ERROR, "ssid:%s; bssid:%s; chl:%d; scan type:%d, mini time:%d, max time:%d\r\n",
        ssid_p, bssid_p, channel, scan_type, scan_time_min, scan_time_max);

    ret = ln_at_cmd_args_get(bssid_p, "%x:%x:%x:%x:%x:%x",
                             &bssid_hex[0], &bssid_hex[1], &bssid_hex[2], &bssid_hex[3], &bssid_hex[4], &bssid_hex[5]);

    LOG(LOG_LVL_ERROR, "ret:%d; mac addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n", ret,
        bssid_hex[0], bssid_hex[1], bssid_hex[2], bssid_hex[3], bssid_hex[4], bssid_hex[5]);

    if (ret != 6)
    {
        goto __exit;
    }

    /* TODO */

    ln_at_printf("+CWLAP:TODO\r\n");
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}
#endif /* 0 */

static void wifi_scan_complete_cb(void * arg)
{
    LN_UNUSED(arg);

    if (sem_scan) {
        ln_at_sem_release(sem_scan);
    }
}

static void output_ap_list(void)
{
    ln_list_t *list;
    uint8_t node_count = 0;
    ap_info_node_t *pnode;

    wifi_manager_ap_list_update_enable(LN_FALSE);
    wifi_manager_get_ap_list(&list, &node_count);

    LN_LIST_FOR_EACH_ENTRY(pnode, ap_info_node_t, list,list)
    {
        uint8_t * mac = (uint8_t*)pnode->info.bssid;
        ap_info_t *info = &pnode->info;

        //a good format
//        ln_at_printf("+CWLAP:[%02X:%02X:%02X:%02X:%02X:%02X]enc=%d,ch=%02d,rssi=%3d,ssid:\"%s\"\r\n", \
//        mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],info->authmode,info->channel,info->rssi,info->ssid);

        ln_at_printf("+CWLAP:");
        ln_at_printf("(%d,", info->authmode);
        ln_at_printf("\"%s\",", info->ssid);
        ln_at_printf("%3d,", info->rssi);
        ln_at_printf("\"%02x:%02x:%02x:%02x:%02x:%02x\",",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
        ln_at_printf("%02d,", info->channel);
        ln_at_printf("%d,", info->freq_offset);
        ln_at_printf("-1,");//freqcal_val
        ln_at_printf("4,");//pairwise_cipher
        ln_at_printf("4,");//group_cipher
        ln_at_printf("%d,", info->bgn);
        ln_at_printf("%d", info->wps_en);
        ln_at_printf(")\r\n");
    }

    wifi_manager_ap_list_update_enable(LN_TRUE);
}

static ln_at_err_t ln_at_exec_scan(const char *name)
{
    LN_UNUSED(name);

#define CONNECTED_SCAN_TIMES     (6)
#define DEFAULT_SCAN_TIMES       (1)
#define SCAN_TIMEOUT             (1500)

    int8_t scan_cnt = DEFAULT_SCAN_TIMES;

    wifi_sta_status_t sta_status = WIFI_STA_STATUS_STARTUP;
    wifi_scan_cfg_t   scan_cfg   = {0,};
    wifi_mode_t mode;

    if (g_ln_at_ctrl_p->mode == WIFI_MODE_MAX)
    {
        mode = g_ln_at_ctrl_p->mode = wifi_current_mode_get();
    }
    else
    {
        mode = g_ln_at_ctrl_p->mode;
    }

    if (mode != WIFI_MODE_STATION)
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_COMMON;
    }

    sysparam_sta_scan_cfg_get(&scan_cfg);

    //1. creat sem, reg scan complete callback.
    sem_scan = ln_at_sem_create(0, 1);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_SCAN_COMPLETE, &wifi_scan_complete_cb);

    //2. start scan, wait scan complete
    wifi_get_sta_status(&sta_status);
    if (sta_status == WIFI_STA_STATUS_CONNECTED || \
            sta_status == WIFI_STA_STATUS_DISCONNECTING)
    {
        scan_cnt = CONNECTED_SCAN_TIMES;
    }
    LOG(LOG_LVL_INFO, "Scan cnt:%d; scan timeout:%d\r\n", scan_cnt, SCAN_TIMEOUT);

    for (; scan_cnt > 0; scan_cnt--)
    {
        wifi_sta_scan(&scan_cfg);
        ln_at_sem_wait(sem_scan, SCAN_TIMEOUT);
    }

    //3. scan complete,output ap list.
    output_ap_list();

    //4. delete sem, callback
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_SCAN_COMPLETE, NULL);
    ln_at_sem_delete(sem_scan);
    sem_scan = NULL;
    wifi_manager_cleanup_scan_results();

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

/**
 * read from flash
 *
 * AT+CWSAP?
 * +CWSAP:<ssid>,<pwd>,<chl>,<ecn>,<max	conn>,<ssid	hidden>
 *
*/
static ln_at_err_t _ln_at_cwsap_get_parse(ln_at_cmd_type_e type, const char *name)
{
    LN_UNUSED(type);

    wifi_mode_t mode;
    int ecn = 0;
    ecn = ecn;

    char ssid[SSID_MAX_LEN + 1] = {0};
    char pwd[PASSWORD_MAX_LEN + 1] = {0};
    wifi_softap_ext_cfg_t ap_cfg;

    if (g_ln_at_ctrl_p->mode == WIFI_MODE_MAX)
    {
        mode = g_ln_at_ctrl_p->mode = wifi_current_mode_get();
    }
    else
    {
        mode = g_ln_at_ctrl_p->mode;
    }

    if (mode != WIFI_MODE_AP)
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_COMMON;
    }

    LN_AT_MEMSET(&ap_cfg, 0x0, sizeof(wifi_softap_ext_cfg_t));
    if (0 != sysparam_softap_ext_cfg_get(&ap_cfg))
    {
        goto __exit;
    }

    if (0 != sysparam_softap_ssidpwd_cfg_get(ssid, pwd))
    {
        goto __exit;
    }

    switch (ap_cfg.authmode)
    {
    case WIFI_AUTH_OPEN:
        ecn = 0;
        break;
    case WIFI_AUTH_WPA_PSK:
        ecn = 2;
        break;
    case WIFI_AUTH_WPA2_PSK:
        ecn = 3;
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        ecn = 4;
        break;
    default:
        break;
    }

    ln_at_printf("%s:\"%s\",\"%s\",%d,%d,%d,%d\r\n",
                 name, ssid, pwd, ap_cfg.channel, ap_cfg.authmode, ap_cfg.max_conn, ap_cfg.ssid_hidden);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_get_ap_start(const char *name)
{
    return _ln_at_cwsap_get_parse(LN_AT_CMD_TYPE_NONE, name);
}

static ln_at_err_t ln_at_get_ap_start_cur(const char *name)
{
    return _ln_at_cwsap_get_parse(LN_AT_CMD_TYPE_CUR, name);
}

static ln_at_err_t ln_at_get_ap_start_def(const char *name)
{
    return _ln_at_cwsap_get_parse(LN_AT_CMD_TYPE_DEF, name);
}

/**
 * AT+CWSAP=<ssid>,<pwd>,<chl>,<ecn>[,<max conn>][,<ssid hidden>]
 *
 * OK OR ERROR
 * AT+CWSAP="ap_name","12345678",1,4,2,0
 *
 * OK
 * +STA_CONNECTED:"96:ee:14:96:4c:55"
 * +DIST_STA_IP:"96:ee:14:96:4c:55","192.168.4.2"
 *
 * +STA_DISCONNECTED:"96:ee:14:96:4c:55"
*/
static ln_at_err_t _ln_at_cwsap_set_parse(ln_at_cmd_type_e type, uint8_t para_num, const char *name)
{
    LN_UNUSED(type);

    bool is_default = false;
    uint8_t para_index = 1;
    wifi_softap_cfg_t ap_cfg;
    wifi_mode_t mode;
    uint8_t psk_value[40] = {0x0};

    char *ssid_p = NULL;
    uint8_t bssid[BSSID_LEN] = {0};
    char *pwd_p = NULL;

    int chl = 6;                           /* default 6 */
    int ecn = 4;                           /* default 4: WPA_WPA2_PSK */
    int max_conn = LN_AT_DEFAULT_MAX_CONN; /* default 4 */
    int is_hidden = 0;

    if (!name || (para_num > 6))
    {
        goto __exit;
    }

    if (g_ln_at_ctrl_p->mode == WIFI_MODE_MAX)
    {
        mode = g_ln_at_ctrl_p->mode = wifi_current_mode_get();
    }
    else
    {
        mode = g_ln_at_ctrl_p->mode;
    }

    if (mode != WIFI_MODE_AP)
    {
        LOG(LOG_LVL_ERROR, "Not work in AP mode\r\n");
        goto __exit;
    }

    if (0 != sysparam_softap_mac_get(bssid))
    {
        goto __exit;
    }

    LOG(LOG_LVL_ERROR, "bssid: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
        bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &ssid_p))
    {
        goto __exit;
    }

    if (is_default || !ssid_p)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pwd_p))
    {
        goto __exit;
    }

    if (is_default)
    {
        pwd_p = NULL; /* open */
    }

    if (SYSPARAM_ERR_NONE != sysparam_softap_ext_cfg_get(&ap_cfg.ext_cfg))
    {
        goto __exit;
    }
    // LOG(LOG_LVL_INFO, "Factory data:\r\n"
    //         "ssid:%s; pwd:%s; channel:%d, ecn:%d, max conn:%d, hidden:%d\r\n",
    // ap_cfg.ssid, ap_cfg.pwd, ap_cfg.ext_cfg.channel, ap_cfg.ext_cfg.authmode,
    // max_conn, ap_cfg.ext_cfg.ssid_hidden);

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &chl))
    {
        goto __exit;
    }
    if (is_default)
    {
        chl = ap_cfg.ext_cfg.channel;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &ecn))
    {
        goto __exit;
    }
    if (is_default)
    {
        ecn = LN_AT_ECN_MAX;
    }

    if (para_num >= 5)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &max_conn))
        {
            goto __exit;
        }
				if(max_conn > LN_AT_DEFAULT_MAX_CONN)
				{
					goto __exit;
				}
        if (is_default)
        {
            max_conn = (int)LN_AT_DEFAULT_MAX_CONN;
        }
    }

    if (para_num >= 6)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &is_hidden))
        {
            goto __exit;
        }
        if (is_default)
        {
            is_hidden = (int)ap_cfg.ext_cfg.ssid_hidden;
        }
    }

    ap_cfg.ext_cfg.channel = chl;

    if (ecn == (int)LN_AT_ECN_OPEN)
    {
        ap_cfg.ext_cfg.authmode = WIFI_AUTH_OPEN;
    }
    else if (ecn == (int)LN_AT_ECN_WPA_PSK)
    {
        ap_cfg.ext_cfg.authmode = WIFI_AUTH_WPA_PSK;
    }
    else if (ecn == (int)LN_AT_ECN_WPA2_PSK)
    {
        ap_cfg.ext_cfg.authmode = WIFI_AUTH_WPA2_PSK;
    }
    else if (ecn == (int)LN_AT_ECN_WPA_WPA2_PSK)
    {
        ap_cfg.ext_cfg.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    }
    else if (ecn == (int)LN_AT_ECN_MAX)
    {
        ap_cfg.ext_cfg.authmode = ap_cfg.ext_cfg.authmode;
    }
    else
    {
        ap_cfg.ext_cfg.authmode = WIFI_AUTH_OPEN;
    }
    ap_cfg.ext_cfg.ssid_hidden = is_hidden;

		//2023.02.08 add ap_cfg.ext_cfg.max_conn
		ap_cfg.ext_cfg.max_conn=max_conn;
    ap_cfg.ssid = ssid_p;
    ap_cfg.pwd = pwd_p;
    ap_cfg.bssid = bssid;

    LOG(LOG_LVL_INFO, "ssid:%s; pwd:%s; channel:%d, ecn:%d, max conn:%d, hidden:%d" \
        "beacon:%d\r\n",
        ap_cfg.ssid, ap_cfg.pwd, ap_cfg.ext_cfg.channel,
        ap_cfg.ext_cfg.authmode, max_conn, ap_cfg.ext_cfg.ssid_hidden,
        ap_cfg.ext_cfg.beacon_interval);

    wifi_stop();
    netdev_set_state(NETIF_IDX_AP, NETDEV_DOWN);

    {
        tcpip_ip_info_t  ip_info = {0};
        server_config_t  server_config = {0};
        sysparam_softap_ip_info_get(&ip_info);
        sysparam_dhcpd_cfg_get(&server_config);

        // ip_info.ip.addr      = ipaddr_addr((const char *)"192.168.4.1");
        // ip_info.gw.addr      = ipaddr_addr((const char *)"192.168.4.1");
        // ip_info.netmask.addr = ipaddr_addr((const char *)"255.255.255.0");

        // server_config.server.addr   = ip_info.ip.addr;
        // server_config.port          = 67;
        // server_config.lease         = 2880;
        // server_config.renew         = 2880;
        // server_config.ip_start.addr = ipaddr_addr((const char *)"192.168.4.100");
        // server_config.ip_end.addr   = ipaddr_addr((const char *)"192.168.4.150");
        // server_config.client_max    = 3;

        if (LN_FALSE != dhcpd_is_running())
        {
            /* dhcpd is running, stop it first */
            dhcpd_stop();
        }

        if (DHCPD_ERR_NONE != dhcpd_curr_config_set(&server_config))
        {
            LOG(LOG_LVL_INFO, "dhcpd has been running!\r\n");
        }

        LOG(LOG_LVL_INFO, "ip info: \r\nip:     %s\r\n", ip4addr_ntoa(&ip_info.ip));
        LOG(LOG_LVL_INFO, "ip info: \r\ngw:     %s\r\n", ip4addr_ntoa(&ip_info.gw));
        LOG(LOG_LVL_INFO, "ip info: \r\nnetmask:%s\r\n", ip4addr_ntoa(&ip_info.netmask));

        LOG(LOG_LVL_INFO, "server config:\r\nip: %s\r\n", ip4addr_ntoa(&server_config.server));
        LOG(LOG_LVL_INFO, "ip info: \r\nip start:%s\r\n", ip4addr_ntoa(&server_config.ip_start));
        LOG(LOG_LVL_INFO, "ip info: \r\nip end:  %s\r\n", ip4addr_ntoa(&server_config.ip_end));


        LOG(LOG_LVL_INFO,
            "port:%d\r\n" \
            "lease:%d\r\n" \
            "renew:%d\r\n" \
            "client:%d\r\n",
            server_config.port,
            server_config.lease, server_config.renew, server_config.client_max);

        netdev_set_mac_addr(NETIF_IDX_AP, bssid);
        netdev_set_ip_info(NETIF_IDX_AP, &ip_info);

        netdev_set_state(NETIF_IDX_AP, NETDEV_UP);
        netdev_set_active(NETIF_IDX_AP);
    }

    ap_cfg.ext_cfg.psk_value = NULL;
    if ((ap_cfg.pwd != NULL && strlen(ap_cfg.pwd) > 0) &&
            (ap_cfg.ext_cfg.authmode != WIFI_AUTH_OPEN) &&
            (ap_cfg.ext_cfg.authmode != WIFI_AUTH_WEP)) {
        memset(psk_value, 0, sizeof(psk_value));
        if (0 == ln_psk_calc(ap_cfg.ssid, ap_cfg.pwd, psk_value, sizeof (psk_value))) {
            ap_cfg.ext_cfg.psk_value = psk_value;
            hexdump(LOG_LVL_INFO, "psk value ", psk_value, sizeof(psk_value));
        }
    }

    //set max conn
    wifi_softap_set_max_supp_sta_num(max_conn);

    if (0 != wifi_softap_start(&ap_cfg))
    {
        LOG(LOG_LVL_ERROR, "[%s:%d] wifi_softap_start() failed!", __func__, __LINE__);
        goto __exit;
    }

    /* save new mode into flash */
    if (0 != sysparam_softap_ssidpwd_cfg_update(ap_cfg.ssid, ap_cfg.pwd))
    {
        goto __exit;
    }

    if (0 != sysparam_softap_ext_cfg_update(&ap_cfg.ext_cfg))
    {
        goto __exit;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_set_ap_start(uint8_t para_num, const char *name)
{
    return _ln_at_cwsap_set_parse(LN_AT_CMD_TYPE_NONE, para_num, name);
}

static ln_at_err_t ln_at_set_ap_start_cur(uint8_t para_num, const char *name)
{
    return _ln_at_cwsap_set_parse(LN_AT_CMD_TYPE_CUR, para_num, name);
}

static ln_at_err_t ln_at_set_ap_start_def(uint8_t para_num, const char *name)
{
    return _ln_at_cwsap_set_parse(LN_AT_CMD_TYPE_DEF, para_num, name);
}

static ln_at_err_t ln_at_get_psmode(const char *name)
{
    int ret;
    sta_ps_mode_t ps_mode;

    ret = wifi_sta_get_powersave(&ps_mode);
    if (ret == WIFI_ERR_NONE)
    {
        ln_at_printf("%s:%d\r\n", name, (int)ps_mode);
        ln_at_printf(LN_AT_RET_OK_STR);
        return LN_AT_ERR_NONE;
    }
    else
    {
        ln_at_printf("%s\r\n", name);
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_COMMON;
    }
}

static ln_at_err_t ln_at_set_psmode(uint8_t para_num, const char *name)
{
    int ret;
    bool is_default = false;
    int ps_mode = WIFI_NO_POWERSAVE;

    if (para_num == 1)
    {
        if (LN_AT_PSR_ERR_NONE == ln_at_parser_get_int_param(1, &is_default, &ps_mode))
        {
            if (is_default)
            {
                ps_mode = WIFI_NO_POWERSAVE;
            }

            if (ps_mode >= WIFI_NO_POWERSAVE && ps_mode <= WIFI_MAX_POWERSAVE)
            {
                ret = wifi_sta_set_powersave((sta_ps_mode_t)ps_mode);
                if (ret == WIFI_ERR_NONE)
                {
                    ln_at_printf(LN_AT_RET_OK_STR);
                    return LN_AT_ERR_NONE;
                }
            }
        }
    }

    ln_at_printf("%s\r\n", name);
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

/**
 * AT+CIPSTAMAC?
 * AT+CIPSTAMAC=<mac>
*/
static ln_at_err_t _ln_at_get_cipstamac_parse(wifi_mode_t mode, const char *name)
{
    uint8_t bssid_hex[BSSID_LEN];

    if (mode == WIFI_MODE_STATION)
    {
        if (0 != sysparam_sta_mac_get(bssid_hex))
        {
            goto __exit;
        }
    }
    else if (mode == WIFI_MODE_AP)
    {
        if (0 != sysparam_softap_mac_get(bssid_hex))
        {
            goto __exit;
        }
    }
    else
    {
        goto __exit;
    }

    ln_at_printf("%s:\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n", name,
                 bssid_hex[0], bssid_hex[1], bssid_hex[2], bssid_hex[3], bssid_hex[4], bssid_hex[5]);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_get_sta_mac(const char *name)
{
    return _ln_at_get_cipstamac_parse(WIFI_MODE_STATION, name);
}

static ln_at_err_t ln_at_get_sta_mac_cur(const char *name)
{
    return _ln_at_get_cipstamac_parse(WIFI_MODE_STATION, name);
}

static ln_at_err_t ln_at_get_sta_mac_def(const char *name)
{
    return _ln_at_get_cipstamac_parse(WIFI_MODE_STATION, name);
}

static ln_at_err_t _ln_at_set_mac_parse(wifi_mode_t mode, uint8_t para_num, const char *name)
{
    int ret = 0;
    bool is_default = false;
    uint8_t para_index = 1;

    uint8_t bssid_hex[BSSID_LEN];
    char *bssid_p = NULL;

    if (para_num != 1)
    {
        goto __exit;
    }

    LN_AT_MEMSET(bssid_hex, 0X0, sizeof(bssid_hex));

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &bssid_p))
    {
        goto __exit;
    }

    if (is_default)
    {
        goto __exit;
    }

    ret = ln_at_cmd_args_get(bssid_p, "%x:%x:%x:%x:%x:%x",
                             &bssid_hex[0], &bssid_hex[1], &bssid_hex[2], &bssid_hex[3], &bssid_hex[4], &bssid_hex[5]);

    LOG(LOG_LVL_ERROR, "ret:%d; mac addr:\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n", ret,
        bssid_hex[0], bssid_hex[1], bssid_hex[2], bssid_hex[3], bssid_hex[4], bssid_hex[5]);

    if (ret != 6)
    {
        goto __exit;
    }

    if (mode == WIFI_MODE_STATION)
    {
        if (0 != sysparam_sta_mac_update((const uint8_t *)bssid_hex))
        {
            goto __exit;
        }
    }
    else if (mode == WIFI_MODE_AP)
    {
        if (0 != sysparam_softap_mac_update((const uint8_t *)bssid_hex))
        {
            goto __exit;
        }
    }
    else
    {
        goto __exit;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_set_sta_mac(uint8_t para_num, const char *name)
{
    return _ln_at_set_mac_parse(WIFI_MODE_STATION, para_num, name);
}

static ln_at_err_t ln_at_set_sta_mac_cur(uint8_t para_num, const char *name)
{
    return _ln_at_set_mac_parse(WIFI_MODE_STATION, para_num, name);
}

static ln_at_err_t ln_at_set_sta_mac_def(uint8_t para_num, const char *name)
{
    return _ln_at_set_mac_parse(WIFI_MODE_STATION, para_num, name);
}

/**
 * CIPAPMAC
*/
static ln_at_err_t ln_at_get_ap_mac(const char *name)
{
    return _ln_at_get_cipstamac_parse(WIFI_MODE_AP, name);
}

static ln_at_err_t ln_at_get_ap_mac_cur(const char *name)
{
    return _ln_at_get_cipstamac_parse(WIFI_MODE_AP, name);
}

static ln_at_err_t ln_at_get_ap_mac_def(const char *name)
{
    return _ln_at_get_cipstamac_parse(WIFI_MODE_AP, name);
}

static ln_at_err_t ln_at_set_ap_mac(uint8_t para_num, const char *name)
{
    return _ln_at_set_mac_parse(WIFI_MODE_AP, para_num, name);
}

static ln_at_err_t ln_at_set_ap_mac_cur(uint8_t para_num, const char *name)
{
    return _ln_at_set_mac_parse(WIFI_MODE_AP, para_num, name);
}

static ln_at_err_t ln_at_set_ap_mac_def(uint8_t para_num, const char *name)
{
    return _ln_at_set_mac_parse(WIFI_MODE_AP, para_num, name);
}

/**
 * exec:
 *
 * AT+CWLIF
 * +CWLIF:<ip	addr>,<mac>
 * OK
*/
static ln_at_err_t ln_at_exec_ap_query_sta_list(const char *name)
{
    wifi_mode_t curr_mode = wifi_current_mode_get();
    dhcpd_ip_item_t * ip_pool = dhcpd_get_ip_pool();
    uint8_t dhcp_client_max = dhcpd_get_client_max();
    dhcpd_ip_item_t *ip_item;
    int i = 0;

    if ( curr_mode == WIFI_MODE_AP && ip_pool!=NULL)
    {
        //ln_at_printf("%s:\r\n", name);

        for (i = 0; i < MIN(DHCPD_IP_POOL_SIZE, dhcp_client_max); i++)
        {
            ip_item = &(ip_pool[i]);
            if(ip_item->allocted == 1)
            {
                ln_at_printf("%s:%d.%d.%d.%d,"MACSTR"\r\n", name, ip4_addr1(&ip_item->ip), ip4_addr2(&ip_item->ip), ip4_addr3(&ip_item->ip), ip4_addr4(&ip_item->ip), MAC2STR(ip_item->mac));
            }
        }
        
        ln_at_printf(LN_AT_RET_OK_STR);
        return LN_AT_ERR_NONE;
    }
    
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

/**
 * AT+CWDHCP?
 * +CWDHCP:3
 * OK
 *
 * AT+CWDHCP=<mode>,<en>
*/
static ln_at_err_t _ln_at_get_dhcp_parse(const char *name)
{
    uint8_t dhcp_c_en = 0;
    uint8_t dhcp_d_en = 0;
    sysparam_dhcp_en_get(&dhcp_c_en);
    sysparam_dhcpd_en_get(&dhcp_d_en);

    ln_at_printf("%s:%d", name, (dhcp_c_en | dhcp_d_en << 1));
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_at_get_dhcp(const char *name)
{
    return _ln_at_get_dhcp_parse(name);
}

static ln_at_err_t ln_at_get_dhcp_cur(const char *name)
{
    return _ln_at_get_dhcp_parse(name);
}

static ln_at_err_t ln_at_get_dhcp_def(const char *name)
{
    return _ln_at_get_dhcp_parse(name);
}

static ln_at_err_t _ln_at_set_dhcp_parse(uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;

    int dhcp_mode = 0;
    int dhcp_en = 0;
	
		LN_WIFI_DRIVER_ADAPTER* pWifi = ln_get_wifi_drv_handle();

    if (para_num != 2)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &dhcp_en))
    {
        goto __exit;
    }
	if (dhcp_en != 0 && dhcp_en != 1)
	{
		goto __exit;
	}
    if (is_default)
    {
        dhcp_en = 1;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &dhcp_mode))
    {
        goto __exit;
    }
    if (is_default)
    {
		dhcp_mode = 2;
    }

    if (dhcp_mode == 3)
    {
        sysparam_dhcpd_en_update(dhcp_en);
        sysparam_dhcp_en_update(dhcp_en);
    }
    else if (dhcp_mode == 1)
    {
			sysparam_dhcp_en_update(dhcp_en);
	
//			if(0 == pWifi->netdev.get_state())
//			{
				if(dhcp_en) //static ip set to dynamic ip
				{
					pWifi->netdev.set_state(dhcp_en);			//re got ip
				}
				else
				{
					tcpip_ip_info_t ip_info;
					if (0 != sysparam_sta_ip_info_get(&ip_info))
					{
						Log_e("kv get ip error.\r\n");
					}
					netdev_set_ip_info(NETIF_IDX_STA, &ip_info);
				}						
//			}
			pWifi->sta.dhcp_en = dhcp_en;
    }
    else if (dhcp_mode == 2)
    {
        sysparam_dhcpd_en_update(dhcp_en);
    }
    else
    {
        goto __exit;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_set_dhcp(uint8_t para_num, const char *name)
{
    return _ln_at_set_dhcp_parse(para_num, name);
}

static ln_at_err_t ln_at_set_dhcp_cur(uint8_t para_num, const char *name)
{
    return _ln_at_set_dhcp_parse(para_num, name);
}

static ln_at_err_t ln_at_set_dhcp_def(uint8_t para_num, const char *name)
{
    return _ln_at_set_dhcp_parse(para_num, name);
}

/**
 * AT+CWDHCPS_CUR?
 * +CWDHCPS_CUR=<lease time>,<start IP>,<end IP>
 * OK
 *
 * AT+CWDHCPS_CUR=<enable>,<lease time>,<start IP>,<end IP>
*/
static ln_at_err_t _ln_at_get_dhcpd_cfg_parse(const char *name)
{
    int ret = 0;
    uint8_t dhcp_d_en = 0;
    wifi_mode_t mode;
    server_config_t dhcpd_cfg;

    char start_ip[16] = {0};
    char end_ip[16] = {0};

    mode = g_ln_at_ctrl_p->mode;
    if (mode != WIFI_MODE_AP)
    {
        goto __exit;
    }

    ret = sysparam_dhcpd_en_get(&dhcp_d_en);
    if (ret != 0 || dhcp_d_en == 0)
    {
        goto __exit;
    }

    ret = sysparam_dhcpd_cfg_get(&dhcpd_cfg);
    if (ret != 0)
    {
        goto __exit;
    }

    if (inet_ntop(AF_INET, &dhcpd_cfg.ip_start, start_ip, sizeof(start_ip)) == NULL)
    {
        goto __exit;
    }

    if (inet_ntop(AF_INET, &dhcpd_cfg.ip_end, end_ip, sizeof(end_ip)) == NULL)
    {
        goto __exit;
    }

    ln_at_printf("%s:%d,%s,%s", name, dhcpd_cfg.lease,
            start_ip, end_ip);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_get_dhcp_cfg(const char *name)
{
    return _ln_at_get_dhcpd_cfg_parse(name);
}

static ln_at_err_t ln_at_get_dhcp_cfg_cur(const char *name)
{
    return _ln_at_get_dhcpd_cfg_parse(name);
}

static ln_at_err_t ln_at_get_dhcp_cfg_def(const char *name)
{
    return _ln_at_get_dhcpd_cfg_parse(name);
}

static ln_at_err_t _ln_at_set_dhcpd_cfg_parse(uint8_t para_num, const char *name)
{
    int ret = 0;
    bool is_default = false;
    uint8_t para_index = 1;
    int dhcp_d_en = 0;

    wifi_mode_t mode;
    server_config_t dhcpd_cfg;

    int lease_time = 0;
    char *start_ip_p = NULL;
    char *end_ip_p = NULL;

    if (para_num < 1)
    {
        goto __exit;
    }

    mode = g_ln_at_ctrl_p->mode;
    if (mode != WIFI_MODE_AP)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &dhcp_d_en))
    {
        goto __exit;
    }
    if (is_default)
    {
        goto __exit;
    }

    if (para_num >= 2)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &lease_time))
        {
            goto __exit;
        }
        if (is_default)
        {
            goto __exit;
        }
    }

    if (para_num >= 3)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &start_ip_p))
        {
            goto __exit;
        }
        if (is_default || !start_ip_p)
        {
            goto __exit;
        }
    }

    if (para_num >= 4)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &end_ip_p))
        {
            goto __exit;
        }
        if (is_default || !end_ip_p)
        {
            goto __exit;
        }
    }

    ret = sysparam_dhcpd_en_update(dhcp_d_en);
    if (ret != 0)
    {
        goto __exit;
    }

    if (dhcp_d_en != 0)
    {
        ret = dhcpd_curr_config_get(&dhcpd_cfg);
        if (ret != 0)
        {
            goto __exit;
        }

        dhcpd_cfg.lease = lease_time;
        ret = inet_aton(start_ip_p, &dhcpd_cfg.ip_start.addr);
        if (ret == 0)
        {
            goto __exit;
        }

        ret = inet_aton(end_ip_p, &dhcpd_cfg.ip_end.addr);
        if (ret == 0)
        {
            goto __exit;
        }

        ret = sysparam_dhcpd_cfg_update(&dhcpd_cfg);
        if (ret != 0)
        {
            goto __exit;
        }
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_set_dhcp_cfg(uint8_t para_num, const char *name)
{
    return _ln_at_set_dhcpd_cfg_parse(para_num, name);
}

static ln_at_err_t ln_at_set_dhcp_cfg_cur(uint8_t para_num, const char *name)
{
    return _ln_at_set_dhcpd_cfg_parse(para_num, name);
}

static ln_at_err_t ln_at_set_dhcp_cfg_def(uint8_t para_num, const char *name)
{
    return _ln_at_set_dhcpd_cfg_parse(para_num, name);
}

/**
 * set:
 *
 * AT+CWAUTOCONN=<enable>
 *
 * only work in sta mode.
 * enable: 1-auto connect ap; 0-don't auto connect ap
*/
static ln_at_err_t ln_at_set_auto_conn(uint8_t para_num, const char *name)
{
    wifi_mode_t mode;
    int enable = 0;
    uint8_t origin = 0;

    bool is_default = false;
    uint8_t para_index = 1;

    if (para_num != 1)
    {
        goto __exit;
    }

//    mode = g_ln_at_ctrl_p->mode;
//    if (mode != WIFI_MODE_STATION)
//    {
//        goto __exit;
//    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &enable))
    {
        goto __exit;
    }
    if (is_default)
    {
        goto __exit;
    }

    if (enable != 0 && enable != 1)
    {
        goto __exit;
    }

    if (0 != sysparam_poweron_auto_conn_get(&origin))
    {
        goto __exit;
    }

    if ((int)origin != enable)
    {
        sysparam_poweron_auto_conn_update((uint8_t)enable);
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_get_auto_conn(const char *name)
{
	uint8_t origin = 0;

    if (SYSPARAM_ERR_NONE != sysparam_poweron_auto_conn_get(&origin))
    {
        goto __exit;
    }
	
	ln_at_printf("%s:%d", name, origin);
	
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
	
__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

/**
 * Set/Get ip:
 *
 * AT+CIPSTA
*/
#include "netif/ethernetif.h"
static ln_at_err_t _ln_at_cipsta_get_parser(ln_at_cmd_type_e cmd_type, wifi_mode_t mode, const char *name)
{
    wifi_mode_t cur_mode;
    wifi_sta_status_t sta_status = WIFI_STA_STATUS_STARTUP;
    tcpip_ip_info_t ip_info;
    char ip[16] = {0};
    char netmask[16] = {0};
    char gw[16] = {0};
    uint8_t dhcp_en =0;
    uint8_t sysstore = 0;

    cur_mode = g_ln_at_ctrl_p->mode;
    sysparam_sys_store_get(&sysstore);
    sysparam_dhcp_en_get(&dhcp_en);

    if(mode == WIFI_MODE_STATION)
    {
        wifi_get_sta_status(&sta_status);
        if(sta_status != WIFI_STA_STATUS_CONNECTED)
        {
            if(dhcp_en)
            {
                goto __default;
            }
            else
            {
                sysparam_sta_ip_info_get(&ip_info);
            }
        }
        else
        {
            if(0 != netdev_get_ip_info(netdev_get_active(), &ip_info))
            {
                goto __exit;
            }
        }
    }
    else if(mode == WIFI_MODE_AP)
    {
        sysparam_softap_ip_info_get(&ip_info);
    }

    if (inet_ntop(AF_INET, &ip_info.ip, ip, sizeof(ip)) == NULL)
    {
        goto __exit;
    }

    if (inet_ntop(AF_INET, &ip_info.netmask, netmask, sizeof(netmask)) == NULL)
    {
        goto __exit;
    }

    if (inet_ntop(AF_INET, &ip_info.gw, gw, sizeof(gw)) == NULL)
    {
        goto __exit;
    }

    ln_at_printf("%s:ip:\"%s\"\r\n", name, ip);
    ln_at_printf("%s:gateway:\"%s\"\r\n", name, gw);
    ln_at_printf("%s:netmask:\"%s\"\r\n", name, netmask);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
	
__default:
	ln_at_printf("%s:ip:\"0.0.0.0\"\r\n", name);
    ln_at_printf("%s:gateway:\"0.0.0.0\"\r\n", name);
    ln_at_printf("%s:netmask:\"0.0.0.0\"\r\n", name);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_at_get_cipsta(const char *name)
{
    /* LN_AT_CMD_TYPE_NONE */
    return _ln_at_cipsta_get_parser(LN_AT_CMD_TYPE_NONE, WIFI_MODE_STATION, name);
}

static ln_at_err_t ln_at_get_cipsta_cur(const char *name)
{
    return _ln_at_cipsta_get_parser(LN_AT_CMD_TYPE_CUR, WIFI_MODE_STATION, name);
}

static ln_at_err_t ln_at_get_cipsta_def(const char *name)
{
    return _ln_at_cipsta_get_parser(LN_AT_CMD_TYPE_DEF, WIFI_MODE_STATION, name);
}

static ln_at_err_t _ln_at_cipsta_set_parser(ln_at_cmd_type_e cmd_type, wifi_mode_t mode, uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;

    tcpip_ip_info_t ip_info;
		tcpip_ip_info_t sys_ip_info;
    wifi_mode_t cur_mode;
    uint8_t dhcp_c_en = 0;
		wifi_sta_status_t sta_status = WIFI_STA_STATUS_STARTUP;

    char *ip_p = NULL;
    char *gw_p = NULL;
    char *netmask_p = NULL;
	
		uint8_t sysstore = 0;
		LN_WIFI_DRIVER_ADAPTER *pWifi = ln_get_wifi_drv_handle();
	
    if (para_num < 1)
    {
        goto __exit;
    }
		
		sysparam_sys_store_get(&sysstore);
		if(mode == WIFI_MODE_STATION)
		{
			sysparam_sta_ip_info_get(&sys_ip_info);
		}
		else
		{
			sysparam_softap_ip_info_get(&sys_ip_info);
		}
		
    cur_mode = g_ln_at_ctrl_p->mode;
//    if (mode != cur_mode)
//    {
//        goto __exit;
//    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &ip_p))
    {
        goto __exit;
    }
    if (is_default)
    {
        goto __exit;
    }
    
    if (para_num >= 2)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &gw_p))
        {
            goto __exit;
        }
        if (is_default)
        {
            goto __exit;
        }
    }

    if (para_num >= 3)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &netmask_p))
        {
            goto __exit;
        }
        if (is_default)
        {
            goto __exit;
        }
    }
		
    if (mode == WIFI_MODE_STATION)
    {
        /* disable dhcp client */
        sysparam_dhcp_en_get(&dhcp_c_en);
        if (dhcp_c_en)
        {
            dhcp_c_en = 0;
            sysparam_dhcp_en_update(dhcp_c_en); /* TODO */
        }
    }
    else if (mode == WIFI_MODE_AP)
    {
        /* disable dhcp server */
        sysparam_dhcpd_en_get(&dhcp_c_en);
        if (dhcp_c_en)
        {
            dhcp_c_en = 0;
            sysparam_dhcpd_en_update(dhcp_c_en); /* TODO */
        }
    }
    else
    {
        goto __exit;
    }

    if (cmd_type != LN_AT_CMD_TYPE_DEF && sysstore == 0)
    {
        if (0 != netdev_get_ip_info(netdev_get_active(), &ip_info))
        {
            goto __exit;
        }
    }
    else
    {
        if (mode == WIFI_MODE_STATION)
        {
            if (0 != sysparam_sta_ip_info_get(&ip_info))
            {
                goto __exit;
            }
        }
        else
        {
            if (0 != sysparam_softap_ip_info_get(&ip_info))
            {
                goto __exit;
            }
        }
    }
		
		//dhcp = 0 , use static ip
		if (mode == WIFI_MODE_STATION)
		{
				if (0 != sysparam_sta_ip_info_get(&ip_info))
				{
						goto __exit;
				}
		}
		else
		{
				if (0 != sysparam_softap_ip_info_get(&ip_info))
				{
						goto __exit;
				}
		}

		if(para_num >= 1)
		{
			if (0 == inet_pton(AF_INET, ip_p, &ip_info.ip))
			{
        goto __exit;
			}
			//A
			ip_info.gw.addr = (ip_info.gw.addr & 0xFF000000) | (ip_info.ip.addr & 0x00FFFFFF);
		}

    if (para_num >= 2)
    {
        if (0 == inet_pton(AF_INET, gw_p, &ip_info.gw))
        {
            goto __exit;
        }

    }

    if (para_num >= 3)
    {
        if (0 == inet_pton(AF_INET, netmask_p, &ip_info.netmask))
        {
            goto __exit;
        }
    }

		if(mode == WIFI_MODE_STATION && cur_mode == mode)
		{
			if (0 != sysparam_sta_ip_info_update(&ip_info))
			{
					goto __exit;
			}		
			/* set static ip addr, and save into flash */
			if (0 != netdev_set_ip_info(netdev_get_active(), &ip_info))
			{
					goto __exit;
			}
		}
		else if(mode == WIFI_MODE_STATION && cur_mode != mode)
		{
			if (0 != sysparam_sta_ip_info_update(&ip_info))
			{
					goto __exit;
			}		
		}
		else if(mode == WIFI_MODE_AP)
		{
			if (0 != sysparam_softap_ip_info_update(&ip_info))
			{
				goto __exit;
			}				
		}
		

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_set_cipsta(uint8_t para_num, const char *name)
{
    return _ln_at_cipsta_set_parser(LN_AT_CMD_TYPE_NONE, WIFI_MODE_STATION, para_num, name);
}

static ln_at_err_t ln_at_set_cipsta_cur(uint8_t para_num, const char *name)
{
    return _ln_at_cipsta_set_parser(LN_AT_CMD_TYPE_CUR, WIFI_MODE_STATION, para_num, name);
}

static ln_at_err_t ln_at_set_cipsta_def(uint8_t para_num, const char *name)
{
    return _ln_at_cipsta_set_parser(LN_AT_CMD_TYPE_DEF, WIFI_MODE_STATION, para_num, name);
}

/**
 * Set/Get ip:
 *
 * AT+CIPAP
*/
static ln_at_err_t ln_at_get_cipap(const char *name)
{
    return _ln_at_cipsta_get_parser(LN_AT_CMD_TYPE_NONE, WIFI_MODE_AP, name);
}

static ln_at_err_t ln_at_get_cipap_cur(const char *name)
{
    return _ln_at_cipsta_get_parser(LN_AT_CMD_TYPE_CUR, WIFI_MODE_AP, name);
}

static ln_at_err_t ln_at_get_cipap_def(const char *name)
{
    return _ln_at_cipsta_get_parser(LN_AT_CMD_TYPE_DEF, WIFI_MODE_AP, name);
}

static ln_at_err_t ln_at_set_cipap(uint8_t para_num, const char *name)
{
    return _ln_at_cipsta_set_parser(LN_AT_CMD_TYPE_NONE, WIFI_MODE_AP, para_num, name);
}

static ln_at_err_t ln_at_set_cipap_cur(uint8_t para_num, const char *name)
{
    return _ln_at_cipsta_set_parser(LN_AT_CMD_TYPE_CUR, WIFI_MODE_AP, para_num, name);
}

static ln_at_err_t ln_at_set_cipap_def(uint8_t para_num, const char *name)
{
    return _ln_at_cipsta_set_parser(LN_AT_CMD_TYPE_DEF, WIFI_MODE_AP, para_num, name);
}

#if 0
/**
 * set/exec
 *
 * AT+CWSTARTSMART
 * AT+CWSTARTSMART=<type>
 *
 * type: 1-ESP-TOUCH; 2-Airkiss; 3-ESP-TOUCH + Airkiss
*/
static ln_at_err_t ln_at_set_start_smartconfig(uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;

    int type;
//    wifi_mode_t mode = WIFI_MODE_STATION;

    if (para_num != 1)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &type))
    {
        goto __exit;
    }
    if (is_default)
    {
        goto __exit;
    }

    if (!(type <= 3 && type >= 1))
    {
        goto __exit;
    }

//    mode = g_ln_at_ctrl_p->mode;

    ln_at_printf("\r\nTODO!!!\r\n");

    ln_at_printf("smartconfig type: <type>\r\n");
    ln_at_printf("smart get wifi info\r\n");
    ln_at_printf("ssid:<AP's SSID>\r\n");
    ln_at_printf("password:<AP's password>\r\n");

    ln_at_printf("WIFI CONNECTED\r\n");
    ln_at_printf("WIFI GOT IP\r\n");
    ln_at_printf("smartconfig connected wifi\r\n");

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_exec_start_smartconfig(const char *name)
{
    ln_at_printf("\r\n%s:TODO!!!\r\n", name);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

//__exit:
//    ln_at_printf(LN_AT_RET_ERR_STR);
//    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_exec_stop_smartconfig(const char *name)
{
    ln_at_printf("\r\n%s:TODO!!!\r\n", name);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

//__exit:
//    ln_at_printf(LN_AT_RET_ERR_STR);
//    return LN_AT_ERR_COMMON;
}

/**
 * AT+CWSTARTDISCOVER=<WeChat number>,<dev_type>,<time>
*/
static ln_at_err_t ln_at_set_start_wechat_discovery(uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;
    char *wechat_number_p = NULL;
    char *wechat_dev_type_p = NULL;

    /**
     * time:
     *
     * Time interval for active send pkt,
     * Used for LAN devices found by WeChat
    */
    int time = 0;

    if (para_num != 3)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &wechat_number_p))
    {
        goto __exit;
    }
    if (is_default)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &wechat_dev_type_p))
    {
        goto __exit;
    }
    if (is_default)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &time))
    {
        goto __exit;
    }
    if (is_default)
    {
        goto __exit;
    }

    ln_at_printf("\r\nwechat number:%s, dev type:%s, time:%d\r\n",
            wechat_number_p, wechat_dev_type_p, time);

    ln_at_printf("\r\n:%s:TODO!!!\r\n", name);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_exec_stop_wechat_discovery(const char *name)
{
    ln_at_printf("\r\n%s:TODO!!!\r\n", name);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

//__exit:
//    ln_at_printf(LN_AT_RET_ERR_STR);
//    return LN_AT_ERR_COMMON;
}

/**
 * AT+WPS
 *
 * Not support!
*/
static ln_at_err_t ln_at_set_wps_cfg(uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;
    int wps_en = 0;
    wifi_mode_t mode = g_ln_at_ctrl_p->mode;
    if (mode != WIFI_MODE_STATION)
    {
        goto __exit;
    }

    if (para_num != 1)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &wps_en))
    {
        goto __exit;
    }
    if (is_default)
    {
        goto __exit;
    }

    if (wps_en != 1 && wps_en != 0)
    {
        goto __exit;
    }
    ln_at_printf("\r\n%s:TODO!!!\r\n", name);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

/**
 * AT+MDNS=<enable>[,<hostname>,<server_name>,<server_port>]
 *
 * Not support!
*/
static ln_at_err_t ln_at_set_mdns_cfg(uint8_t para_num, const char *name)
{
    wifi_mode_t mode = g_ln_at_ctrl_p->mode;
    if (mode != WIFI_MODE_STATION)
    {
        goto __exit;
    }

    if (para_num < 1)
    {
        goto __exit;
    }

    ln_at_printf("\r\n%s:TODO!!!\r\n", name);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

/**
 * Get/Set
 * AT+CWHOSTNAME?
 * AT+CWHOSTNAME=<hostname>
*/
#define LN_AT_HOST_NAME_LEN (33u)
static ln_at_err_t ln_at_set_hostname_cfg(uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;
    char *host_nam_p = NULL;
    wifi_mode_t mode = g_ln_at_ctrl_p->mode;

    if (para_num != 1)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &host_nam_p))
    {
        goto __exit;
    }
    if (is_default)
    {
        goto __exit;
    }

    if (mode == WIFI_MODE_STATION)
    {
        if (0 != sysparam_sta_hostname_update(host_nam_p))
        {
            goto __exit;
        }
    }
    else if (mode == WIFI_MODE_AP)
    {
        if (0 != sysparam_softap_hostname_update(host_nam_p))
        {
            goto __exit;
        }
    }
    else
    {
        goto __exit;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_get_hostname(const char *name)
{
    char host_name[LN_AT_HOST_NAME_LEN];

    if (sysparam_sta_hostname_get(host_name) != 0)
    {
        goto __exit;
    }

    ln_at_printf("%s:%s", name, host_name);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

/**
 * Get/Set
 * AT+CWCOUNTRY_CUR
 * AT+CWCOUNTRY_DEF
*/
#define LN_AT_DEFAULT_COUNTRY_POLICY (1u)
static ln_at_err_t _ln_at_set_country_cfg_parse(ln_at_cmd_type_e type, uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;
    wifi_country_code_t country;

    int policy;
    int start_channel;
    int channel_num;

    char *country_code_p = NULL;

    if (para_num != 4)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &policy))
    {
        goto __exit;
    }
    if (is_default)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &country_code_p))
    {
        goto __exit;
    }
    if (is_default || !country_code_p)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &start_channel))
    {
        goto __exit;
    }
    if (is_default)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &channel_num))
    {
        goto __exit;
    }
    if (is_default)
    {
        goto __exit;
    }

    LOG(LOG_LVL_ERROR, "policy:%d, country code:%s, start channel:%d, total channel:%d\r\n",
            policy, country_code_p, start_channel, channel_num);

    if (0 == strncmp(country_code_p, "CN", 2))
    {
        country = CTRY_CODE_CN;
    }
    else if (0 == strncmp(country_code_p, "US", 2))
    {
        country = CTRY_CODE_US;
    }
    else if (0 == strncmp(country_code_p, "JP", 2))
    {
        country = CTRY_CODE_JP;
    }
    else if (0 == strncmp(country_code_p, "IL", 2)) /* Israel */
    {
        country = CTRY_CODE_ISR;
    }
    else
    {
        country = CTRY_CODE_CN;
    }

    LOG(LOG_LVL_ERROR, "country:%d\r\n", country);

    if (type == LN_AT_CMD_TYPE_CUR)
    {
        wifi_set_country_code(country);
    }
    else if (type == LN_AT_CMD_TYPE_DEF)
    {
        sysparam_country_code_update(country);
    }
    else
    {
        goto __exit;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_set_country_cfg_cur(uint8_t para_num, const char *name)
{
    return _ln_at_set_country_cfg_parse(LN_AT_CMD_TYPE_CUR, para_num, name);
}

static ln_at_err_t ln_at_set_country_cfg_def(uint8_t para_num, const char *name)
{
    return _ln_at_set_country_cfg_parse(LN_AT_CMD_TYPE_DEF, para_num, name);
}

/**
 * +CWCOUNTRY_CUR:<country_policy>,<country_code>,<start_channel>,<total_channel_number>
*/
static ln_at_err_t _ln_at_get_country_cfg_parse(ln_at_cmd_type_e type, const char *name)
{
    wifi_country_code_t country;
    const char *country_code;

    if (type == LN_AT_CMD_TYPE_CUR)
    {
        if (0 != wifi_get_country_code(&country))
        {
            goto __exit;
        }
    }
    else if (type == LN_AT_CMD_TYPE_DEF)
    {
        if (0 != sysparam_country_code_get(&country))
        {
            goto __exit;
        }
    }
    LOG(LOG_LVL_ERROR, "country:%d\r\n", country);

    if (country == CTRY_CODE_CN)
    {
        country_code = "CN";
    }
    else if (country == CTRY_CODE_US)
    {
        country_code = "US";
    }
    else if (country == CTRY_CODE_JP)
    {
        country_code = "JP";
    }
    else if (country == CTRY_CODE_ISR)
    {
        country_code = "IL";
    }
    else
    {
        country_code = "CN";
    }

    ln_at_printf("%s:%d,%s,1,13\r\n", name, LN_AT_DEFAULT_COUNTRY_POLICY, country_code);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_get_country_cfg_cur(const char *name)
{
    return _ln_at_get_country_cfg_parse(LN_AT_CMD_TYPE_CUR, name);
}

static ln_at_err_t ln_at_get_country_cfg_def(const char *name)
{
    return _ln_at_get_country_cfg_parse(LN_AT_CMD_TYPE_DEF, name);
}
#endif /* 0 */


/**
 * name, get, set, test, exec
*/
LN_AT_CMD_REG(CWMODE,     ln_at_get_wifi_mode,         ln_at_set_wifi_mode,         ln_at_help_wifi_mode,         NULL);
//LN_AT_CMD_REG(CWMODE_CUR, ln_at_get_wifi_mode_current, ln_at_set_wifi_mode_current, ln_at_help_wifi_mode_current, NULL);
//LN_AT_CMD_REG(CWMODE_DEF, ln_at_get_wifi_mode_def,     ln_at_set_wifi_mode_def,     ln_at_help_wifi_mode_def,     NULL);


LN_AT_CMD_REG(CWJAP,     ln_at_get_wifi_join_ap,         ln_at_set_wifi_join_ap,         ln_at_help_wifi_join_ap,         NULL);
//LN_AT_CMD_REG(CWJAP_CUR, ln_at_get_wifi_join_ap_current, ln_at_set_wifi_join_ap_current, ln_at_help_wifi_join_ap_current, NULL);
//LN_AT_CMD_REG(CWJAP_DEF, ln_at_get_wifi_join_ap_def,     ln_at_set_wifi_join_ap_def,     ln_at_help_wifi_join_ap_def,     NULL);

LN_AT_CMD_REG(CWQAP,     NULL, NULL, NULL, ln_at_exec_sta_disconn_ap);

LN_AT_CMD_REG(CWLAP, NULL, NULL, NULL, ln_at_exec_scan);

LN_AT_CMD_REG(CWSAP,     ln_at_get_ap_start,     ln_at_set_ap_start,     NULL, NULL);
//LN_AT_CMD_REG(CWSAP_CUR, ln_at_get_ap_start_cur, ln_at_set_ap_start_cur, NULL, NULL);
//LN_AT_CMD_REG(CWSAP_DEF, ln_at_get_ap_start_def, ln_at_set_ap_start_def, NULL, NULL);

LN_AT_CMD_REG(PVTCMD, NULL, ln_at_set_pvtcmd, NULL, NULL);

LN_AT_CMD_REG(CWSLEEP, ln_at_get_psmode, ln_at_set_psmode, NULL, NULL);


#if 1
LN_AT_CMD_REG(CIPSTAMAC,     ln_at_get_sta_mac,     ln_at_set_sta_mac,     NULL, NULL);
LN_AT_CMD_REG(CIPSTAMAC_CUR, ln_at_get_sta_mac_cur, ln_at_set_sta_mac_cur, NULL, NULL);
LN_AT_CMD_REG(CIPSTAMAC_DEF, ln_at_get_sta_mac_def, ln_at_set_sta_mac_def, NULL, NULL);

LN_AT_CMD_REG(CIPAPMAC,     ln_at_get_ap_mac,     ln_at_set_ap_mac,     NULL, NULL);
LN_AT_CMD_REG(CIPAPMAC_CUR, ln_at_get_ap_mac_cur, ln_at_set_ap_mac_cur, NULL, NULL);
LN_AT_CMD_REG(CIPAPMAC_DEF, ln_at_get_ap_mac_def, ln_at_set_ap_mac_def, NULL, NULL);

LN_AT_CMD_REG(CWLIF, NULL, NULL, NULL, ln_at_exec_ap_query_sta_list);

LN_AT_CMD_REG(CWDHCP,     ln_at_get_dhcp,     ln_at_set_dhcp,     NULL, NULL);
LN_AT_CMD_REG(CWDHCP_CUR, ln_at_get_dhcp_cur, ln_at_set_dhcp_cur, NULL, NULL);
LN_AT_CMD_REG(CWDHCP_DEF, ln_at_get_dhcp_def, ln_at_set_dhcp_def, NULL, NULL);

LN_AT_CMD_REG(CWDHCPS,     ln_at_get_dhcp_cfg,     ln_at_set_dhcp_cfg,     NULL, NULL);
LN_AT_CMD_REG(CWDHCPS_CUR, ln_at_get_dhcp_cfg_cur, ln_at_set_dhcp_cfg_cur, NULL, NULL);
LN_AT_CMD_REG(CWDHCPS_DEF, ln_at_get_dhcp_cfg_def, ln_at_set_dhcp_cfg_def, NULL, NULL);

LN_AT_CMD_REG(CWAUTOCONN,  ln_at_get_auto_conn, ln_at_set_auto_conn, NULL, NULL);

/**
 * Set/Get ip:
 *
 * AT+CIPSTA
*/
LN_AT_CMD_REG(CIPSTA,     ln_at_get_cipsta,     ln_at_set_cipsta,     NULL, NULL);
LN_AT_CMD_REG(CIPSTA_CUR, ln_at_get_cipsta_cur, ln_at_set_cipsta_cur, NULL, NULL);
LN_AT_CMD_REG(CIPSTA_DEF, ln_at_get_cipsta_def, ln_at_set_cipsta_def, NULL, NULL);

/**
 * Set/Get ip:
 *
 * AT+CIPSTA
*/
LN_AT_CMD_REG(CIPAP,     ln_at_get_cipap,     ln_at_set_cipap,     NULL, NULL);
LN_AT_CMD_REG(CIPAP_CUR, ln_at_get_cipap_cur, ln_at_set_cipap_cur, NULL, NULL);
LN_AT_CMD_REG(CIPAP_DEF, ln_at_get_cipap_def, ln_at_set_cipap_def, NULL, NULL);

/**
 * Set/Exec
 *
 * AT+CWSTARTSMART
 * AT+CWSTARTSMART=<type>
 *
 * type: 1-ESP-TOUCH; 2-Airkiss; 3-ESP-TOUCH + Airkiss
*/
// LN_AT_CMD_ITEM_DEF("CWSTARTSMART", NULL, ln_at_set_start_smartconfig, NULL, ln_at_exec_start_smartconfig)
// LN_AT_CMD_ITEM_DEF("CWSTOPSMART", NULL, NULL, NULL, ln_at_exec_stop_smartconfig)

/**
 * AT+CWSTARTDISCOVER=<WeChat number>,<dev_type>,<time>
 * AT+CWSTOPDISCOVER
*/
// LN_AT_CMD_ITEM_DEF("CWSTARTDISCOVER", NULL, ln_at_set_start_wechat_discovery, NULL, NULL)
// LN_AT_CMD_ITEM_DEF("CWSTOPDISCOVER", NULL, NULL, NULL, ln_at_exec_stop_wechat_discovery)

// LN_AT_CMD_ITEM_DEF("WPS",  NULL, ln_at_set_wps_cfg,  NULL, NULL)
// LN_AT_CMD_ITEM_DEF("MDNS", NULL, ln_at_set_mdns_cfg, NULL, NULL)
// LN_AT_CMD_ITEM_DEF("CWHOSTNAME", ln_at_get_hostname, ln_at_set_hostname_cfg, NULL, NULL)

// LN_AT_CMD_ITEM_DEF("CWCOUNTRY_CUR", ln_at_get_country_cfg_cur, ln_at_set_country_cfg_cur, NULL, NULL)
// LN_AT_CMD_ITEM_DEF("CWCOUNTRY_DEF", ln_at_get_country_cfg_def, ln_at_set_country_cfg_def, NULL, NULL)
#endif /* 0 */


/**
 * Set/Get ATE_OK
 *
 * AT+ATE_OK=1
 * AT+ATE_OK?
*/  

static ln_at_err_t ln_at_get_ate_ok(const char *name)
{
    int8_t val = 0;

    ln_nvds_get_xtal_comp_val((uint8_t *)&val);
    ln_at_printf("+XTAL_COMP:%d\r\n", val);

    ln_nvds_get_tx_power_comp((uint8_t *)&val);
    ln_at_printf("+TXPOWER_COMP:%d\r\n", val);

    ln_nvds_get_tx_power_b_comp((uint8_t *)&val);
    ln_at_printf("+TXPOWER_B_COMP:%d\r\n", val);

    ln_nvds_get_tx_power_gn_comp((uint8_t *)&val);
    ln_at_printf("+TXPOWER_GN_COMP:%d\r\n", val);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_at_set_ate_ok(uint8_t para_num, const char *name)
{
    bool is_default = false;
    int val = 0;
    int8_t xtal_cap = 0, tx_power = 0;
    int8_t tx_pwr_b = 0, tx_pwr_gn = 0;

    if (para_num != 1) {
        goto __exit;
    }

    if (LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(1, &is_default, &val)) {
        goto __exit;
    }

    if (val == 0) {
        ln_nvds_set_ate_result('F');
    } 
    else if (val == 1) {
        extern int ate_get_xtalcap_calibration_info(void);
        extern int ate_get_txpower_calibration_info(void);
        extern int ate_get_xtalcap_txpower_offset (int8_t *xtal_cap, int8_t *tx_power);

        extern int ate_is_calibrated_txpower_b(void);
        extern int ate_is_calibrated_txpower_gn(void);
        extern int ate_get_txpower_b_gn_offset (int8_t *tx_pwr_b, int8_t *tx_pwr_gn);

        ate_get_xtalcap_txpower_offset(&xtal_cap, &tx_power);
        ate_get_txpower_b_gn_offset(&tx_pwr_b, &tx_pwr_gn);

        /* 1. save to nvds(flash) */
        //1.1 save XTAL_CAP
        if (ate_get_xtalcap_calibration_info()) {
            ln_nvds_set_xtal_comp_val(xtal_cap);
        }

        //1.2 save TX_POWER
        if (ate_get_txpower_calibration_info()) {
            ln_nvds_set_tx_power_comp(tx_power);
        }
        if (ate_is_calibrated_txpower_b()) {
            ln_nvds_set_tx_power_b_comp(tx_pwr_b);
        }
        if (ate_is_calibrated_txpower_gn()) {
            ln_nvds_set_tx_power_gn_comp(tx_pwr_gn);
        }

        //1.3 save ATE result
        //('S'=ate_successful, 'F'=ate_failed)
        ln_nvds_set_ate_result('S');
    }
    else {
        goto __exit;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

LN_AT_CMD_REG(ATE_OK, ln_at_get_ate_ok, ln_at_set_ate_ok, NULL, NULL);

static void * ln_at_wifi_ap_scan_cpl_sem = NULL;
static void ln_at_wifi_ap_scan_cpl_cb(void *arg)
{
    // release semaphore
    ln_at_sem_release(ln_at_wifi_ap_scan_cpl_sem);
}

static ln_at_err_t ln_at_ap_scan(wifi_scan_cfg_t *scan_cfg)
{
    ln_at_err_t ret = LN_AT_ERR_NONE;
    ap_info_t *ap_info = NULL;
    int ap_info_items = 20;
    ap_info_t *ap_info_rst = NULL;
    int ap_info_rst_items = 0;

    ln_at_wifi_ap_scan_cpl_sem = ln_at_sem_create(0, 1);
    if (!ln_at_wifi_ap_scan_cpl_sem) {
        ret = LN_AT_ERR_COMMON;
        goto __exit;
    }

    ap_info = OS_Malloc(ap_info_items * sizeof(ap_info_t));
    if (!ap_info) {
        ret = LN_AT_ERR_COMMON;
        goto __exit;
    }

    wifi_softap_scan(scan_cfg, ap_info, ap_info_items, ln_at_wifi_ap_scan_cpl_cb);

    // wait scan cpl semaphore
    ln_at_sem_wait(ln_at_wifi_ap_scan_cpl_sem, OS_WAIT_FOREVER);
    ln_at_sem_delete(ln_at_wifi_ap_scan_cpl_sem);

    // get scan result
    wifi_softap_scan_results_get(&ap_info_rst, &ap_info_rst_items);
    ln_at_printf("ap scan results:%d\r\n", ap_info_rst_items);

    if (!ap_info_rst) {
        ret = LN_AT_ERR_COMMON;
        goto __exit;
    }

    ln_at_printf("%10.10s   %19.16s   %4.4s   %2.2s   %4.4s\r\n",
        "ssid", "bssid", "rssi", "ch", "auth");

    for (int i = 0; i < ap_info_rst_items; i++) {
        ln_at_printf("%10.10s   [%02x:%02x:%02x:%02x:%02x:%02x]"
            "   %4d   %2d   %4d\r\n", 
            ap_info_rst[i].ssid, ap_info_rst[i].bssid[0],
            ap_info_rst[i].bssid[1], ap_info_rst[i].bssid[2],
            ap_info_rst[i].bssid[3], ap_info_rst[i].bssid[4],
            ap_info_rst[i].bssid[5], ap_info_rst[i].rssi,
            ap_info_rst[i].channel, ap_info_rst[i].authmode);
    }

__exit:
    if (ap_info) {
        OS_Free(ap_info);
        ap_info = NULL;
    }

    if (ret != LN_AT_ERR_NONE) {
        ln_at_printf(LN_AT_RET_ERR_STR);
    } else {
        ln_at_printf(LN_AT_RET_OK_STR);
    }

    return ret;
}

/**
 * AT+AP_SCAN=<timeout>
*/
static ln_at_err_t ln_at_set_ap_scan(uint8_t para_num, const char *name)
{
    bool is_default = false;
    int timeout = 0;

    wifi_scan_cfg_t scan_cfg = {
        .scan_time = 50,
    };

    if (para_num != 1) {
        goto __exit;
    }

    if (LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(1, &is_default, &timeout)) {
        goto __exit;
    }

    if (timeout > 100) {
        ln_at_printf("Note: 0 < timeout <= 100\r\n");
    }

    scan_cfg.scan_time = timeout;

    return ln_at_ap_scan(&scan_cfg);
__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

/**
 * AT+AP_SCAN
*/
static ln_at_err_t ln_at_exec_ap_scan(const char *name)
{
    return ln_at_ap_scan(NULL);
}
LN_AT_CMD_REG(AP_SCAN, NULL, ln_at_set_ap_scan, NULL, ln_at_exec_ap_scan);


/****************************************************************************************************/
/****************************************************************************************************/
/****************************************************************************************************/



/**
 * Get/Set
 * AT+CWHOSTNAME?
 * AT+CWHOSTNAME=<hostname>
*/
#define LN_AT_HOST_NAME_LEN (33u)
static ln_at_err_t ln_at_set_hostname_cfg(uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;
    char *host_nam_p = NULL;

    if (para_num != 1)
    {
        goto __exit;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &host_nam_p))
    {
        goto __exit;
    }
    if (is_default)
    {
        goto __exit;
    }


    if (0 != sysparam_sta_hostname_update(host_nam_p))
    {
        goto __exit;
    }
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_get_hostname(const char *name)
{
    char host_name[LN_AT_HOST_NAME_LEN];

    if (sysparam_sta_hostname_get(host_name) != 0)
    {
        goto __exit;
    }

    ln_at_printf("%s:%s", name, host_name);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

LN_AT_CMD_REG(CWHOSTNAME, ln_at_get_hostname, ln_at_set_hostname_cfg, NULL, NULL);


/**
 * set/exec
 *
 * AT+CWSTARTSMART
 * AT+CWSTARTSMART=<type>
 *
 * type: 1-ESP-TOUCH; 2-Airkiss; 3-ESP-TOUCH + Airkiss
*/
static ln_at_err_t ln_at_set_start_smartconfig(uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;

    int type = 0;
    int auth = 0;       //default
    char *key_str = NULL;
    char key[16] = {0};
    key_str = key;
    wifi_mode_t mode = wifi_current_mode_get();
    at_wifi_adapter_t *pada = at_wifi_adapter_get_handle();

    if(mode != WIFI_MODE_STATION)
    {
        goto __exit;
    }

    if (para_num < 1 || para_num > 3)
    {
        goto __exit;
    }

    if(para_num >= 1)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &type))
        {
            goto __exit;
        }
        if (is_default)
        {
            goto __exit;
        }
        if(type < 1 || type > 4)
        {
            goto __exit;
        }
    }

    if(para_num >= 2)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &auth))
        {
            goto __exit;
        }
        if (is_default)
        {
            goto __exit;
        }
        if(type < 0 || type > 7)
        {
            goto __exit;
        }
    }

    if(para_num >= 3)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &key_str))
        {
            goto __exit;
        }
        if (is_default)
        {
            goto __exit; 
        }
        if(strlen(key) > 16)
        {
            goto __exit;
        }
    }

    pada->at_func.smartconfig_func.wifi_sta_smartconfig_start_func(type, auth, key_str);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_exec_start_smartconfig(const char *name)
{
    at_wifi_adapter_t *pada = at_wifi_adapter_get_handle();

    pada->at_func.smartconfig_func.wifi_sta_smartconfig_start_func(1,0,NULL);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

}

static ln_at_err_t ln_at_exec_stop_smartconfig(const char *name)
{
    at_wifi_adapter_t *pada = at_wifi_adapter_get_handle();

    if(pada->at_func.smartconfig_func.is_running)
    {
        pada->at_func.smartconfig_func.wifi_sta_smartconfig_stop_func();
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(CWSTARTSMART, NULL, ln_at_set_start_smartconfig, NULL, ln_at_exec_start_smartconfig);
LN_AT_CMD_REG(CWSTOPSMART, NULL, NULL, NULL, ln_at_exec_stop_smartconfig);


