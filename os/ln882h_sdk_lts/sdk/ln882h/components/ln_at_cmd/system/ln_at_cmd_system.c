/**
 * @file   ln_at_cmd_system.c
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-02-01     MurphyZhao   the first version
 */

#include "ln_at.h"
#include "ln_at_cmd/ln_at_cmd_version.h"
#include "hal/hal_misc.h"
#include "wifi.h"
#include "ln_utils.h"
#include "ln882h.h"
#include "utils/reboot_trace/reboot_trace.h"
#include "utils/system_parameter.h"
#include "ln_at_cmd.h"

#include "serial.h"
#include "serial_hw.h"
#include "sys_driver_port.h"
#include "utils/power_mgmt/ln_pm.h"


/**
 * name, get, set, test, exec
*/

/* declarations */
ln_at_err_t ln_at_exec_sys_rst(const char *name);
ln_at_err_t ln_at_exec_gmr(const char *name);

ln_at_err_t ln_at_exec_sys_rst(const char *name)
{
    LN_UNUSED(name);
    ln_chip_reboot();

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(RST, NULL, NULL, NULL, ln_at_exec_sys_rst);

ln_at_err_t ln_at_exec_gmr(const char *name)
{
    LN_UNUSED(name);

    ln_at_printf("AT       version: v%d.%d.%d\r\n", LN_AT_VERSION, LN_AT_SUBVERSION, LN_AT_REVISION);
    ln_at_printf("AT CMD   version: v%d.%d.%d\r\n", LN_AT_CMD_VERSION, LN_AT_CMD_SUBVERSION, LN_AT_CMD_REVISION);
    ln_at_printf("SDK      version: v%s\r\n", LN882H_SDK_VERSION_STRING);
    ln_at_printf("WiFi lib version: %s[build time:%s]\r\n",
            wifi_lib_version_string_get(), wifi_lib_build_time_get());

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(GMR, NULL, NULL, NULL, ln_at_exec_gmr);

ln_at_err_t ln_at_exec_ble_fw_start(const char *name)
{
    LN_UNUSED(name);

    ln_at_printf("Will jump to ble code area, can't jump back!\r\n"); 
    ln_at_printf(LN_AT_RET_OK_STR);

    hal_misc_awo_set_r_idle_reg(0xaeaebebe);
    __NVIC_SystemReset();
    while(1);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(BLE_START, NULL, NULL, NULL, ln_at_exec_ble_fw_start);

ln_at_err_t ln_at_set_sysstore(uint8_t para_num, const char *name);
ln_at_err_t ln_at_get_sysstore(const char *name);

ln_at_err_t ln_at_set_sysstore(uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;
    int param = 0;
    uint8_t enable = 0;
    
    if(para_num != 1)
    {
        goto __err;
    }
    
    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &param))
    {
        goto __err;
    }

    if(param != 1 && param != 0)
    {
        goto __err;
    }

    enable = param;
    if(0 != sysparam_sys_store_update(enable))
    {
        goto __err;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
__err:

    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

ln_at_err_t ln_at_get_sysstore(const char *name)
{
    uint8_t enable = 0;

    if(0 != sysparam_sys_store_get(&enable))
    {
        goto __err;
    }

    ln_at_printf("%s:%d\r\n", name, enable);
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:

    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;    

}

LN_AT_CMD_REG(SYSSTORE, ln_at_get_sysstore, ln_at_set_sysstore, NULL, NULL);


//************************
//UART
//************************
static ln_at_err_t ln_at_set_uart(ln_at_cmd_type_e type, uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;
    int baud = 0;
    int databit = 0;
    int stopbit = 0;
    int parity = 0;
    int flow = 0;

   at_sys_adapter_t *pada = at_sys_adapter_get_handle();
    
    if(para_num != 5)
    {
        goto __err;
    }
    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &baud))
    {
        goto __err;
    }
    if(baud < 50 || baud > 5000000)
    {
        goto __err;
    }
    
    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &databit))
    {
        goto __err;
    }
    if(databit < 5 || databit > 8)
    {
        goto __err;
    }
    
    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &stopbit))
    {
        goto __err;
    }
    if(stopbit < 1 || stopbit > 3)
    {
        goto __err;
    }
    
    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &parity))
    {
        goto __err;
    }
    if(parity < 0 || parity > 2)
    {
        goto __err;
    }
    
    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &flow))
    {
        goto __err;
    }
    if(flow < 0 || flow > 3)
    {
        goto __err;
    }

    pada->sys_status.uart_status.baud = (uint32_t)baud;
    pada->sys_status.uart_status.databit = (uint8_t)databit;
    pada->sys_status.uart_status.flow = (uint8_t)flow;
    pada->sys_status.uart_status.parity = (uint8_t)parity;
    pada->sys_status.uart_status.stopbit = (uint8_t)stopbit;

    pada->sys_func.uart_func.uart_set_baud(baud);
    
    if(type == LN_AT_CMD_TYPE_DEF)
    {
        sysparam_sys_uart_update(&(pada->sys_status.uart_status));
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;

}
static ln_at_err_t ln_at_set_uart_cur(uint8_t para_num, const char *name)
{
    return ln_at_set_uart(LN_AT_CMD_TYPE_CUR, para_num, name);
}

