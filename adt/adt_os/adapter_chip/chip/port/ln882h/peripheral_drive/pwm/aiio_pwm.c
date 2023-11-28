/** @brief PWM intermediate device port adaptation
 *
 *  @file        aiio_pwm.c
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/02/06          <td>V1.0.0          <td>hewm            <td>PWM device port adaptation
 *  <table>
 *
 */
// #include "stdint.h"

// #include "hal/hal_common.h"
// #include "hal/hal_adv_timer.h"
// #include "hal/hal_gpio.h"
// #include "hal/hal_clock.h"

// #include "aiio_errcode.h"
// #include "aiio_gpio.h"
// #include "aiio_pwm.h"

// static void pwm_init(uint32_t freq, float duty,aiio_pwm_chann_t pwm_channel_num,uint32_t gpio_port,gpio_pin_t gpio_pin)
// {
//     uint32_t reg_base = 0;
//     uint32_t gpio_reg_base = 0;

//     /* PWM引脚初始化 */
//     switch(gpio_port)
//     {
//         case 0: gpio_reg_base = GPIOA_BASE; break;
//         case 1: gpio_reg_base = GPIOB_BASE; break;
//     }

//     hal_gpio_pin_afio_select(gpio_reg_base,gpio_pin,(afio_function_t)(ADV_TIMER_PWM0 + pwm_channel_num));
//     hal_gpio_pin_afio_en(gpio_reg_base,gpio_pin,HAL_ENABLE);

//     switch(pwm_channel_num)
//     {
//         case AIIO_PWM_CHA_0: reg_base = ADV_TIMER_0_BASE; break;
//         case AIIO_PWM_CHA_1: reg_base = ADV_TIMER_0_BASE; break;
//         case AIIO_PWM_CHA_2: reg_base = ADV_TIMER_1_BASE; break;
//         case AIIO_PWM_CHA_3: reg_base = ADV_TIMER_1_BASE; break;
//         case AIIO_PWM_CHA_4: reg_base = ADV_TIMER_2_BASE; break;
//         case AIIO_PWM_CHA_5: reg_base = ADV_TIMER_2_BASE; break;
//         case AIIO_PWM_CHA_6: reg_base = ADV_TIMER_3_BASE; break;
//         case AIIO_PWM_CHA_7: reg_base = ADV_TIMER_3_BASE; break;
//         case AIIO_PWM_CHA_8: reg_base = ADV_TIMER_4_BASE; break;
//         case AIIO_PWM_CHA_9: reg_base = ADV_TIMER_4_BASE; break;
//         case AIIO_PWM_CHA_10: reg_base = ADV_TIMER_5_BASE; break;
//         case AIIO_PWM_CHA_11: reg_base = ADV_TIMER_5_BASE; break;
//     }

//     /* PWM参数初始化 */
//     adv_tim_init_t_def adv_tim_init;
//     memset(&adv_tim_init,0,sizeof(adv_tim_init));

//     // 设置PWM频率
//     if(freq >= 10000){
//         adv_tim_init.adv_tim_clk_div = 0;
//         adv_tim_init.adv_tim_load_value =  hal_clock_get_apb0_clk() * 1.0 / freq - 2;
//     }else{
//         adv_tim_init.adv_tim_clk_div = (uint32_t)(hal_clock_get_apb0_clk() / 1000000) - 1;
//         adv_tim_init.adv_tim_load_value =  1000000 / freq - 2;
//     }

//     // 设置PWM占空比
//     if((pwm_channel_num & 0x01) == 0)
//         adv_tim_init.adv_tim_cmp_a_value = (adv_tim_init.adv_tim_load_value + 2) * duty / 100.0f;      //设置通道a比较值
//     else
//         adv_tim_init.adv_tim_cmp_b_value = (adv_tim_init.adv_tim_load_value + 2) * duty / 100.0f;      //设置通道b比较值

