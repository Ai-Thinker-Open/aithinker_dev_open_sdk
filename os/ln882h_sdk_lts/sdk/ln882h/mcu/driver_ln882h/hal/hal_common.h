/**
 * @file     hal_common.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-17
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __HAL_COMMON_H__
#define __HAL_COMMON_H__

#ifdef __cpluscplus
    extern "C" {
#endif // __cpluscplus

#include "proj_config.h"
#include "ln_types.h"
#include "ln882h.h"


#include "reg_sysc_awo.h"
#include "reg_sysc_cmp.h"

        
//---------------------HAL define------------------------------
/* enum definitions */
typedef enum {
    HAL_DISABLE  = 0u, 
    HAL_ENABLE   = 1u
} hal_en_t;

typedef enum {
    HAL_RESET    = 0u, 
    HAL_SET      = 1u
} hal_flag_t, hal_it_status_t;

typedef enum 
{
    HAL_OK       = 0u,
    HAL_ERROR    = 1u,
    HAL_BUSY     = 2u,
    HAL_TIMEOUT  = 3u
} hal_status_t;

typedef enum 
{
    HAL_UNLOCKED = 0u,
    HAL_LOCKED   = 1u  
} hal_lock_t;

#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == HAL_DISABLE) || ((STATE) == HAL_ENABLE))


#if (defined(HAL_ASSERT_EN) && (HAL_ASSERT_EN))
/**
  * @brief  The hal_assert macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed. 
  *         If expr is true, it returns no value.
  * @retval None
  */
    void assert_failed(uint8_t* file, uint32_t line);
    #define hal_assert(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
#else
    #define hal_assert(expr) ((void)0U)
#endif /* HAL_ASSERT_EN */  


#ifdef __cpluscplus
    }
#endif // __cpluscplus
#endif // __HAL_COMMON_H_
