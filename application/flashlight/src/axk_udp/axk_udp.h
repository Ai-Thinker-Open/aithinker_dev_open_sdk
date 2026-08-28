#ifndef _AXK_UDP_H_
#define _AXK_UDP_H_

#include <stdio.h>
#include <stdint.h>

#include "aiio_os_port.h"

#define UDP_CFG_WIFI_STEP2      "{\"cmdId\":2,\"deviceId\":\"productCode\",\"productId\":\"deviceCode\"}"
#define UDP_CFG_WIFI_SUCCEED    "{\"cmdId\":4,\"connectTest\":0}"

typedef struct axk_udp
{
    int32_t (*init)(void);
    int32_t (*deinit)(void);
    int32_t (*wifi_cfg_udp_sendto)(uint8_t *buff, uint16_t len);
    aiio_os_thread_handle_t (*get_udp_handle)(void);
} axk_udp_t;

extern axk_udp_t g_axk_udp;

#endif //_AXK_UDP_H_
