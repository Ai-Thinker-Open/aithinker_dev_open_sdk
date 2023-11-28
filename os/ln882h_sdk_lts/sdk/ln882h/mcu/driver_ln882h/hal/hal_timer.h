/**
 * @file     hal_timer.h
 * @author   BSP Team 
 * @brief    This file contains all of TIMER functions prototypes.
 * @version  0.0.0.1
 * @date     2020-12-19
 * 
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */



#ifndef __HAL_TIMER_H
#define __HAL_TIMER_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "reg_timer.h"
        
/* TIMERx(x=0,1,2,3) definitions */

#define TIMER0_BASE                 (TIMER_BASE + 0x00000000U)
#define TIMER1_BASE                 (TIMER_BASE + 0x00000018U)
#define TIMER2_BASE                 (TIMER_BASE + 0x00000030U)

/* The TIMER3 will be used by WiFi Lib !!!*/
#define TIMER3_BASE                 (TIMER_BASE + 0x00000048U)


#define IS_TIMER_ALL_PERIPH(PERIPH) (((PERIPH) == TIMER0_BASE) || ((PERIPH) == TIMER1_BASE) || \
                                     ((PERIPH) == TIMER2_BASE) || ((PERIPH) == TIMER3_BASE))
                
typedef enum
{
    TIM_IT_NO_MASK    = 0,
    TIM_IT_MASK       = 1,
}tim_it_mask_sta_t;

#define IS_TIM_IT_MASK_STA(STA) (((STA) == TIM_IT_NO_MASK) || ((STA) == TIM_IT_MASK))

typedef enum
{
    TIM_FREE_RUNNING_MODE       = 0,
    TIM_USER_DEF_CNT_MODE       = 1,
}tim_mode_t;

#define IS_TIM_MODE(MODE)           (((MODE) == TIM_FREE_RUNNING_MODE) || ((MODE) == TIM_USER_DEF_CNT_MODE))

#define IS_TIM_LOAD_VALUE(VALUE)     (((VALUE) <= 0xFFFFFF))



/*this is a 8bit value, 0---not div  1--div2  2--div3  ...*/
#define IS_TIM_DIV_VALUE(VALUE)     (((VALUE) <= 0xFF))

typedef enum
{
    TIM_IT_FLAG_ACTIVE           = 0x01,
}tim_it_flag_t;

#define TIM_IT_FLAG_BIT_NUM         1
#define IS_TIM_IT_FLAG(FLAG)        (((((uint32_t)FLAG) & 0x01) != 0x00U) && ((((uint32_t)FLAG) & ~0x01) == 0x00U))


typedef struct
{

    tim_mode_t              tim_mode;                       /*!< Specifies the timer mode.
                                                            This parameter can be a value of @ref tim_mode_t */

    uint32_t                tim_load_value;                        /*!< Specifies the timer count.
                                                            The range of parameters can be referred to @ref IS_WDT_TOP_VALUE */
                                                            
    uint32_t                tim_load2_value;                        /*!< Specifies the timer count.
                                                            The range of parameters can be referred to @ref IS_WDT_TOP_VALUE */

    uint8_t                 tim_div;                        /*!< Specifies the timer clock div.
                                                            The range of parameters can be referred to @ref IS_TIM_DIV_VALUE */
                                                            /*!< this is a 8bit value, 0---not div  1--div2  2--div3      ...*/

}tim_init_t_def;

            //timer init
void        hal_tim_init(uint32_t tim_x_base,tim_init_t_def* tim_init);
void        hal_tim_deinit(void);
void        hal_tim_en(uint32_t tim_x_base,hal_en_t en);
void        hal_tim_set_load_value(uint32_t tim_x_base,uint32_t value);
void        hal_tim_set_load2_value(uint32_t tim_x_base,uint32_t value);
uint32_t    hal_tim_get_current_cnt_value(uint32_t tim_x_base);
void        hal_tim_pwm_en(uint32_t tim_x_base,hal_en_t en);
uint8_t     hal_tim_get_div(uint32_t tim_x_base);
uint32_t    hal_tim_get_load_value(uint32_t tim_x_base);
uint32_t    hal_tim_get_load2_value(uint32_t tim_x_base);

            //timer interrupt init
void        hal_tim_it_cfg(uint32_t tim_x_base,tim_it_flag_t tim_it_flag,hal_en_t en);
uint8_t     hal_tim_get_it_flag(uint32_t tim_x_base,tim_it_flag_t tim_it_flag);
void        hal_tim_clr_it_flag(uint32_t tim_x_base,tim_it_flag_t tim_it_flag);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __HAL_TIMER_H

