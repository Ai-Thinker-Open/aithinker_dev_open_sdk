 //SDK相关
#include "ameba_soc.h"
#include "os_wrapper.h"
#include "wdt_api.h"

//中间件相关
 #include "aiio_log.h"

//标准库
#include <stdio.h>

static bool WdtIsInit = false;

void aiio_wdt_init(void)
{
    if (WdtIsInit)
    {
        aiio_log_w("wdt already init, not aganst init \r\n");
        //看门狗已经初始化，只需要重新启动
        watchdog_refresh();
        watchdog_start();
        return;
    }

    watchdog_init(10 * 1000); // setup 10s watchdog
    watchdog_start();

    WdtIsInit = true;
}

void aiio_wdt_init_timeouts(uint32_t timeouts_ms)
{
    if (WdtIsInit)
    {
        aiio_log_e("wdt already init, not aganst init \r\n");
        return;
    }

    watchdog_init(timeouts_ms);
    watchdog_start();

    WdtIsInit = true;
}

void aiio_wdt_deinit(void)
{
    // aiio_log_e("deinitializing watchdog is not supported\r\n");
    watchdog_stop();
}

void aiio_wdt_reload(void)
{
    watchdog_refresh();
}
