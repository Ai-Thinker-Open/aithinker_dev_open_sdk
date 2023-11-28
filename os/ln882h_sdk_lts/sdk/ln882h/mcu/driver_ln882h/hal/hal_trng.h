/**
 * @file     hal_trng.h
 * @author   BSP Team 
 * @brief    This file contains all of TRNG functions prototypes.
 * @version  0.0.0.1
 * @date     2020-12-23
 * 
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


#ifndef __HAL_TRNG_H
#define __HAL_TRNG_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "reg_trng.h"

#define IS_TRNG_ALL_PERIPH(PERIPH) (((PERIPH) == TRNG_BASE))

typedef enum
{
    TRNG_DIS     = 0,     
    TRNG_EN      = 1,        
}trng_hal_en_t;

#define IS_TRNG_EN_STATUS(EN)  (((EN) == TRNG_DIS)  || ((EN) == TRNG_EN)) 

typedef enum
{
    TRNG_FAST_MODE_DIS     = 0,     
    TRNG_FAST_MODE_EN      = 1,        
}trng_fast_mode_hal_en_t;

#define IS_TRNG_FAST_MODE_EN_STATUS(EN)  (((EN) == TRNG_FAST_MODE_DIS)  || ((EN) == TRNG_FAST_MODE_EN)) 

typedef enum
{
    TRNG_IT_FLAG_TRNG_DONE           = 1,
}trng_it_flag_t;

#define TRNG_IT_FLAG_BIT_NUM         1
#define IS_TRNG_IT_FLAG(FLAG)        (((((uint32_t)FLAG) & 0x01) != 0x00U) && ((((uint32_t)FLAG) & ~0x01) == 0x00U))


#define IS_TRNG_GAP_VALUE(VALUE)     (((VALUE) <= 0x0F)) 


typedef struct
{
    trng_fast_mode_hal_en_t      trng_fast_mode_en_status;   /*!< Specifies the trng fast mode enable status.
                                                                This parameter can be a value of @trng_fast_mode_hal_en_t */

    uint8_t                         trng_gap;                   /*!< Specifies the trng gap value.
                                                                The range of parameters can be referred to @ref IS_TRNG_GAP_VALUE */ 

}trng_init_t_def;



            //trng init and config
void        hal_trng_init(uint32_t trng_base,trng_init_t_def* trng_init);
void        hal_trng_deinit(void);
void        hal_trng_en(uint32_t trng_base,hal_en_t en);
uint8_t     hal_trng_get_en_status(uint32_t trng_base);
void        hal_trng_set_gap(uint32_t trng_base,uint8_t gap);
uint32_t    hal_trng_get_data_0_31(uint32_t trng_base);
uint32_t    hal_trng_get_data_32_63(uint32_t trng_base);
uint32_t    hal_trng_get_data_64_95(uint32_t trng_base);
uint32_t    hal_trng_get_data_96_127(uint32_t trng_base);

            //trng it config
void        hal_trng_it_cfg(uint32_t trng_base,trng_it_flag_t trng_it_flag ,hal_en_t en);
uint8_t     hal_trng_get_it_flag(uint32_t trng_base,trng_it_flag_t trng_it_flag);
void        hal_trng_clr_it_flag(uint32_t trng_base,trng_it_flag_t trng_it_flag);




#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __HAL_TRNG_H







