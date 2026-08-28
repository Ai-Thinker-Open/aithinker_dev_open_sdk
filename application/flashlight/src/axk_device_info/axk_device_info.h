#ifndef _AXK_DEVICE_INFO_H_
#define _AXK_DEVICE_INFO_H_

#include <stdio.h>
#include <stdint.h>

#define FW_VER              "2.1.4"

typedef struct axk_device_info
{
    uint8_t mac_str[13];
    uint8_t deviceCode[16];
    uint8_t productCode[16];
    uint8_t mcu_ver[6];
    void (*init)(void);
} axk_device_info_t;

extern axk_device_info_t g_axk_device_info;

int at_query_productCode_cmd(uint32_t argc, const char** argv);
int at_query_fw_version_cmd(uint32_t argc, const char** argv);
int at_query_deviceCode_cmd(uint32_t argc, const char** argv);
int at_setup_productCode_cmd(uint32_t argc, const char** argv);
int at_query_mcu_version_cmd(uint32_t argc, const char** argv);
int at_setup_mcu_version_cmd(uint32_t argc, const char** argv);
int at_query_sta_mac_cmd(uint32_t argc, const char** argv);

#endif