//     adv_tim_init.adv_tim_dead_gap_value = 0;                                //设置死区时间
//     adv_tim_init.adv_tim_dead_en        = ADV_TIMER_DEAD_DIS;               //不开启死区
//     adv_tim_init.adv_tim_cnt_mode       = ADV_TIMER_CNT_MODE_INC;           //向上计数模式
//     adv_tim_init.adv_tim_cha_inv_en     = ADV_TIMER_CHA_INV_EN;
//     adv_tim_init.adv_tim_chb_inv_en     = ADV_TIMER_CHB_INV_EN;

//     hal_adv_tim_init(reg_base,&adv_tim_init);
// }

// static void pwm_start(aiio_pwm_chann_t pwm_channel_num)
// {
//     uint32_t reg_base = 0;
//     switch(pwm_channel_num)
//     {
//         case AIIO_PWM_CHA_0: reg_base = ADV_TIMER_0_BASE; break;
//         case AIIO_PWM_CHA_1: reg_base = ADV_TIMER_0_BASE; break;
//         case AIIO_PWM_CHA_2: reg_base = ADV_TIMER_1_BASE; break;
//         case AIIO_PWM_CHA_3: reg_base = ADV_TIMER_1_BASE; break;
//         case AIIO_PWM_CHA_4: reg_base = ADV_TIMER_2_BASE; break;
//         case AIIO_PWM_CHA_5: reg_base = ADV_TIMER_2_BASE; break;
//         case AIIO_PWM_CHA_6: reg_base = ADV_TIMER_3_BASE; break;
//         case AIIO_PWM_CHA_7: reg_base = ADV_TIMER_3_BASE; break;
//         case AIIO_PWM_CHA_8: reg_base = ADV_TIMER_4_BASE; break;
//         case AIIO_PWM_CHA_9: reg_base = ADV_TIMER_4_BASE; break;
//         case AIIO_PWM_CHA_10: reg_base = ADV_TIMER_5_BASE; break;
//         case AIIO_PWM_CHA_11: reg_base = ADV_TIMER_5_BASE; break;
//     }
//     if((pwm_channel_num & 0x01) == 0)
//         hal_adv_tim_a_en(reg_base,HAL_ENABLE);                  //使能通道a
//     else
//         hal_adv_tim_b_en(reg_base,HAL_ENABLE);                  //使能通道b
// }

// static void pwm_set_duty(float duty, aiio_pwm_chann_t pwm_channel_num)
// {
//     uint32_t reg_base = 0;
//     switch(pwm_channel_num)
//     {
//         case AIIO_PWM_CHA_0: reg_base = ADV_TIMER_0_BASE; break;
//         case AIIO_PWM_CHA_1: reg_base = ADV_TIMER_0_BASE; break;
//         case AIIO_PWM_CHA_2: reg_base = ADV_TIMER_1_BASE; break;
//         case AIIO_PWM_CHA_3: reg_base = ADV_TIMER_1_BASE; break;
//         case AIIO_PWM_CHA_4: reg_base = ADV_TIMER_2_BASE; break;
//         case AIIO_PWM_CHA_5: reg_base = ADV_TIMER_2_BASE; break;
//         case AIIO_PWM_CHA_6: reg_base = ADV_TIMER_3_BASE; break;
//         case AIIO_PWM_CHA_7: reg_base = ADV_TIMER_3_BASE; break;
//         case AIIO_PWM_CHA_8: reg_base = ADV_TIMER_4_BASE; break;
//         case AIIO_PWM_CHA_9: reg_base = ADV_TIMER_4_BASE; break;
//         case AIIO_PWM_CHA_10: reg_base = ADV_TIMER_5_BASE; break;
//         case AIIO_PWM_CHA_11: reg_base = ADV_TIMER_5_BASE; break;
//     }
//     if((pwm_channel_num & 0x01) == 0)
//         hal_adv_tim_set_comp_a(reg_base,(hal_adv_tim_get_load_value(reg_base) + 2) * duty / 100.0f);    //设置通道a比较值
//     else
//         hal_adv_tim_set_comp_b(reg_base,(hal_adv_tim_get_load_value(reg_base) + 2) * duty / 100.0f);    //设置通道b比较值
// }

