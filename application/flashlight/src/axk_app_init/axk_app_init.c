#include "aiio_adapter_include.h"
#include "axk_app_init.h"
#include "axk_uart_init.h"
#include "axk_ringbuff.h"
#include "axk_uart_msg.h"
#include "axk_nvs.h"
#include "axk_nvs_info.h"
#include "axk_wifi_init.h"
#include "axk_at_cmd.h"
#include "axk_at.h"
#include "axk_device_info.h"
#include "axk_sleep.h"
#include "aiio_autoconf.h"

// #include "hal_hbn.h"

static void axk_app_init(void);

axk_app_init_t g_app_init =
{
    .init = axk_app_init,
};

// static int at_exe_sleep_cmd(uint32_t argc, const char **argv)
// {
//     aiio_os_tick_dealy(10000);
//     uint8_t weakup_pin = 0xff;

//     // AT_RESPONSE_OK;

//     // aiio_os_tick_dealy(10);

//     hal_hbn_init(&weakup_pin, 1);
//     hal_hbn_enter(10000);

//     return 0;
// }

static void axk_app_init_task(void *param)
{
    (void)(param);

    // device info
    g_axk_device_info.init();
    //NVS init
    g_axk_nvs.init();

    //load flash info
    g_nvs_info.load_info();

    //wifi init
    g_axk_wifi.init();

    //ringbuff init
    g_ringbuff_ctrl.init();

    //uart init
    g_uart.init();
    g_uart_msg.init();

    axk_at_printf("version:%s, time:" __DATE__ " " __TIME__ "\r\n", FW_VER);
    axk_at_printf("ready\r\n");

#if CONFIG_AXK_AUTO_SLEEP
    //create auto sleep timer
    axk_auto_sleep_timer_init();
#endif

    axk_mqtt_info_t *mqtt_info = g_axk_mqtt.get_conn_info();
    aiio_log_i("mqtt url: %s", mqtt_info->uri);

    // at_exe_sleep_cmd(0, NULL);
    
    aiio_os_thread_delete(NULL);
}

static void axk_app_init(void)
{
    aiio_os_thread_create(NULL, "init", axk_app_init_task, 2048, NULL, 10);
}

