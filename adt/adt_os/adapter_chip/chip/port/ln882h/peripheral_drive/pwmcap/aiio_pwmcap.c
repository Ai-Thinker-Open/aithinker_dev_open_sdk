/** @brief      pwmcap application interface.
 *
 *  @file       aiio_pwmcap.C
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       pwmcap application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>pwmcap Init
 *  </table>
 *
 */
#include "aiio_pwmcap.h"
#include "utils/debug/log.h"
#include "utils/debug/ln_assert.h"
#include "hal/hal_gpio.h"
#include "hal/hal_timer.h"
#include "aiio_os_port.h"

static uint32_t pulse_cap_port = 0;
static uint32_t pulse_cap_pin = 0;
static uint32_t result_buf = 0;

static volatile uint32_t result_arr[1024];
static volatile uint32_t result_len = 1024;
static volatile uint32_t result_cnt = 0;
static volatile uint8_t cap_edge = 0;
static volatile uint32_t bench_value = 0;
static volatile uint8_t bench_flag = 0;

#define TIME_T 1000000

// #define GPIO_INT_START  GPIO_INT_FALLING
// #define GPIO_INT_TURN   GPIO_INT_RISING

#define GPIO_INT_START  GPIO_INT_RISING
#define GPIO_INT_TURN   GPIO_INT_FALLING

PULSE_CAP_STA cap_sta = PULSE_CAP_NONE;

void pulse_cap_init(uint32_t gpio_base, gpio_pin_t pin)
{
    pulse_cap_port = gpio_base;
    pulse_cap_pin = pin;

    gpio_init_t_def gpio_init;
    memset(&gpio_init, 0, sizeof(gpio_init));
    gpio_init.dir = GPIO_INPUT;
    gpio_init.pin = pulse_cap_pin;
    gpio_init.speed = GPIO_HIGH_SPEED;
    hal_gpio_init(pulse_cap_port, &gpio_init);
    hal_gpio_pin_it_cfg(pulse_cap_port, pulse_cap_pin, GPIO_INT_START);
    hal_gpio_pin_it_en(pulse_cap_port, pulse_cap_pin, HAL_DISABLE);

    if (pulse_cap_port == GPIOA_BASE)
    {
        NVIC_SetPriority(GPIOA_IRQn, 0);
        NVIC_EnableIRQ(GPIOA_IRQn);
    }
    if (pulse_cap_port == GPIOB_BASE)
    {
        NVIC_SetPriority(GPIOB_IRQn, 0);
        NVIC_EnableIRQ(GPIOB_IRQn);
    }

    tim_init_t_def tim_init;
    memset(&tim_init, 0, sizeof(tim_init));
    tim_init.tim_mode = TIM_USER_DEF_CNT_MODE;
    tim_init.tim_div = 40 - 1;
    tim_init.tim_load_value = 0xFFFFFF;

    hal_tim_init(TIMER2_BASE, &tim_init);
    hal_tim_en(TIMER2_BASE, HAL_DISABLE);
    hal_tim_it_cfg(TIMER2_BASE, TIM_IT_FLAG_ACTIVE, HAL_ENABLE);

    NVIC_SetPriority(TIMER2_IRQn, 4);
    NVIC_DisableIRQ(TIMER2_IRQn);
}

void pulse_cap_start(void)
{
    result_cnt = 0;
    cap_sta = 0;
    cap_edge = 0;
    hal_gpio_pin_it_cfg(pulse_cap_port, pulse_cap_pin, GPIO_INT_START);
    hal_gpio_pin_it_en(pulse_cap_port, pulse_cap_pin, HAL_ENABLE);
    cap_sta = PULSE_CAP_WAIT;
}

void pulse_cap_stop()
{
    hal_gpio_pin_it_en(pulse_cap_port, pulse_cap_pin, HAL_DISABLE);
}

PULSE_CAP_STA pulse_cap_get_result()
{
    uint32_t count = 0;
    if (bench_flag == 1)
    {
        aiio_os_tick_count(&count);
        bench_value = aiio_os_tick2ms(count);
        bench_flag = 0;
    }
    else
    {
        aiio_os_tick_count(&count);
        if (aiio_os_tick2ms(count) - bench_value > 200)
        {
            hal_gpio_pin_it_en(pulse_cap_port, pulse_cap_pin, HAL_DISABLE);
            hal_tim_en(TIMER2_BASE, HAL_DISABLE);
            bench_flag = 0;
            result_arr[0] = 0xFFFFFF;
            for (int i = 0; i < result_cnt - 1; i++)
            {
                result_arr[i] = result_arr[i] - result_arr[i + 1];
            }

            return PULSE_CAP_SUCESS;
        }
    }

    if (cap_sta == PULSE_CAP_DONE)
    {
        result_arr[0] = 0xFFFFFF;
        for (int i = 0; i < result_len - 1; i++)
        {
            result_arr[i] = result_arr[i] - result_arr[i + 1];
        }
        result_arr[result_len - 1] = result_arr[result_len - 1] - result_buf;
        cap_sta = PULSE_CAP_SUCESS;
    }

    return cap_sta;
}

