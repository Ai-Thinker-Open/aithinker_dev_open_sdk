#include "aiio_wdt.h"
#include <hosal_wdg.h>
#include <stdbool.h>

static hosal_wdg_dev_t wdg;
static bool WdtIsInit = false;


void aiio_wdt_init(void)
{
    if(WdtIsInit)
    {
        aiio_log_e("wdt already init, not aganst init \r\n");
        return;
    }

    /* wdg port set */
    wdg.port = 0;
    /* max timeout is 65532/16 ms */
    wdg.config.timeout = 10000;
    /* init wdg with the given settings */
    hosal_wdg_init(&wdg);
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

    hosal_wdg_finalize(&wdg);
    WdtIsInit = false;
    aiio_log_d("wdt Deinit \r\n");
}

void aiio_wdt_reload(void)
{
    hosal_wdg_reload(&wdg);
}
