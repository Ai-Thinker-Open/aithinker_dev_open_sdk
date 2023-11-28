/** @brief PWM intermediate device port adaptation
 *  
 *  @file        aiio_pwm.c
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note 
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/07/24          <td>V1.0.0          <td>hewm            <td>PWM device port adaptation
 *  <table>
 *  
 */

#include "aiio_pwm.h"
#include <hosal_pwm.h>

#define USER_PARAM_CHECK (1)
#if USER_PARAM_CHECK
#define aiio_param_check(expr, ERR) if(!(expr)) return ERR
#else
#define aiio_param_check(expr) ((void)0)
#endif

typedef struct
{
    uint8_t inited : 1;
    uint8_t reserved : 7;
    uint8_t pwm_num;
    hosal_pwm_dev_t pwm;
} aiio_pwm_dev_t;

static aiio_pwm_dev_t pwm_instance[] =
    {
        [AIIO_PWM_CHA_0] = {
            .inited = 0,
            .pwm = {
                .port = AIIO_GPIO_NUM_0%5,/* pwm port and pin set  note: There is corresponding relationship between port and pin, for bl602, map is  port = pin%5 */
                .config.pin = AIIO_GPIO_NUM_0,
                .config.duty_cycle = 5000, // duty cycle range is 0~10000 correspond to 0~100%
                .config.freq = 1000,
            },
        },
        [AIIO_PWM_CHA_1] = {
            .inited = 0,
            .pwm = {
                .port = AIIO_GPIO_NUM_1%5,/* pwm port and pin set  note: There is corresponding relationship between port and pin, for bl602, map is  port = pin%5 */
                .config.pin = AIIO_GPIO_NUM_1,
                .config.duty_cycle = 5000, // duty cycle range is 0~10000 correspond to 0~100%
                .config.freq = 1000,
            },
        },
        [AIIO_PWM_CHA_2] = {
            .inited = 0,
            .pwm = {
                .port = AIIO_GPIO_NUM_2%5,/* pwm port and pin set  note: There is corresponding relationship between port and pin, for bl602, map is  port = pin%5 */
                .config.pin = AIIO_GPIO_NUM_2,
                .config.duty_cycle = 5000, // duty cycle range is 0~10000 correspond to 0~100%
                .config.freq = 1000,
            },
        },
        [AIIO_PWM_CHA_3] = {
            .inited = 0,
            .pwm = {
                .port = AIIO_GPIO_NUM_3%5,/* pwm port and pin set  note: There is corresponding relationship between port and pin, for bl602, map is  port = pin%5 */
                .config.pin = AIIO_GPIO_NUM_3,
                .config.duty_cycle = 5000, // duty cycle range is 0~10000 correspond to 0~100%
                .config.freq = 1000,
            },
        },
        [AIIO_PWM_CHA_4] = {
            .inited = 0,
            .pwm = {
                .port = AIIO_GPIO_NUM_4%5,/* pwm port and pin set  note: There is corresponding relationship between port and pin, for bl602, map is  port = pin%5 */
                .config.pin = AIIO_GPIO_NUM_4,
                .config.duty_cycle = 5000, // duty cycle range is 0~10000 correspond to 0~100%
                .config.freq = 1000,
            },
        },

};

int aiio_pwm_init(aiio_pwm_init_t *init)
{

    uint8_t pwm_num = init->gpio_pin%5;
    aiio_param_check(pwm_num < 5, AIIO_ERROR);
    aiio_param_check(init->gpio_pin%5 == pwm_num, AIIO_ERROR);
    aiio_param_check(init->duty>0 && init->duty<=100, AIIO_ERROR);

    hosal_pwm_dev_t *pwm_dev = &pwm_instance[pwm_num].pwm;
    pwm_dev->config.freq = init->freq;
    pwm_dev->config.duty_cycle = init->duty*100;
    pwm_dev->config.pin = init->gpio_pin;
    pwm_dev->port = pwm_dev->config.pin % 5;

    if (hosal_pwm_init(pwm_dev) == AIIO_OK)
    {
        pwm_instance[pwm_num].inited = 1;
        pwm_instance[pwm_num].pwm_num = pwm_num;
        return pwm_num;
    }
    return AIIO_ERROR;
}

