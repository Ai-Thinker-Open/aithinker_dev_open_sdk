/**
 * @file     ln_drv_pwm.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-01
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
#ifndef __LN_DRV_PWM_H
#define __LN_DRV_PWM_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "hal/hal_adv_timer.h"
#include "hal/hal_gpio.h"
#include "hal/hal_clock.h"

typedef enum
{
    PWM_CHA_0 = 0,
    PWM_CHA_1 = 1,
    
    PWM_CHA_2 = 2,
    PWM_CHA_3 = 3,
    
    PWM_CHA_4 = 4,
    PWM_CHA_5 = 5,

    PWM_CHA_6 = 6,
    PWM_CHA_7 = 7,
    
    PWM_CHA_8 = 8,
    PWM_CHA_9 = 9,
    
    PWM_CHA_10 = 10,
    PWM_CHA_11 = 11,
}pwm_channel_t;


typedef enum
{
    GPIO_A = 0,
    GPIO_B = 1,
}gpio_port_t;

void     pwm_init(uint32_t freq, float duty,pwm_channel_t pwm_channel_num,gpio_port_t gpio_port,gpio_pin_t gpio_pin);
void     pwm_start(pwm_channel_t pwm_channel_num);
void     pwm_set_duty(float duty, pwm_channel_t pwm_channel_num);
float    pwm_get_duty(pwm_channel_t pwm_channel_num);
void     pwm_set_freq(pwm_channel_t pwm_channel_num,uint32_t period);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_PWM_H