// static float pwm_get_duty(aiio_pwm_chann_t pwm_channel_num)
// {
//     uint32_t reg_base = 0;
//     float    ret_val = 0;
//     switch(pwm_channel_num)
//     {
//         case AIIO_PWM_CHA_0: reg_base = ADV_TIMER_0_BASE; break;
//         case AIIO_PWM_CHA_1: reg_base = ADV_TIMER_0_BASE; break;
//         case AIIO_PWM_CHA_2: reg_base = ADV_TIMER_1_BASE; break;
//         case AIIO_PWM_CHA_3: reg_base = ADV_TIMER_1_BASE; break;
//         case AIIO_PWM_CHA_4: reg_base = ADV_TIMER_2_BASE; break;
//         case AIIO_PWM_CHA_5: reg_base = ADV_TIMER_2_BASE; break;
//         case AIIO_PWM_CHA_6: reg_base = ADV_TIMER_3_BASE; break;
//         case AIIO_PWM_CHA_7: reg_base = ADV_TIMER_3_BASE; break;
//         case AIIO_PWM_CHA_8: reg_base = ADV_TIMER_4_BASE; break;
//         case AIIO_PWM_CHA_9: reg_base = ADV_TIMER_4_BASE; break;
//         case AIIO_PWM_CHA_10: reg_base = ADV_TIMER_5_BASE; break;
//         case AIIO_PWM_CHA_11: reg_base = ADV_TIMER_5_BASE; break;
//     }
//     if((pwm_channel_num & 0x01) == 0)
//         ret_val = hal_adv_tim_get_comp_a(reg_base) * 1.0f / (hal_adv_tim_get_load_value(reg_base) + 2) * 100;
//     else
//         ret_val = hal_adv_tim_get_comp_b(reg_base) * 1.0f / (hal_adv_tim_get_load_value(reg_base) + 2) * 100;

//     return ret_val;
// }

// static void pwm_set_freq(aiio_pwm_chann_t pwm_channel_num,uint32_t freq)
// {
//     uint32_t reg_base = 0;
//     switch(pwm_channel_num)
//     {
//         case AIIO_PWM_CHA_0: reg_base = ADV_TIMER_0_BASE; break;
//         case AIIO_PWM_CHA_1: reg_base = ADV_TIMER_0_BASE; break;
//         case AIIO_PWM_CHA_2: reg_base = ADV_TIMER_1_BASE; break;
//         case AIIO_PWM_CHA_3: reg_base = ADV_TIMER_1_BASE; break;
//         case AIIO_PWM_CHA_4: reg_base = ADV_TIMER_2_BASE; break;
//         case AIIO_PWM_CHA_5: reg_base = ADV_TIMER_2_BASE; break;
//         case AIIO_PWM_CHA_6: reg_base = ADV_TIMER_3_BASE; break;
//         case AIIO_PWM_CHA_7: reg_base = ADV_TIMER_3_BASE; break;
//         case AIIO_PWM_CHA_8: reg_base = ADV_TIMER_4_BASE; break;
//         case AIIO_PWM_CHA_9: reg_base = ADV_TIMER_4_BASE; break;
//         case AIIO_PWM_CHA_10: reg_base = ADV_TIMER_5_BASE; break;
//         case AIIO_PWM_CHA_11: reg_base = ADV_TIMER_5_BASE; break;
//     }
//     if((pwm_channel_num & 0x01) == 0)
//         hal_adv_tim_a_en(reg_base,HAL_DISABLE);                 //失能通道a
//     else
//         hal_adv_tim_b_en(reg_base,HAL_DISABLE);                 //失能通道b

