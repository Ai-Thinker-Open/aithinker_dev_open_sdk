#ifndef __WIFI_DRIVER_PORT_H__
#define __WIFI_DRIVER_PORT_H__

#include "ln_types.h"
#include "ln_wifi_err.h"
#include "wifi_manager.h"
#include "wifi.h"
#include "wifi_port.h"
#include "wifi_manager.h"
#include "netif/ethernetif.h"
#include "utils/debug/log.h"
#include "utils/ln_psk_calc.h"
#include "utils/debug/ln_assert.h"
#include "ln_misc.h"
#include "utils/ln_psk_calc.h"
#include "ln_nvds.h"
#include "ln_misc.h"
#include <stdbool.h>
#include "osal/osal.h"

#include "usr_ctrl.h"

#include "smartlink/smartcfg/ln_smartcfg_port.h"
#include "smartlink/smartcfg/ln_smartcfg_api.h"
#include "smartlink/smartcfg/ln_smartcfg.h"


#define LOCAL_IP						"192.168.4.1"

#define PM_DEFAULT_SLEEP_MODE             (ACTIVE)
#define PM_WIFI_DEFAULT_PS_MODE           (WIFI_NO_POWERSAVE)

#define WLAN_SSID_MAX_LEN_PORT           (32)//rang:1-32
#define WLAN_PWD_MAX_LEN_PORT            (64)//rang:8-63ascii 8-64hex
#define WLAN_BSSID_MAX_LEN_PORT          (8)//rang:8-63ascii 8-64hex
#define TOKEN_MAX_LEN               (48)

typedef struct
{
	bool isSet;
	tcpip_ip_info_t cur_ip_info;
}ln_wifi_sta_ip_t;

typedef struct
{
	uint8_t dhcp_en;
	
	uint8_t pci_en;
	uint8_t scan_mode;
	uint8_t pmf;
	uint8_t listen_interval;
	uint16_t reconn_interval;
	uint16_t jap_timeout;
	
	char ssid[SSID_MAX_LEN];
	char pwd[PASSWORD_MAX_LEN];
	uint8_t bssid[BSSID_LEN];
	ln_wifi_sta_ip_t sta_ip;
} LN_WIFI_STA_CFG;

typedef struct
{
	int (*get_state)(void);
	void (*set_state)(uint8_t);
}ln_wifi_netdev_t;

typedef struct
{
	wifi_sta_connect_t conn;
	LN_WIFI_STA_CFG sta;
	LN_WIFI_STA_CFG sap;
	
	bool reConn;
	
	void (*init)(void);
	
	void (*init_sta)(void);
	void (*sta_connect)(void);
	void (*reset_ip_sem)(void);
	void (*release_ip_sem)(void);
	int (*wait_for_ip)(int _ms);

	int (*start_softap)(void);
	void (*cfg_softap)(const char* _ssid, const char* _pwd);
	void (*stop)(void);
	
	char* (*ssid_buf)(void);
	char* (*pwd_buf)(void);
	uint8_t* (*bssid_buf)(void);
	ln_wifi_netdev_t netdev;
} LN_WIFI_DRIVER_ADAPTER;

extern LN_WIFI_DRIVER_ADAPTER* ln_get_wifi_drv_handle(void);


/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
//adapt esp8266 cur/none set
typedef struct
{
    uint8_t pci_en;
    uint8_t scan_mode;
    uint8_t pmf;
    uint8_t listen_interval;
    uint16_t reconn_interval;
    uint16_t jap_timeout;
}wifi_sta_join_ap_cfg_t;

typedef struct
{
    bool reconn;

    wifi_sta_join_ap_cfg_t join_ap_cfg;

    char ssid[SSID_MAX_LEN];
    char pwd[PASSWORD_MAX_LEN];
    uint8_t bssid[BSSID_LEN];
    wifi_scan_cfg_t sta_scan_cfg;
}wifi_sta_connect_ext_t;

typedef struct
{
    char ssid[SSID_MAX_LEN];
    char pwd[PASSWORD_MAX_LEN];
    uint8_t bssid[BSSID_LEN];
}wifi_ap_cfg_t;

typedef struct
{
    wifi_mode_t cur_mode;
    uint8_t dhcp_cur;
    uint8_t dhcpd_cur;
    tcpip_ip_info_t sta_ip_info_cur;
    tcpip_ip_info_t ap_ip_info_cur;
}wifi_status_info_t;


#define SMARTCONFIG_TASK_STACK_SIZE    1024 * 6

typedef struct
{
    uint8_t type;
    uint8_t auth;
    char key[16];
}smartconfig_config_t;

typedef struct
{
    bool is_running;
    bool stop_flag;
    //semaphore
    OS_Semaphore_t smartconfig_get_wifi_info_sem;
    OS_Semaphore_t smartconfig_get_raw_data_sem;
    OS_Semaphore_t smartconfig_connected_wifi_sem;
    OS_Semaphore_t smartconfig_stop_sem;
    //task thread
    OS_Thread_t smartconfig_thread;

    //task entry
    void (*smartconfig_task_entry)(void *param);

    //smartconfig func
    void (*wifi_sta_smartconfig_start_func)(uint8_t type, uint8_t auth, const char *key);
    void (*wifi_sta_smartconfig_stop_func)(void);
}smartconfig_func_t;

typedef struct
{
    
}at_wifi_semaphore_t;

typedef struct
{
    //at wifi function register
    void (*adapter_init_func)(void);

    //sta func
    void (*wifi_sta_init_func)(void);
    void (*wifi_sta_connect_func)(void);

    //softap func
    void (*wifi_ap_start_func)(void);

    //stop wifi
    void (*wifi_stop_func)(void);

    //smart_config func
    smartconfig_func_t smartconfig_func;
}at_wifi_func_t;

typedef struct
{
    //set ap mode, ap cfg
    wifi_ap_cfg_t ap_cfg;
    
    //set sta mode, connect cfg and ext connect cfg
    wifi_sta_connect_t conn_cfg;
    wifi_sta_connect_ext_t conn_ext_cfg;
    
    //wifi status manage
    wifi_status_info_t status_info;
    
    //wifi semaphore manage
    at_wifi_semaphore_t sem;
    
    //adapte at wifi function
    at_wifi_func_t at_func;
}at_wifi_adapter_t;

extern at_wifi_adapter_t* at_wifi_adapter_get_handle(void);

#endif /* #ifndef __WIFI_DRIVER_PORT_H__ */
