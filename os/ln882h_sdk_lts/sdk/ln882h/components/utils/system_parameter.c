#include "ln_compiler.h"
#include "ln_kv_api.h"
#include "sysparam_factory_setting.h"
#include "utils/system_parameter.h"

typedef struct {
    char  ssid[SSID_MAX_LEN]; 
    char  pwd[PASSWORD_MAX_LEN];  
    char  bssid[BSSID_LEN];
} sta_conn_param_t; 

typedef struct {
    char  ssid[SSID_MAX_LEN];
    char  pwd[PASSWORD_MAX_LEN];
} softap_basic_param_t; 

__STATIC_INLINE__ int sysparam_store(const char *key, const void *value, size_t value_len)
{
    int ret = ln_kv_set(key, value, value_len);
    if (ret != KV_ERR_NONE) {
        return SYSPARAM_ERR_STORE;
    }
    return SYSPARAM_ERR_NONE;
}

__STATIC_INLINE__ int sysparam_load(const char *key, void *value_buf, size_t value_buf_size)
{
    size_t v_len = 0;
    int ret = ln_kv_get(key, value_buf, value_buf_size, &v_len);
    if (ret != KV_ERR_NONE) {
        return SYSPARAM_ERR_LOAD;
    } 
    return SYSPARAM_ERR_NONE;
}

__STATIC_INLINE__ int sysparam_has_stored(const char *key)
{
    return ln_kv_has_key(key);
}


////////////////////////////////////// factory_reset ///////////////////////////////////////////////////////////////
// sta scan/connect param
int sysparam_sta_scan_cfg_factory_reset(void)
{
    wifi_scan_cfg_t scan_cfg;
    scan_cfg.channel   = STA_SCAN_CHANNEL;
    scan_cfg.scan_time = STA_SCAN_TIME;
    scan_cfg.scan_type = (wifi_scan_type_t)STA_SCAN_TYPE;

    return sysparam_store(KV_SYSPARAM_STA_SCAN_CFG, (void *)&scan_cfg, sizeof(wifi_scan_cfg_t));
}

int sysparam_sta_conn_cfg_factory_reset(void)
{
    sta_conn_param_t conn;
    memset(&conn, 0, sizeof(sta_conn_param_t));
    memcpy(&conn.ssid, TARGET_AP_SSID, strlen(TARGET_AP_SSID));
    memcpy(&conn.pwd, TARGET_AP_PWD, strlen(TARGET_AP_PWD));
    
    return sysparam_store(KV_SYSPARAM_STA_CONN_CFG, (void *)&conn, sizeof(sta_conn_param_t));
}

// softap cfg param
int sysparam_softap_ssidpwd_cfg_factory_reset(void)
{
    softap_basic_param_t basic_param;
    memset(&basic_param, 0, sizeof(softap_basic_param_t));
    memcpy(&basic_param.ssid, SOFTAP_SSID, strlen(SOFTAP_SSID));
    memcpy(&basic_param.pwd, SOFTAP_PWD, strlen(SOFTAP_PWD));

    return sysparam_store(KV_SYSPARAM_SOFTAP_BASIC_CFG, (void *)&basic_param, sizeof(softap_basic_param_t));
}

int sysparam_softap_ext_cfg_factory_reset(void)
{
    wifi_softap_ext_cfg_t ext_cfg;
    ext_cfg.channel         = SOFTAP_CHANNEL;
    ext_cfg.authmode        = (wifi_auth_mode_t)SOFTAP_AUTH_MODE;
    ext_cfg.ssid_hidden     = SOFTAP_SSID_HIDDEN;
    ext_cfg.beacon_interval = SOFTAP_BEACON_INTERVAL;

    return sysparam_store(KV_SYSPARAM_SOFTAP_EXT_CFG, (void *)&ext_cfg, sizeof(wifi_softap_ext_cfg_t));
}

//country code
int sysparam_country_code_factory_reset(void)
{
    wifi_country_code_t country_code = (wifi_country_code_t)SOFTAP_COUNTRY_CODE;
    return sysparam_store(KV_SYSPARAM_COUNTRY_CODE, (void *)&country_code, sizeof(wifi_country_code_t));
}

// sta powersave
int sysparam_sta_powersave_factory_reset(void)
{
    sta_ps_mode_t powersave_mode = (sta_ps_mode_t)STA_POWERSAVE_MODE;
    return sysparam_store(KV_SYSPARAM_STA_PS_MODE, (void *)&powersave_mode, sizeof(sta_ps_mode_t));
}

