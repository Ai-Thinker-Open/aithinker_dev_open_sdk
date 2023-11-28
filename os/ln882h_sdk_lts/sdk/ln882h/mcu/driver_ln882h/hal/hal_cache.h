/**
 * @file     hal_cache.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-17
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __HAL_CACHE_H
#define __HAL_CACHE_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#include "hal/hal_common.h"
#include "hal/hal_qspi.h"

#include "reg_cache.h"
#include "reg_qspi.h"

void  flash_cache_init(uint32_t flash_base_addr);
void  flash_cache_disable(void);
void  flash_cache_flush(uint32_t low_addr,uint32_t high_addr);
void  flash_cache_flush_all(void);

#ifdef __cplusplus
}
#endif // __cplusplus
    
#endif /* __HAL_CACHE_H */


