/** @brief      pwmcap application interface.
 *
 *  @file       aiio_pwmcap.C
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       pwmcap application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/05/04      <td>1.0.0       <td>liq         <td>pwmcap Init
 *  </table>
 *
 */
#include "aiio_pwmcap.h"
#include <stdio.h>
#include <hosal_gpio.h>
#include <hosal_dma.h>
#include "aiio_os_port.h"
#include <cli.h>
#include <hosal_timer.h>
#include <bl_gpio.h>

// #define GPIO_INT_START HOSAL_IRQ_TRIG_NEG_PULSE
// #define GPIO_INT_TURN HOSAL_IRQ_TRIG_POS_PULSE

#define GPIO_INT_START  HOSAL_IRQ_TRIG_POS_PULSE
#define GPIO_INT_TURN   HOSAL_IRQ_TRIG_NEG_PULSE

static volatile uint32_t result_arr[1024];
static volatile uint32_t result_len = 1024;
static volatile uint32_t result_cnt = 0;
static volatile uint8_t cap_edge = 0;
static uint32_t result_buf = 0;
PULSE_CAP_STA cap_sta = PULSE_CAP_NONE;
static volatile uint32_t bench_value = 0;
static volatile uint8_t bench_flag = 0;

static hosal_gpio_dev_t pwmcap;
static uint8_t arg1 = 2;

static uint32_t count_cnt = 0;

hosal_timer_dev_t timer;

void timer_callback(void *arg)
{
    count_cnt++;
}

void pwmcap_irq(void *arg)
{
    uint8_t val1 = *(uint8_t *)arg;
    bench_flag = 1;

    if (result_cnt < result_len)
    {
        result_arr[result_cnt++] = count_cnt;
    }
    else
    {
        result_buf = count_cnt;
        result_cnt++;
    }

    if (result_cnt > result_len)
    {
        result_cnt = result_len + 1;
        hosal_gpio_irq_mask(&pwmcap, 1);
        cap_sta = PULSE_CAP_DONE;
    }

    if (cap_edge == 0)
    {
        hosal_gpio_irq_set(&pwmcap, GPIO_INT_TURN, pwmcap_irq, &arg1);
        cap_edge = 1;
    }
    else
    {
        hosal_gpio_irq_set(&pwmcap, GPIO_INT_START, pwmcap_irq, &arg1);
        cap_edge = 0;
    }
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
            hosal_gpio_irq_mask(&pwmcap, 1);
            bench_flag = 0;
            if (result_cnt == 0)
            {
                return PULSE_CAP_SUCESS;
            }
            for (int i = 0; i < result_cnt - 1; i++)
            {
                result_arr[i] = result_arr[i + 1] - result_arr[i];
            }

            return PULSE_CAP_SUCESS;
        }
    }

    if (cap_sta == PULSE_CAP_DONE)
    {
        if (result_cnt == 0)
        {
            return PULSE_CAP_SUCESS;
        }
        for (int i = 0; i < result_len - 1; i++)
        {
            result_arr[i] = result_arr[i + 1] - result_arr[i];
        }
        result_arr[result_len - 1] = result_buf - result_arr[result_len - 1];
        cap_sta = PULSE_CAP_SUCESS;
    }

    return cap_sta;
}

int32_t aiio_pwmcap_capture_init(aiio_pwmcap_pin_cfg_t *aiio_i2c_cfg)
{
    pwmcap.port = aiio_i2c_cfg->aiio_gpio_pwmcap_pin;
    pwmcap.config = INPUT_PULL_UP;
    hosal_gpio_init(&pwmcap);

    hosal_gpio_irq_set(&pwmcap, GPIO_INT_START, pwmcap_irq, &arg1);
    hosal_gpio_irq_mask(&pwmcap, 1);

    return AIIO_OK;
}

int32_t aiio_pwmcap_capture_start(void)
{
    result_cnt = 0;
    cap_edge = 0;
    count_cnt = 0;
    timer.port = 0;
    timer.config.period = 10; /* 100us */
    timer.config.reload_mode = TIMER_RELOAD_PERIODIC;
    timer.config.cb = timer_callback;
    timer.config.arg = NULL;

    hosal_timer_init(&timer);
    hosal_timer_start(&timer);
    hosal_gpio_irq_mask(&pwmcap, 0);
    cap_sta = PULSE_CAP_WAIT;
    return AIIO_OK;
}

int32_t aiio_pwmcap_capture_close(void)
{
    hosal_gpio_irq_mask(&pwmcap, 1);
    hosal_timer_stop(&timer);
    hosal_timer_finalize(&timer);
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
        freq = (100000 / (result_arr[2 * i] + result_arr[2 * i + 1]));
        dutylist[i] = duty;
        freqlist[i] = freq;
    }

    for (int i = 0; i < *size; i++)
    {
        aiio_log_w("PWM_CAP_DATA[%d]::duty = %01f , freq = %d", i, dutylist[i], freqlist[i]);
    }
    memset(result_arr, 0, sizeof(uint32_t) * 1024);
    return AIIO_OK;
}