// mac param
int sysparam_sta_mac_factory_reset(void)
{
    char  macaddr[MAC_ADDRESS_LEN] = {0};
    macaddr[0] = STA_MAC_ADDR0;
    macaddr[1] = STA_MAC_ADDR1;
    macaddr[2] = STA_MAC_ADDR2;
    macaddr[3] = STA_MAC_ADDR3;
    macaddr[4] = STA_MAC_ADDR4;
    macaddr[5] = STA_MAC_ADDR5;
    return sysparam_store(KV_SYSPARAM_STA_MAC, (void *)&macaddr, sizeof(macaddr));
}

int sysparam_softap_mac_factory_reset(void)
{
    char  macaddr[MAC_ADDRESS_LEN] = {0};
    macaddr[0] = SOFTAP_MAC_ADDR0;
    macaddr[1] = SOFTAP_MAC_ADDR1;
    macaddr[2] = SOFTAP_MAC_ADDR2;
    macaddr[3] = SOFTAP_MAC_ADDR3;
    macaddr[4] = SOFTAP_MAC_ADDR4;
    macaddr[5] = SOFTAP_MAC_ADDR5;
    return sysparam_store(KV_SYSPARAM_SOFTAP_MAC, (void *)&macaddr, sizeof(macaddr));
}

// ip info param
int sysparam_sta_ip_info_factory_reset(void)
{
    tcpip_ip_info_t sta_ip_info;
    if (ip4addr_aton(STA_IP_ADDR,    &sta_ip_info.ip) && \
        ip4addr_aton(STA_IP_NETMASK, &sta_ip_info.netmask) && \
        ip4addr_aton(STA_IP_GATWAY,  &sta_ip_info.gw)) 
    {
        return sysparam_store(KV_SYSPARAM_STA_IP_INFO, (void *)&sta_ip_info, sizeof(tcpip_ip_info_t));
    }
    return SYSPARAM_ERR_INVALID_PARAM;
}

int sysparam_softap_ip_info_factory_reset(void)
{
    tcpip_ip_info_t softap_ip_info;
    if (ip4addr_aton(SOFTAP_IP_ADDR,    &softap_ip_info.ip) && \
        ip4addr_aton(SOFTAP_IP_NETMASK, &softap_ip_info.netmask) && \
        ip4addr_aton(SOFTAP_IP_GATWAY,  &softap_ip_info.gw)) 
    {
        return sysparam_store(KV_SYSPARAM_SOFTAP_IP_INFO, (void *)&softap_ip_info, sizeof(tcpip_ip_info_t));
    }
    return SYSPARAM_ERR_INVALID_PARAM;
}

// netdev hostname
int sysparam_sta_hostname_factory_reset(void)
{
    uint16_t len = strlen(STA_NETDEV_HOSTNAME) + 1;
    len = (len > NETDEV_HOSTNAME_LEN_MAX) ? NETDEV_HOSTNAME_LEN_MAX : len;

    return sysparam_store(KV_SYSPARAM_STA_HOSTNAME, (void *)STA_NETDEV_HOSTNAME, len);
}

int sysparam_softap_hostname_factory_reset(void)
{
    uint16_t len = strlen(SOFTAP_NETDEV_HOSTNAME) + 1;
    len = (len > NETDEV_HOSTNAME_LEN_MAX) ? NETDEV_HOSTNAME_LEN_MAX : len;
    
    return sysparam_store(KV_SYSPARAM_SOFTAP_HOSTNAME, (void *)SOFTAP_NETDEV_HOSTNAME, len);
}

// wifi mode after poweron
int sysparam_poweron_wifi_mode_factory_reset(void)
{
    wifi_mode_t wifi_mode = (wifi_mode_t)WIFI_MODE_DEFAULT;
    return sysparam_store(KV_SYSPARAM_WIFI_MODE, (void *)&wifi_mode, sizeof(wifi_mode_t));
}

// auto connect after poweron
int sysparam_poweron_auto_conn_factory_reset(void)
{
    uint8_t auto_conn = STA_POWERON_AUTO_CONN;
    return sysparam_store(KV_SYSPARAM_AUTO_CONNECT, (void *)&auto_conn, sizeof(uint8_t));
}

