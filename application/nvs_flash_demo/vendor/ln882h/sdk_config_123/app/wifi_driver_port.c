#include "wifi_driver_port.h"
#include "usr_ctrl.h"

#include "transparent.h"
#include "ln_at_cmd_base_plus.h"

#define SOFTAPCFG_TASK_STACK_SIZE   (3 * 512)

static int ln_softapcfg_start(void);
static void ln_cfg_softap(const char* _ssid, const char* _pwd);
static void ln_wifi_stop(void);
static void ln_reset_wifi_connect_info(void);
static void ln_wifi_init_sta(void);
static void ln_connect_to_ap(void);
static void ln_sta_reset_ip_sem(void);
static void ln_sta_release_ip_sem(void);
static int ln_sta_wait_for_ip(int _ms);
static void wifi_scan_complete_cb(void * arg);
static char* ln_get_sta_ssid_buf(void);
static char* ln_get_sta_pwd_buf(void);
static uint8_t* ln_get_sta_bssid_buf(void);

static uint8_t      psk_value[40]      = {0x0};
static uint8_t      mac_addr[6]        = {0x00, 0x50, 0xC2, 0x5E, 0x88, 0x99};
static OS_Semaphore_t s_ipGot;

static wifi_scan_cfg_t s_scanCfg = {
        .channel   = 0,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = 20,
};

void    ln_at_printf(const char *format, ...);

static LN_WIFI_DRIVER_ADAPTER s_lnWifiDrvAdapter = {

    .reConn = true,

    .init = ln_reset_wifi_connect_info,

    .stop = ln_wifi_stop,

    .init_sta = ln_wifi_init_sta,
    .sta_connect = ln_connect_to_ap,
    .reset_ip_sem = ln_sta_reset_ip_sem,
    .release_ip_sem = ln_sta_release_ip_sem,
    .wait_for_ip = ln_sta_wait_for_ip,

    .start_softap = ln_softapcfg_start,
    .cfg_softap = ln_cfg_softap,

    .ssid_buf = ln_get_sta_ssid_buf,
    .pwd_buf = ln_get_sta_pwd_buf,
    .bssid_buf = ln_get_sta_bssid_buf,
};

LN_WIFI_DRIVER_ADAPTER* ln_get_wifi_drv_handle()
{
    return &s_lnWifiDrvAdapter;
}

static void ln_reset_wifi_connect_info()
{
    LN_WIFI_DRIVER_ADAPTER *pWifi = ln_get_wifi_drv_handle();

    memset(&pWifi->sta, 0, sizeof(LN_WIFI_STA_CFG));
    memset(&pWifi->sap, 0, sizeof(LN_WIFI_STA_CFG));

    pWifi->conn.ssid = pWifi->sta.ssid;
    pWifi->conn.pwd = pWifi->sta.pwd;
    pWifi->conn.bssid = pWifi->sta.bssid;

    if (OS_SemaphoreCreateBinary(&s_ipGot) != OS_OK)
    {
        Log_e("OS_SemaphoreCreate sem fail.");
        while(1);
    }
}

static char* ln_get_sta_ssid_buf()
{
    return ln_get_wifi_drv_handle()->sta.ssid;
}

static char* ln_get_sta_pwd_buf()
{
    return ln_get_wifi_drv_handle()->sta.pwd;
}

static uint8_t* ln_get_sta_bssid_buf()
{
    return ln_get_wifi_drv_handle()->sta.bssid;
}

static void ln_wifi_connected_cb(void * arg)
{
    Log_i("connect ap success!!");
    ln_at_printf("WIFI CONNECTED\r\n");
}

static void ln_wifi_disconnected_cb(void * arg)
{
    Log_i("connect ap success!!");
    ln_at_printf("WIFI DISCONNECT\r\n");
}

static void ln_wifi_get_ip_cb(struct netif *nif)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    Log_i("IP get success!!");
    p->cStaStat = STA_GOT_IP;
    ln_at_printf("WIFI GOT IP\r\n");
    ln_get_wifi_drv_handle()->release_ip_sem();
}

