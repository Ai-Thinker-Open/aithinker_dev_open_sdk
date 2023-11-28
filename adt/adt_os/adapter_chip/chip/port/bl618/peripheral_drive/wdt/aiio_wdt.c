#include "aiio_wdt.h"
#include "bflb_wdg.h"
#include <stdbool.h>

struct bflb_device_s *wdg;
static bool WdtIsInit = false;

void wdg_isr(int irq, void *arg)
{
    aiio_restart();
}

void aiio_wdt_init(void)
{
    if(WdtIsInit)
    {
        aiio_log_e("wdt already init, not aganst init \r\n");
        return;
    }
    
    struct bflb_wdg_config_s wdg_cfg;
    wdg_cfg.clock_source = WDG_CLKSRC_32K;
    wdg_cfg.clock_div = 31;
    wdg_cfg.comp_val = 10000;
    wdg_cfg.mode = WDG_MODE_INTERRUPT;
    
    wdg = bflb_device_get_by_name("watchdog");
    bflb_wdg_init(wdg, &wdg_cfg);
    bflb_irq_attach(wdg->irq_num, wdg_isr, wdg);
    bflb_irq_enable(wdg->irq_num);
    bflb_wdg_start(wdg);
    WdtIsInit = true;
    aiio_log_d("wdt init \r\n");
}

void aiio_wdt_deinit(void)
{
    bflb_wdg_stop(wdg);
    WdtIsInit = false;
}

void aiio_wdt_reload(void)
{
    bflb_wdg_reset_countervalue(wdg);
}