// ap list info mask
int sysparam_ap_list_info_mask_factory_reset(void)
{
    uint16_t ap_list_info_mask = AP_LIST_INFO_MASK;
    return sysparam_store(KV_SYSPARAM_AP_LIST_MASK, (void *)&ap_list_info_mask, sizeof(uint16_t));
}

// dhcp client/server
int sysparam_dhcp_en_factory_reset(void)
{
    uint8_t dhcp_en = DHCP_ENABLE;
    return sysparam_store(KV_SYSPARAM_DHCP_EN, (void *)&dhcp_en, sizeof(uint8_t));
}

int sysparam_dhcpd_en_factory_reset(void)
{
    uint8_t dhcpd_en = DHCPD_ENABLE;
    return sysparam_store(KV_SYSPARAM_DHCPD_EN, (void *)&dhcpd_en, sizeof(uint8_t));
}

int sysparam_dhcpd_cfg_factory_reset(void)
{
    server_config_t dhcpd_cfg;
    dhcpd_cfg.lease = DHCPD_IP_LEASE_TIME;
    dhcpd_cfg.renew = DHCPD_IP_RENEW_TIME;
    dhcpd_cfg.port  = DHCPD_LISTEN_PORT;
    dhcpd_cfg.client_max  = DHCPD_CLIENT_MAX;

    if (ip4addr_aton(DHCPD_SER_IP,        &dhcpd_cfg.server) && \
        ip4addr_aton(DHCPD_IP_POOL_START, &dhcpd_cfg.ip_start) && \
        ip4addr_aton(DHCPD_IP_POOL_END,   &dhcpd_cfg.ip_end)) 
    {
        return sysparam_store(KV_SYSPARAM_DHCPD_CFG, (void *)&dhcpd_cfg, sizeof(server_config_t));
    }
    return SYSPARAM_ERR_INVALID_PARAM;
}

int sysparam_sys_store_factory_reset(void)
{
    uint8_t sysstore = SYSSTORE_DEFAULT;
    return sysparam_store(KV_SYS_STORE_MODE, (void *)&sysstore, sizeof(uint8_t));
}

int sysparam_sys_uart_factory_reset(void)
{
    sys_uart_status_t uart_cfg;

    uart_cfg.baud = SYS_UART_BAUD;
    uart_cfg.databit = SYS_UART_DATA_BITS;
    uart_cfg.flow = SYS_UART_FLOWCONTROL;
    uart_cfg.parity = SYS_UART_PARITY;
    uart_cfg.stopbit = SYS_UART_STOP_BITS;

    return sysparam_store(KV_SYS_UART_CFG, (void *)&uart_cfg, sizeof(sys_uart_status_t));
}

int sysparam_cipsta_dhcp_cur_factory_reset(void)
{
    uint8_t cipsta_dhcp = 1;
    return sysparam_store(KV_CIPSTA_DHCP_CUR_STAT, (void *)&cipsta_dhcp, sizeof(uint8_t));
}

int sysparam_tcpip_dns_factory_reset(void)
{
    dns_ip_t dns_ip;
    memset(&dns_ip, 0, sizeof(dns_ip_t));
    dns_ip.enable = DNS_ENABLE;
    memcpy(&dns_ip.dns_ip_1[0], DNS_SERVER1, strlen(DNS_SERVER1));
    memcpy(&dns_ip.dns_ip_2[0], DNS_SERVER2, strlen(DNS_SERVER2));
    memcpy(&dns_ip.dns_ip_3[0], DNS_SERVER3, strlen(DNS_SERVER3));
    return sysparam_store(KV_TCPIP_DNS_CFG, (void *)&dns_ip, sizeof(dns_ip_t));
}

int sysparam_sta_jap_cfg_factory_reset(void)
{
    wifi_sta_join_ap_cfg_t cfg;
    memset(&cfg, 0, sizeof(wifi_sta_join_ap_cfg_t));
    cfg.jap_timeout = STA_JOIN_AP_CFG_JAP_TIMEOUT;
    cfg.pci_en = STA_JOIN_AP_CFG_PCIEN;
    cfg.pmf = STA_JOIN_AP_CFG_PMF;
    cfg.reconn_interval = STA_JOIN_AP_CFG_RECONN_INTERVAL;
    cfg.scan_mode = STA_JOIN_AP_CFG_SCANMODE;
    cfg.listen_interval = STA_JOIN_AP_CFG_LISTEN_INTERVAL;
    return sysparam_store(KV_WIFI_STA_JAP_CFG, (void *)&cfg, sizeof(wifi_sta_join_ap_cfg_t));
}


