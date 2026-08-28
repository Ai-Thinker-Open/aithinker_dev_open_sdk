#ifndef _AXK_WIFI_INIT_H_
#define _AXK_WIFI_INIT_H_

#include <stdint.h>

typedef enum
{
    AXK_WIFI_AP = (uint8_t)0,
    AXK_WIFI_STA = (uint8_t)1,
} axk_wifi_mode_t;

typedef struct axk_sta_info
{
    uint8_t ssid[33];
    uint8_t pwd[66];
    int32_t channel;
} axk_sta_info_t;

typedef struct axk_wifi_init
{
    uint8_t discon_cnt;
    uint8_t got_ip;
    int32_t (*init)(void);
    int32_t (*set_sta_info)(const axk_sta_info_t *info);
    void (*set_mode)(axk_wifi_mode_t mode);
    axk_wifi_mode_t (*get_mode)(void);
    int32_t (*start_connect)(void);
    uint8_t (*get_ap_start_flag)(void);
} axk_wifi_init_t;





extern axk_wifi_init_t g_axk_wifi;

#endif
