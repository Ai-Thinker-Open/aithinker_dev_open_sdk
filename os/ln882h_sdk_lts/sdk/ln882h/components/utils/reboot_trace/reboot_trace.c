#include "ln882h.h"
#include "ln_compiler.h"
#include "ln_types.h"
#include "utils/reboot_trace/reboot_trace.h"

#include "hal/hal_misc.h"

static chip_no_init_data_t s_chip_no_init_data __attribute__((section(".no_init_data"),aligned(8))) __ZERO_INIT__;

static int ln_chip_set_reboot_cause(chip_reboot_cause_t cause)
{
    chip_no_init_data_t * pdata = NULL;
    pdata = (chip_no_init_data_t *)(&s_chip_no_init_data);

    switch (cause)
    {
        case CHIP_REBOOT_WATCHDOG:
            pdata->reboot_magic = WATCHDOG_BOOT_MAGIC;
            break;
        case CHIP_REBOOT_SOFTWARE:
            pdata->reboot_magic = SOFTWARE_BOOT_MAGIC;
            break;
        default:
            return LN_FALSE;
    }

    return LN_TRUE;
}

static chip_reboot_cause_t get_reboot_cause(void)
{
    chip_reboot_cause_t cause = CHIP_REBOOT_POWER_ON;
    chip_no_init_data_t * pdata = NULL;
    pdata = (chip_no_init_data_t *)(&s_chip_no_init_data);

    if (pdata->reboot_magic == WATCHDOG_BOOT_MAGIC) {
        cause = CHIP_REBOOT_WATCHDOG;
    }
    else if(pdata->reboot_magic == SOFTWARE_BOOT_MAGIC) {
        cause = CHIP_REBOOT_SOFTWARE;
    }
    else{
        cause = CHIP_REBOOT_POWER_ON;
    }

    ln_chip_set_reboot_cause(CHIP_REBOOT_WATCHDOG);

    return cause;
}

chip_reboot_cause_t ln_chip_get_reboot_cause(void)
{
    static uint8_t s_power_on_env_is_init = 0;
    static chip_reboot_cause_t s_reboot_cause = CHIP_REBOOT_POWER_ON;

    if (s_power_on_env_is_init == 0) {
        s_reboot_cause = get_reboot_cause();
        s_power_on_env_is_init = 1;
    }
    return s_reboot_cause;
}

void ln_chip_reboot(void)
{
    ln_chip_set_reboot_cause(CHIP_REBOOT_SOFTWARE);
    hal_misc_reset_all();
}