static ln_at_err_t ln_at_set_uart_def(uint8_t para_num, const char *name)
{
    return ln_at_set_uart(LN_AT_CMD_TYPE_DEF, para_num, name);
}

static ln_at_err_t ln_at_get_uart(ln_at_cmd_type_e type, const char *name)
{
    if(type == LN_AT_CMD_TYPE_CUR)
    {
        at_sys_adapter_t *pada = at_sys_adapter_get_handle();
        ln_at_printf("%s:%d,%d,%d,%d,%d\r\n", name, pada->sys_status.uart_status.baud, pada->sys_status.uart_status.databit, \
            pada->sys_status.uart_status.stopbit, pada->sys_status.uart_status.parity, pada->sys_status.uart_status.flow);
    }
    else if(type == LN_AT_CMD_TYPE_DEF)
    {
        sys_uart_status_t uart_cfg;
        sysparam_sys_uart_get(&uart_cfg);
        ln_at_printf("%s:%d,%d,%d,%d,%d\r\n", name, uart_cfg.baud, uart_cfg.databit, \
            uart_cfg.stopbit, uart_cfg.parity, uart_cfg.flow);
    }
    
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_at_get_uart_cur(const char *name)
{
    return ln_at_get_uart(LN_AT_CMD_TYPE_CUR, name);
}

static ln_at_err_t ln_at_get_uart_def(const char *name)
{
    return ln_at_get_uart(LN_AT_CMD_TYPE_DEF, name);
}

//UART_DEF
LN_AT_CMD_REG(UART_CUR,     ln_at_get_uart_cur,         ln_at_set_uart_cur,         NULL,         NULL);
LN_AT_CMD_REG(UART_DEF,     ln_at_get_uart_def,         ln_at_set_uart_def,         NULL,         NULL);


//************************
//RESTORE
//************************
static ln_at_err_t ln_at_exec_restore(const char *name)
{
    LN_UNUSED(name);

    if(SYSPARAM_ERR_NONE != sysparam_factory_reset_all())
    {
        ln_at_printf(LN_AT_RET_FAIL_STR);
        return LN_AT_ERR_COMMON;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    ln_chip_reboot();
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(RESTORE, NULL, NULL, NULL, ln_at_exec_restore);

//************************
//GSLP
//************************
ln_at_err_t ln_at_set_gslp(uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;
    int sleeptime = 0;
    
    if(para_num >= 1)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &sleeptime))
        {
            goto __err;
        }
        if(is_default)
        {
            goto __err;
        }
    }
    ln_at_printf(LN_AT_RET_OK_STR);
    ln_pm_sleep_mode_set(DEEP_SLEEP);
    ln_pm_rtos_pre_sleep_proc((uint32_t *)&sleeptime);
    OS_MsDelay((uint32_t)sleeptime);
    ln_chip_reboot();
    return LN_AT_ERR_NONE;
__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}


LN_AT_CMD_REG(GSLP, NULL, ln_at_set_gslp, NULL, NULL);

