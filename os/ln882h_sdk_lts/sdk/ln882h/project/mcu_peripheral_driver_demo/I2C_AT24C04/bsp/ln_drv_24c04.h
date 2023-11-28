/**
 * @file     ln_drv_24c04.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-04
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


#ifndef __LN_DRV_24C04_H
#define __LN_DRV_24C04_H 

#include "hal/hal_common.h"
#include "ln_drv_i2c.h"


#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

int i2c_24c04_read(uint16_t word_addr, uint8_t *buf, uint16_t buf_len);
int i2c_24c04_write(uint16_t word_addr, const uint8_t *buf, uint16_t buf_len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_24C04_H