#if 0
    // TODO: mDNS param
    int sysparam_mdns_en_factory_reset(void);
    int sysparam_mdns_cfg_factory_reset(void);

    // TODO: DNS param
    int sysparam_dns_en_factory_reset(void);
    int sysparam_dns_cfg_factory_reset(void);

    // TODO: SNTP server param
    int sysparam_sntp_en_factory_reset(void);
    int sysparam_sntp_cfg_factory_reset(void);
#endif

int sysparam_factory_reset_all(void)
{
    int ret = SYSPARAM_ERR_NONE;

    ret += sysparam_sta_scan_cfg_factory_reset();
    ret += sysparam_sta_conn_cfg_factory_reset();

    // softap cfg param
    ret += sysparam_softap_ssidpwd_cfg_factory_reset();
    ret += sysparam_softap_ext_cfg_factory_reset();

    //country code
    ret += sysparam_country_code_factory_reset();

    // sta powersave
    ret += sysparam_sta_powersave_factory_reset();

    // mac param
    ret += sysparam_sta_mac_factory_reset();
    ret += sysparam_softap_mac_factory_reset();

    // ip info param
    ret += sysparam_sta_ip_info_factory_reset();
    ret += sysparam_softap_ip_info_factory_reset();

    // netdev hostname
    ret += sysparam_sta_hostname_factory_reset();
    ret += sysparam_softap_hostname_factory_reset();

    // wifi mode after poweron
    ret += sysparam_poweron_wifi_mode_factory_reset();

    // auto connect after poweron
    ret += sysparam_poweron_auto_conn_factory_reset();

    // ap list info mask
    ret += sysparam_ap_list_info_mask_factory_reset();

    // dhcp client/server
    ret += sysparam_dhcp_en_factory_reset();
    ret += sysparam_dhcpd_en_factory_reset();
    ret += sysparam_dhcpd_cfg_factory_reset();

    //sysstore
    ret += sysparam_sys_store_factory_reset();

    //sys uart cfg
    ret += sysparam_sys_uart_factory_reset();

    //cipsta_cur
    ret += sysparam_cipsta_dhcp_cur_factory_reset();

    //dns cfg
    ret += sysparam_tcpip_dns_factory_reset();

    //wifi sta jap cfg
    ret += sysparam_sta_jap_cfg_factory_reset();
    
    if (ret != SYSPARAM_ERR_NONE) {
        return SYSPARAM_ERR_RESET_ALL;
    }
    return SYSPARAM_ERR_NONE;
}

int sysparam_integrity_check_all(void)
{
    int ret = SYSPARAM_ERR_NONE;

    if (!sysparam_has_stored(KV_SYSPARAM_STA_SCAN_CFG)) {
        ret += sysparam_sta_scan_cfg_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_STA_CONN_CFG)) {
        ret += sysparam_sta_conn_cfg_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_SOFTAP_BASIC_CFG)) {
        ret += sysparam_softap_ssidpwd_cfg_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_SOFTAP_EXT_CFG)) {
        ret += sysparam_softap_ext_cfg_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_COUNTRY_CODE)) {
        ret += sysparam_country_code_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_STA_PS_MODE)) {
        ret += sysparam_sta_powersave_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_STA_MAC)) {
        ret += sysparam_sta_mac_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_SOFTAP_MAC)) {
        ret += sysparam_softap_mac_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_STA_IP_INFO)) {
        ret += sysparam_sta_ip_info_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_SOFTAP_IP_INFO)) {
        ret += sysparam_softap_ip_info_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_STA_HOSTNAME)) {
        ret += sysparam_sta_hostname_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_SOFTAP_HOSTNAME)) {
        ret += sysparam_softap_hostname_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_WIFI_MODE)) {
        ret += sysparam_poweron_wifi_mode_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_AUTO_CONNECT)) {
        ret += sysparam_poweron_auto_conn_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_AP_LIST_MASK)) {
        ret += sysparam_ap_list_info_mask_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_DHCP_EN)) {
        ret += sysparam_dhcp_en_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_DHCPD_EN)) {
        ret += sysparam_dhcpd_en_factory_reset();
    }
    if (!sysparam_has_stored(KV_SYSPARAM_DHCPD_CFG)) {
        ret += sysparam_dhcpd_cfg_factory_reset();
    }
    if(!sysparam_has_stored(KV_SYS_STORE_MODE)){
        ret += sysparam_sys_store_factory_reset();
    }
    if(!sysparam_has_stored(KV_SYS_UART_CFG)){
        ret += sysparam_sys_uart_factory_reset();
    }
    if(!sysparam_has_stored(KV_CIPSTA_DHCP_CUR_STAT)){
        ret += sysparam_sys_uart_factory_reset();
    }
    if(!sysparam_has_stored(KV_TCPIP_DNS_CFG)){
        ret += sysparam_tcpip_dns_factory_reset();
    }
    if(!sysparam_has_stored(KV_WIFI_STA_JAP_CFG)){
        ret += sysparam_sta_jap_cfg_factory_reset();
    }

