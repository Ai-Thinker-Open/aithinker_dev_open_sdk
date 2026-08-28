#ifndef _AXK_HTTP_SERVER_SCAN_H_
#define _AXK_HTTP_SERVER_SCAN_H_

#include <stdint.h>

#include "aiio_wifi.h"

typedef struct axk_http_server_scan
{
    int32_t (*init)(void);
    uint32_t (*get_ap_num)(void);
    aiio_wifi_ap_item_t **(*get_ap_item)(void);
} axk_http_server_scan_t;

extern axk_http_server_scan_t g_axk_http_server_scan;

#endif
