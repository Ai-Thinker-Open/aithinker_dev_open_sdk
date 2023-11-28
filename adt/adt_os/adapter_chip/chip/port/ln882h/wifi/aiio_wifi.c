/** @brief      Wifi application interface.
 *
 *  @file       aiio_wifi.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       Wifi application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/03/28      <td>1.0.0       <td>liq         <td>wifi init
 *  </table>
 *
 */
#include "aiio_wifi.h"
#include "osal/osal.h"
#include "utils/debug/log.h"
#include "wifi.h"
#include "wifi_port.h"
#include "netif/ethernetif.h"
#include "wifi_manager.h"
#include "lwip/tcpip.h"
#include "drv_adc_measure.h"
#include "utils/debug/ln_assert.h"
#include "utils/system_parameter.h"
#include "utils/sysparam_factory_setting.h"
#include "utils/ln_psk_calc.h"
#include "utils/power_mgmt/ln_pm.h"
#include "hal/hal_adc.h"
#include "ln_nvds.h"
#include "ln_wifi_err.h"
#include "ln_misc.h"
#include "ln882h.h"

static aiio_event_cb wifi_cb;
aiio_input_event_t aiio_event = {0};
static aiio_wifi_mode_t g_wifi_mode = AIIO_WIFI_MODE_MAX;
static aiio_sniffer_cb_t aiio_sniffer_cb = NULL;
aiio_wifi_config_t wifi_config = {0};
static aiio_os_semaphore_handle_t sem_scan = NULL;

int32_t scan_cnt = 0;

static aiio_wifi_ip_params_t ap_ip_params =
{
    .ip = 0x0164A8C0,       //192.168.100.1
    .gateway = 0x0164A8C0,  //192.168.100.1
    .gateway = 0x00ffffff,  //255.255.255.0
};
static aiio_wifi_ip_params_t sta_ip_params = { 0 };

static aiio_wifi_sta_basic_info_t aiio_sta_info= { 0 };
static aiio_wifi_sta_basic_info_t aiio_sta_info_temp[3]={0};

#define AIIO_SNIFFER_MODE 0
#define AIIO_MONITOR_MODE 1

static void aiio_wifi_get_ip_cb(struct netif *nif);
static void aiio_wifi_sta_startup_cb(void *arg);
static void aiio_wifi_sta_connected_cb(void *arg);
static void aiio_wifi_sta_disconnected_cb(void *arg);
static void aiio_wifi_sta_scan_complete_cb(void *arg);
static void aiio_wifi_sta_connect_failed_cb(void *arg);
static void aiio_wifi_softap_startup_cb(void *arg);
static void aiio_wifi_softap_associated_cb(void *arg);
static void aiio_wifi_softap_disassociated_cb(void *arg);
static void aiio_wifi_softap_associat_filed_cb(void *arg);

static aiio_err_t aiio_wifi_init_sta(void)
{
    aiio_err_t ret = AIIO_OK;
    uint8_t mac_addr[6];
    sta_ps_mode_t ps_mode = WIFI_NO_POWERSAVE;

    //1. sta mac get
    //get mac addr from Flash OTP
    if(ln_get_read_param_from_fotp_flag() == 0x01)
    {
        ln_fotp_get_mac_val(mac_addr);
        sysparam_softap_mac_update((const uint8_t *)mac_addr);
    }
    else
    {
        if (SYSPARAM_ERR_NONE != sysparam_sta_mac_get(mac_addr)) 
        {
            aiio_log_e("sta mac get filed!!!");
            ret = AIIO_ERROR;
            goto __exit;
        }
        if (mac_addr[0] == STA_MAC_ADDR0 &&
            mac_addr[1] == STA_MAC_ADDR1 &&
            mac_addr[2] == STA_MAC_ADDR2 &&
            mac_addr[3] == STA_MAC_ADDR3 &&
            mac_addr[4] == STA_MAC_ADDR4 &&
            mac_addr[5] == STA_MAC_ADDR5) 
        {
            ln_generate_random_mac(mac_addr);
        }
    }
	sysparam_sta_mac_update((const uint8_t *)mac_addr);	

	//2. stop wifi
    wifi_sta_status_t staStat;
    wifi_get_sta_status(&staStat);
    //aiio_log_d("status: %d", staStat);
    if (staStat != WIFI_STA_STATUS_SCANING && staStat != WIFI_STA_STATUS_DISCONNECTED && staStat != WIFI_STA_STATUS_STARTUP)
    {
        wifi_sta_disconnect();
    }

    wifi_stop();
    netdev_set_state(NETIF_IDX_STA, NETDEV_DOWN);
	netdev_get_ip_cb_set(NULL);

    //3. net device(lwip)
    netdev_set_mac_addr(NETIF_IDX_STA, mac_addr);
    netdev_set_active(NETIF_IDX_STA);

	if(wifi_config.sta.use_dhcp == 0)   
	{
		tcpip_ip_info_t ip_info;

        ip_info.ip.addr = sta_ip_params.ip;
        ip_info.gw.addr = sta_ip_params.gateway;
        ip_info.netmask.addr = sta_ip_params.netmask;

		netdev_set_ip_info(NETIF_IDX_STA, &ip_info);
	}
	else
	{
		netdev_get_ip_cb_set(aiio_wifi_get_ip_cb);
	}

    if(WIFI_ERR_NONE != wifi_sta_start(mac_addr, ps_mode))
    {
        aiio_log_e("wifi sta start filed!!!");
        ret = AIIO_ERROR;
        goto __exit;
    }

__exit:
    return ret;
}

static void aiio_wifi_get_ip_cb(struct netif *nif)
{
    //aiio_log_d("ip:%x",  nif->ip_addr.addr);

    if (wifi_cb)
    {
        aiio_event.code = AIIO_WIFI_EVENT_STA_GOT_IP;
        wifi_cb(&aiio_event, (void *)nif);
    }
}

static void aiio_wifi_sta_startup_cb(void *arg)
{
    if (wifi_cb)
    {
        aiio_event.code = AIIO_WIFI_EVENT_STA_STARTUP;
        wifi_cb(&aiio_event, 0);
    }
}

static void aiio_wifi_sta_connected_cb(void *arg)
{
    if (wifi_cb)
    {
        aiio_event.code = AIIO_WIFI_EVENT_STA_CONNECTED;
        wifi_cb(&aiio_event, NULL);
    }
}

static void aiio_wifi_sta_disconnected_cb(void *arg)
{
    if (wifi_cb)
    {
        aiio_event.value = AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_CONNECTION;
        aiio_event.code = AIIO_WIFI_EVENT_STA_DISCONNECTED;
        wifi_cb(&aiio_event, NULL);
    }
}