#if 0 //TODO:
    sysparam_has_stored(KV_SYSPARAM_MDNS_EN );
    sysparam_has_stored(KV_SYSPARAM_MDNS_CFG);
    sysparam_has_stored(KV_SYSPARAM_DNS_EN  );
    sysparam_has_stored(KV_SYSPARAM_DNS_CFG );
    sysparam_has_stored(KV_SYSPARAM_SNPT_EN );
    sysparam_has_stored(KV_SYSPARAM_SNPT_CFG);
#endif

    if (ret != SYSPARAM_ERR_NONE) {
        return SYSPARAM_ERR_RESET_ALL;
    }
    return SYSPARAM_ERR_NONE;
}


////////////////////////////////////// param update ///////////////////////////////////////////////////////////////
// sta scan/connect param
int sysparam_sta_scan_cfg_update(const wifi_scan_cfg_t *cfg)
{
    return sysparam_store(KV_SYSPARAM_STA_SCAN_CFG, (void *)cfg, sizeof(wifi_scan_cfg_t));
}
int sysparam_sta_conn_cfg_update(const wifi_sta_connect_t *cfg)
{
    sta_conn_param_t conn;
    if (!cfg || !cfg->ssid) {
        return SYSPARAM_ERR_INVALID_PARAM;
    }

    memset(&conn, 0, sizeof(sta_conn_param_t));
    memcpy(&conn.ssid, cfg->ssid, strlen(cfg->ssid));

    if (cfg->pwd) {
        memcpy(&conn.pwd, cfg->pwd, strlen(cfg->pwd));
    }
    if (cfg->bssid) {
        memcpy(&conn.bssid, cfg->bssid, BSSID_LEN);
    }
    return sysparam_store(KV_SYSPARAM_STA_CONN_CFG, (void *)&conn, sizeof(sta_conn_param_t));
}

// softap cfg param
int sysparam_softap_ssidpwd_cfg_update(const char *ssid, const char *pwd)
{
    softap_basic_param_t basic_param;
    if (!ssid || !pwd  || (strlen(ssid) >= SSID_MAX_LEN) || (strlen(pwd) >= PASSWORD_MAX_LEN)) {
        return SYSPARAM_ERR_INVALID_PARAM;
    }

    memset(&basic_param, 0, sizeof(softap_basic_param_t));
    memcpy(&basic_param.ssid, ssid, strlen(ssid));
    memcpy(&basic_param.pwd, pwd, strlen(pwd));
    return sysparam_store(KV_SYSPARAM_SOFTAP_BASIC_CFG, (void *)&basic_param, sizeof(softap_basic_param_t));
}

int sysparam_softap_ext_cfg_update(const wifi_softap_ext_cfg_t *cfg)
{
    return sysparam_store(KV_SYSPARAM_SOFTAP_EXT_CFG, (void *)cfg, sizeof(wifi_softap_ext_cfg_t));
}

//country code
int sysparam_country_code_update(wifi_country_code_t code)
{
    wifi_country_code_t country_code = code;
    return sysparam_store(KV_SYSPARAM_COUNTRY_CODE, (void *)&country_code, sizeof(wifi_country_code_t));
}

// sta powersave
int sysparam_sta_powersave_update(sta_ps_mode_t ps_mode)
{
    sta_ps_mode_t powersave_mode = ps_mode;
    return sysparam_store(KV_SYSPARAM_STA_PS_MODE, (void *)&powersave_mode, sizeof(sta_ps_mode_t));
}

// mac param
int sysparam_sta_mac_update(const uint8_t *macaddr)
{
    return sysparam_store(KV_SYSPARAM_STA_MAC, (void *)macaddr, MAC_ADDRESS_LEN);
}

