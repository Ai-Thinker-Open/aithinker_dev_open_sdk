#ifndef __SYSTEM_PARAMETER_H__
#define __SYSTEM_PARAMETER_H__

#include "wifi.h"
#include "dhcpd_api.h"
#include "netif/ethernetif.h"
#include "sys_driver_port.h"
#include "ln_at_tcpip.h"
#include "wifi_driver_port.h"

typedef enum {
    SYSPARAM_ERR_NONE          = 0,
    SYSPARAM_ERR_INVALID_PARAM = 1,
    SYSPARAM_ERR_STORE         = 2,
    SYSPARAM_ERR_LOAD          = 3,
    SYSPARAM_ERR_RESET_ALL     = 4,
} sysparam_err_t;

int sysparam_integrity_check_all(void);
int sysparam_factory_reset_all(void);

// sta scan/connect param
int sysparam_sta_scan_cfg_factory_reset(void);
int sysparam_sta_scan_cfg_update(const wifi_scan_cfg_t *cfg);
int sysparam_sta_scan_cfg_get(wifi_scan_cfg_t *cfg);

int sysparam_sta_conn_cfg_factory_reset(void);
int sysparam_sta_conn_cfg_update(const wifi_sta_connect_t *cfg);
int sysparam_sta_conn_cfg_get(wifi_sta_connect_t *cfg);

// softap cfg param
int sysparam_softap_ssidpwd_cfg_factory_reset(void);
int sysparam_softap_ssidpwd_cfg_update(const char *ssid, const char *pwd);
int sysparam_softap_ssidpwd_cfg_get(char *ssid, char *pwd);

int sysparam_softap_ext_cfg_factory_reset(void);
int sysparam_softap_ext_cfg_update(const wifi_softap_ext_cfg_t *cfg);
int sysparam_softap_ext_cfg_get(wifi_softap_ext_cfg_t *cfg);

//country code
int sysparam_country_code_factory_reset(void);
int sysparam_country_code_update(wifi_country_code_t code);
int sysparam_country_code_get(wifi_country_code_t *code);

// sta powersave
int sysparam_sta_powersave_factory_reset(void);
int sysparam_sta_powersave_update(sta_ps_mode_t ps_mode);
int sysparam_sta_powersave_get(sta_ps_mode_t *ps_mode);

// mac param
int sysparam_sta_mac_factory_reset(void);
int sysparam_sta_mac_update(const uint8_t *macaddr);
int sysparam_sta_mac_get(uint8_t *macaddr);

int sysparam_softap_mac_factory_reset(void);
int sysparam_softap_mac_update(const uint8_t *macaddr);
int sysparam_softap_mac_get(uint8_t *macaddr);

// ip info param
int sysparam_sta_ip_info_factory_reset(void);
int sysparam_sta_ip_info_update(const tcpip_ip_info_t *ip_info);
int sysparam_sta_ip_info_get(tcpip_ip_info_t *ip_info);

int sysparam_softap_ip_info_factory_reset(void);
int sysparam_softap_ip_info_update(const tcpip_ip_info_t *ip_info);
int sysparam_softap_ip_info_get(tcpip_ip_info_t *ip_info);

// netdev hostname
int sysparam_sta_hostname_factory_reset(void);
int sysparam_sta_hostname_update(const char *hostname);
int sysparam_sta_hostname_get(char *hostname);

int sysparam_softap_hostname_factory_reset(void);
int sysparam_softap_hostname_update(const char *hostname);
int sysparam_softap_hostname_get(char *hostname);

// wifi mode after poweron
int sysparam_poweron_wifi_mode_factory_reset(void);
int sysparam_poweron_wifi_mode_update(wifi_mode_t mode);
int sysparam_poweron_wifi_mode_get(wifi_mode_t *mode);

// auto connect after poweron
int sysparam_poweron_auto_conn_factory_reset(void);
int sysparam_poweron_auto_conn_update(uint8_t en);
int sysparam_poweron_auto_conn_get(uint8_t *en);

// ap list info mask
int sysparam_ap_list_info_mask_factory_reset(void);
int sysparam_ap_list_info_mask_update(uint16_t mask);
int sysparam_ap_list_info_mask_get(uint16_t *mask);

// dhcp client/server
int sysparam_dhcp_en_factory_reset(void);
int sysparam_dhcp_en_update(uint8_t en);
int sysparam_dhcp_en_get(uint8_t *en);

int sysparam_dhcpd_en_factory_reset(void);
int sysparam_dhcpd_en_update(uint8_t en);
int sysparam_dhcpd_en_get(uint8_t *en);

int sysparam_dhcpd_cfg_factory_reset(void);
int sysparam_dhcpd_cfg_update(const server_config_t *cfg);
int sysparam_dhcpd_cfg_get(server_config_t *cfg);

int sysparam_sys_store_factory_reset(void);
int sysparam_sys_store_update(uint8_t en);
int sysparam_sys_store_get(uint8_t *en);

int sysparam_sys_uart_factory_reset(void);
int sysparam_sys_uart_update(const sys_uart_status_t *cfg);
int sysparam_sys_uart_get(sys_uart_status_t *cfg);

int sysparam_cipsta_dhcp_cur_factory_reset(void);
int sysparam_cipsta_dhcp_cur_update(uint8_t dhcp_cur);
int sysparam_cipsta_dhcp_cur_get(uint8_t *dhcp_cur);

int sysparam_tcpip_dns_factory_reset(void);
int sysparam_tcpip_dns_update(dns_ip_t *dns_ip);
int sysparam_tcpip_dns_get(dns_ip_t *dns_ip);

int sysparam_sta_jap_cfg_factory_reset(void);
int sysparam_sta_jap_cfg_update(wifi_sta_join_ap_cfg_t *cfg);
int sysparam_sta_jap_cfg_get(wifi_sta_join_ap_cfg_t *cfg);


#if 0
  // TODO: mDNS param
  int sysparam_mdns_en_factory_reset(void);
  int sysparam_mdns_en_update(uint8_t en);
  int sysparam_mdns_en_get(uint8_t *en);

  int sysparam_mdns_cfg_factory_reset(void);
  int sysparam_mdns_cfg_update(const mdns_config_t *cfg);
  int sysparam_mdns_cfg_get(mdns_config_t *cfg);
  
  // TODO: DNS param
  int sysparam_dns_en_factory_reset(void);
  int sysparam_dns_en_update(uint8_t en);
  int sysparam_dns_en_get(uint8_t *en);

  int sysparam_dns_cfg_factory_reset(void);
  int sysparam_dns_cfg_update(const dns_config_t *cfg);
  int sysparam_dns_cfg_get(dns_config_t *cfg);
    
  // TODO: SNTP server param
  int sysparam_sntp_en_factory_reset(void);
  int sysparam_sntp_en_update(uint8_t en);
  int sysparam_sntp_en_get(uint8_t *en);
  
  int sysparam_sntp_cfg_factory_reset(void);
  int sysparam_sntp_cfg_update(const sntp_config_t *cfg);
  int sysparam_sntp_cfg_get(sntp_config_t *cfg);
#endif


#endif /* __SYSTEM_PARAMETER_H__ */

