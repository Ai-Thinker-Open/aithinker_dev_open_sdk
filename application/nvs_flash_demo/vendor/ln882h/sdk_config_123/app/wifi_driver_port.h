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
#include "utils/system_parameter.h"
#include "utils/ln_psk_calc.h"
#include "utils/debug/ln_assert.h"
#include "ln_misc.h"
#include "utils/ln_psk_calc.h"
#include "ln_nvds.h"
#include "ln_misc.h"
#include "utils/sysparam_factory_setting.h"
#include <stdbool.h>
#include "osal/osal.h"

#include "usr_ctrl.h"

#define LOCAL_IP                        "192.168.4.1"

#define PM_DEFAULT_SLEEP_MODE             (ACTIVE)
#define PM_WIFI_DEFAULT_PS_MODE           (WIFI_NO_POWERSAVE)

#define WLAN_SSID_MAX_LEN           (32)//rang:1-32
#define WLAN_PWD_MAX_LEN            (64)//rang:8-63ascii 8-64hex
#define WLAN_BSSID_MAX_LEN          (8)//rang:8-63ascii 8-64hex
#define TOKEN_MAX_LEN               (48)

typedef struct
{
    char ssid[SSID_MAX_LEN];
    char pwd[PASSWORD_MAX_LEN];
    uint8_t bssid[BSSID_LEN];
} LN_WIFI_STA_CFG;

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
} LN_WIFI_DRIVER_ADAPTER;

extern LN_WIFI_DRIVER_ADAPTER* ln_get_wifi_drv_handle(void);

#endif /* #ifndef __WIFI_DRIVER_PORT_H__ */