int sysparam_softap_mac_update(const uint8_t *macaddr)
{
    return sysparam_store(KV_SYSPARAM_SOFTAP_MAC, (void *)macaddr, MAC_ADDRESS_LEN);
}

// ip info param
int sysparam_sta_ip_info_update(const tcpip_ip_info_t *ip_info)
{
    return sysparam_store(KV_SYSPARAM_STA_IP_INFO, (void *)ip_info, sizeof(tcpip_ip_info_t));
}

int sysparam_softap_ip_info_update(const tcpip_ip_info_t *ip_info)
{
    return sysparam_store(KV_SYSPARAM_SOFTAP_IP_INFO, (void *)ip_info, sizeof(tcpip_ip_info_t));
}

// netdev hostname
int sysparam_sta_hostname_update(const char *hostname)
{
    uint16_t len = strlen(hostname) + 1;
    len = (len > NETDEV_HOSTNAME_LEN_MAX) ? NETDEV_HOSTNAME_LEN_MAX : len;

    return sysparam_store(KV_SYSPARAM_STA_HOSTNAME, (void *)hostname, len);
}

int sysparam_softap_hostname_update(const char *hostname)
{
    uint16_t len = strlen(hostname) + 1;
    len = (len > NETDEV_HOSTNAME_LEN_MAX) ? NETDEV_HOSTNAME_LEN_MAX : len;
    
    return sysparam_store(KV_SYSPARAM_SOFTAP_HOSTNAME, (void *)hostname, len);
}

// wifi mode after poweron
int sysparam_poweron_wifi_mode_update(wifi_mode_t mode)
{
    wifi_mode_t wifi_mode = mode;
    return sysparam_store(KV_SYSPARAM_WIFI_MODE, (void *)&wifi_mode, sizeof(wifi_mode_t));
}

// auto connect after poweron
int sysparam_poweron_auto_conn_update(uint8_t en)
{
    uint8_t auto_conn = en;
    return sysparam_store(KV_SYSPARAM_AUTO_CONNECT, (void *)&auto_conn, sizeof(uint8_t));
}

// ap list info mask
int sysparam_ap_list_info_mask_update(uint16_t mask)
{
    uint16_t ap_list_info_mask = mask;
    return sysparam_store(KV_SYSPARAM_AP_LIST_MASK, (void *)&ap_list_info_mask, sizeof(uint16_t));
}

// dhcp client/server
int sysparam_dhcp_en_update(uint8_t en)
{
    uint8_t dhcp_en = en;
    return sysparam_store(KV_SYSPARAM_DHCP_EN, (void *)&dhcp_en, sizeof(uint8_t));
}

int sysparam_dhcpd_en_update(uint8_t en)
{
    uint8_t dhcpd_en = en;
    return sysparam_store(KV_SYSPARAM_DHCPD_EN, (void *)&dhcpd_en, sizeof(uint8_t));
}

int sysparam_dhcpd_cfg_update(const server_config_t *cfg)
{
    return sysparam_store(KV_SYSPARAM_DHCPD_CFG, (void *)cfg, sizeof(server_config_t));
}

int sysparam_sys_store_update(uint8_t en)
{
    uint8_t sys_store = en;
    return sysparam_store(KV_SYS_STORE_MODE, (void *)&sys_store, sizeof(uint8_t));
}

int sysparam_sys_uart_update(const sys_uart_status_t *cfg)
{
    return sysparam_store(KV_SYS_UART_CFG, (void *)cfg, sizeof(sys_uart_status_t));
}

int sysparam_cipsta_dhcp_cur_update(uint8_t dhcp_cur)
{
    uint8_t dhcp = dhcp_cur;
    return sysparam_store(KV_CIPSTA_DHCP_CUR_STAT, (void *)&dhcp, sizeof(uint8_t));
}

int sysparam_tcpip_dns_update(dns_ip_t * dns_ip)
{
    return sysparam_store(KV_TCPIP_DNS_CFG, (void *)dns_ip, sizeof(dns_ip_t));
}

int sysparam_sta_jap_cfg_update(wifi_sta_join_ap_cfg_t *cfg)
{
    return sysparam_store(KV_WIFI_STA_JAP_CFG, (void *)cfg, sizeof(wifi_sta_join_ap_cfg_t));
}


