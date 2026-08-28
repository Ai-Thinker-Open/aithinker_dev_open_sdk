#ifndef _AXK_AT_CMD_H_
#define _AXK_AT_CMD_H_

#include <stdint.h>
#include <stdio.h>

typedef struct {
    // uint16_t magic_num; /* check config is valid */
    uint8_t echo;
    // at_uart_t uart;
    // at_sleep_t sleep;
    // at_sysmsg_t sysmsg;
    // at_wifi_config_t wifi;
    // at_ip_config_t ip;
    // at_ble_config_t ble;
    // at_ota_config_t ota;
    // uint32_t systime_stamp;
    // uint8_t save_nv_flag;
} at_cmd_config_t;

void axk_at_esp_cmd_register(void);

extern at_cmd_config_t g_at_cmd_config;

#endif //_AXK_AT_H_
