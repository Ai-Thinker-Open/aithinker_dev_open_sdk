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
#include "pinctrl.h"
#include "pwm.h"
#include "tcxo.h"
#include "soc_osal.h"
#include "app_init.h"

typedef struct
{
    uint8_t pwm_flag;    // 用于判断PWM是否被使用
    uint32_t freq;       // 频率
    uint32_t duty;       // 占空比
    pwm_config_t repeat; // PWM配置参数
    uint8_t channel_id;  // 通道ID
} aiio_bs21_pwm_type_t;

static uint8_t aiio_pwm_flag = 0;
static aiio_bs21_pwm_type_t aiio_pwm_list[12] = {0};

static void aiio_bs21_config_process(aiio_bs21_pwm_type_t *pwm, uint32_t clock)
{
    aiio_log_w("clock: %u", clock);

    // 默认配置
    pwm->repeat.cycles = 0; // 连续输出
    pwm->repeat.repeat = true;
    pwm->repeat.offset_time = 0;

    // 检查参数合法性
    if (pwm->freq == 0 || pwm->duty > 10000)
    {
        aiio_log_e("Invalid freq or duty: freq=%u, duty=%u", pwm->freq, pwm->duty);
        pwm->repeat.low_time = 0;
        pwm->repeat.high_time = 0;
        return;
    }

    // 总周期数（clock周期数 = clock / pwm_freq）
    uint32_t total_cycle = clock / pwm->freq;

    // 计算高电平时间（占空比单位是 0.01%，所以除以 10000）
    pwm->repeat.high_time = (total_cycle * pwm->duty) / 10000;

    // 低电平时间 = 总周期 - 高电平周期
    pwm->repeat.low_time = total_cycle - pwm->repeat.high_time;

    aiio_log_i("PWM Config -> freq: %uHz, duty: %u (%.2f%%), high_time: %u, low_time: %u",
               pwm->freq,
               pwm->duty,
               pwm->duty / 100.0,
               pwm->repeat.high_time,
               pwm->repeat.low_time);
}

// 32000000
int aiio_pwm_init(aiio_pwm_init_t *init)
{
    if (aiio_pwm_flag == 0)
    {
        uapi_pwm_deinit();
        uapi_pwm_init();
        aiio_pwm_flag = 1;
    }

    for (int i = 0; i < 12; i++)
    {
        if (aiio_pwm_list[i].pwm_flag == 0)
        {
            // 设置频率和占空比
            aiio_pwm_list[i].freq = init->freq;
            aiio_pwm_list[i].duty = (uint32_t)(init->duty * 100);
            aiio_pwm_list[i].channel_id = i;
            aiio_pwm_list[i].pwm_flag = 1;
            aiio_bs21_config_process(&aiio_pwm_list[i], uapi_pwm_get_frequency(i));
            // 配置引脚并设置
            uapi_pin_set_mode(init->gpio_pin, 40 + i);
            uapi_pwm_open(i, &aiio_pwm_list[i].repeat);
            uapi_pwm_set_group(i, &aiio_pwm_list[i].channel_id, 1);

            // uapi_pwm_start_group(i);
            return i;
        }
    }

    return AIIO_ERROR;
}

int aiio_pwm_start(aiio_pwm_chann_t pwm_channel_num)
{
    if (aiio_pwm_list[pwm_channel_num].pwm_flag == 0)
    {
        return AIIO_ERROR;
    }

    uapi_pwm_start_group(pwm_channel_num);

    return AIIO_SUCC;
}

int32_t aiio_pwm_output_close(aiio_pwm_chann_t pwm_channel_num)
{
    if (aiio_pwm_list[pwm_channel_num].pwm_flag == 0)
    {
        return AIIO_ERROR;
    }

    uapi_pwm_close(pwm_channel_num);

    return AIIO_SUCC;
}

int aiio_pwm_set_duty(aiio_pwm_chann_t pwm_channel_num, float duty)
{
    if (aiio_pwm_output_close(pwm_channel_num) != AIIO_SUCC)
    {
        return AIIO_ERROR;
    }

    aiio_pwm_list[pwm_channel_num].duty = (uint32_t)(duty * 100);

    aiio_bs21_config_process(&aiio_pwm_list[pwm_channel_num], uapi_pwm_get_frequency(pwm_channel_num));

    uapi_pwm_open(pwm_channel_num, &aiio_pwm_list[pwm_channel_num].repeat);

    aiio_pwm_start(pwm_channel_num);

    return AIIO_SUCC;
}

float aiio_pwm_get_duty(aiio_pwm_chann_t pwm_channel_num)
{
    if (aiio_pwm_list[pwm_channel_num].pwm_flag == 0)
    {
        return AIIO_ERROR;
    }

    return (float)aiio_pwm_list[pwm_channel_num].duty / 100.00;
}

int aiio_pwm_set_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t period)
{
    if (aiio_pwm_output_close(pwm_channel_num) != AIIO_SUCC)
    {
        return AIIO_ERROR;
    }

    aiio_pwm_list[pwm_channel_num].freq = period;

    aiio_bs21_config_process(&aiio_pwm_list[pwm_channel_num], uapi_pwm_get_frequency(pwm_channel_num));

    uapi_pwm_open(pwm_channel_num, &aiio_pwm_list[pwm_channel_num].repeat);

    aiio_pwm_start(pwm_channel_num);

    return AIIO_SUCC;
}

int32_t aiio_pwm_get_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t *period)
{
    if (aiio_pwm_list[pwm_channel_num].pwm_flag == 0)
    {
        return AIIO_ERROR;
    }

    *period = aiio_pwm_list[pwm_channel_num].freq;

    return AIIO_SUCC;
}
