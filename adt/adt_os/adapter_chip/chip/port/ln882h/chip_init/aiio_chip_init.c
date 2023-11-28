#include "aiio_error.h"
#include "aiio_chip_init.h"

#include "proj_config.h"
#include "osal/osal.h"
#include "wifi.h"
#include "wifi_port.h"
#include "netif/ethernetif.h"
//#include "usr_app.h"
#include "utils/system_parameter.h"
#include "utils/debug/CmBacktrace/cm_backtrace.h"
#include "utils/reboot_trace/reboot_trace.h"
#include "utils/runtime/runtime.h"
#include "hal/hal_interrupt.h"
#include "ln_utils.h"
#include "ln_misc.h"
#include "ln_nvds.h"
#include "ln_kv_api.h"
#include "ln_at.h"
#include "flash_partition_table.h"

int32_t aiio_chip_init(void)
{
    //0. check reboot cause
    ln_chip_get_reboot_cause();
    //1.sys clock,interrupt
    SetSysClock();
    set_interrupt_priority();
    switch_global_interrupt(HAL_ENABLE);
    ln_runtime_measure_init();
    //2. register os heap mem
    OS_DefineHeapRegions();
    //log_init();
    cm_backtrace_init("combo_mcu_basic_example", "hw", "sw");

    if (NVDS_ERR_OK != ln_nvds_init(NVDS_SPACE_OFFSET))
    {
        LOG(LOG_LVL_ERROR, "NVDS init filed!\r\n");
    }

    if (KV_ERR_NONE
            != ln_kv_port_init(KV_SPACE_OFFSET,
                               (KV_SPACE_OFFSET + KV_SPACE_SIZE)))
    {
        LOG(LOG_LVL_ERROR, "KV init filed!\r\n");
    }

    //init system parameter
    sysparam_integrity_check_all();

    //at init
    if (LN_AT_ERR_NONE != ln_at_init()) {
        LOG(LOG_LVL_ERROR, "ln at init fail.\r\n");
        return -1;
    }   

    //3.rf preprocess,img cal
    wifi_rf_calibration();
    //Init wifi stack.
    wifi_init();
    //Init lwip stack.
    lwip_tcpip_init();
    {
        uint8_t mac[6] = { 0 };
        ln_generate_random_mac(mac);
        mac[5] |= 0xC0; // This address is random generated, so assign 0x11 => Static Random Address
        extern void rw_init(uint8_t mac[6]);
        rw_init(mac);
    }
    return AIIO_OK;
}



