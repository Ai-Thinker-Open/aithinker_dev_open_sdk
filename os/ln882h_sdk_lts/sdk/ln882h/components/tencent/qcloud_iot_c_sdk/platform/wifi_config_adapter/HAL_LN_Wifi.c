#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "qcloud_wifi_config.h"

#include "wifi.h"
#include "ln_wifi_err.h"
#include "wifi_manager.h"
#include "utils/system_parameter.h"
#include "utils/sysparam_factory_setting.h"
#include "utils/ln_misc.h"
#include "utils/ln_psk_calc.h"
#include "netif/ethernetif.h"
#include "lwip/dhcp.h"


////////////////////////////////////////////////////////////////////////////////
///////////////////////////////  LN PORTING APIs  //////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static uint8_t sg_mac_addr[6] = {0x00, 0x50, 0xC2, 0x5E, 0x88, 0x99};
static uint8_t psk_value[40]  = {0x0};

static wifi_sta_connect_t sg_connect  = {0};
static wifi_scan_cfg_t    sg_scan_cfg = {0};

typedef struct {
    char ssid[SSID_MAX_LEN];
    char pwd[PASSWORD_MAX_LEN];
} wifi_connected_param_t;

static wifi_connected_param_t sg_connected_param = {0};


static void wifi_scan_complete_cb(void * arg)
{
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

int port_wifi_sta_start_connect(const char *ssid, const char *pwd, uint8_t channel)
{
    if ( (NULL == ssid) || (NULL == pwd) ) {
        return -1;
    }
    
    Log_e("[%s] <ssid:%s> <pwd:%s>\r\n", __func__, ssid, pwd);

    //1. sta mac get
    if (SYSPARAM_ERR_NONE != sysparam_sta_mac_get(sg_mac_addr)) {
        Log_e("[%s]sta mac get filed!!!\r\n", __func__);
        return -1;
    }

    if (sg_mac_addr[0] == STA_MAC_ADDR0 &&
        sg_mac_addr[1] == STA_MAC_ADDR1 &&
        sg_mac_addr[2] == STA_MAC_ADDR2 &&
        sg_mac_addr[3] == STA_MAC_ADDR3 &&
        sg_mac_addr[4] == STA_MAC_ADDR4 &&
        sg_mac_addr[5] == STA_MAC_ADDR5) {
        ln_generate_random_mac(sg_mac_addr);
        sysparam_sta_mac_update((const uint8_t *)sg_mac_addr);
    }

    //2. net device(lwip)
    netdev_set_mac_addr(NETIF_IDX_STA, sg_mac_addr);
    netdev_set_active(NETIF_IDX_STA);
    sysparam_sta_mac_update((const uint8_t *)sg_mac_addr);

    //3. wifi start
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_SCAN_COMPLETE, &wifi_scan_complete_cb);

    if(WIFI_ERR_NONE != wifi_sta_start(sg_mac_addr, WIFI_NO_POWERSAVE)){
        Log_e("[%s]wifi sta start filed!!!\r\n", __func__);
        return -1;
    }

    sg_connect.ssid      = (char *)ssid;
    sg_connect.pwd       = (char *)pwd;
    sg_connect.bssid     = NULL;
    sg_connect.psk_value = NULL;
    
    sg_scan_cfg.channel   = 0; //chan
    sg_scan_cfg.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    sg_scan_cfg.scan_time = 20;

    if (strlen(sg_connect.pwd) != 0) {
        if (0 == ln_psk_calc(sg_connect.ssid, sg_connect.pwd, psk_value, sizeof (psk_value))) {
            sg_connect.psk_value = psk_value;
            hexdump(LOG_LVL_INFO, "psk value ", psk_value, sizeof(psk_value));
        }
    }

    if ( 0 != wifi_sta_connect(&sg_connect, &sg_scan_cfg)) {
        return -1;
    }

    return 0;
}

bool port_wifi_is_sta_got_ip(void)
{
    tcpip_ip_info_t ip_info;
    netif_idx_t netif_id = netdev_get_active();

    if (NETIF_IDX_STA != netif_id) {
        return false;
    }

    if ((NETDEV_LINK_UP == netdev_get_link_state(netif_id)) && \
         dhcp_supplied_address(netdev_get_netif(netif_id))) {
        return true;
    }

    return false;
}

int port_wifi_set_connected_param(char *ssid, uint8_t ssid_len, char *pwd, uint8_t pwd_len)
{
    if ((NULL != ssid) && (ssid_len < SSID_MAX_LEN) && (NULL != pwd) && (pwd_len < PASSWORD_MAX_LEN)) {
        memset(&sg_connected_param, 0, sizeof(wifi_connected_param_t));
        
        memcpy(sg_connected_param.ssid, ssid, ssid_len);
        memcpy(sg_connected_param.pwd,  pwd, pwd_len);
    } else {
        return -1;
    }

    return 0;
}

char *port_wifi_get_connected_ap_ssid(void)
{
    return sg_connected_param.ssid;
}

char *port_wifi_get_connected_ap_pwd(void)
{
    return sg_connected_param.pwd;
}

