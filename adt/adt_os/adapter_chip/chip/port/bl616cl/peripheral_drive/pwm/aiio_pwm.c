/** @brief PWM intermediate device port adaptation
 *  
 *  @file        aiio_pwm.c
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note 
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/07/25          <td>V1.0.0          <td>hewm            <td>PWM device port adaptation
 *  <table>
 *  
 */

#include "aiio_pwm.h"

#include "bflb_mtimer.h"
#include "bflb_pwm_v2.h"
#include "bflb_gpio.h"
#include "bflb_clock.h"
#include "board.h"

#ifndef USER_PARAM_CHECK
#define USER_PARAM_CHECK (1)
#if USER_PARAM_CHECK
#define aiio_param_check(expr, ERR) if(!(expr)) return ERR
#else
#define aiio_param_check(expr) ((void)0)
#endif
#endif

typedef struct
{
    uint8_t inited : 1;
    uint8_t polarity : 1;/**< 0 positive 1 negative*/
    uint8_t reserved : 6;
    uint8_t pwm_num;
    uint8_t pwm_pin;
    uint8_t pwm_duty;
    uint32_t pwm_freq;
    struct bflb_device_s *pwm;
    struct bflb_pwm_v2_config_s cfg;
} aiio_pwm_dev_t;

static aiio_pwm_dev_t pwm_instance[] =
    {
        [AIIO_PWM_CHA_0] = {
            .inited = 0,
            .pwm =NULL,
            .pwm_pin = GPIO_PIN_24,
            .cfg = {
                /* period = .PBCLK / .clk_div / .period = 80MHz / 80 / 1000 = 1KHz */
                .clk_source = BFLB_SYSTEM_PBCLK,
                .clk_div = 40,
                .period = 1000,
            },
        },
        [AIIO_PWM_CHA_1] = {
            .inited = 0,
            .pwm =NULL,
            .pwm_pin = GPIO_PIN_24,
            .cfg = {
                /* period = .PBCLK / .clk_div / .period = 80MHz / 80 / 1000 = 1KHz */
                .clk_source = BFLB_SYSTEM_PBCLK,
                .clk_div = 80,
                .period = 1000,
            },
        },
        [AIIO_PWM_CHA_2] = {
            .inited = 0,
            .pwm =NULL,
            .pwm_pin = GPIO_PIN_24,
            .cfg = {
                /* period = .PBCLK / .clk_div / .period = 80MHz / 80 / 1000 = 1KHz */
                .clk_source = BFLB_SYSTEM_PBCLK,
                .clk_div = 80,
                .period = 1000,
            },
        },
        [AIIO_PWM_CHA_3] = {
            .inited = 0,
            .pwm =NULL,
            .pwm_pin = GPIO_PIN_24,
            .cfg = {
                /* period = .PBCLK / .clk_div / .period = 80MHz / 80 / 1000 = 1KHz */
                .clk_source = BFLB_SYSTEM_PBCLK,
                .clk_div = 80,
                .period = 1000,
            },
        },
};

static void aiio_pwm_gpio_init(uint8_t pin)
{
    struct bflb_device_s *gpio;

    gpio = bflb_device_get_by_name("gpio");
    bflb_gpio_init(gpio, pin, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_1);
}

static void aiio_pwm_gpio_deinit(uint8_t pin)
{
    struct bflb_device_s *gpio;

    gpio = bflb_device_get_by_name("gpio");
    bflb_gpio_deinit(gpio, pin);
}

int aiio_pwm_init(aiio_pwm_init_t *init)
{
    uint8_t pwm_num = init->gpio_pin%PWM_V2_CH_MAX;
    aiio_param_check(pwm_num < PWM_V2_CH_MAX, AIIO_ERROR);
    aiio_param_check(init->gpio_pin >= 0 && init->gpio_pin < 31, AIIO_ERROR);
    aiio_param_check(init->duty>0 && init->duty<=100, AIIO_ERROR);

    aiio_pwm_gpio_init(init->gpio_pin);

    pwm_instance[pwm_num].pwm = bflb_device_get_by_name("pwm_v2_0");
    struct bflb_device_s *pwm_dev = pwm_instance[pwm_num].pwm;
    struct bflb_pwm_v2_config_s *cfg = &pwm_instance[pwm_num].cfg;
    pwm_instance[pwm_num].pwm_freq = init->freq;
    cfg->clk_div = cfg->clk_source / cfg->period / pwm_instance[pwm_num].pwm_freq;
    if (cfg->clk_div < 1)
        cfg->clk_div = 1;

    pwm_instance[pwm_num].pwm_duty = init->duty;

    bflb_pwm_v2_init(pwm_dev, cfg);
    bflb_pwm_v2_channel_set_threshold(pwm_dev, pwm_num, 100, (pwm_instance[pwm_num].pwm_duty*10+100)); /* duty = (500-100)/1000 = 40% */
    pwm_instance[pwm_num].inited = 1;
    pwm_instance[pwm_num].pwm_num = pwm_num;
    return pwm_num;
}