static void ln_wifi_init_sta(void)
{
    sta_ps_mode_t ps_mode = PM_WIFI_DEFAULT_PS_MODE;

    //1. sta mac get
    if (SYSPARAM_ERR_NONE != sysparam_sta_mac_get(mac_addr)) {
        Log_e("sta mac get filed!!!");
        return;
    }
    if (mac_addr[0] == STA_MAC_ADDR0 &&
        mac_addr[1] == STA_MAC_ADDR1 &&
        mac_addr[2] == STA_MAC_ADDR2 &&
        mac_addr[3] == STA_MAC_ADDR3 &&
        mac_addr[4] == STA_MAC_ADDR4 &&
        mac_addr[5] == STA_MAC_ADDR5) {
        ln_generate_random_mac(mac_addr);
        sysparam_sta_mac_update((const uint8_t *)mac_addr);
    }

    //2. stop wifi
    wifi_sta_status_t staStat;
    wifi_get_sta_status(&staStat);
    Log_i("sta: %d", staStat);
    if (staStat != WIFI_STA_STATUS_SCANING && staStat != WIFI_STA_STATUS_DISCONNECTED)
    {
        wifi_sta_disconnect();
    }

    wifi_stop();
    netdev_set_state(NETIF_IDX_STA, NETDEV_DOWN);

    //3. net device(lwip)
    netdev_set_mac_addr(NETIF_IDX_STA, mac_addr);
    netdev_set_active(NETIF_IDX_STA);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_CONNECTED, &ln_wifi_connected_cb);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_DISCONNECTED, &ln_wifi_disconnected_cb);
    netdev_get_ip_cb_set(ln_wifi_get_ip_cb);

    //4. wifi start
    if (SYSPARAM_ERR_NONE != sysparam_sta_powersave_get(&ps_mode))
    {
        Log_e("sysparam_sta_powersave_get filed!!!");
    }

    if(WIFI_ERR_NONE != wifi_sta_start(mac_addr, ps_mode)){
        Log_e("wifi sta start filed!!!");
    }
}

static void ln_set_scan_cfg(uint8_t *_cfg)
{
    //cfg 0:show_hidden, 1:scan_type(0:active,1:passive,2:mixed), 2:scan_time(s)
    s_scanCfg.scan_type = (wifi_scan_type_t)_cfg[1];
    s_scanCfg.scan_time = _cfg[2] * 1000 / 13;
}

static void ln_connect_to_ap()
{
    LN_WIFI_DRIVER_ADAPTER* pWifi = ln_get_wifi_drv_handle();
    wifi_sta_connect_t staCfg = {0};
    //uint8_t bssid_hex[BSSID_LEN];

    ln_wifi_init_sta();
    pWifi->reset_ip_sem();

    staCfg.ssid = pWifi->sta.ssid;
    staCfg.pwd = pWifi->sta.pwd;
    if (pWifi->sta.bssid[0] == 0 &&
        pWifi->sta.bssid[1] == 0 &&
        pWifi->sta.bssid[2] == 0 &&
        pWifi->sta.bssid[3] == 0 &&
        pWifi->sta.bssid[4] == 0 &&
        pWifi->sta.bssid[5] == 0)
    {
        staCfg.bssid = NULL;
    }
    else
    {
        staCfg.bssid = pWifi->sta.bssid;
    }

    Log_d("ssid: %s", staCfg.ssid);
    Log_d("pwd: %s", staCfg.pwd);

    if (SYSPARAM_ERR_NONE != sysparam_sta_scan_cfg_get(&s_scanCfg))
    {
        Log_e("sysparam_sta_scan_cfg_get filed!!!");
    }

    staCfg.psk_value = NULL;
    if (strlen(staCfg.pwd) != 0) {
        if (0 == ln_psk_calc(staCfg.ssid, staCfg.pwd, psk_value, sizeof (psk_value))) {
            staCfg.psk_value = psk_value;
            hexdump(LOG_LVL_INFO, "psk value ", psk_value, sizeof(psk_value));
        }
    }

    if ( WIFI_ERR_NONE !=  wifi_sta_connect(&staCfg, &s_scanCfg))
    {
        Log_e("Join failed! ssid:%s, pwd:%s", staCfg.ssid, staCfg.pwd);
    }
}

static void ln_sta_release_ip_sem(void)
{
    OS_SemaphoreRelease(&s_ipGot);
}

static int ln_sta_wait_for_ip(int _ms)
{
    if (OS_OK == OS_SemaphoreWait(&s_ipGot, _ms))
    {
        return LN_OK;
    }

    return LN_FAIL;
}

static void ln_sta_reset_ip_sem(void)
{
    ln_sta_wait_for_ip(0);
}

static void wifi_scan_complete_cb(void * arg)
{
    LN_UNUSED(arg);

    ln_list_t *list;
    uint8_t node_count = 0;
    ap_info_node_t *pnode;

    wifi_manager_ap_list_update_enable(LN_FALSE);

    // 1.get ap info list.
    wifi_manager_get_ap_list(&list, &node_count);

    // 2.print all ap info in the list.
    LN_LIST_FOR_EACH_ENTRY(pnode, ap_info_node_t, list,list)
    {
        uint8_t * mac = (uint8_t*)pnode->info.bssid;
        ap_info_t *ap_info = &pnode->info;

        LOG(LOG_LVL_INFO, "\tCH=%2d,RSSI= %3d,", ap_info->channel, ap_info->rssi);
        LOG(LOG_LVL_INFO, "BSSID:[%02X:%02X:%02X:%02X:%02X:%02X],SSID:\"%s\"\r\n", \
                           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ap_info->ssid);
    }

    wifi_manager_ap_list_update_enable(LN_TRUE);
}