#if 0
  // TODO: mDNS param
  int sysparam_mdns_en_update(int enable);
  int sysparam_mdns_cfg_update(const mdns_config_t *cfg);
  
  // TODO: DNS param
  int sysparam_dns_en_update(int en);
  int sysparam_dns_cfg_update(const dns_config_t *cfg);
    
  // TODO: SNTP server param
  int sysparam_sntp_en_update(int en);
  int sysparam_sntp_cfg_update(const sntp_config_t *cfg);
#endif

//////////////////////////////////////   param get  ///////////////////////////////////////////////////////////////
#define INPUT_PARAM_CHECK(v)  do{ \
                                    if(v) { return SYSPARAM_ERR_INVALID_PARAM; } \
                                }while(0)

// sta scan/connect param
int sysparam_sta_scan_cfg_get(wifi_scan_cfg_t *cfg)
{
    INPUT_PARAM_CHECK(!cfg);
    return sysparam_load(KV_SYSPARAM_STA_SCAN_CFG, (void *)cfg, sizeof(wifi_scan_cfg_t));
}

int sysparam_sta_conn_cfg_get(wifi_sta_connect_t *cfg)
{
    INPUT_PARAM_CHECK(!cfg || !cfg->ssid || !cfg->pwd || !cfg->bssid);

    sta_conn_param_t conn;
    memset(&conn, 0, sizeof(sta_conn_param_t));
    int ret = sysparam_load(KV_SYSPARAM_STA_CONN_CFG, (void *)&conn, sizeof(sta_conn_param_t));
    if (ret == SYSPARAM_ERR_NONE) {
        memcpy(cfg->ssid, &conn.ssid, SSID_MAX_LEN);
        memcpy(cfg->pwd, &conn.pwd, PASSWORD_MAX_LEN);
        memcpy(cfg->bssid, &conn.bssid, BSSID_LEN);
    }

    return ret;
}

// softap cfg param
int sysparam_softap_ssidpwd_cfg_get(char *ssid, char *pwd)
{
    INPUT_PARAM_CHECK(!ssid || !pwd);

    softap_basic_param_t basic_param;
    memset(&basic_param, 0, sizeof(softap_basic_param_t));
    int ret = sysparam_load(KV_SYSPARAM_SOFTAP_BASIC_CFG, (void *)&basic_param, sizeof(softap_basic_param_t));
    if (ret == SYSPARAM_ERR_NONE) {
        memcpy(ssid, &basic_param.ssid, SSID_MAX_LEN);
        memcpy(pwd, &basic_param.pwd, PASSWORD_MAX_LEN);
    }

    return ret;
}

int sysparam_softap_ext_cfg_get(wifi_softap_ext_cfg_t *cfg)
{
    if (!cfg) {
        return SYSPARAM_ERR_INVALID_PARAM;
    }
    return sysparam_load(KV_SYSPARAM_SOFTAP_EXT_CFG, (void *)cfg, sizeof(wifi_softap_ext_cfg_t));
}

//country code
int sysparam_country_code_get(wifi_country_code_t *code)
{
    INPUT_PARAM_CHECK(!code);
    return sysparam_load(KV_SYSPARAM_COUNTRY_CODE, (void *)code, sizeof(wifi_country_code_t));
}

// sta powersave
int sysparam_sta_powersave_get(sta_ps_mode_t *ps_mode)
{
    INPUT_PARAM_CHECK(!ps_mode);
    return sysparam_load(KV_SYSPARAM_STA_PS_MODE, (void *)ps_mode, sizeof(sta_ps_mode_t));
}

// mac param
int sysparam_sta_mac_get(uint8_t *macaddr)
{
    INPUT_PARAM_CHECK(!macaddr);
    return sysparam_load(KV_SYSPARAM_STA_MAC, (void *)macaddr, MAC_ADDRESS_LEN);
}

int sysparam_softap_mac_get(uint8_t *macaddr)
{
    INPUT_PARAM_CHECK(!macaddr);
    return sysparam_load(KV_SYSPARAM_SOFTAP_MAC, (void *)macaddr, MAC_ADDRESS_LEN);
}

// ip info param
int sysparam_sta_ip_info_get(tcpip_ip_info_t *ip_info)
{
    if (!ip_info) {
        return SYSPARAM_ERR_INVALID_PARAM;
    }
    return sysparam_load(KV_SYSPARAM_STA_IP_INFO, (void *)ip_info, sizeof(tcpip_ip_info_t));
}