int aiio_pwm_start(aiio_pwm_chann_t pwm_channel_num)
{
    uint8_t pwm_num = pwm_channel_num;
    aiio_param_check(pwm_num < PWM_V2_CH_MAX, AIIO_ERROR);
    aiio_param_check(pwm_num == pwm_instance[pwm_num].pwm_num, AIIO_ERROR);

    struct bflb_device_s *pwm_dev = pwm_instance[pwm_num].pwm;
    if (pwm_instance[pwm_num].polarity)
    {
        bflb_pwm_v2_channel_negative_start(pwm_dev, pwm_num);
    }
    else
    {
        bflb_pwm_v2_channel_positive_start(pwm_dev, pwm_num);
    }
    bflb_pwm_v2_start(pwm_dev);
    return AIIO_OK;
}

int aiio_pwm_set_duty(aiio_pwm_chann_t pwm_channel_num, float duty)
{
    uint8_t pwm_num = pwm_channel_num;
    aiio_param_check(pwm_num < PWM_V2_CH_MAX, AIIO_ERROR);
    aiio_param_check(pwm_num == pwm_instance[pwm_num].pwm_num, AIIO_ERROR);
    aiio_param_check(duty>0 && duty<=100, AIIO_ERROR);

    struct bflb_device_s *pwm_dev = pwm_instance[pwm_num].pwm;
    pwm_instance[pwm_num].pwm_duty = duty;
    // if (pwm_instance[pwm_num].polarity)
    // {
    //     bflb_pwm_v2_channel_negative_stop(pwm_dev, pwm_num);
    // }
    // else
    // {
    //     bflb_pwm_v2_channel_positive_stop(pwm_dev, pwm_num);
    // }
    bflb_pwm_v2_channel_set_threshold(pwm_dev, pwm_num, 100, (pwm_instance[pwm_num].pwm_duty*10+100)); /* duty = (500-100)/1000 = 40% */

    return AIIO_OK;
}

float aiio_pwm_get_duty(aiio_pwm_chann_t pwm_channel_num)
{
    uint8_t pwm_num = pwm_channel_num;
    aiio_param_check(pwm_num < PWM_V2_CH_MAX, AIIO_ERROR);
    aiio_param_check(pwm_num == pwm_instance[pwm_num].pwm_num, AIIO_ERROR);
    
    return pwm_instance[pwm_num].pwm_duty;
}

int aiio_pwm_set_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t period)
{
    uint8_t pwm_num = pwm_channel_num;
    aiio_param_check(pwm_num < PWM_V2_CH_MAX, AIIO_ERROR);
    aiio_param_check(pwm_num == pwm_instance[pwm_num].pwm_num, AIIO_ERROR);
    aiio_param_check(period>0 && period<=40000000, AIIO_ERROR);

    struct bflb_device_s *pwm_dev = pwm_instance[pwm_num].pwm;
    struct bflb_pwm_v2_config_s *cfg = &pwm_instance[pwm_num].cfg;
    
    pwm_instance[pwm_num].pwm_freq = period;
    cfg->clk_div = cfg->clk_source / cfg->period / pwm_instance[pwm_num].pwm_freq;
    if (cfg->clk_div < 1)
        cfg->clk_div = 1;

    if (pwm_instance[pwm_num].polarity)
    {
        bflb_pwm_v2_channel_negative_stop(pwm_dev, pwm_num);
    }
    else
    {
        bflb_pwm_v2_channel_positive_stop(pwm_dev, pwm_num);
    }
    bflb_pwm_v2_stop(pwm_dev);
    bflb_pwm_v2_init(pwm_dev, cfg);
    bflb_pwm_v2_channel_set_threshold(pwm_dev, pwm_num, 100, (pwm_instance[pwm_num].pwm_duty * 10 + 100)); /* duty = (500-100)/1000 = 40% */
    if (pwm_instance[pwm_num].polarity)
    {
        bflb_pwm_v2_channel_negative_start(pwm_dev, pwm_num);
    }
    else
    {
        bflb_pwm_v2_channel_positive_start(pwm_dev, pwm_num);
    }
    return AIIO_OK;
}

int32_t aiio_pwm_get_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t *period)
{
    uint8_t pwm_num = pwm_channel_num;
    aiio_param_check(pwm_num < PWM_V2_CH_MAX, AIIO_ERROR);
    aiio_param_check(pwm_num == pwm_instance[pwm_num].pwm_num, AIIO_ERROR);

    (*period) = pwm_instance[pwm_num].pwm_freq;
    return AIIO_OK;
}

int32_t aiio_pwm_output_close(aiio_pwm_chann_t pwm_channel_num)
{
    uint8_t pwm_num = pwm_channel_num;
    aiio_param_check(pwm_num < PWM_V2_CH_MAX, AIIO_ERROR);
    aiio_param_check(pwm_num == pwm_instance[pwm_num].pwm_num, AIIO_ERROR);

    struct bflb_device_s *pwm_dev = pwm_instance[pwm_num].pwm;
    struct bflb_pwm_v2_config_s *cfg = &pwm_instance[pwm_num].cfg;
    if (pwm_instance[pwm_num].polarity)
    {
        bflb_pwm_v2_channel_negative_stop(pwm_dev, pwm_num);
    }
    else
    {
        bflb_pwm_v2_channel_positive_stop(pwm_dev, pwm_num);
    }
    bflb_pwm_v2_stop(pwm_dev);

    aiio_pwm_gpio_deinit(pwm_instance[pwm_num].pwm_pin);
    pwm_instance[pwm_num].inited=0;
    return AIIO_OK;
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


