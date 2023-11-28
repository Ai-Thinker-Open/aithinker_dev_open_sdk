#include "aiio_adapter_include.h"
#include "utils_aes.h"
#include "wifi_config.h"

#include "wifi_config_comm_service.h"

/**
 * @brief 打印固件版本信息
 *
 */
void print_system_info(void)
{
    aiio_log_i("################################################");
    aiio_log_i("company:Ai-Thinker|B&T");
    aiio_log_i("SDK version:%s", "1.0.0");
    aiio_log_i("compile_time:" __DATE__ " " __TIME__ "");
    aiio_log_i("################################################");
}

static aiio_os_thread_handle_t *aiio_udp_thread = NULL;

/**
 * @brief 应用程序入口
 *
 * @param params
 */
void aiio_main(void *params)
{
    aiio_mdns_deinit();
    print_system_info();
    aiio_nfcconfig_init();
    aiio_softAP_start();

    // wifi_sta_config_t config =
    //     {
    //         .ssid = "HUAWEI",
    //         .password = "12345678909",
    //     };
    // aiio_wifistation_connect(config);
    if (AIIO_OK != aiio_os_thread_create(&aiio_udp_thread, "udp_task", aiio_udp_start, 8 * 1024, NULL, 9))
    {
    }

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
