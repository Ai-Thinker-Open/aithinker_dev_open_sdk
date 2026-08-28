/**
 * @brief   Define interface of led application function
 *
 * @file    aiio_cloud_led.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note     This file is mainly describing led application
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-16          <td>1.0.0            <td>zhuolm             <td> The interface of led application
 */
#include "aiio_cloud_led.h"
#include "aiio_adapter_include.h"
#include "hosal_pwm.h"
#include "hosal_gpio.h"
#include "bl602_glb.h"
#include "aiio_cloud_control.h"

#define AIIO_GPIO_R_LED_PORT CONFIG_GPIO_R_LED_PORT
#define AIIO_GPIO_R_LED_PIN CONFIG_GPIO_R_LED_PIN

#define AIIO_GPIO_G_LED_PORT CONFIG_GPIO_G_LED_PORT
#define AIIO_GPIO_G_LED_PIN CONFIG_GPIO_G_LED_PIN

#define AIIO_GPIO_B_LED_PORT CONFIG_GPIO_B_LED_PORT
#define AIIO_GPIO_B_LED_PIN CONFIG_GPIO_B_LED_PIN

static bool led_status_change_enable = false; /*!< switch variable of led twinkling*/
static uint8_t led_triggle_status = ALL_LED_TRIGGLE_STATUS;
static bool blue_led_status = false;

static hosal_pwm_dev_t led_r_drive_pwm;
static hosal_pwm_dev_t led_g_drive_pwm;
static hosal_pwm_dev_t led_b_drive_pwm;

static void qyq_led_drive_pwm_init(hosal_pwm_dev_t *pwm, uint8_t pin, uint32_t duty_cycle)
{
    uint32_t p_freq;
    uint32_t duty;
    /* led_drive_pwm port and pin set  note: There is corresponding relationship between port and pin, for bl602, map is  port = pin%5 */
    pwm->port = pin % 5;
    /* led_drive_pwm config */
    pwm->config.pin = pin;
    pwm->config.duty_cycle = duty_cycle; // 0 - 10000
    pwm->config.freq = 1000;
    /* init led_drive_pwm with given settings */
    hosal_pwm_init(pwm);
}

static void qyq_led_drive_set_duty(hosal_pwm_dev_t *pwm, uint32_t duty)
{
    /* change pwm param */
    hosal_pwm_config_t para;
    para.duty_cycle = duty; // 8000/10000=80%
    para.freq = 1000;

    hosal_pwm_para_chg(pwm, para);
}

static void qyq_led_drive_pwm_start(hosal_pwm_dev_t *pwm)
{
    hosal_pwm_start(pwm);
}

static void qyq_led_drive_pwm_stop(hosal_pwm_dev_t *pwm)
{
    /* stop led_drive_pwm */
    hosal_pwm_stop(pwm);
}

void HSVtoRGB(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    float r_temp, g_temp, b_temp;
    int i;
    float f, p, q, t;

    // 确保 h 在 [0, 360] 之间
    if (h < 0)
        h = 0;
    if (h >= 360)
        h = 360;

    // 将 s 和 v 转换为 [0, 1] 之间
    s = s / 100.0f;
    v = v / 100.0f;

    if (s == 0)
    {
        // 饱和度为 0，灰色
        r_temp = g_temp = b_temp = v;
    }
    else
    {
        h /= 60;    // 将 h 转换为 [0, 6)
        i = (int)h; // 整数部分
        f = h - i;  // 小数部分
        p = v * (1 - s);
        q = v * (1 - f * s);
        t = v * (1 - (1 - f) * s);

        switch (i)
        {
        case 0:
            r_temp = v;
            g_temp = t;
            b_temp = p;
            break;
        case 1:
            r_temp = q;
            g_temp = v;
            b_temp = p;
            break;
        case 2:
            r_temp = p;
            g_temp = v;
            b_temp = t;
            break;
        case 3:
            r_temp = p;
            g_temp = q;
            b_temp = v;
            break;
        case 4:
            r_temp = t;
            g_temp = p;
            b_temp = v;
            break;
        case 5:
        default:
            r_temp = v;
            g_temp = p;
            b_temp = q;
            break;
        }
    }

    // 将 RGB 转换为 0-255 的 uint8_t
    *r = (uint8_t)(r_temp * 255);
    *g = (uint8_t)(g_temp * 255);
    *b = (uint8_t)(b_temp * 255);
}

void qyq_led_drive_set_led(uint8_t red, uint8_t green, uint8_t blue)
{
    qyq_led_drive_set_duty(&led_r_drive_pwm, 39 * red);
    qyq_led_drive_set_duty(&led_g_drive_pwm, 39 * green);
    qyq_led_drive_set_duty(&led_b_drive_pwm, 39 * blue);
}

