#ifndef _AXK_NVS_INFO_H_
#define _AXK_NVS_INFO_H_

#include <stdint.h>
#include "axk_wifi_init.h"
#include "axk_mqtt.h"
#include "axk_sleep.h"

#define NVS_MAGIC_KEY           "magic"
#define WIFI_MODE_KEY           "wifi_mode"
#define WIFI_CONN_STA_INFO_KEY  "sta_info"
#define PRODUCT_CODE_KEY        "product_code"
#define MQTT_INFO_KEY           "mqtt_info"
#define MQTT_SAVE_KEY           "mqtt_save"
#define MCU_VER_KEY             "mcu_ver"
#define AUTO_SLEEP_KEY          "auto_sleep"

#define NVS_MAGIC_VALUE         (0x55)

typedef struct axk_nvs_info
{
    void (*get_wifi_mode)(axk_wifi_mode_t *mode);
    int32_t (*set_wifi_mode)(axk_wifi_mode_t mode);
    int32_t (*get_sta_info)(axk_sta_info_t *sta_info);
    int32_t (*set_sta_info)(const axk_sta_info_t sta_info);
    int32_t (*set_product_code)(uint8_t *code, uint8_t len);
    int32_t (*get_product_code)(uint8_t *code);
    int32_t (*set_mqtt_info)(axk_mqtt_info_t *info);
    int32_t (*set_mcu_ver)(uint8_t *version);
    int32_t (*get_mcu_ver)(uint8_t *version);
    int32_t (*set_mqtt_save_flag)(uint8_t *is_save);
    int32_t (*get_auto_sleep_info)(axk_sleep_info_t *info);
    int32_t (*set_auto_sleep_info)(const axk_sleep_info_t info);
    void (*load_info)(void);
} axk_nvs_info_t;

extern axk_nvs_info_t g_nvs_info;

int at_exe_restore_cmd(uint32_t argc, const char** argv);
int at_exe_echo_on_cmd(uint32_t argc, const char** argv);
int at_exe_echo_off_cmd(uint32_t argc, const char** argv);

#endif //_AXK_NVS_INFO_H_