//     hal_adv_tim_set_load_value(reg_base,hal_clock_get_apb0_clk() * 1.0 / (hal_adv_tim_get_clock_div(reg_base) + 1) / freq - 2);

//     if((pwm_channel_num & 0x01) == 0)
//         hal_adv_tim_a_en(reg_base,HAL_ENABLE);                  //使能通道a
//     else
//         hal_adv_tim_b_en(reg_base,HAL_ENABLE);                  //使能通道b
// }

// int aiio_pwm_init(aiio_pwm_init_t *init)
// {
//     pwm_init(init->freq, init->duty, init->pwm_channel_num, init->gpio_port, init->gpio_pin);
//     return 0;
// }
// int aiio_pwm_start(aiio_pwm_chann_t pwm_channel_num)
// {
//     pwm_start(pwm_channel_num);
//     return 0;
// }
// int aiio_pwm_set_duty(aiio_pwm_chann_t pwm_channel_num, float duty)
// {
//     pwm_set_duty(duty, pwm_channel_num);
//     return 0;
// }
// float aiio_pwm_get_duty(aiio_pwm_chann_t pwm_channel_num)
// {
//     return pwm_get_duty(pwm_channel_num);
// }
// int aiio_pwm_set_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t period)
// {
//     pwm_set_freq(pwm_channel_num, period);
//     return 0;
// }

// #ifdef TEST_PWM_FUNC

// #include "aiio_chip_spec.h"
// #include "aiio_pwm.h"
// void test_pwm_function(void *params)
// {

//     uint32_t pwm_duty = 0;

//     aiio_pwm_init(10000, 20, AIIO_PWM_CHA_0, AIIO_PWM_PORTB, __AIIO_GPIO_NUM_5); // 初始化PWM,设置频率为10K,占空比为20%
//     aiio_pwm_init(10000, 20, AIIO_PWM_CHA_1, AIIO_PWM_PORTB, __AIIO_GPIO_NUM_6); // 初始化PWM
//     aiio_pwm_init(10000, 20, AIIO_PWM_CHA_2, AIIO_PWM_PORTB, __AIIO_GPIO_NUM_7); // 初始化PWM
//     aiio_pwm_start(AIIO_PWM_CHA_0);                                      // 开始产生PWM

//     while (1)
//     {
//         pwm_duty++;
//         if (pwm_duty > 100)
//             pwm_duty = 0;

//         aiio_pwm_set_duty(pwm_duty, AIIO_PWM_CHA_0); // 配置占空比
//         aiio_pwm_set_duty(pwm_duty, AIIO_PWM_CHA_1);
//         aiio_pwm_set_duty(pwm_duty, AIIO_PWM_CHA_2);

//         aiio_log_a("PWM Duty = %f\n", aiio_pwm_get_duty(AIIO_PWM_CHA_0));
//         aiio_os_tick_dealy(aiio_os_ms2tick(2000));
//     }
// }

// void test_pwm_func(void)
// {
//     static aiio_os_thread_handle_t *aiio_pwm_thread = NULL;
//     aiio_os_thread_create(&aiio_pwm_thread, "test_pwm", test_pwm_function, 4096, NULL, 3);
// }

// #endif

#include "hal/hal_interrupt.h"
#include "hal/hal_gpio.h"
#include "hal/hal_timer.h"
#include "hal/hal_adv_timer.h"
#include "hal/hal_clock.h"
#include "hal/hal_common.h"
#include "utils/debug/log.h"

#include "aiio_pwm.h"

//**********************************************
typedef enum
{
    TIMER_CH_0,
    TIMER_CH_1,
    TIMER_CH_2,
    TIMER_CH_3,
} timer_ch_t;

void ADV_TIMER_IRQHandle(void);
void (*timer2_it_handler)(void);
volatile uint8_t pwm_start_flag = 0;
volatile int g_pwm_cnt = 0;
volatile aiio_pwm_chann_t pwm_channel;