static void aiio_wifi_sta_scan_complete_cb(void *arg)
{
    LN_UNUSED(arg);

    if (sem_scan) 
    {
        aiio_os_semaphore_put(sem_scan);
        for(;scan_cnt==0;scan_cnt--){
            if (wifi_cb)
            {
                aiio_event.code = AIIO_WIFI_EVENT_SCAN_DONE;
                wifi_cb(&aiio_event, 0);
            }
        }
    }
}

static void aiio_wifi_sta_connect_failed_cb(void *arg)
{

}

static void aiio_wifi_softap_startup_cb(void *arg)
{
    netdev_set_state(NETIF_IDX_AP, NETDEV_UP);

    if (wifi_cb)
    {
        aiio_event.code = AIIO_WIFI_EVENT_AP_START;
        wifi_cb(&aiio_event, NULL);
    }
}

static void aiio_wifi_softap_Compare_associated(aiio_wifi_sta_basic_info_t *aiio_sta_info_compare,uint8_t *idx)
{
    uint8_t i=0;
    uint8_t mac_null[6]={0};
    for(i=0;i<3;i++){
        if(memcmp(&aiio_sta_info_temp[i].sta_mac,&mac_null,6)==0){
            // if(memcmp(aiio_sta_info_compare->sta_mac,&aiio_sta_info_temp[i].sta_mac,6)!=0)
            // {
                aiio_sta_info_temp[i].is_used=1;
                aiio_sta_info_temp[i].sta_idx=i;
                memcpy(aiio_sta_info_temp[i].sta_mac,aiio_sta_info_compare->sta_mac,6);
                break;
            // }
        }
    }
    *idx=i;

    return AIIO_OK;

}

static void aiio_wifi_softap_Compare_Disassociated(aiio_wifi_sta_basic_info_t *aiio_sta_info_compare)
{
    uint8_t mac_null[6]={0};
    for(uint8_t i=0;i<3;i++){
        if(memcmp(aiio_sta_info_compare->sta_mac,&aiio_sta_info_temp[i].sta_mac,6)==0)
        {
            aiio_sta_info_temp[i].is_used=0;
            aiio_sta_info_temp[i].sta_idx=0;
            memcpy(aiio_sta_info_temp[i].sta_mac,&mac_null,6);
            break;
        }
    }
    return AIIO_OK;
}

static void aiio_wifi_softap_associated_cb(void *arg)
{
    uint8_t *mac = (uint8_t *)arg;
    uint8_t idx=0;
    memcpy(aiio_sta_info.sta_mac,mac,6);
    aiio_wifi_softap_Compare_associated(&aiio_sta_info,&idx);
    if (wifi_cb)
    {
        aiio_event.value=idx;
        aiio_event.code = AIIO_WIFI_EVENT_AP_STACONNECTED;
        wifi_cb(&aiio_event, (void *)mac);
    }
}

static void aiio_wifi_softap_disassociated_cb(void *arg)
{
    uint8_t *mac = (uint8_t *)arg;

    memcpy(aiio_sta_info.sta_mac,mac,6);

    aiio_wifi_softap_Compare_Disassociated(&aiio_sta_info.sta_mac);
    if (wifi_cb)
    {
        aiio_event.code = AIIO_WIFI_EVENT_AP_STADISCONNECTED;
        wifi_cb(&aiio_event, (void *)mac);
    }
}

//a station connected to softAP failed cb
static void aiio_wifi_softap_associat_filed_cb(void *arg)
{
    //Do nothing
}

aiio_err_t aiio_wifi_register_event_cb(aiio_event_cb cb)
{
    wifi_cb = cb;
    netdev_get_ip_cb_set(aiio_wifi_get_ip_cb);
    return AIIO_OK;
}

aiio_err_t aiio_wifi_unregister_event_cb(void)
{
    wifi_cb = NULL;
    netdev_get_ip_cb_set(NULL);
    return AIIO_OK;
}

aiio_err_t aiio_wifi_set_mode(aiio_wifi_mode_t mode)
{
    aiio_err_t ret = AIIO_OK;

    if (mode == g_wifi_mode)
    {
        aiio_log_d("There is no need to switch modes, mode(%d)", g_wifi_mode);
        goto __exit;
    }

    if (mode >= AIIO_WIFI_MODE_APSTA)
    {
        aiio_log_e("This mode is not supported! mode:%d", mode);
        ret = AIIO_ERROR;
        goto __exit;
    }

    if (g_wifi_mode != AIIO_WIFI_MODE_MAX)
    {
        wifi_stop();
    }

    if (g_wifi_mode == AIIO_WIFI_MODE_STA)
    {
        netdev_set_state(NETIF_IDX_STA, NETDEV_DOWN);
    }
    else if (g_wifi_mode == AIIO_WIFI_MODE_AP)
    {
        netdev_set_state(NETIF_IDX_AP, NETDEV_DOWN);
    }
    
    if (mode == AIIO_WIFI_MODE_NULL)
    {
        aiio_log_d("turn off wifi");
    }

    if (mode == AIIO_WIFI_MODE_STA)
    {
        netdev_set_active(NETIF_IDX_STA);
        aiio_wifi_init_sta();
    }
    else if (mode == AIIO_WIFI_MODE_AP)
    {
        netdev_set_active(NETIF_IDX_AP);
    }

    g_wifi_mode = mode;

__exit:
    return ret;
}

aiio_err_t aiio_wifi_get_mode(aiio_wifi_mode_t *mode)
{
    aiio_err_t ret = AIIO_OK;
    wifi_mode_t ln_mode;

    if (mode == NULL)
    {
        ret = AIIO_ERROR;
        goto __exit;
    }

    if (g_wifi_mode == AIIO_WIFI_MODE_MAX)
    {
        ln_mode = wifi_current_mode_get();
        if (ln_mode == WIFI_MODE_STATION)
        {
            *mode = g_wifi_mode = AIIO_WIFI_MODE_STA;
        }
        else if (ln_mode == WIFI_MODE_AP)
        {
            *mode = g_wifi_mode = AIIO_WIFI_MODE_AP;
        }
        else if (ln_mode == WIFI_MODE_OFF)
        {
            *mode = g_wifi_mode = AIIO_WIFI_MODE_NULL;
        }
        else
        {
            aiio_log_e("Get mode error! ln wifi mode:%d", ln_mode);
            ret = AIIO_ERROR;
            goto __exit;
        }
    }
    else
    {
        *mode = g_wifi_mode;
    }

__exit:
    return ret;
}