void aiio_cloud_led_init(void)
{
    // 初始化PWM
    qyq_led_drive_pwm_init(&led_r_drive_pwm, 14, 0);
    qyq_led_drive_pwm_init(&led_g_drive_pwm, 17, 0);
    qyq_led_drive_pwm_init(&led_b_drive_pwm, 3, 0);

    // 启动PWM
    qyq_led_drive_pwm_start(&led_r_drive_pwm);
    qyq_led_drive_pwm_start(&led_g_drive_pwm);
    qyq_led_drive_pwm_start(&led_b_drive_pwm);

    // aiio_hal_gpio_init(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN);
    // aiio_hal_gpio_pin_direction_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, AIIO_GPIO_OUTPUT);
    // aiio_hal_gpio_pin_pull_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, AIIO_GPIO_PULL_DOWN);

    // aiio_hal_gpio_init(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN);
    // aiio_hal_gpio_pin_direction_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, AIIO_GPIO_OUTPUT);
    // aiio_hal_gpio_pin_pull_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, AIIO_GPIO_PULL_DOWN);

    // aiio_hal_gpio_init(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN);
    // aiio_hal_gpio_pin_direction_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, AIIO_GPIO_OUTPUT);
    // aiio_hal_gpio_pin_pull_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, AIIO_GPIO_PULL_DOWN);
}

void aiio_red_led_triggle(void)
{
    static bool status = false;

#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
    if (status)
    {
        // aiio_hal_gpio_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, 0);
        qyq_led_drive_set_duty(&led_r_drive_pwm, 0);
        status = false;
    }
    else
    {
        // aiio_hal_gpio_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, 1);
        qyq_led_drive_set_duty(&led_r_drive_pwm, 10000);
        status = true;
    }
#endif
}

void aiio_red_led_status(bool status)
{
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
    if (status)
    {
        qyq_led_drive_set_duty(&led_r_drive_pwm, 10000);
        // aiio_hal_gpio_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, 1);
    }
    else
    {
        qyq_led_drive_set_duty(&led_r_drive_pwm, 0);
        // aiio_hal_gpio_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, 0);
    }
#endif
}

void aiio_green_led_triggle(void)
{
    static bool status = false;
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
    if (status)
    {
        // aiio_hal_gpio_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, 0);
        qyq_led_drive_set_duty(&led_g_drive_pwm, 0);
        status = false;
    }
    else
    {
        // aiio_hal_gpio_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, 1);
        qyq_led_drive_set_duty(&led_g_drive_pwm, 10000);
        status = true;
    }
#endif
}

void aiio_green_led_status(bool status)
{
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
    if (status)
    {
        qyq_led_drive_set_duty(&led_g_drive_pwm, 10000);
        // aiio_hal_gpio_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, 1);
    }
    else
    {
        qyq_led_drive_set_duty(&led_g_drive_pwm, 0);
        // aiio_hal_gpio_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, 0);
    }
#endif
}

void aiio_blue_led_triggle(void)
{
    static bool status = false;

    if (status)
    {
        qyq_led_drive_set_duty(&led_b_drive_pwm, 0);
        // aiio_hal_gpio_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, 0);
        status = false;
    }
    else
    {
        qyq_led_drive_set_duty(&led_b_drive_pwm, 10000);
        // aiio_hal_gpio_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, 1);
        status = true;
    }
    blue_led_status = status;
    // #ifdef CONFIG_CLOUD_CONTROL_LED_MODULE
    //     aiio_set_powerstate_status(blue_led_status);
    // #endif
}

void aiio_blue_led_status(bool status)
{
    if (status)
    {
        qyq_led_drive_set_duty(&led_b_drive_pwm, 10000);
        // aiio_hal_gpio_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, 1);
    }
    else
    {
        qyq_led_drive_set_duty(&led_b_drive_pwm, 0);
        // aiio_hal_gpio_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, 0);
    }
    blue_led_status = status;
    // #ifdef CONFIG_CLOUD_CONTROL_LED_MODULE
    //     aiio_set_powerstate_status(blue_led_status);
    // #endif
}

void aiio_all_led_triggle(void)
{
    aiio_red_led_status(false);
    aiio_green_led_status(false);
    aiio_blue_led_status(false);
    aiio_red_led_triggle();
    aiio_green_led_triggle();
    aiio_blue_led_triggle();
}

bool aiio_get_blue_led_status(void)
{
    return blue_led_status;
}