int aiio_pwm_start(aiio_pwm_chann_t pwm_channel_num)
{
    uint8_t pwm_num = pwm_channel_num;
    aiio_param_check(pwm_instance[pwm_num].inited, AIIO_ERROR);
    aiio_param_check(pwm_num == pwm_instance[pwm_num].pwm_num, AIIO_ERROR);

    hosal_pwm_dev_t *pwm_dev = &pwm_instance[pwm_num].pwm;

    if (hosal_pwm_start(pwm_dev) == AIIO_OK)
        return AIIO_OK;
    return AIIO_ERROR;
}

int aiio_pwm_set_duty(aiio_pwm_chann_t pwm_channel_num, float duty)
{
    uint8_t pwm_num = pwm_channel_num;
    aiio_param_check(pwm_instance[pwm_num].inited, AIIO_ERROR);
    aiio_param_check(pwm_num == pwm_instance[pwm_num].pwm_num, AIIO_ERROR);
    aiio_param_check(duty>0 && duty<=100, AIIO_ERROR);

    hosal_pwm_dev_t *pwm_dev = &pwm_instance[pwm_num].pwm;
    hosal_pwm_config_t para;
    para.duty_cycle = duty*100;
    para.freq = pwm_dev->config.freq;

    if (hosal_pwm_para_chg(pwm_dev, para) == AIIO_OK)
        return AIIO_OK;
    return AIIO_ERROR;
}

float aiio_pwm_get_duty(aiio_pwm_chann_t pwm_channel_num)
{
    uint8_t pwm_num = pwm_channel_num;
    aiio_param_check(pwm_instance[pwm_num].inited, AIIO_ERROR);
    aiio_param_check(pwm_num == pwm_instance[pwm_num].pwm_num, AIIO_ERROR);

    hosal_pwm_dev_t *pwm_dev = &pwm_instance[pwm_num].pwm;
    
    return pwm_dev->config.duty_cycle/100;
}

int aiio_pwm_set_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t period)
{
    uint8_t pwm_num = pwm_channel_num;
    aiio_param_check(pwm_instance[pwm_num].inited, AIIO_ERROR);
    aiio_param_check(pwm_num == pwm_instance[pwm_num].pwm_num, AIIO_ERROR);
    aiio_param_check(period>0 && period<=40000000, AIIO_ERROR);

    hosal_pwm_dev_t *pwm_dev = &pwm_instance[pwm_num].pwm;
    hosal_pwm_config_t para;
    para.duty_cycle = pwm_dev->config.duty_cycle;
    para.freq = period;

    if (hosal_pwm_para_chg(pwm_dev, para) == AIIO_OK)
        return AIIO_OK;
    return AIIO_ERROR;
}

int32_t aiio_pwm_get_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t *period)
{
    uint8_t pwm_num = pwm_channel_num;
    aiio_param_check(pwm_instance[pwm_num].inited, AIIO_ERROR);
    aiio_param_check(pwm_num == pwm_instance[pwm_num].pwm_num, AIIO_ERROR);

    hosal_pwm_dev_t *pwm_dev = &pwm_instance[pwm_num].pwm;
    (*period) = pwm_dev->config.freq;
    return AIIO_OK;
}

int32_t aiio_pwm_output_close(aiio_pwm_chann_t pwm_channel_num)
{
    uint8_t pwm_num = pwm_channel_num;
    aiio_param_check(pwm_instance[pwm_num].inited, AIIO_ERROR);
    aiio_param_check(pwm_num == pwm_instance[pwm_num].pwm_num, AIIO_ERROR);

    hosal_pwm_dev_t *pwm_dev = &pwm_instance[pwm_num].pwm;
    if (hosal_pwm_stop(pwm_dev) == AIIO_OK)
    {
        pwm_instance->inited = 0;
        if (hosal_pwm_finalize(pwm_dev) == AIIO_OK)
        {
            return AIIO_OK;
        }
    }
    return AIIO_ERROR;
}

int32_t aiio_pwm_output(aiio_pwm_chann_t pwm_channel_num,int cnt, uint32_t freq, float duty,uint32_t gpio_port, uint32_t gpio_pin)
{
    (void)pwm_channel_num;
    (void)cnt;
    (void)freq;
    (void)duty;
    (void)duty;
    (void)gpio_port;
    (void)gpio_pin;
    return AIIO_ERROR;
}