aiio_err_t aiio_wifi_set_config(aiio_wifi_port_t port, aiio_wifi_config_t *conf)
{
    if (port == AIIO_WIFI_IF_STA)
    { // sta config
        if (conf->sta.ssid)
        {
            aiio_strncpy((char *)wifi_config.sta.ssid, (char *)conf->sta.ssid, SSID_LEN);
        }
        if (conf->sta.password)
        {
            aiio_strncpy((char *)wifi_config.sta.password, (char *)conf->sta.password, PASSWORD_LEN);
        }
        if (conf->sta.bssid)
        {
            aiio_strncpy((char *)wifi_config.sta.bssid, (char *)conf->sta.bssid, MAC_LEN);
        }
        if (conf->sta.band)
        {
            wifi_config.sta.band = conf->sta.band;
        }
        if (conf->sta.channel)
        {
            wifi_config.sta.channel = conf->sta.channel;
        }
        wifi_config.sta.use_dhcp = conf->sta.use_dhcp;
        wifi_config.sta.flags = conf->sta.flags;
    }
    else
    { // ap config
        if (conf->ap.ssid)
        {
            aiio_strncpy((char *)wifi_config.ap.ssid, (char *)conf->ap.ssid, SSID_LEN);
        }
        if (conf->ap.password)
        {
            aiio_strncpy((char *)wifi_config.ap.password, (char *)conf->ap.password, PASSWORD_LEN);
        }
        if (conf->ap.max_connection)
        {
            wifi_config.ap.max_connection = conf->ap.max_connection;
        }
        else
        {
            wifi_config.ap.max_connection = 4;
        }
        if (conf->ap.channel)
        {
            wifi_config.ap.channel = conf->ap.channel;
        }
        else
        {
            wifi_config.ap.channel = 1;
        }
        if (conf->ap.ssid_hidden)
        {
            wifi_config.ap.ssid_hidden = conf->ap.ssid_hidden;
        }
        else
        {
            wifi_config.ap.ssid_hidden = 0;
        }
        if (conf->ap.ssid_len)
        {
            wifi_config.ap.ssid_len = conf->ap.ssid_len;
        }
        if (conf->ap.dhcp_params.lease_time)
        {
            wifi_config.ap.dhcp_params.lease_time = conf->ap.dhcp_params.lease_time;
        }
        else
        {
            wifi_config.ap.dhcp_params.lease_time = 120;
        }
        if (conf->ap.dhcp_params.start)
        {
            wifi_config.ap.dhcp_params.start = conf->ap.dhcp_params.start;
        }
        else
        {
            wifi_config.ap.dhcp_params.start = 2;
        }
        if (conf->ap.dhcp_params.end)
        {
            wifi_config.ap.dhcp_params.end = conf->ap.dhcp_params.end;
        }
        else
        {
            wifi_config.ap.dhcp_params.end = 100;
        }
        wifi_config.ap.use_dhcp = conf->ap.use_dhcp;
    }
    return AIIO_OK;
}

aiio_err_t aiio_wifi_get_config(aiio_wifi_port_t port, aiio_wifi_config_t *conf)
{
    aiio_err_t ret = AIIO_ERROR;

    if (conf == NULL)
    {
        aiio_log_e("The conf is null!");
        goto __exit;
    }

    if (port == AIIO_WIFI_IF_STA)
    {
        strncpy((char *)conf->sta.ssid, (char *)wifi_config.sta.ssid, SSID_LEN);
        strncpy((char *)conf->sta.password, (char *)wifi_config.sta.password, PASSWORD_LEN);
        strncpy((char *)conf->sta.bssid, (char *)wifi_config.sta.bssid, MAC_LEN);
        conf->sta.band = wifi_config.sta.band;
        conf->sta.channel = wifi_config.sta.channel;
        conf->sta.use_dhcp = wifi_config.sta.use_dhcp;
        conf->sta.flags = wifi_config.sta.flags;
    }
    else if (port == AIIO_WIFI_IF_AP)
    {
        strncpy((char *)conf->ap.ssid, (char *)wifi_config.ap.ssid, SSID_LEN);
        strncpy((char *)conf->ap.password, (char *)wifi_config.ap.password, PASSWORD_LEN);
        conf->ap.max_connection = wifi_config.ap.max_connection;
        conf->ap.channel = wifi_config.ap.channel;
        conf->ap.ssid_hidden = wifi_config.ap.ssid_hidden;
        conf->ap.ssid_len = wifi_config.ap.ssid_len;
        conf->ap.use_dhcp = wifi_config.ap.use_dhcp;
        conf->ap.dhcp_params.lease_time = wifi_config.ap.dhcp_params.lease_time;
        conf->ap.dhcp_params.start = wifi_config.ap.dhcp_params.start;
        conf->ap.dhcp_params.end = wifi_config.ap.dhcp_params.end;
    }
    else
    {
        aiio_log_e("port not supported!");
        goto __exit;
    }

    ret = AIIO_OK;

__exit:
    return ret;
}

aiio_err_t aiio_wifi_connect(void)
{
    aiio_err_t ret = AIIO_OK;
    wifi_sta_connect_t connect;
    wifi_scan_cfg_t scan_cfg;
    uint8_t psk_value[40] = {0x0};

    if (g_wifi_mode == AIIO_WIFI_MODE_STA)
    {
        ret = aiio_wifi_init_sta();
        if (ret != AIIO_OK)
        {
            goto __exit;
        }

        connect.ssid = &wifi_config.sta.ssid;
        connect.pwd = &wifi_config.sta.password;
        connect.bssid = &wifi_config.sta.bssid;
        connect.psk_value = NULL;
        scan_cfg.channel = wifi_config.sta.channel;
        // scan_cfg.channel = 0;
        scan_cfg.scan_type = WIFI_SCAN_TYPE_ACTIVE;
        scan_cfg.scan_time = 20;
        aiio_log_d("SSID:%s  PWD:%s  channel:%d  ssid_lenght:%d pwd_lenght:%d", connect.ssid, connect.pwd, scan_cfg.channel, strlen(connect.ssid), strlen(connect.pwd));

        connect.psk_value = NULL;
        if (strlen(connect.pwd) != 0)
        {
            if (0 == ln_psk_calc(connect.ssid, connect.pwd, psk_value, sizeof(psk_value)))
            {
                connect.psk_value = psk_value;
                hexdump(LOG_LVL_INFO, "psk value ", psk_value, sizeof(psk_value));
            }
        }
        if (WIFI_ERR_NONE != wifi_sta_connect(&connect, &scan_cfg))
        {
            aiio_log_e("fail: ssid:%s pwd:%s", connect.ssid, connect.pwd);
            ret = AIIO_ERROR;
            goto __exit;
        }
    }

__exit:
    return ret;
}

aiio_err_t aiio_wifi_disconnect(void)
{
    if (g_wifi_mode == AIIO_WIFI_MODE_STA)
    {
        wifi_sta_disconnect();
        return AIIO_OK;
    }
    return AIIO_ERROR;
}

