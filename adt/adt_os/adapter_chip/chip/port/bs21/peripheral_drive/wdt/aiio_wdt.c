#include "aiio_wdt.h"
#include "pinctrl.h"
#include "watchdog.h"
#include "soc_osal.h"
#include "app_init.h"

#define TIME_OUT                  2
static bool WdtIsInit = false;


void aiio_wdt_init(void)
{
    if(WdtIsInit)
    {
        aiio_log_e("wdt already init, not aganst init \r\n");
        return;
    }

    (void)uapi_watchdog_deinit();
    errcode_t ret = uapi_watchdog_init(TIME_OUT);
    if (ret == ERRCODE_INVALID_PARAM) {
        return;
    }

    (void)uapi_watchdog_enable((wdt_mode_t)WDT_MODE_RESET);

    aiio_log_d("wdt init \r\n");
    WdtIsInit = true;
}

void aiio_wdt_init_timeouts(uint32_t timeouts_ms)
{
    if(WdtIsInit)
    {
        aiio_log_e("wdt already init, not aganst init \r\n");
        return;
    }

    (void)uapi_watchdog_deinit();
    errcode_t ret = uapi_watchdog_init(timeouts_ms);
    if (ret == ERRCODE_INVALID_PARAM) {
        return;
    }

    (void)uapi_watchdog_enable((wdt_mode_t)WDT_MODE_RESET);

    aiio_log_d("wdt init \r\n");
    WdtIsInit = true;
}

void aiio_wdt_deinit(void)
{
    if(!WdtIsInit)
    {
        aiio_log_e("wdt already Deinit, not aganst Deinit \r\n");
        return;
    }

    (void)uapi_watchdog_deinit();
    WdtIsInit = false;
    aiio_log_d("wdt Deinit \r\n");
}

void aiio_wdt_reload(void)
{
    // 喂狗操作
    uapi_watchdog_kick();
}
