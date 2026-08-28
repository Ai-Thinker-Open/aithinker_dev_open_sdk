// /**
//  * @file     pulse_cap.c
//  * @author   BSP Team
//  * @brief
//  * @version  0.0.0.1
//  * @date     2023-02-16
//  *
//  * @copyright Copyright (c) 2023 Shanghai Lightning Semiconductor Technology Co. Ltd
//  *
//  */

// #include "pwm_cap.h"

// static uint32_t pulse_cap_port = 0;
// static uint32_t pulse_cap_pin = 0;
// static uint32_t *result_arr = NULL;
// static uint32_t result_buf = 0;
// static uint32_t result_len = NULL;
// static uint32_t result_cnt = 0;
// static uint8_t cap_edge = 0;

// PULSE_CAP_STA cap_sta = PULSE_CAP_NONE;

// /**
//  * @brief 脉冲捕获初始化
//  *
//  * @param gpio_base GPIO Port Base
//  * @param pin       GPIO Pin
//  */
// void pulse_cap_init(uint32_t gpio_base, gpio_pin_t pin)
// {
//     pulse_cap_port = gpio_base;
//     pulse_cap_pin = pin;

//     gpio_init_t_def gpio_init;
//     memset(&gpio_init, 0, sizeof(gpio_init));
//     gpio_init.dir = GPIO_INPUT;
//     gpio_init.pin = pulse_cap_pin;
//     gpio_init.speed = GPIO_HIGH_SPEED;
//     hal_gpio_init(pulse_cap_port, &gpio_init);
//     hal_gpio_pin_it_cfg(pulse_cap_port, pulse_cap_pin, GPIO_INT_RISING);
//     hal_gpio_pin_it_en(pulse_cap_port, pulse_cap_pin, HAL_DISABLE);

//     if (pulse_cap_port == GPIOA_BASE)
//     {
//         NVIC_SetPriority(GPIOA_IRQn, 0);
//         NVIC_EnableIRQ(GPIOA_IRQn);
//     }
//     if (pulse_cap_port == GPIOB_BASE)
//     {
//         NVIC_SetPriority(GPIOB_IRQn, 0);
//         NVIC_EnableIRQ(GPIOB_IRQn);
//     }

//     tim_init_t_def tim_init;
//     memset(&tim_init, 0, sizeof(tim_init));
//     tim_init.tim_mode = TIM_USER_DEF_CNT_MODE;
//     tim_init.tim_div = 40 - 1;
//     tim_init.tim_load_value = 0xFFFFFF;

//     hal_tim_init(TIMER2_BASE, &tim_init);
//     hal_tim_en(TIMER2_BASE, HAL_DISABLE);
//     hal_tim_it_cfg(TIMER2_BASE, TIM_IT_FLAG_ACTIVE, HAL_ENABLE);

//     NVIC_SetPriority(TIMER2_IRQn, 4);
//     NVIC_DisableIRQ(TIMER2_IRQn);
// }

// /**
//  * @brief 开始脉冲捕获
//  *
//  * @param time_arr 捕获数据指针
//  * @param len      捕获的数据长度
//  */
// void pulse_cap_start(uint32_t *time_arr, uint32_t len)
// {
//     result_arr = time_arr;
//     result_len = len;
//     result_cnt = 0;
//     cap_sta = 0;
//     cap_edge = 0;
//     hal_gpio_pin_it_cfg(pulse_cap_port, pulse_cap_pin, GPIO_INT_RISING);
//     hal_gpio_pin_it_en(pulse_cap_port, pulse_cap_pin, HAL_ENABLE);
//     cap_sta = PULSE_CAP_WAIT;
// }

// /**
//  * @brief 停止脉冲捕获
//  *
//  */
// void pulse_cap_stop()
// {
//     hal_gpio_pin_it_en(pulse_cap_port, pulse_cap_pin, HAL_DISABLE);
// }

// /**
//  * @brief 获取脉冲捕获结果
//  *
//  * @return PULSE_CAP_STA 返回结果
//  */
// PULSE_CAP_STA pulse_cap_get_result()
// {
//     if (cap_sta == PULSE_CAP_DONE)
//     {
//         result_arr[0] = 0xFFFFFF;
//         for (int i = 0; i < result_len - 1; i++)
//         {
//             result_arr[i] = result_arr[i] - result_arr[i + 1];
//         }
//         result_arr[result_len - 1] = result_arr[result_len - 1] - result_buf;
//         cap_sta = PULSE_CAP_SUCESS;
//     }
//     return cap_sta;
// }

// void GPIOA_IRQHandler()
// {
//     if (hal_gpio_pin_get_it_flag(pulse_cap_port, pulse_cap_pin) == HAL_SET)
//     {
//         hal_gpio_pin_clr_it_flag(pulse_cap_port, pulse_cap_pin);
//         hal_tim_en(TIMER2_BASE, HAL_ENABLE);

//         if (result_cnt < result_len)
//         {
//             result_arr[result_cnt++] = hal_tim_get_current_cnt_value(TIMER2_BASE);
//         }
//         else
//         {
//             result_buf = hal_tim_get_current_cnt_value(TIMER2_BASE);
//             result_cnt++;
//         }
//         if (result_cnt > result_len)
//         {
//             hal_gpio_pin_it_en(pulse_cap_port, pulse_cap_pin, HAL_DISABLE);
//             hal_tim_en(TIMER2_BASE, HAL_DISABLE);
//             cap_sta = PULSE_CAP_DONE;
//         }
//         if (cap_edge == 0)
//         {
//             hal_gpio_pin_it_cfg(pulse_cap_port, pulse_cap_pin, GPIO_INT_FALLING);
//             cap_edge = 1;
//         }
//         else
//         {
//             hal_gpio_pin_it_cfg(pulse_cap_port, pulse_cap_pin, GPIO_INT_RISING);
//             cap_edge = 0;
//         }
//     }
// }

// void GPIOB_IRQHandler()
// {
//     if (hal_gpio_pin_get_it_flag(pulse_cap_port, pulse_cap_pin) == HAL_SET)
//     {
//         hal_gpio_pin_clr_it_flag(pulse_cap_port, pulse_cap_pin);
//         hal_tim_en(TIMER2_BASE, HAL_ENABLE);

//         if (result_cnt < result_len)
//         {
//             result_arr[result_cnt++] = hal_tim_get_current_cnt_value(TIMER2_BASE);
//         }
//         else
//         {
//             result_buf = hal_tim_get_current_cnt_value(TIMER2_BASE);
//             result_cnt++;
//         }
//         if (result_cnt > result_len)
//         {
//             hal_gpio_pin_it_en(pulse_cap_port, pulse_cap_pin, HAL_DISABLE);
//             hal_tim_en(TIMER2_BASE, HAL_DISABLE);
//             cap_sta = PULSE_CAP_DONE;
//         }
//         if (cap_edge == 0)
//         {
//             hal_gpio_pin_it_cfg(pulse_cap_port, pulse_cap_pin, GPIO_INT_FALLING);
//             cap_edge = 1;
//         }
//         else
//         {
//             hal_gpio_pin_it_cfg(pulse_cap_port, pulse_cap_pin, GPIO_INT_RISING);
//             cap_edge = 0;
//         }
//     }
// }