aiio_err_t aiio_wifi_start(void)
{
    aiio_err_t ret = AIIO_ERROR;

    if (g_wifi_mode == AIIO_WIFI_MODE_AP)
    {
        uint8_t mac[6] = {0x94, 0xc9, 0x60, 0x0e, 0x2b, 0xaf};
        uint8_t psk_value[40] = {0};
        tcpip_ip_info_t ip_info;
        server_config_t server_config;
        wifi_softap_cfg_t ap_cfg = {0};
        
        //get mac addr from Flash OTP
        if(ln_get_read_param_from_fotp_flag() == 0x01)
        {
            ln_fotp_get_mac_val(mac);
            sysparam_sta_mac_update((const uint8_t *)mac);	
        }
        else
        {
            ln_generate_random_mac(mac);
        }

        wifi_stop();
        netdev_set_state(NETIF_IDX_AP, NETDEV_DOWN);

        //ap cfg info
        // ap_cfg.ssid = "LN_AP_DEMO";
        ap_cfg.ssid = wifi_config.ap.ssid;
        // ap_cfg.pwd = "12345678";
        ap_cfg.pwd = wifi_config.ap.password;
        ap_cfg.bssid = mac;
        ap_cfg.ext_cfg.authmode = WIFI_AUTH_OPEN;
        ap_cfg.ext_cfg.beacon_interval = 100;
        ap_cfg.ext_cfg.channel = wifi_config.ap.channel;
        ap_cfg.ext_cfg.max_conn = wifi_config.ap.max_connection;
        ap_cfg.ext_cfg.ssid_hidden = wifi_config.ap.ssid_hidden;
        
        if (strlen(ap_cfg.pwd) > 0)
        {
            ap_cfg.ext_cfg.authmode = WIFI_AUTH_WPA2_PSK;
        }
        else
        {
            ap_cfg.ext_cfg.max_conn = 0;    //if max conn not set 0 on auth mode , it have bug
        }

        ip_info.ip.addr = ap_ip_params.ip;
        ip_info.gw.addr = ap_ip_params.gateway;
        ip_info.netmask.addr = ap_ip_params.netmask;

        server_config.server.addr   = ip_info.ip.addr;
        server_config.port          = 67;
        server_config.lease         = wifi_config.ap.dhcp_params.lease_time;
        server_config.renew         = wifi_config.ap.dhcp_params.lease_time;
        // server_config.ip_start.addr = ipaddr_addr((const char *)"192.168.4.2");
        server_config.ip_start.addr = ip_info.ip.addr & 0x00ffffff | (wifi_config.ap.dhcp_params.start << 24);
        server_config.ip_end.addr   = ip_info.ip.addr & 0x00ffffff | (wifi_config.ap.dhcp_params.end << 24);
        server_config.client_max    = wifi_config.ap.max_connection;
        dhcpd_curr_config_set(&server_config);

        //1. net device(lwip).
        netdev_set_mac_addr(NETIF_IDX_AP, mac);
        netdev_set_ip_info(NETIF_IDX_AP, &ip_info);
        netdev_set_active(NETIF_IDX_AP);
        // wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_STARTUP, &ap_startup_cb);

        sysparam_softap_mac_update((const uint8_t *)mac);

        ap_cfg.ext_cfg.psk_value = NULL;
        if ((strlen(ap_cfg.pwd) != 0) &&
            (ap_cfg.ext_cfg.authmode != WIFI_AUTH_OPEN) &&
            (ap_cfg.ext_cfg.authmode != WIFI_AUTH_WEP)) 
        {
            memset(psk_value, 0, sizeof(psk_value));
            if (0 == ln_psk_calc(ap_cfg.ssid, ap_cfg.pwd, psk_value, sizeof (psk_value))) 
            {
                ap_cfg.ext_cfg.psk_value = psk_value;
                hexdump(LOG_LVL_INFO, "psk value ", psk_value, sizeof(psk_value));
            }
        }

        aiio_log_d("SSID:%s", ap_cfg.ssid);
        aiio_log_d("PWD:%s", ap_cfg.pwd);
        aiio_log_d("BSSI:%02X %02X %02X %02X %02X %02X", ap_cfg.bssid[0], ap_cfg.bssid[1], ap_cfg.bssid[2], ap_cfg.bssid[3], ap_cfg.bssid[4], ap_cfg.bssid[5]);
        aiio_log_d("ap_cfg.authmode:%d", ap_cfg.ext_cfg.authmode);
        aiio_log_d("ap_cfg.beacon_interval:%d", ap_cfg.ext_cfg.beacon_interval);
        aiio_log_d("ap_cfg.channel:%d", ap_cfg.ext_cfg.channel);
        aiio_log_d("ap_cfg.max_conn:%d", ap_cfg.ext_cfg.max_conn);
        aiio_log_d("ap_cfg.ssid_hidden:%d", ap_cfg.ext_cfg.ssid_hidden);
        if (ap_cfg.ext_cfg.psk_value != NULL)
        {
            //elog_hexdump("ap_cfg.psk_value",16,(char *)ap_cfg.ext_cfg.psk_value, sizeof(ap_cfg.ext_cfg.psk_value));
        }

        //2. wifi
        if(WIFI_ERR_NONE !=  wifi_softap_start(&ap_cfg))
        {
            aiio_log_e("[%s, %d]wifi_start() fail.\r\n", __func__, __LINE__);
            goto __exit;
        }

        ret = AIIO_OK;
    }
    else if (g_wifi_mode == AIIO_WIFI_MODE_STA)
    {
        ret = aiio_wifi_connect();
        if (ret != AIIO_OK)
        {
            goto __exit;
        }
    }

__exit:
    return ret;
}

void aiio_wifi_sniffer_cb_t(void *buf, uint16_t len, wifi_pkt_type_t pkt_type, int8_t rssi_db)
{
    // aiio_log_d("aiio_wifi_sniffer_cb_t");
    aiio_bl_rx_info_t aiio_bl_rx_info;
    uint8_t *env = NULL;

    aiio_bl_rx_info.rssi = rssi_db;

    if (aiio_sniffer_cb != NULL)
    {
        aiio_sniffer_cb((void *)env, buf, len, &aiio_bl_rx_info);
    }
}

aiio_err_t aiio_wifi_sniffer_register(void *env, aiio_sniffer_cb_t cb)
{
    aiio_sniffer_cb = cb;

    return AIIO_OK;
}

aiio_err_t aiio_wifi_sniffer_unregister(void *env)
{
    aiio_sniffer_cb = NULL;
    return AIIO_OK;
}

aiio_err_t aiio_wifi_sniffer_enable(void)
{
    sniffer_cfg_t sniffer_cfg = {
        .enable = true,
        .type = AIIO_SNIFFER_MODE,
        .filter_mask = WIFI_SNIFFER_FILTER_MASK_DATA,
    };

    aiio_wifi_init_sta();
    wifi_sta_set_sniffer_cfg(&sniffer_cfg, aiio_wifi_sniffer_cb_t);
    return AIIO_OK;
}