static void pwm_init(uint32_t freq, float duty, aiio_pwm_chann_t pwm_channel_num, uint32_t gpio_port, uint32_t gpio_pin)
{
    uint32_t reg_base = 0;
    uint32_t gpio_reg_base = 0;
    uint16_t cmp_data = 0;

    switch (gpio_port)
    {
    case 0:
        gpio_reg_base = GPIOA_BASE;
        break;
    case 1:
        gpio_reg_base = GPIOB_BASE;
        break;
    }

    switch (pwm_channel_num)
    {
    case AIIO_PWM_CHA_0:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_1:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_2:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_3:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_4:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_5:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_6:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_7:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_8:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_9:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_10:
        reg_base = ADV_TIMER_5_BASE;
        break;
    case AIIO_PWM_CHA_11:
        reg_base = ADV_TIMER_5_BASE;
        break;
    }

    adv_tim_init_t_def adv_tim_init;
    memset(&adv_tim_init, 0, sizeof(adv_tim_init));

    if (freq >= 10000)
    {
        adv_tim_init.adv_tim_clk_div = 0;
        adv_tim_init.adv_tim_load_value = hal_clock_get_apb0_clk() * 1.0 / freq - 2;
    }
    else
    {
        adv_tim_init.adv_tim_clk_div = (uint32_t)(hal_clock_get_apb0_clk() / 1000000) - 1;
        adv_tim_init.adv_tim_load_value = 1000000 / freq - 2;
    }

    if ((pwm_channel_num & 0x01) == 0)
        adv_tim_init.adv_tim_cmp_a_value = (adv_tim_init.adv_tim_load_value + 2) * duty / 100.0f;
    else
        adv_tim_init.adv_tim_cmp_b_value = (adv_tim_init.adv_tim_load_value + 2) * duty / 100.0f;

    adv_tim_init.adv_tim_dead_gap_value = 0;
    adv_tim_init.adv_tim_dead_en = ADV_TIMER_DEAD_DIS;
    adv_tim_init.adv_tim_cnt_mode = ADV_TIMER_CNT_MODE_INC;
    adv_tim_init.adv_tim_cha_inv_en = ADV_TIMER_CHA_INV_EN;
    adv_tim_init.adv_tim_chb_inv_en = ADV_TIMER_CHB_INV_EN;

    adv_tim_init.adv_tim_cha_it_mode = ADV_TIMER_CHA_IT_MODE_INC;
    adv_tim_init.adv_tim_chb_it_mode = ADV_TIMER_CHB_IT_MODE_INC;

    hal_adv_tim_init(reg_base, &adv_tim_init);
    hal_adv_tim_it_cfg(reg_base, ADV_TIMER_IT_FLAG_CMPA, HAL_ENABLE);
    hal_adv_tim_it_cfg(reg_base, ADV_TIMER_IT_FLAG_CMPB, HAL_ENABLE);
    hal_adv_tim_it_cfg(reg_base, ADV_TIMER_IT_FLAG_LOAD, HAL_ENABLE);

    hal_gpio_pin_afio_select(gpio_reg_base, gpio_pin, (afio_function_t)(ADV_TIMER_PWM0 + pwm_channel_num));
    hal_gpio_pin_afio_en(gpio_reg_base, gpio_pin, HAL_ENABLE);
}

static void pwm_start(aiio_pwm_chann_t pwm_channel_num)
{
    uint32_t reg_base = 0;
    switch (pwm_channel_num)
    {
    case AIIO_PWM_CHA_0:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_1:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_2:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_3:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_4:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_5:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_6:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_7:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_8:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_9:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_10:
        reg_base = ADV_TIMER_5_BASE;
        break;
    case AIIO_PWM_CHA_11:
        reg_base = ADV_TIMER_5_BASE;
        break;
    }
    if ((pwm_channel_num & 0x01) == 0)
        hal_adv_tim_a_en(reg_base, HAL_ENABLE);
    else
        hal_adv_tim_b_en(reg_base, HAL_ENABLE);
}