static void ln_ap_startup_cb(void * arg)
{
    netdev_set_state(NETIF_IDX_AP, NETDEV_UP);
}

static int ln_wifi_softap_startup(const char * ssid, const char * pwd, const char * local_ip)
{
    //1. macaddr
    char apssid[64 + 1] = {0};
    char appwd[64 + 1] = {0};
    uint8_t macaddr[6] = {0};
    ln_generate_random_mac(macaddr);

    //2. ap cfg
    wifi_softap_cfg_t ap_cfg = {
        .ssid            = apssid,
        .pwd             = appwd,
        .bssid           = macaddr,
        .ext_cfg = {
            .channel         = 7,
            .authmode        = WIFI_AUTH_WPA_WPA2_PSK,//WIFI_AUTH_OPEN,
            .ssid_hidden     = 0,
            .beacon_interval = 100,
            .psk_value = NULL,
        }
    };

    if ((ssid == NULL) || ((ssid != NULL) && (strlen(ssid) > WLAN_SSID_MAX_LEN))) {
        Log_e("wifi softap ssid invalid!");
        return LN_FALSE;
    }
    strcpy((char *)ap_cfg.ssid, ssid);

    if (pwd == NULL) {
        ap_cfg.ext_cfg.authmode = WIFI_AUTH_OPEN;
    }
    else {
        uint16_t pwdlen = strlen(pwd);
        if (pwdlen < 8) {
            Log_e("wifi softap pwd len < 8!");
            return LN_FALSE;
        }
        else if (pwdlen >= WLAN_PWD_MAX_LEN) {
            Log_e("wifi softap pwd len >= %d!", WLAN_PWD_MAX_LEN);
            return LN_FALSE;
        }
        else{
            strcpy((char *)ap_cfg.pwd, pwd);
            ap_cfg.ext_cfg.authmode = WIFI_AUTH_WPA2_PSK;
        }
    }

    //3. tcpip info
    tcpip_ip_info_t ip_info;
    ip_info.ip.addr = ipaddr_addr(local_ip);
    ip_info.gw.addr = ipaddr_addr(local_ip);
    ip_info.netmask.addr = ipaddr_addr((const char *)"255.255.255.0");

    //4. dhcp server
    server_config_t  server_config;
    server_config.server.addr   = ip_info.ip.addr;
    server_config.port          = 67;
    server_config.lease         = 2880;
    server_config.renew         = 2880;
    server_config.ip_start.addr = ipaddr_addr((const char *)"192.168.4.100");
    server_config.ip_end.addr   = ipaddr_addr((const char *)"192.168.4.150");
    server_config.client_max    = 3;
    dhcpd_curr_config_set(&server_config);

    //1. net device(lwip).
    uint8_t psk_value[40]      = {0x0};
    netdev_set_mac_addr(NETIF_IDX_AP, macaddr);
    netdev_set_ip_info(NETIF_IDX_AP, &ip_info);
    netdev_set_active(NETIF_IDX_AP);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_STARTUP, &ln_ap_startup_cb);

    sysparam_softap_mac_update((const uint8_t *)macaddr);

    ap_cfg.ext_cfg.psk_value = NULL;
    if ((strlen(ap_cfg.pwd) != 0) &&
        (ap_cfg.ext_cfg.authmode != WIFI_AUTH_OPEN) &&
        (ap_cfg.ext_cfg.authmode != WIFI_AUTH_WEP)) {
        memset(psk_value, 0, sizeof(psk_value));
        if (0 == ln_psk_calc(ap_cfg.ssid, ap_cfg.pwd, psk_value, sizeof (psk_value))) {
            ap_cfg.ext_cfg.psk_value = psk_value;
            hexdump(LOG_LVL_INFO, "psk value ", psk_value, sizeof(psk_value));
        }
    }

    //2. wifi
    if(WIFI_ERR_NONE !=  wifi_softap_start(&ap_cfg)){
        Log_e("wifi softap start failed!");
        return LN_FALSE;
    }

    return LN_TRUE;
}

static int ln_softapcfg_start(void)
{
    LN_WIFI_DRIVER_ADAPTER* pWifi = ln_get_wifi_drv_handle();
    Log_i("[APCFG] ssid:[%s],pwd:[%s],local ip:[%s]",pWifi->sap.ssid, pWifi->sap.pwd, LOCAL_IP);

    if (LN_TRUE != ln_wifi_softap_startup(pWifi->sap.ssid, pWifi->sap.pwd, LOCAL_IP)) {
        return LN_FALSE;
    }

    OS_MsDelay(500);//wait for ap start

    return LN_TRUE;
}

static void ln_cfg_softap(const char* _ssid, const char* _pwd)
{
    strcpy(ln_get_wifi_drv_handle()->sap.ssid, _ssid);
    strcpy(ln_get_wifi_drv_handle()->sap.pwd, _pwd);
}

static void ln_wifi_stop(void)
{
    OS_MsDelay(100);
    wifi_stop();
}