aiio_err_t aiio_wifi_sniffer_disable(void)
{
    sniffer_cfg_t sniffer_cfg = {0};

    wifi_sta_set_sniffer_cfg(&sniffer_cfg, aiio_wifi_sniffer_cb_t);
    return AIIO_OK;
}

aiio_err_t aiio_wifi_set_country_code(char *country_code)
{
    aiio_err_t ret = AIIO_OK;
    wifi_country_code_t tmp_country_code;

    if (country_code == NULL)
    {
        aiio_log_e("input param is null");
        ret = AIIO_ERROR;
        goto __exit;
    }

    if (strcmp((const char *)country_code, "CN") == 0)
    {
        tmp_country_code = CTRY_CODE_CN;
    }
    else if (strcmp((const char *)country_code, "US") == 0)
    {
        tmp_country_code = CTRY_CODE_US;
    }
    else if (strcmp((const char *)country_code, "JP") == 0)
    {
        tmp_country_code = CTRY_CODE_JP;
    }
    else if (strcmp((const char *)country_code, "ISR") == 0)
    {
        tmp_country_code = CTRY_CODE_ISR;
    }
    else
    {
        aiio_log_e("unknown country code str:%s", country_code);
        ret = AIIO_ERROR;
        goto __exit;
    }

    wifi_set_country_code(tmp_country_code);

__exit:
    return ret;
}

aiio_err_t aiio_wifi_get_country_code(char *country_code)
{
    aiio_err_t ret = AIIO_OK;
    wifi_country_code_t tmp_country_code;

    if (country_code == NULL)
    {
        aiio_log_e("input param is null");
        ret = AIIO_ERROR;
        goto __exit;
    }

    if (wifi_get_country_code(&tmp_country_code) != 0)
    {
        ret = AIIO_ERROR;
        goto __exit;
    }

    switch (tmp_country_code)
    {
    case CTRY_CODE_CN:
        strcpy(country_code, "CN");
        break;
    case CTRY_CODE_US:
        strcpy(country_code, "US");
        break;
    case CTRY_CODE_JP:
        strcpy(country_code, "JP");
        break;
    case CTRY_CODE_ISR:
        strcpy(country_code, "ISR");
        break;
    default:
        aiio_log_e("unknown country code:%d", tmp_country_code);
        ret = AIIO_ERROR;
        break;
    }

__exit:
    return ret;
}

aiio_err_t aiio_wifi_channel_get(int32_t *channel)
{
    if (channel == NULL)
    {
        return AIIO_ERROR;
    }
    wifi_get_channel(channel);
    return AIIO_OK;
}
aiio_err_t aiio_wifi_channel_set(int32_t channel)
{
    wifi_set_channel(channel);
    return AIIO_OK;
}

aiio_err_t aiio_wifi_rssi_get(int32_t *rssi)
{
    aiio_err_t ret = AIIO_ERROR;
    int8_t rssi_data;

    if (rssi == NULL)
    {
        aiio_log_e("input param is null");
        goto __exit;
    }

    if (wifi_sta_get_rssi(&rssi_data) != 0)
    {
        aiio_log_e("get rssi failed");
        goto __exit;
    }

    *rssi = rssi_data;
    ret = AIIO_OK;

__exit:
    return ret;
}

aiio_err_t aiio_wifi_sta_mac_set(uint8_t *mac)
{
    // if (mac == NULL)
    // {
    //     return AIIO_ERROR;
    // }
    // return wifi_set_macaddr(STATION_IF, mac);
    aiio_err_t ret = AIIO_ERROR;

    if (mac == NULL)
    {
        aiio_log_e("input param is null");
        goto __exit;
    }

    if (0 != sysparam_sta_mac_update((const uint8_t *)mac))
    {
        aiio_log_e("sta update mac failed!");
        goto __exit;
    }

    ret = AIIO_OK;

__exit:
    return ret;
}

aiio_err_t aiio_wifi_sta_mac_get(uint8_t *mac)
{
    // if (mac == NULL)
    // {
    //     return AIIO_ERROR;
    // }
    // return wifi_get_macaddr(STATION_IF, mac);
    aiio_err_t ret = AIIO_ERROR;

    if (mac == NULL)
    {
        aiio_log_e("input param is null");
        goto __exit;
    }

    if (0 != sysparam_sta_mac_get(mac))
    {
        aiio_log_e("sta get mac failed!");
        goto __exit;
    }

    ret = AIIO_OK;

__exit:
    return ret;
}

aiio_err_t aiio_wifi_sta_ip_get(uint32_t *ip, uint32_t *gw, uint32_t *mask)
{
    aiio_err_t ret = AIIO_ERROR;
    tcpip_ip_info_t ip_info;

    if (g_wifi_mode != AIIO_WIFI_MODE_STA)
    {
        aiio_log_e("wifi mode error:%d", g_wifi_mode);
        goto __exit;
    }

    if (ip == NULL || gw == NULL || mask == NULL)
    {
        aiio_log_e("input param is null");
        goto __exit;
    }

    if (0 != netdev_get_ip_info(netdev_get_active(), &ip_info))
    {
        aiio_log_e("netdev get ip failed");
        goto __exit;
    }

    *ip = ip_info.ip.addr;
    *gw = ip_info.gw.addr;
    *mask = ip_info.netmask.addr;

    ret = AIIO_OK;

__exit:
    return ret;
}
aiio_err_t aiio_wifi_sta_netif_get(uint32_t *ip, uint32_t *gw, uint32_t *mask)
{
    aiio_err_t ret = AIIO_ERROR;
    tcpip_ip_info_t ip_info;

    if (g_wifi_mode != AIIO_WIFI_MODE_STA)
    {
        aiio_log_e("wifi mode error:%d", g_wifi_mode);
        goto __exit;
    }

    if (ip == NULL || gw == NULL || mask == NULL)
    {
        aiio_log_e("input param is null");
        goto __exit;
    }

    if (0 != netdev_get_ip_info(netdev_get_active(), &ip_info))
    {
        aiio_log_e("netdev get ip failed");
        goto __exit;
    }

    *ip = ip_info.ip.addr;
    *gw = ip_info.gw.addr;
    *mask = ip_info.netmask.addr;

    ret = AIIO_OK;

__exit:
    return ret;
}

