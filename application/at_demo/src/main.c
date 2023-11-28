#include "aiio_adapter_include.h"

#define FW_VER "release/V4.18_P1.1.0"

void print_boot_info(void)
{
    aiio_at_printf("\r\n################################################\r\n");
    aiio_at_printf("\r\n");
    aiio_at_printf("arch:%s,%s\r\n", __AIIO_CHIP_NAME, "NULL");
    aiio_at_printf("company:Ai-Thinker|B&T\r\n");
#if CONFIG_AT_BLE_CMD_ENABLE
    uint8_t ble_mac[6] = {0};
    aiio_ble_config_t ble_config;

    aiio_ble_init(AIIO_BLE_MODE_PERIPHERAL, &ble_config);
    aiio_ble_get_mac(ble_mac);
    aiio_ble_deinit();
    aiio_at_printf("ble_mac:%02x%02x%02x%02x%02x%02x\r\n",
                    ble_mac[0], ble_mac[1], ble_mac[2], ble_mac[3], ble_mac[4], ble_mac[5]);
#endif
#if CONFIG_AT_WIFI_CMD_ENABLE
    uint8_t wifi_mac[6] = {0};

    aiio_wifi_sta_mac_get(wifi_mac);
    aiio_at_printf("wifi_mac:%02x%02x%02x%02x%02x%02x\r\n",
                    wifi_mac[0], wifi_mac[1], wifi_mac[2], wifi_mac[3], wifi_mac[4], wifi_mac[5]);
#endif
    aiio_at_printf("sdk_version:%s\r\n", "release_bl_iot_sdk_1.6.40");
    aiio_at_printf("firmware_version:%s\r\n", FW_VER);
    aiio_at_printf("compile_time:%s %s\r\n", __DATE__, __TIME__);
    aiio_at_printf("\r\n");
    aiio_at_printf("ready\r\n");
    aiio_at_printf("\r\n");
    aiio_at_printf("################################################\r\n");
}

void aiio_main(void *params)
{
    aiio_at_init_param_t init_param = {
        .fw_version = FW_VER,
    };

    aiio_log_i("Combo AT!");
    
    aiio_nvs_init();
    
    aiio_at_nv_load_param_all();

    aiio_at_init(&init_param);

    aiio_at_basic_cmd_regist();
    aiio_at_io_control_cmd_regist();

#if CONFIG_AT_WIFI_CMD_ENABLE
    aiio_at_wifi_cmd_regist();
    aiio_at_sntp_cmd_regist();
#endif
#if CONFIG_AT_BLE_CMD_ENABLE
    aiio_at_ble_cmd_regist();
#endif
#if CONFIG_AT_HTTP_CMD_ENABLE
    aiio_at_http_cmd_regist();
#endif
#if CONFIG_AT_MQTT_CMD_ENABLE
    aiio_at_mqtt_cmd_regist();
#endif
#if CONFIG_AT_SOCKET_CMD_ENABLE
    aiio_at_socket_cmd_regist();
#endif

    aiio_at_run();
    aiio_at_enable_echo();
    print_boot_info();

#if CONFIG_AT_WIFI_CMD_ENABLE
    aiio_wifi_auto_connect_onboot();
#endif
#if CONFIG_AT_BLE_CMD_ENABLE
    aiio_ble_auto_connect_onboot();
#endif

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