static void pwm_set_duty(float duty, aiio_pwm_chann_t pwm_channel_num)
{
    uint32_t reg_base = 0;
    switch (pwm_channel_num)
    {
    case AIIO_PWM_CHA_0:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_1:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_2:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_3:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_4:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_5:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_6:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_7:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_8:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_9:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_10:
        reg_base = ADV_TIMER_5_BASE;
        break;
    case AIIO_PWM_CHA_11:
        reg_base = ADV_TIMER_5_BASE;
        break;
    }
    if ((pwm_channel_num & 0x01) == 0)
        hal_adv_tim_set_comp_a(reg_base, (hal_adv_tim_get_load_value(reg_base) + 2) * duty / 100.0f);
    else
        hal_adv_tim_set_comp_b(reg_base, (hal_adv_tim_get_load_value(reg_base) + 2) * duty / 100.0f);
}

static float pwm_get_duty(aiio_pwm_chann_t pwm_channel_num)
{
    uint32_t reg_base = 0;
    float ret_val = 0;
    switch (pwm_channel_num)
    {
    case AIIO_PWM_CHA_0:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_1:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_2:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_3:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_4:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_5:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_6:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_7:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_8:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_9:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_10:
        reg_base = ADV_TIMER_5_BASE;
        break;
    case AIIO_PWM_CHA_11:
        reg_base = ADV_TIMER_5_BASE;
        break;
    }
    if ((pwm_channel_num & 0x01) == 0)
        ret_val = hal_adv_tim_get_comp_a(reg_base) * 1.0f / (hal_adv_tim_get_load_value(reg_base) + 2) * 100;
    else
        ret_val = hal_adv_tim_get_comp_b(reg_base) * 1.0f / (hal_adv_tim_get_load_value(reg_base) + 2) * 100;

    return ret_val;
}

static void pwm_set_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t freq)
{
    uint32_t reg_base = 0;
    switch (pwm_channel_num)
    {
    case AIIO_PWM_CHA_0:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_1:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_2:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_3:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_4:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_5:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_6:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_7:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_8:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_9:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_10:
        reg_base = ADV_TIMER_5_BASE;
        break;
    case AIIO_PWM_CHA_11:
        reg_base = ADV_TIMER_5_BASE;
        break;
    }

    if ((pwm_channel_num & 0x01) == 0)
        hal_adv_tim_a_en(reg_base, HAL_DISABLE);
    else
        hal_adv_tim_b_en(reg_base, HAL_DISABLE);

    if (freq >= 10000)
    {
        pwm_div_setf(reg_base, 0);
        hal_adv_tim_set_load_value(reg_base, hal_clock_get_apb0_clk() * 1.0 / freq - 2);
    }
    else
    {
        pwm_div_setf(reg_base, (uint32_t)(hal_clock_get_apb0_clk() / 1000000) - 1);
        hal_adv_tim_set_load_value(reg_base, 1000000 / freq - 2);
    }

    if ((pwm_channel_num & 0x01) == 0)
        hal_adv_tim_a_en(reg_base, HAL_ENABLE);
    else
        hal_adv_tim_b_en(reg_base, HAL_ENABLE);
}

static uint32_t pwm_get_freq(aiio_pwm_chann_t pwm_channel_num)
{
    uint32_t reg_base = 0;
    uint32_t freq = 0;
    switch (pwm_channel_num)
    {
    case AIIO_PWM_CHA_0:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_1:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_2:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_3:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_4:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_5:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_6:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_7:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_8:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_9:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_10:
        reg_base = ADV_TIMER_5_BASE;
        break;
    case AIIO_PWM_CHA_11:
        reg_base = ADV_TIMER_5_BASE;
        break;
    }

    freq = hal_adv_tim_get_load_value(reg_base);

    if (freq < 3998)
    {
        freq = 1000000 / (freq + 2);
    }
    else
    {
        freq = (hal_clock_get_apb0_clk()) / (hal_adv_tim_get_load_value(reg_base) + 2);
    }

    return freq;
}