aiio_err_t aiio_wifi_init(void)
{
    wifi_manager_init();

    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_STARTUP, &aiio_wifi_sta_startup_cb);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_CONNECTED, &aiio_wifi_sta_connected_cb);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_DISCONNECTED, &aiio_wifi_sta_disconnected_cb);
    // wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_SCAN_COMPLETE, &aiio_wifi_sta_scan_complete_cb);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_CONNECT_FAILED, &aiio_wifi_sta_connect_failed_cb);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_STARTUP, &aiio_wifi_softap_startup_cb);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_ASSOCIATED, &aiio_wifi_softap_associated_cb);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_DISASSOCIATED, &aiio_wifi_softap_disassociated_cb);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_ASSOCIAT_FILED, &aiio_wifi_softap_associat_filed_cb);
    // netdev_get_ip_cb_set(aiio_wifi_get_ip_cb);

    if (wifi_cb)
    {
        aiio_event.code = AIIO_WIFI_EVENT_WIFI_READY;
        wifi_cb(&aiio_event, 0);
    }
    
    return AIIO_OK;
}

aiio_err_t aiio_wifi_get_state(int32_t *state)
{
    int32_t cur_state = 0;
    if (state == NULL)
    {
        return AIIO_ERROR;
    }

    wifi_get_sta_status(&cur_state);

    switch (cur_state)
    {
    case WIFI_STA_STATUS_STARTUP:
        cur_state = AIIO_WIFI_STATE_IDLE;
        break;
    // case WIFI_STA_STATUS_SCANING:
    //     cur_state = AIIO_WIFI_EVENT_STA_SCANING;
    //     break;
    case WIFI_STA_STATUS_CONNECTING:
        cur_state = AIIO_WIFI_STATE_CONNECTING;
        break;
    case WIFI_STA_STATUS_CONNECTED:
        cur_state = AIIO_WIFI_STATE_CONNECTED_IP_GOT;
        break;
    // case WIFI_STA_STATUS_DISCONNECTING:
    //     cur_state = AIIO_WIFI_STATE_DISCONNECT;
    //     break;
    case WIFI_STA_STATUS_DISCONNECTED:
        cur_state = AIIO_WIFI_STATE_DISCONNECT;
        break;
    }
    *state = cur_state;
    return AIIO_OK;
}

aiio_err_t aiio_wifi_ap_stop(void)
{
    if (g_wifi_mode == AIIO_WIFI_MODE_AP)
    {
        wifi_stop();
        return AIIO_OK;
    }

    return AIIO_ERROR;
}

aiio_err_t aiio_wifi_ap_mac_set(uint8_t *mac)
{
    aiio_err_t ret = AIIO_ERROR;

    if (mac == NULL)
    {
        aiio_log_e("input param is null");
        goto __exit;
    }

    aiio_log_d("mac: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if (0 != sysparam_softap_mac_update((const uint8_t *)mac))
    {
        aiio_log_e("set ap mac failed!");
        goto __exit;
    }

    ret = AIIO_OK;

__exit:
    return ret;
}

aiio_err_t aiio_wifi_ap_mac_get(uint8_t *mac)
{
    aiio_err_t ret = AIIO_ERROR;

    if (mac == NULL)
    {
        aiio_log_e("input param is null");
        goto __exit;
    }

    if (0 != sysparam_softap_mac_get(mac))
    {
        aiio_log_e("get ap mac failed!");
        goto __exit;
    }

    ret = AIIO_OK;

__exit:
    return ret;
}

aiio_err_t aiio_wifi_ap_ip_get(uint32_t *ip, uint32_t *gw, uint32_t *mask)
{
    aiio_err_t ret = AIIO_OK;
    tcpip_ip_info_t ip_info;

    if (g_wifi_mode != AIIO_WIFI_MODE_AP)
    {
        aiio_log_e("wifi mode error:%d", g_wifi_mode);
        ret = AIIO_ERROR;
        goto __exit;
    }

    if (0 != netdev_get_ip_info(netdev_get_active(), &ip_info))
    {
        aiio_log_e("netdev get ip failed");
        ret = AIIO_ERROR;
        goto __exit;
    }

    *ip = ip_info.ip.addr;
    *gw = ip_info.gw.addr;
    *mask = ip_info.netmask.addr;

__exit:

    return ret;
}



aiio_err_t aiio_wifi_sta_ip_set(char* sta_ip)
{
    aiio_err_t ret = AIIO_OK;

    if (sta_ip == NULL)
    {
        aiio_log_e("input param is null");
        ret = AIIO_ERROR;
        goto __exit;
    }

    aiio_log_d("sta_ip:%s", sta_ip);

    sta_ip_params.ip = ipaddr_addr((const char *)sta_ip);
    sta_ip_params.gateway = sta_ip_params.ip & 0x00ffffff | 0x01000000;
    sta_ip_params.netmask = 0x00ffffff;

    aiio_log_d("ip:%d.%d.%d.%d", (uint8_t)sta_ip_params.ip, (uint8_t)(sta_ip_params.ip >> 8), (uint8_t)(sta_ip_params.ip >> 16), (uint8_t)(sta_ip_params.ip >> 24));
    aiio_log_d("gw:%d.%d.%d.%d", (uint8_t)sta_ip_params.gateway, (uint8_t)(sta_ip_params.gateway >> 8), (uint8_t)(sta_ip_params.gateway >> 16), (uint8_t)(sta_ip_params.gateway >> 24));
    aiio_log_d("netmask:%d.%d.%d.%d", (uint8_t)sta_ip_params.netmask, (uint8_t)(sta_ip_params.netmask >> 8), (uint8_t)(sta_ip_params.netmask >> 16), (uint8_t)(sta_ip_params.netmask >> 24));

__exit:
    return ret;
}

aiio_err_t aiio_wifi_sta_ip_parameter_set(uint32_t ip, uint32_t mask, uint32_t gw)
{
    tcpip_ip_info_t sta_ip_info;

    sta_ip_params.ip = ip;
    sta_ip_params.gateway = gw;
    sta_ip_params.netmask = mask;

    aiio_log_d("ip:%d.%d.%d.%d", (uint8_t)sta_ip_params.ip, (uint8_t)(sta_ip_params.ip >> 8), (uint8_t)(sta_ip_params.ip >> 16), (uint8_t)(sta_ip_params.ip >> 24));
    aiio_log_d("gw:%d.%d.%d.%d", (uint8_t)sta_ip_params.gateway, (uint8_t)(sta_ip_params.gateway >> 8), (uint8_t)(sta_ip_params.gateway >> 16), (uint8_t)(sta_ip_params.gateway >> 24));
    aiio_log_d("netmask:%d.%d.%d.%d", (uint8_t)sta_ip_params.netmask, (uint8_t)(sta_ip_params.netmask >> 8), (uint8_t)(sta_ip_params.netmask >> 16), (uint8_t)(sta_ip_params.netmask >> 24));

    return AIIO_OK;
}

aiio_err_t aiio_wifi_sta_ip_unset(void)
{
    //TODO
    aiio_log_e("Unsupported API");
    return AIIO_ERROR;
}

aiio_err_t aiio_wifi_ap_ip_set(char* ap_ip)
{
    aiio_err_t ret = AIIO_OK;
    tcpip_ip_info_t  ip_info;

    // ret = aiio_wifi_ap_stop();
    // if (ret != AIIO_OK)
    // {
    //     goto __exit;
    // }
    if (ap_ip == NULL)
    {
        ret = AIIO_ERROR;
        goto __exit;
    }

    aiio_log_d("input ip:%s", ap_ip);

    ip_info.ip.addr = ipaddr_addr((const char *)ap_ip);
    ip_info.gw.addr = ip_info.ip.addr & 0x00ffffff | 0x01000000;
    ip_info.netmask.addr = ipaddr_addr((const char *)"255.255.255.0");

    ap_ip_params.ip = ip_info.ip.addr;
    ap_ip_params.gateway = ip_info.gw.addr;
    ap_ip_params.netmask = ip_info.netmask.addr;

    aiio_log_d("ip:%d.%d.%d.%d", (uint8_t)ap_ip_params.ip, (uint8_t)(ap_ip_params.ip >> 8), (uint8_t)(ap_ip_params.ip >> 16), (uint8_t)(ap_ip_params.ip >> 24));
    aiio_log_d("gw:%d.%d.%d.%d", (uint8_t)ap_ip_params.gateway, (uint8_t)(ap_ip_params.gateway >> 8), (uint8_t)(ap_ip_params.gateway >> 16), (uint8_t)(ap_ip_params.gateway >> 24));
    aiio_log_d("netmask:%d.%d.%d.%d", (uint8_t)ap_ip_params.netmask, (uint8_t)(ap_ip_params.netmask >> 8), (uint8_t)(ap_ip_params.netmask >> 16), (uint8_t)(ap_ip_params.netmask >> 24));
    // if (netdev_set_ip_info(NETIF_IDX_AP, &ip_info) != 0)
    // {
    //     aiio_log_e("set ip info error!");
    //     ret = AIIO_ERROR;
    //     goto __exit;
    // } 

__exit:
    return ret;
}

aiio_err_t aiio_wifi_stop(void)
{
    if (g_wifi_mode == AIIO_WIFI_MODE_AP)
    {
        aiio_wifi_ap_stop();
    }
    else if (g_wifi_mode == AIIO_WIFI_MODE_STA)
    {
        aiio_wifi_disconnect();
    }
    else
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

aiio_err_t aiio_wifi_scan(void *data, aiio_scan_complete_cb_t cb, bool filter)
{
    (void)(filter);

    aiio_err_t ret = AIIO_OK;

    wifi_sta_status_t sta_status = WIFI_STA_STATUS_STARTUP;
    wifi_scan_cfg_t scan_cfg;
    

    if (g_wifi_mode != AIIO_WIFI_MODE_STA)
    {
        aiio_log_e("wifi is not in sta mode!");
        ret = AIIO_ERROR;
        goto __exit;
    }

    // set scan count
    wifi_get_sta_status(&sta_status);
    if (sta_status == WIFI_STA_STATUS_CONNECTED || sta_status == WIFI_STA_STATUS_DISCONNECTING)
    {
        scan_cnt = 6;
    }
    else
    {
        scan_cnt = 1;
    }
    aiio_log_d("scan cnt:%d", scan_cnt);

    //set scan params
    scan_cfg.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    scan_cfg.channel = 0;
    scan_cfg.scan_time = 50;
    aiio_log_d("scan_cfg.channel:%d", scan_cfg.channel);
    aiio_log_d("scan_cfg.scan_type:%d", scan_cfg.scan_type);
    aiio_log_d("scan_cfg.scan_time:%d", scan_cfg.scan_time);

    ret = aiio_os_semaphore_create_counting(&sem_scan, 1, 0);
    if (ret != AIIO_OK)
    {
        aiio_log_e("semaphore create failed!");
        goto __exit;
    }

    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_SCAN_COMPLETE, aiio_wifi_sta_scan_complete_cb);

    for (; scan_cnt > 0; scan_cnt--)
    {
        wifi_sta_scan(&scan_cfg);
        aiio_os_semaphore_get(sem_scan, aiio_os_ms2tick(1500));
    }

    //get scan info
    if (cb != NULL)
    {
        cb((void *)data, NULL);
    }

    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_SCAN_COMPLETE, NULL);
    aiio_os_semaphore_delete(sem_scan);
    sem_scan = NULL;
    wifi_manager_cleanup_scan_results();

__exit:
    return ret;
}

