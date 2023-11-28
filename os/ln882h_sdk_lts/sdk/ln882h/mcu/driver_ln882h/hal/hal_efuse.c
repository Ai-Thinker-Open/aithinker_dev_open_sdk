/**
 * @file     hal_efuse.c
 * @author   BSP Team 
 * @brief    This file provides EFUSE function.
 * @version  0.0.0.1
 * @date     2021-10-20
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/* Includes ------------------------------------------------------------------*/

#include "hal/hal_efuse.h"


void hal_efuse_write_shadow_reg(efuse_reg_t reg_x,uint32_t reg_value)
{
    uint32_t key1 = 0x8C9DAEBF;
    uint32_t key2 = 0x13141516;

    efuse_key1_set(key1);
    efuse_key2_set(key2);

    efuse_pwr_ctrl1_set(0x0c);                  // LDO parameter setting.
    efuse_pwr_ctrl0_set(0x01);                  // avdd enable

    *(volatile uint32_t*)(EFUSE_BASE + 0x20 + (reg_x << 2)) = reg_value;

    while((efuse_status_get() & 0x01));         // wait when busy.

    efuse_pwr_ctrl0_set(0x00);                  // avdd disable

    for (volatile uint32_t t = 0; t < 1600; t++) // delay 
    {
        __NOP();
    }
    efuse_pwr_ctrl1_set(0x00);
}

uint32_t hal_efuse_read_shadow_reg(efuse_reg_t reg_x)
{
    uint32_t reg_value;

    efuse_rd_trig_set(0x01);              // trigger read.

    while((efuse_status_get() & 0x01));   // wait when busy.

    reg_value = *(volatile uint32_t*)(EFUSE_BASE + 0x20 + (reg_x << 2));

    return reg_value;
}

uint32_t hal_efuse_read_corrent_reg(efuse_reg_t reg_x)
{
    uint32_t reg_value;

    efuse_rd_trig_set(0x01);              // trigger read.

    while((efuse_status_get() & 0x01));   // wait when busy.

    reg_value = *(volatile uint32_t*)(EFUSE_BASE + 0x40 + (reg_x << 2));

    return reg_value;
}