static void pwm_disable(aiio_pwm_chann_t pwm_channel_num)
{
    uint32_t reg_base = 0;
    uint32_t freq = 0;
    switch (pwm_channel_num)
    {
    case AIIO_PWM_CHA_0:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_1:
        reg_base = ADV_TIMER_0_BASE;
        break;
    case AIIO_PWM_CHA_2:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_3:
        reg_base = ADV_TIMER_1_BASE;
        break;
    case AIIO_PWM_CHA_4:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_5:
        reg_base = ADV_TIMER_2_BASE;
        break;
    case AIIO_PWM_CHA_6:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_7:
        reg_base = ADV_TIMER_3_BASE;
        break;
    case AIIO_PWM_CHA_8:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_9:
        reg_base = ADV_TIMER_4_BASE;
        break;
    case AIIO_PWM_CHA_10:
        reg_base = ADV_TIMER_5_BASE;
        break;
    case AIIO_PWM_CHA_11:
        reg_base = ADV_TIMER_5_BASE;
        break;
    }

    if ((pwm_channel_num & 0x01) == 0)
        hal_adv_tim_a_en(reg_base, HAL_DISABLE);
    else
        hal_adv_tim_b_en(reg_base, HAL_DISABLE);
}

static void pwm_cnt_set(aiio_pwm_chann_t pwm_channel_num, int cnt, uint32_t freq, float duty, uint32_t gpio_port, uint32_t gpio_pin)
{
    while (pwm_start_flag)
        ;
    pwm_start_flag = 1;
    g_pwm_cnt = cnt - 1;
    pwm_channel = pwm_channel_num;
    pwm_init(freq, duty, pwm_channel, gpio_port, gpio_pin);
    pwm_start(pwm_channel);
    NVIC_SetPriority(ADV_TIMER_IRQn, 4);
    NVIC_EnableIRQ(ADV_TIMER_IRQn);
}

static void timer_init(timer_ch_t timer_ch, uint32_t us, void (*timer_it_callback)(void))
{
    uint32_t timer_base = 0;
    switch (timer_ch)
    {
    case TIMER_CH_0:
        timer_base = TIMER0_BASE;
        break;
    case TIMER_CH_1:
        timer_base = TIMER1_BASE;
        break;
    case TIMER_CH_2:
        timer_base = TIMER2_BASE;
        break;
    case TIMER_CH_3:
        timer_base = TIMER3_BASE;
        break;
    }

    tim_init_t_def tim_init;
    memset(&tim_init, 0, sizeof(tim_init));
    tim_init.tim_mode = TIM_USER_DEF_CNT_MODE;

    if (us < 1000)
    {
        tim_init.tim_div = 0;
        tim_init.tim_load_value = us * (uint32_t)(hal_clock_get_apb0_clk() / 1000000) - 1;
    }
    else
    {
        tim_init.tim_div = (uint32_t)(hal_clock_get_apb0_clk() / 1000000) - 1;
        tim_init.tim_load_value = us - 1;
    }
    hal_tim_init(timer_base, &tim_init);
    hal_tim_en(timer_base, HAL_ENABLE);
    hal_tim_it_cfg(timer_base, TIM_IT_FLAG_ACTIVE, HAL_ENABLE);

    switch (timer_ch)
    {
    case TIMER_CH_0:
    {
        NVIC_SetPriority(TIMER0_IRQn, 4);
        NVIC_EnableIRQ(TIMER0_IRQn);
        if (timer_it_callback != NULL)
            break;
    }
    case TIMER_CH_1:
    {
        NVIC_SetPriority(TIMER1_IRQn, 4);
        NVIC_EnableIRQ(TIMER1_IRQn);
        if (timer_it_callback != NULL)

            break;
    }
    case TIMER_CH_2:
    {
        NVIC_SetPriority(TIMER2_IRQn, 4);
        NVIC_EnableIRQ(TIMER2_IRQn);
        if (timer_it_callback != NULL)
            timer2_it_handler = timer_it_callback;
        break;
    }
    case TIMER_CH_3:
    {
    }
    }
}

