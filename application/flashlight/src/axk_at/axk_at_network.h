#ifndef _AXK_AT_NETWORK_H_
#define _AXK_AT_NETWORK_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum
{
    WIFI_CFG_BLE,
    WIFI_CFG_WEB,
    WIFI_CFG_APP,
    WIFI_CFG_NONE,
} axk_wifi_cfg_type_t;

typedef struct axk_at_network
{
    axk_wifi_cfg_type_t (*get_wifi_cfg_type)(void);
    void (*set_wifi_cfg_type)(uint8_t type);
} axk_at_network_t;

extern axk_at_network_t g_axk_network;

int at_setup_webcfg_cmd(uint32_t argc, const char** argv);
int at_setup_appcfg_cmd(uint32_t argc, const char** argv);

#endif //_AXK_AT_NETWORK_H_
