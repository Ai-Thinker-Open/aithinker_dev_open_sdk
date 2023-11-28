#ifndef WIFI_CONFING_H__
#define WIFI_CONFING_H__

#include "aiio_errcode.h"

#include "stdio.h"

typedef struct
{
    uint8_t ssid[32];     /**< SSID of target AP*/
    uint8_t password[64]; /**< password of target AP*/
    uint8_t bssid[7];     /**< MAC address of target AP*/
} wifi_sta_config_t;

typedef struct
{

} wifi_ap_config_t;

int aiio_wifi_connect_status_get(void);
int aiio_wifi_ap_status_get(void);
void aiio_nfcconfig_init(void);
void aiio_softAP_start(void);
void aiio_softAP_stop(void);
aiio_ret_t aiio_wifistation_connect(wifi_sta_config_t config);

#endif // !WIFI_CONFING_H__