int sysparam_softap_ip_info_get(tcpip_ip_info_t *ip_info)
{
    INPUT_PARAM_CHECK(!ip_info);
    return sysparam_load(KV_SYSPARAM_SOFTAP_IP_INFO, (void *)ip_info, sizeof(tcpip_ip_info_t));
}

// netdev hostname
int sysparam_sta_hostname_get(char *hostname)
{
    INPUT_PARAM_CHECK(!hostname);
    return sysparam_load(KV_SYSPARAM_STA_HOSTNAME, (void *)hostname, NETDEV_HOSTNAME_LEN_MAX);
}

int sysparam_softap_hostname_get(char *hostname)
{
    INPUT_PARAM_CHECK(!hostname);
    return sysparam_load(KV_SYSPARAM_SOFTAP_HOSTNAME, (void *)hostname, NETDEV_HOSTNAME_LEN_MAX);
}

// wifi mode after poweron
int sysparam_poweron_wifi_mode_get(wifi_mode_t *mode)
{
    INPUT_PARAM_CHECK(!mode);
    return sysparam_load(KV_SYSPARAM_WIFI_MODE, (void *)mode, sizeof(wifi_mode_t));
}

// auto connect after poweron
int sysparam_poweron_auto_conn_get(uint8_t *en)
{
    INPUT_PARAM_CHECK(!en);
    return sysparam_load(KV_SYSPARAM_AUTO_CONNECT, (void *)en, sizeof(uint8_t));
}

// ap list info mask
int sysparam_ap_list_info_mask_get(uint16_t *mask)
{
    INPUT_PARAM_CHECK(!mask);
    return sysparam_load(KV_SYSPARAM_AP_LIST_MASK, (void *)mask, sizeof(uint16_t));
}

// dhcp client/server
int sysparam_dhcp_en_get(uint8_t *en)
{
    INPUT_PARAM_CHECK(!en);
    return sysparam_load(KV_SYSPARAM_DHCP_EN, (void *)en, sizeof(uint8_t));
}

int sysparam_dhcpd_en_get(uint8_t *en)
{
    INPUT_PARAM_CHECK(!en);
    return sysparam_load(KV_SYSPARAM_DHCPD_EN, (void *)en, sizeof(uint8_t));
}

int sysparam_dhcpd_cfg_get(server_config_t *cfg)
{
    INPUT_PARAM_CHECK(!cfg);
    return sysparam_load(KV_SYSPARAM_DHCPD_CFG, (void *)cfg, sizeof(server_config_t));
}

int sysparam_sys_store_get(uint8_t *en)
{
    INPUT_PARAM_CHECK(!en);
    return sysparam_load(KV_SYS_STORE_MODE, (void *)en, sizeof(uint8_t));
}

int sysparam_sys_uart_get(sys_uart_status_t *cfg)
{
    INPUT_PARAM_CHECK(!cfg);
    return sysparam_load(KV_SYS_UART_CFG, (void *)cfg, sizeof(sys_uart_status_t));
}

int sysparam_cipsta_dhcp_cur_get(uint8_t *dhcp_cur)
{
    INPUT_PARAM_CHECK(!dhcp_cur);
    return sysparam_load(KV_CIPSTA_DHCP_CUR_STAT, (void *)dhcp_cur, sizeof(uint8_t));
}

int sysparam_tcpip_dns_get(dns_ip_t * dns_ip)
{
    INPUT_PARAM_CHECK(!dns_ip);
    return sysparam_load(KV_TCPIP_DNS_CFG, (void *)dns_ip, sizeof(dns_ip_t));
}

int sysparam_sta_jap_cfg_get(wifi_sta_join_ap_cfg_t *cfg)
{
    INPUT_PARAM_CHECK(!cfg);
    return sysparam_load(KV_WIFI_STA_JAP_CFG, (void *)cfg, sizeof(wifi_sta_join_ap_cfg_t));
}


#if 0
  // TODO: mDNS param
  int sysparam_mdns_en_get(uint8_t *en);
  int sysparam_mdns_cfg_get(mdns_config_t *cfg);
  
  // TODO: DNS param
  int sysparam_dns_en_get(uint8_t *en);
  int sysparam_dns_cfg_get(dns_config_t *cfg);
    
  // TODO: SNTP server param
  int sysparam_sntp_en_get(uint8_t *en);
  int sysparam_sntp_cfg_get(sntp_config_t *cfg);
#endif






