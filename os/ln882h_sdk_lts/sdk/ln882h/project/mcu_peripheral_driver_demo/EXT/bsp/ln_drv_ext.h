/**
 * @file     ln_drv_ext.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-09
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __LN_DRV_EXT_H
#define __LN_DRV_EXT_H


#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "hal_ext.h"
#include "hal_gpio.h"

void ext_init(ext_int_sense_t ext_int_sense,ext_trig_mode_t ext_trig_mode,void (*ext_it_callback)(void));

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_EXT_H


