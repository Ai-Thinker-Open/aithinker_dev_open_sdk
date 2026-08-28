#include "axk_at_basic.h"
#include "axk_at.h"
#include "axk_at_cmd.h"
#include "axk_wifi_init.h"

#include "aiio_wifi.h"
#include "aiio_log.h"
#include "aiio_os_port.h"

#include "hal_hbn.h"

void axk_enter_sleep(void)
{
    uint8_t weakup_pin = 7;

    hal_hbn_init(&weakup_pin, 1);
    hal_hbn_enter(0);
}

int at_exe_sleep_cmd(uint32_t argc, const char **argv)
{
    AT_RESPONSE_OK;

    aiio_os_tick_dealy(20);

    axk_enter_sleep();

    return 0;
}

int at_query_rssi_cmd(uint32_t argc, const char **argv)
{
    int ret = -1;
    int32_t rssi;

    if (g_axk_wifi.got_ip == 0)
    {
        aiio_log_e("wifi is not connect");
        goto __exit;
    }

    aiio_wifi_rssi_get(&rssi);

    axk_at_printf("+RSSI:%d\r\n", rssi);
    AT_RESPONSE_OK;

    ret = 0;

__exit:
    return ret;
}