aiio_err_t aiio_wifi_scan_adv(void *data, aiio_scan_complete_cb_t cb, bool filter, aiio_wifi_scan_params_t *scan_params)
{
    (void)(filter);

    aiio_err_t ret = AIIO_OK;
    wifi_sta_status_t sta_status = WIFI_STA_STATUS_STARTUP;
    wifi_scan_cfg_t scan_cfg;
    int scan_cnt = 1;

    if (scan_params == NULL)
    {
        aiio_log_e("input params is null!");
        ret = AIIO_ERROR;
        goto __exit;
    }

    if (g_wifi_mode != AIIO_WIFI_MODE_STA)
    {
        aiio_log_e("wifi is not in sta mode!");
        ret = AIIO_ERROR;
        goto __exit;
    }

    // set scan count
    wifi_get_sta_status(&sta_status);
    aiio_log_i("sta_status:%d", sta_status);
    if (sta_status == WIFI_STA_STATUS_CONNECTED || sta_status == WIFI_STA_STATUS_DISCONNECTING || sta_status == WIFI_STA_STATUS_DISCONNECTED)
    {
        scan_cnt = 25;
    }
    else
    {
        scan_cnt = 1;
    }
    aiio_log_d("scan cnt:%d", scan_cnt);

    //set scan params
    if (scan_params->scan_mode == AIIO_SCAN_PASSIVE)
    {
        scan_cfg.scan_type = WIFI_SCAN_TYPE_PASSIVE;
    }
    else
    {
        scan_cfg.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    }

    if (scan_params->channel_num == 1)   //scan a channel
    {
        scan_cfg.channel = scan_params->channels[0];
    }
    else                                //scan all channels
    {
        scan_cfg.channel = 0;
    }
    scan_cfg.scan_time = 50;    //FIXME扫描次数暂定50，与AT指令保持一致
    aiio_log_d("scan_cfg.channel:%d", scan_cfg.channel);
    aiio_log_d("scan_cfg.scan_type:%d", scan_cfg.scan_type);
    aiio_log_d("scan_cfg.scan_time:%d", scan_cfg.scan_time);

    ret = aiio_os_semaphore_create_counting(&sem_scan, 1, 0);
    if (ret != AIIO_OK)
    {
        aiio_log_e("semaphore create failed!");
        goto __exit;
    }

    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_SCAN_COMPLETE, aiio_wifi_sta_scan_complete_cb);

    for (; scan_cnt > 0; scan_cnt--)
    {
        wifi_sta_scan(&scan_cfg);
        aiio_os_semaphore_get(sem_scan, aiio_os_ms2tick(1500));
    }

    //get scan info
    if (cb != NULL)
    {
        cb((void *)data, NULL);
    }

    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_SCAN_COMPLETE, NULL);
    aiio_os_semaphore_delete(sem_scan);
    sem_scan = NULL;
    wifi_manager_cleanup_scan_results();