void aiio_turn_on_led_status(aiio_led_status_t led_status)
{
    switch (led_status)
    {
    case ONLY_GREEN_LED_TURN_ON:
    {
        led_status_change_enable = false;
        aiio_red_led_status(false);
        aiio_green_led_status(true);
        aiio_blue_led_status(false);
    }
    break;

    case ONLY_BLUE_LED_TURN_ON:
    {
        led_status_change_enable = false;
        aiio_red_led_status(false);
        aiio_green_led_status(false);
        aiio_blue_led_status(true);
    }
    break;

    case ONLY_RED_LED_TURN_ON:
    {
        led_status_change_enable = false;
        aiio_red_led_status(true);
        aiio_green_led_status(false);
        aiio_blue_led_status(false);
    }
    break;

    case GREEN_RED_LED_TURN_ON:
    {
        led_status_change_enable = false;
        aiio_red_led_status(true);
        aiio_green_led_status(true);
        aiio_blue_led_status(false);
    }
    break;

    case BLUE_RED_LED_TURN_ON:
    {
        led_status_change_enable = false;
        aiio_red_led_status(true);
        aiio_green_led_status(false);
        aiio_blue_led_status(true);
    }
    break;

    case BLUE_GREEN_TURN_ON:
    {
        led_status_change_enable = false;
        aiio_red_led_status(false);
        aiio_green_led_status(true);
        aiio_blue_led_status(true);
    }
    break;

    case ALL_LED_TURN_ON:
    {
        led_status_change_enable = false;
        aiio_red_led_status(true);
        aiio_green_led_status(true);
        aiio_blue_led_status(true);
    }
    break;

    case ALL_LED_TURN_OFF:
    {
        led_status_change_enable = false;
        aiio_red_led_status(false);
        aiio_green_led_status(false);
        aiio_blue_led_status(false);
    }
    break;

    case ONLY_GREEN_LED_TRIGGLE_STATUS:
    {
        aiio_red_led_status(false);
        aiio_green_led_status(false);
        aiio_blue_led_status(false);
        led_status_change_enable = true;
        led_triggle_status = ONLY_GREEN_LED_TRIGGLE_STATUS;
    }
    break;

    case ONLY_BLUE_LED_TRIGGLE_STATUS:
    {
        aiio_red_led_status(false);
        aiio_green_led_status(false);
        aiio_blue_led_status(false);
        led_status_change_enable = true;
        led_triggle_status = ONLY_BLUE_LED_TRIGGLE_STATUS;
    }
    break;

    case ONLY_RED_LED_TRIGGLE_STATUS:
    {
        aiio_red_led_status(false);
        aiio_green_led_status(false);
        aiio_blue_led_status(false);
        led_status_change_enable = true;
        led_triggle_status = ONLY_RED_LED_TRIGGLE_STATUS;
    }
    break;

    case GREEN_RED_LED_TRIGGLE_STATUS:
    {
        aiio_red_led_status(false);
        aiio_green_led_status(false);
        aiio_blue_led_status(false);
        led_status_change_enable = true;
        led_triggle_status = GREEN_RED_LED_TRIGGLE_STATUS;
    }
    break;

    case BLUE_RED_LED_TRIGGLE_STATUS:
    {
        aiio_red_led_status(false);
        aiio_green_led_status(false);
        aiio_blue_led_status(false);
        led_status_change_enable = true;
        led_triggle_status = BLUE_RED_LED_TRIGGLE_STATUS;
    }
    break;

    case GREEN_BLUE_LED_TRIGGLE_STATUS:
    {
        aiio_red_led_status(false);
        aiio_green_led_status(false);
        aiio_blue_led_status(false);
        led_status_change_enable = true;
        led_triggle_status = GREEN_BLUE_LED_TRIGGLE_STATUS;
    }
    break;

    case ALL_LED_TRIGGLE_STATUS:
    {
        aiio_red_led_status(false);
        aiio_green_led_status(false);
        aiio_blue_led_status(false);
        led_status_change_enable = true;
        led_triggle_status = ALL_LED_TRIGGLE_STATUS;
    }
    break;

    default:
        break;
    }
}

void aiio_led_status_process(void)
{
    if (led_status_change_enable)
    {
        switch (led_triggle_status)
        {
        case ONLY_GREEN_LED_TRIGGLE_STATUS:
        {
            aiio_green_led_triggle();
        }
        break;

        case ONLY_BLUE_LED_TRIGGLE_STATUS:
        {
            aiio_blue_led_triggle();
        }
        break;

        case ONLY_RED_LED_TRIGGLE_STATUS:
        {
            aiio_red_led_triggle();
        }
        break;

        case GREEN_RED_LED_TRIGGLE_STATUS:
        {
            aiio_green_led_triggle();
            aiio_red_led_triggle();
        }
        break;

        case BLUE_RED_LED_TRIGGLE_STATUS:
        {
            aiio_red_led_triggle();
            aiio_blue_led_triggle();
        }
        break;

        case GREEN_BLUE_LED_TRIGGLE_STATUS:
        {
            aiio_blue_led_triggle();
            aiio_green_led_triggle();
        }
        break;

        case ALL_LED_TRIGGLE_STATUS:
        {
            aiio_blue_led_triggle();
            aiio_green_led_triggle();
            aiio_red_led_triggle();
        }
        break;

        default:
            break;
        }
    }
}
static uint8_t receptacle_report_status = 0;
void aiio_receptacle_report_status_set(void)
{
    receptacle_report_status = 1;
}

void aiio_receptacle_report_status_clr(void)
{
    receptacle_report_status = 0;
}

void aiio_receptacle_report_task(void *msg)
{
    uint32_t cnt = 3;
    while (1)
    {
        if (receptacle_report_status)
        {
            cnt--;
            aiio_report_all_attibute_status(NULL, NULL);
            if (cnt == 0)
            {
                cnt = 3;
                receptacle_report_status = 0;
            }
        }
        aiio_log_e("aiio_receptacle_report_task\r\n");
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
