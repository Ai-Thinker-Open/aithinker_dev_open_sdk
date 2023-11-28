
#ifndef __SYSPARAM_FACTORY_SETTING_H__
#define __SYSPARAM_FACTORY_SETTING_H__


/**<  network(STA) default parameters */ 
#define STA_IP_ADDR               ("192.168.1.1")   /**<  static ip addr */
#define STA_IP_NETMASK            ("255.255.255.0") /**<  subnet mask    */
#define STA_IP_GATWAY             ("192.168.1.1")   /**<  gateway        */
#define STA_NETDEV_HOSTNAME       ("LN882H_STA")


/**< STA default mac addr */
#define STA_MAC_ADDR0             (0x00)
#define STA_MAC_ADDR1             (0x50)
#define STA_MAC_ADDR2             (0xC2)
#define STA_MAC_ADDR3             (0x5E)
#define STA_MAC_ADDR4             (0x10)
#define STA_MAC_ADDR5             (0x88)

/**<  network(SoftAP) default parameters */
#define SOFTAP_IP_ADDR            ("192.168.4.1")   /**<  ip addr        */
#define SOFTAP_IP_NETMASK         ("255.255.255.0") /**<  subnet mask    */
#define SOFTAP_IP_GATWAY          ("192.168.4.1")   /**<  gateway        */
#define SOFTAP_NETDEV_HOSTNAME    ("LN882H_AP")

/**<  SoftAP default mac addr */
#define SOFTAP_MAC_ADDR0          (0x00)
#define SOFTAP_MAC_ADDR1          (0x50)
#define SOFTAP_MAC_ADDR2          (0xC2)
#define SOFTAP_MAC_ADDR3          (0x5E)
#define SOFTAP_MAC_ADDR4          (0x10)
#define SOFTAP_MAC_ADDR5          (0x99)


/**<  wifi(SoftAP mode) cfg default parameters */ 
#define SOFTAP_SSID               ("LN882H_WiFi")
#define SOFTAP_PWD                ("12345678")
#define SOFTAP_CHANNEL            (6)
#define SOFTAP_AUTH_MODE          (0)    /**<  reference wifi_auth_mode_enum_t(wifi.h) */
#define SOFTAP_SSID_HIDDEN        (0)
#define SOFTAP_BEACON_INTERVAL    (100)  /**<  (uints:ms) */
//#define SOFTAP_MAX_CONNECTION     (3)
#define SOFTAP_COUNTRY_CODE       (0)    /**<  reference wifi_country_code_t(wifi.h) */

/**<  wifi(STA mode) cfg default parameters */ 
#define TARGET_AP_SSID            ("TP-LINK_58E6")
#define TARGET_AP_PWD             ("12345678")
//#define TARGET_AP_BSSID           (0)
#define STA_SCAN_CHANNEL          (0)
#define STA_SCAN_TYPE             (0)    /**<  reference wifi_scan_type_t(wifi.h) */
#define STA_SCAN_TIME             (50)   /**<  (uints:ms)scan time per channel */
#define STA_POWERSAVE_MODE        (0)    /**<  reference sta_ps_mode_t(wifi.h) */


/**<  DHCP server cfg default parameters */ 
#define DHCP_ENABLE               (1)    /**<  (STA mode)DHCP client enable */
#define DHCPD_ENABLE              (1)    /**<  (SoftAP mode)DHCP server enable */
#define DHCPD_IP_LEASE_TIME       (2880) /**<  (uints:minute) */
#define DHCPD_IP_RENEW_TIME       (2880) /**<  (uints:minute) */
#define DHCPD_LISTEN_PORT         (67)
#define DHCPD_CLIENT_MAX          (3)
#define DHCPD_SER_IP              ("192.168.4.1")
#define DHCPD_IP_POOL_START       ("192.168.4.101")
#define DHCPD_IP_POOL_END         ("192.168.4.103")

/**<  mDNS cfg default parameters */ 
#define MDNS_ENABLE               (0)
#define MDNS_HOSTNAME             ("lightning_semi")
#define MDNS_SERVER_NAME          ("iot")
#define MDNS_SERVER_PORT          ("8080")

/**<  DNS cfg default parameters */
#define DNS_ENABLE                (0)
#define DNS_SERVER1               ("208.67.222.222")
#define DNS_SERVER2               ("114.114.114.114")
#define DNS_SERVER3               ("8.8.8.8")

/**<  SNTP server cfg default parameters */ 
#define SNTP_ENABLE               (0)
#define SNTP_TIMEZONE             (8)
#define SNTP_SERVER0              ("time1.aliyun.com")
#define SNTP_SERVER1              ("s1c.time.edu.cn")
#define SNTP_SERVER2              ("time.windows.com")


/**<  Misc cfg default parameters */ 
#define WIFI_MODE_DEFAULT         (1)    /**<  reference wifi_mode_t(wifi.h) */
#define STA_POWERON_AUTO_CONN     (1)    /**<  whether to automatically connect to AP after power on */
#define AP_LIST_INFO_MASK         (0x7FF)/**<  bit0:auth_mode,bit1:ssid,bit2:rssi,bit3:mac,bit4:ch,bit5:freq_ofst, \
                                               bit6:freq_cal,bit7:pairwise_cipher,bit8:group_cipher,bit9:bgn,bit10:wps*/

/**< Sysstore */
#define SYSSTORE_DEFAULT           1

/**< Sys Uart Cfg */
#define SYS_UART_BAUD              115200
#define SYS_UART_DATA_BITS              8
#define SYS_UART_STOP_BITS              1
#define SYS_UART_PARITY                 0
#define SYS_UART_FLOWCONTROL            3

/**< store cipsta_cur dhcp cfg*/
#define CIPSTA_DHCP_STAT                1

/**< store cipsta_cur dhcp cfg*/
#define STA_JOIN_AP_CFG_PCIEN           0
#define STA_JOIN_AP_CFG_SCANMODE        0
#define STA_JOIN_AP_CFG_PMF             0
#define STA_JOIN_AP_CFG_LISTEN_INTERVAL 3
#define STA_JOIN_AP_CFG_RECONN_INTERVAL 1
#define STA_JOIN_AP_CFG_JAP_TIMEOUT     15


#endif /* __SYSPARAM_FACTORY_SETTING_H__ */