__exit:
    return ret;
}

// env、param1 are cb input params
aiio_err_t aiio_wifi_scan_ap_all(aiio_wifi_ap_item_t *env, uint32_t *param1, aiio_scan_item_cb_t cb)
{
    aiio_err_t ret = AIIO_OK;

    ln_list_t *list;
    uint8_t node_count = 0;
    ap_info_node_t *pnode;
    aiio_wifi_ap_item_t item;

    wifi_manager_ap_list_update_enable(LN_FALSE);
    wifi_manager_get_ap_list(&list, &node_count);

    aiio_log_d("aiio_wifi_scan_ap_all run");

    LN_LIST_FOR_EACH_ENTRY(pnode, ap_info_node_t, list, list)
    {
        uint8_t * mac = (uint8_t*)pnode->info.bssid;
        ap_info_t *info = &pnode->info;

        memset(&item, 0, sizeof(aiio_wifi_ap_item_t));

        memcpy(item.bssid, mac, 6);
        memcpy(item.ssid, info->ssid, sizeof(item.ssid));
        item.auth = info->authmode;
        item.channel = info->channel;
        item.rssi = info->rssi;
        item.ssid_len = strlen(item.ssid);

        // aiio_log_d("````````````````````````````````````````````````````````````````````");
        // aiio_log_d("item.bssid:%02X:%02X:%02X:%02X:%02X:%02X", item.bssid[0], item.bssid[1], item.bssid[2], item.bssid[3], item.bssid[4], item.bssid[5]);
        // aiio_log_d("item.ssid:%s", item.ssid);
        // aiio_log_d("item.ssid_len:%d", item.ssid_len);
        // aiio_log_d("item.auth:%d", item.auth);
        // aiio_log_d("item.channel:%d", item.channel);
        // aiio_log_d("item.rssi:%d", item.rssi);

        if (cb != NULL)
        {
            cb(env, param1, &item);
        }
    }

    wifi_manager_ap_list_update_enable(LN_TRUE);

__exit:
    return ret;
}

aiio_err_t aiio_wifi_sta_connect_ind_stat_get(aiio_wifi_sta_connect_ind_stat_info_t *wifi_ind_stat)
{
    uint8_t pwd_len; 
    const char    *ssid  = NULL;
    const uint8_t *passphr  = NULL;
    const uint8_t *bssid = NULL;
    const uint8_t *psk = NULL;

    if(WIFI_ERR_NONE != wifi_get_sta_conn_info(&ssid, &bssid)||  \
        WIFI_ERR_NONE != wifi_get_psk_info(&ssid, &passphr,&pwd_len,&psk))
    {
        aiio_log_e("aiio_wifi_sta_connect_ind_stat_get fail");
        return AIIO_ERROR;
    }
    wifi_get_channel(wifi_ind_stat->chan_id);

    // aiio_log_d("chan = %d,pwd_len = %d,ssid=%s,pwd=%s,psk=%x,%02x:%02x:%02x:%02x:%02x:%02x", 
    //             wifi_ind_stat->chan_id,pwd_len,
    //             ssid,passphr,psk,
    //             bssid[0], bssid[1], bssid[2], 
    //             bssid[3], bssid[4], bssid[5]);

    memcpy(wifi_ind_stat->ssid,ssid,SSID_LEN);
    memcpy(wifi_ind_stat->bssid,bssid,MAC_LEN);
    memcpy(wifi_ind_stat->passphr,passphr,PASSWORD_LEN);
    memcpy(wifi_ind_stat->psk,psk,64);

    return AIIO_OK;
}

aiio_err_t aiio_wifi_ap_sta_info_get(aiio_wifi_sta_basic_info_t *sta_info_p,uint8_t idx)
{
    
    sta_info_p->is_used=aiio_sta_info_temp[idx].is_used;
    sta_info_p->sta_idx=aiio_sta_info_temp[idx].sta_idx;
    sta_info_p->sta_mac[0]=aiio_sta_info_temp[idx].sta_mac[0];
    sta_info_p->sta_mac[1]=aiio_sta_info_temp[idx].sta_mac[1];
    sta_info_p->sta_mac[2]=aiio_sta_info_temp[idx].sta_mac[2];
    sta_info_p->sta_mac[3]=aiio_sta_info_temp[idx].sta_mac[3];
    sta_info_p->sta_mac[4]=aiio_sta_info_temp[idx].sta_mac[4];
    sta_info_p->sta_mac[5]=aiio_sta_info_temp[idx].sta_mac[5];
    
    // aiio_log_i("%02x:%02x:%02x:%02x:%02x:%02x",              
    //             sta_info_p->sta_mac[0], sta_info_p->sta_mac[1], sta_info_p->sta_mac[2], 
    //             sta_info_p->sta_mac[3], sta_info_p->sta_mac[4], sta_info_p->sta_mac[5]);

    return AIIO_OK;
}


aiio_err_t aiio_wifi_ap_sta_list_get(uint8_t *sta_num,aiio_wifi_sta_basic_info_t *sta_info_p)
{
    uint8_t sta_cnt = 0;
    uint8_t i=0;
    
    uint8_t counter =0;

    if (g_wifi_mode != AIIO_WIFI_MODE_AP){
        aiio_log_e("wifi AP is not enabled");
        return AIIO_ERROR;
    }
    for(i=0;i<3;i++){
        if(!aiio_sta_info_temp[i].is_used){
            continue;
        }
        (sta_info_p+counter)->sta_idx=aiio_sta_info_temp[i].sta_idx;
        (sta_info_p+counter)->sta_mac[0]=aiio_sta_info_temp[i].sta_mac[0];
        (sta_info_p+counter)->sta_mac[1]=aiio_sta_info_temp[i].sta_mac[1];
        (sta_info_p+counter)->sta_mac[2]=aiio_sta_info_temp[i].sta_mac[2];
        (sta_info_p+counter)->sta_mac[3]=aiio_sta_info_temp[i].sta_mac[3];
        (sta_info_p+counter)->sta_mac[4]=aiio_sta_info_temp[i].sta_mac[4];
        (sta_info_p+counter)->sta_mac[5]=aiio_sta_info_temp[i].sta_mac[5];
        counter++;
    }
    *sta_num=counter;
    return AIIO_OK;
}

aiio_wifi_beacon_auth_t aiio_wifi_get_sta_security(void)
{
    return AIIO_WIFI_EVENT_BEACON_IND_AUTH_UNKNOWN;
}
