/**
 * @file     hal_efuse.h
 * @author   BSP Team 
 * @brief    This file contains all of EFUSE functions prototypes.
 * @version  0.0.0.1
 * @date     2021-10-20
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __HAL_EFUSE_H
#define __HAL_EFUSE_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "reg_efuse.h"
#include "hal/hal_common.h"

typedef enum
{
    EFUSE_REG_0     = 0,     
    EFUSE_REG_1     = 1,     
    EFUSE_REG_2     = 2,     
    EFUSE_REG_3     = 3,     
    EFUSE_REG_4     = 4,     
    EFUSE_REG_5     = 5,     
    EFUSE_REG_6     = 6,     
    EFUSE_REG_7     = 7,     
      
}efuse_reg_t;     

void        hal_efuse_write_shadow_reg(efuse_reg_t reg_x,uint32_t reg_value);
uint32_t    hal_efuse_read_shadow_reg(efuse_reg_t reg_x);
uint32_t    hal_efuse_read_corrent_reg(efuse_reg_t reg_x);        


#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __HAL_EFUSE_H
