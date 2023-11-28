/**
 * @file     ln_drv_ext.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-09
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_ext.h"


void (*ext_it_handler)(void);

/**
    EXT_INT_SENSE_0    ->    PA0
    EXT_INT_SENSE_1    ->    PA1
    EXT_INT_SENSE_2    ->    PA2
    EXT_INT_SENSE_3    ->    PA3
    EXT_INT_SENSE_4    ->    PA5
    EXT_INT_SENSE_5    ->    PA6
    EXT_INT_SENSE_6    ->    PA7
    EXT_INT_SENSE_7    ->    PB9
*/

/**
 * @brief 设置外部中断初始化
 * 
 * @param ext_int_sense   设置中断线
 * @param ext_trig_mode   设置中断模式
 * @param ext_it_callback 设置中断回调函数
 */
void ext_init(ext_int_sense_t ext_int_sense,ext_trig_mode_t ext_trig_mode,void (*ext_it_callback)(void))
{
    switch(ext_int_sense)
    {
        case EXT_INT_SENSE_0: 
            if(ext_trig_mode == EXT_INT_POSEDEG)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_0,GPIO_PULL_DOWN);
            else if(ext_trig_mode == EXT_INT_NEGEDGE)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_0,GPIO_PULL_UP);
            break;

        case EXT_INT_SENSE_1:   
            if(ext_trig_mode == EXT_INT_POSEDEG)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_1,GPIO_PULL_DOWN);
            else if(ext_trig_mode == EXT_INT_NEGEDGE)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_1,GPIO_PULL_UP);
            break;

        case EXT_INT_SENSE_2:   
            if(ext_trig_mode == EXT_INT_POSEDEG)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_2,GPIO_PULL_DOWN);
            else if(ext_trig_mode == EXT_INT_NEGEDGE)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_2,GPIO_PULL_UP);
            break;

        case EXT_INT_SENSE_3:   
            if(ext_trig_mode == EXT_INT_POSEDEG)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_3,GPIO_PULL_DOWN);
            else if(ext_trig_mode == EXT_INT_NEGEDGE)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_3,GPIO_PULL_UP);
            break;

        case EXT_INT_SENSE_4:   
            if(ext_trig_mode == EXT_INT_POSEDEG)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_5,GPIO_PULL_DOWN);
            else if(ext_trig_mode == EXT_INT_NEGEDGE)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_5,GPIO_PULL_UP);
            break;

        case EXT_INT_SENSE_5:   
            if(ext_trig_mode == EXT_INT_POSEDEG)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_6,GPIO_PULL_DOWN);
            else if(ext_trig_mode == EXT_INT_NEGEDGE)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_6,GPIO_PULL_UP);
            break;

        case EXT_INT_SENSE_6:   
            if(ext_trig_mode == EXT_INT_POSEDEG)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_7,GPIO_PULL_DOWN);
            else if(ext_trig_mode == EXT_INT_NEGEDGE)  
                hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_7,GPIO_PULL_UP);
            break;

        case EXT_INT_SENSE_7:   
            if(ext_trig_mode == EXT_INT_POSEDEG)  
                hal_gpio_pin_pull_set(GPIOB_BASE,GPIO_PIN_9,GPIO_PULL_DOWN);
            else if(ext_trig_mode == EXT_INT_NEGEDGE)  
                hal_gpio_pin_pull_set(GPIOB_BASE,GPIO_PIN_9,GPIO_PULL_UP);
            break;
    }

    hal_ext_init(ext_int_sense,ext_trig_mode,HAL_ENABLE);  
    NVIC_SetPriority(EXT_IRQn,1);                           
    NVIC_EnableIRQ(EXT_IRQn);  

    if(ext_it_callback != NULL)
    {
        ext_it_handler = ext_it_callback;
    }
}

void EXT_IRQHandler()
{
    if(hal_ext_get_it_flag(EXT_INT_SENSE_0_IT_FLAG) == HAL_SET)
        hal_ext_clr_it_flag(EXT_INT_SENSE_0_IT_FLAG);

    if(hal_ext_get_it_flag(EXT_INT_SENSE_1_IT_FLAG) == HAL_SET)
        hal_ext_clr_it_flag(EXT_INT_SENSE_1_IT_FLAG);

    if(hal_ext_get_it_flag(EXT_INT_SENSE_2_IT_FLAG) == HAL_SET)
        hal_ext_clr_it_flag(EXT_INT_SENSE_2_IT_FLAG);

    if(hal_ext_get_it_flag(EXT_INT_SENSE_3_IT_FLAG) == HAL_SET)
        hal_ext_clr_it_flag(EXT_INT_SENSE_3_IT_FLAG);

    if(hal_ext_get_it_flag(EXT_INT_SENSE_4_IT_FLAG) == HAL_SET)
        hal_ext_clr_it_flag(EXT_INT_SENSE_4_IT_FLAG);

    if(hal_ext_get_it_flag(EXT_INT_SENSE_5_IT_FLAG) == HAL_SET)
        hal_ext_clr_it_flag(EXT_INT_SENSE_5_IT_FLAG);

    if(hal_ext_get_it_flag(EXT_INT_SENSE_6_IT_FLAG) == HAL_SET)
        hal_ext_clr_it_flag(EXT_INT_SENSE_6_IT_FLAG);

    if(hal_ext_get_it_flag(EXT_INT_SENSE_7_IT_FLAG) == HAL_SET)
        hal_ext_clr_it_flag(EXT_INT_SENSE_7_IT_FLAG);

    if(ext_it_handler != NULL)
        ext_it_handler();
}