void GPIOA_IRQHandler()
{
    bench_flag = 1;
    if (hal_gpio_pin_get_it_flag(pulse_cap_port, pulse_cap_pin) == HAL_SET)
    {
        hal_gpio_pin_clr_it_flag(pulse_cap_port, pulse_cap_pin);
        hal_tim_en(TIMER2_BASE, HAL_ENABLE);

        if (result_cnt < result_len)
        {
            result_arr[result_cnt++] = hal_tim_get_current_cnt_value(TIMER2_BASE);
        }
        else
        {
            result_buf = hal_tim_get_current_cnt_value(TIMER2_BASE);
            result_cnt++;
        }
        if (result_cnt > result_len)
        {
            hal_gpio_pin_it_en(pulse_cap_port, pulse_cap_pin, HAL_DISABLE);
            hal_tim_en(TIMER2_BASE, HAL_DISABLE);
            cap_sta = PULSE_CAP_DONE;
        }
        if (cap_edge == 0)
        {
            hal_gpio_pin_it_cfg(pulse_cap_port, pulse_cap_pin, GPIO_INT_TURN);
            cap_edge = 1;
        }
        else
        {
            hal_gpio_pin_it_cfg(pulse_cap_port, pulse_cap_pin, GPIO_INT_START);
            cap_edge = 0;
        }
    }
}

void GPIOB_IRQHandler()
{
    bench_flag = 1;
    if (hal_gpio_pin_get_it_flag(pulse_cap_port, pulse_cap_pin) == HAL_SET)
    {
        hal_gpio_pin_clr_it_flag(pulse_cap_port, pulse_cap_pin);
        hal_tim_en(TIMER2_BASE, HAL_ENABLE);

        if (result_cnt < result_len)
        {
            result_arr[result_cnt++] = hal_tim_get_current_cnt_value(TIMER2_BASE);
        }
        else
        {
            result_buf = hal_tim_get_current_cnt_value(TIMER2_BASE);
            result_cnt++;
        }
        if (result_cnt > result_len)
        {
            hal_gpio_pin_it_en(pulse_cap_port, pulse_cap_pin, HAL_DISABLE);
            hal_tim_en(TIMER2_BASE, HAL_DISABLE);
            cap_sta = PULSE_CAP_DONE;
        }
        if (cap_edge == 0)
        {
            hal_gpio_pin_it_cfg(pulse_cap_port, pulse_cap_pin, GPIO_INT_TURN);
            cap_edge = 1;
        }
        else
        {
            hal_gpio_pin_it_cfg(pulse_cap_port, pulse_cap_pin, GPIO_INT_START);
            cap_edge = 0;
        }
    }
}

int32_t aiio_pwmcap_capture_init(aiio_pwmcap_pin_cfg_t *aiio_i2c_cfg)
{
    uint32_t pwmcap_gpio_base;
    gpio_pin_t pwmcap_pin;

    if (aiio_i2c_cfg->aiio_gpio_pwmcap_port == AIIO_GPIO_A)
    {
        pwmcap_gpio_base = GPIOA_BASE;
    }
    else if (aiio_i2c_cfg->aiio_gpio_pwmcap_port == AIIO_GPIO_B)
    {
        pwmcap_gpio_base = GPIOB_BASE;
    }
    else
    {
        return AIIO_ERROR;
    }
    pulse_cap_init(pwmcap_gpio_base, aiio_i2c_cfg->aiio_gpio_pwmcap_pin);
    return AIIO_OK;
}

int32_t aiio_pwmcap_capture_start(void)
{
    pulse_cap_start();
    return AIIO_OK;
}

int32_t aiio_pwmcap_capture_close(void)
{
    pulse_cap_stop();
    return AIIO_OK;
}

int32_t aiio_pwmcap_capture_value(uint32_t *freqlist, float *dutylist, uint32_t *size)
{
    while (pulse_cap_get_result() != PULSE_CAP_SUCESS)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(1));
    }
    *size = result_cnt / 2;

    uint32_t freq = 0;
    float duty = 0;

    for (uint16_t i = 0; i < result_cnt / 2; i++)
    {
        duty = (float)(100 * result_arr[2 * i] / (result_arr[2 * i + 1] + result_arr[2 * i]));
        freq = (TIME_T / (result_arr[2 * i] + result_arr[2 * i + 1]));
        dutylist[i] = duty;
        freqlist[i] = freq;
    }
    memset(result_arr, 0, sizeof(uint32_t) * 1024);
    return AIIO_OK;
}