static uint32_t timer_get_timer_cnt_time(timer_ch_t timer_ch)
{
    uint32_t timer_base = 0;
    switch (timer_ch)
    {
    case TIMER_CH_0:
        timer_base = TIMER0_BASE;
        break;
    case TIMER_CH_1:
        timer_base = TIMER1_BASE;
        break;
    case TIMER_CH_2:
        timer_base = TIMER2_BASE;
        break;
    case TIMER_CH_3:
        timer_base = TIMER3_BASE;
        break;
    }
    return (uint32_t)((hal_tim_get_load_value(timer_base) - hal_tim_get_current_cnt_value(timer_base)) / ((hal_clock_get_apb0_clk() / 1000000) * 1.0f / ((hal_tim_get_div(timer_base) + 1))));
}

void TIMER2_IRQHandler()
{
    if (hal_tim_get_it_flag(TIMER2_BASE, TIM_IT_FLAG_ACTIVE))
    {
        hal_tim_clr_it_flag(TIMER2_BASE, TIM_IT_FLAG_ACTIVE);
        if (timer2_it_handler != NULL)
            timer2_it_handler();
    }
}

void ADV_TIMER_IRQHandler(void)
{
    if (hal_adv_tim_get_it_flag(ADV_TIMER_0_BASE, ADV_TIMER_IT_FLAG_LOAD))
    {
        if (pwm_start_flag)
        {
            if (g_pwm_cnt > 0)
            {
                g_pwm_cnt--;
            }
            else
            {
                pwm_start_flag = 0;
                pwm_disable(pwm_channel);
                NVIC_DisableIRQ(ADV_TIMER_IRQn);
            }
        }

        hal_adv_tim_clr_it_flag(ADV_TIMER_0_BASE, ADV_TIMER_IT_FLAG_LOAD);
    }
}

int aiio_pwm_init(aiio_pwm_init_t *init)
{
    uint8_t chann_num = init->gpio_pin%5;
    pwm_init(init->freq, init->duty, chann_num, init->gpio_port, init->gpio_pin);
    return chann_num;
}

int aiio_pwm_start(aiio_pwm_chann_t pwm_channel_num)
{
    pwm_start(pwm_channel_num);
    return 0;
}

int aiio_pwm_set_duty(aiio_pwm_chann_t pwm_channel_num, float duty)
{
    pwm_set_duty(duty, pwm_channel_num);
    return 0;
}

float aiio_pwm_get_duty(aiio_pwm_chann_t pwm_channel_num)
{
    return pwm_get_duty(pwm_channel_num);
}

int aiio_pwm_set_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t period)
{
    pwm_set_freq(pwm_channel_num, period);
    return 0;
}

int32_t aiio_pwm_get_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t *period)
{
    *period = pwm_get_freq(pwm_channel_num);
    return 0;
}

int32_t aiio_pwm_output_close(aiio_pwm_chann_t pwm_channel_num)
{
    pwm_disable(pwm_channel_num);
    return 0;
}

int32_t aiio_pwm_output(aiio_pwm_chann_t pwm_channel_num, int cnt, uint32_t freq, float duty, uint32_t gpio_port, uint32_t gpio_pin)
{
    pwm_cnt_set(pwm_channel_num, cnt, freq, duty, gpio_port, gpio_pin);
    return 0;
}
