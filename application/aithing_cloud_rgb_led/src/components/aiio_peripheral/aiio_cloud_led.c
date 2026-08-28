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

#include "aiio_pwm.h"
#include "aiio_gpio.h"




#define AIIO_GPIO_R_LED_PORT AIIO_GPIO_A
#define AIIO_GPIO_R_LED_PIN AIIO_GPIO_NUM_14

#define AIIO_GPIO_G_LED_PORT AIIO_GPIO_A
#define AIIO_GPIO_G_LED_PIN AIIO_GPIO_NUM_17

#define AIIO_GPIO_B_LED_PORT AIIO_GPIO_A
#define AIIO_GPIO_B_LED_PIN AIIO_GPIO_NUM_3

#define AIIO_GPIO_C_LED_PORT AIIO_GPIO_A
#define AIIO_GPIO_C_LED_PIN AIIO_GPIO_NUM_5

#define AIIO_GPIO_W_LED_PORT AIIO_GPIO_A
#define AIIO_GPIO_W_LED_PIN AIIO_GPIO_NUM_1



static bool led_status_change_enable = false;                               /*!< switch variable of led twinkling*/
static uint8_t led_triggle_status = ALL_LED_TRIGGLE_STATUS;
static bool blue_led_status = false;

static int pwm_num[5];

aiio_pwm_init_t init_ch[5] = {
    {
        .freq = 5000,
        .duty = 1,
        .gpio_port = AIIO_GPIO_R_LED_PORT,
        .gpio_pin = AIIO_GPIO_R_LED_PIN,
    },

    {
        .freq = 5000,
        .duty = 1,
        .gpio_port = AIIO_GPIO_G_LED_PORT,
        .gpio_pin = AIIO_GPIO_G_LED_PIN,
    },

    {
        .freq = 5000,
        .duty = 1,
        .gpio_port = AIIO_GPIO_B_LED_PORT,
        .gpio_pin = AIIO_GPIO_B_LED_PIN,
    },

    {
        .freq = 5000,
        .duty = 1,
        .gpio_port = AIIO_GPIO_C_LED_PORT,
        .gpio_pin = AIIO_GPIO_C_LED_PIN,
    },

    {
        .freq = 5000,
        .duty = 1,
        .gpio_port = AIIO_GPIO_W_LED_PORT,
        .gpio_pin = AIIO_GPIO_W_LED_PIN,
    }
};

void aiio_cloud_led_init(void)
{
    // aiio_hal_gpio_init(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN);
    // aiio_hal_gpio_pin_direction_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, AIIO_GPIO_OUTPUT);
    // aiio_hal_gpio_pin_pull_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, AIIO_GPIO_PULL_DOWN);

    // aiio_hal_gpio_init(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN);
    // aiio_hal_gpio_pin_direction_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, AIIO_GPIO_OUTPUT);
    // aiio_hal_gpio_pin_pull_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, AIIO_GPIO_PULL_DOWN);

    // aiio_hal_gpio_init(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN);
    // aiio_hal_gpio_pin_direction_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, AIIO_GPIO_OUTPUT);
    // aiio_hal_gpio_pin_pull_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, AIIO_GPIO_PULL_DOWN);

    
    int ret;

    // aiio_pwm_init_t init_ch0 = {
    //     .freq = 5000,
    //     .duty = 1,
    //     .gpio_port = AIIO_GPIO_R_LED_PORT,
    //     .gpio_pin = AIIO_GPIO_R_LED_PIN,
    // };

    // aiio_pwm_init_t init_ch1 = {
    //     .freq = 5000,
    //     .duty = 1,
    //     .gpio_port = AIIO_GPIO_G_LED_PORT,
    //     .gpio_pin = AIIO_GPIO_G_LED_PIN,
    // };

    // aiio_pwm_init_t init_ch2 = {
    //     .freq = 5000,
    //     .duty = 1,
    //     .gpio_port = AIIO_GPIO_B_LED_PORT,
    //     .gpio_pin = AIIO_GPIO_B_LED_PIN,
    // };

    // aiio_pwm_init_t init_ch3 = {
    //     .freq = 5000,
    //     .duty = 1,
    //     .gpio_port = AIIO_GPIO_C_LED_PORT,
    //     .gpio_pin = AIIO_GPIO_C_LED_PIN,
    // };

    // aiio_pwm_init_t init_ch4 = {
    //     .freq = 5000,
    //     .duty = 1,
    //     .gpio_port = AIIO_GPIO_W_LED_PORT,
    //     .gpio_pin = AIIO_GPIO_W_LED_PIN,
    // };


    pwm_num[0] = aiio_pwm_init(&init_ch[0]);           // 初始化PWM,设置频率为10K,占空比为20%
    // ret = aiio_pwm_start(pwm_num[0]);             // 开始产生PWM

    pwm_num[1] = aiio_pwm_init(&init_ch[1]);           // 初始化PWM,设置频率为10K,占空比为20%
    // ret = aiio_pwm_start(pwm_num[1]);             // 开始产生PWM
    
    pwm_num[2] = aiio_pwm_init(&init_ch[2]);           // 初始化PWM,设置频率为10K,占空比为20%
    // ret = aiio_pwm_start(pwm_num[2]);             // 开始产生PWM

    pwm_num[3] = aiio_pwm_init(&init_ch[3]);

    pwm_num[4] = aiio_pwm_init(&init_ch[4]);

    // aiio_cloud_led_pwm_set(pwm_num[0], 50);
    // aiio_cloud_led_pwm_set(pwm_num[1], 50);
    // aiio_cloud_led_pwm_set(pwm_num[2], 50);
    // aiio_cloud_led_pwm_set(pwm_num[3], 50);
    // aiio_cloud_led_pwm_set(pwm_num[4], 50);
}

void aiio_cloud_led_pwm_set(int ch, uint32_t pwm_duty)
{
    uint32_t duty = pwm_duty;
    // uint32_t duty = pwm_duty * 100 / 8196;
    if(ch > 5 || ch < 0){
        aiio_log_e("aiio_cloud_led_pwm_set error ch:%d\r\n", ch);
        return;
    }
    aiio_log_d("aiio_cloud_led_pwm_set ch:%d,duty:%d\r\n", ch,duty);
    if (duty > 100)
        duty = 100;

    if(duty == 0){
        // if(ch == 3){
        //     aiio_pwm_set_duty(pwm_num[ch], 1);
        //     aiio_pwm_start(pwm_num[ch]);
        // }
        aiio_pwm_output_close(pwm_num[ch]);
        return;
    }
    // pwm_num[ch] = aiio_pwm_init(&init_ch[ch]);
    aiio_pwm_set_duty(pwm_num[ch], duty); // 配置占空比
    aiio_pwm_start(pwm_num[ch]);
}

void aiio_cloud_led_all_off(void)
{
    aiio_pwm_output_close(pwm_num[0]);
    aiio_pwm_output_close(pwm_num[1]);
    aiio_pwm_output_close(pwm_num[2]);
    aiio_pwm_output_close(pwm_num[3]);
    // aiio_pwm_set_duty(pwm_num[3], 1);
    // aiio_pwm_start(pwm_num[3]);
    aiio_pwm_output_close(pwm_num[4]);
}


void aiio_red_led_triggle(void)
{
    static bool status = false;

#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
    if(status)
    {
        // aiio_hal_gpio_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, 0);
        // aiio_cloud_led_pwm_set(0, 1);
        aiio_pwm_output_close(pwm_num[0]);
        status = false;
    }
    else
    {
        // aiio_hal_gpio_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, 1);
        aiio_cloud_led_pwm_set(0, 100);
        aiio_pwm_start(pwm_num[0]);
        status = true;
    }
#endif
}


void aiio_red_led_status(bool status)
{
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
    if(status)
    {
        // aiio_hal_gpio_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, 1);
        aiio_cloud_led_pwm_set(0, 100);
        aiio_pwm_start(pwm_num[0]);
    }
    else
    {
        // aiio_hal_gpio_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, 0);
        // aiio_cloud_led_pwm_set(0, 1);
        aiio_pwm_output_close(pwm_num[0]);
    }
#endif
}



void aiio_green_led_triggle(void)
{
    static bool status = false;
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
    if(status)
    {
        // aiio_hal_gpio_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, 0);
        // aiio_cloud_led_pwm_set(1, 1);
        aiio_pwm_output_close(pwm_num[1]);
        status = false;
    }
    else
    {
        // aiio_hal_gpio_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, 1);
        aiio_cloud_led_pwm_set(1, 100);
        aiio_pwm_start(pwm_num[1]);
        status = true;
    }
#endif
}


void aiio_green_led_status(bool status)
{
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
    if(status)
    {
        // aiio_hal_gpio_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, 1);
        aiio_cloud_led_pwm_set(1, 100);
        aiio_pwm_start(pwm_num[1]);
    }
    else
    {
        // aiio_hal_gpio_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, 0);
        // aiio_cloud_led_pwm_set(1, 1);
        aiio_pwm_output_close(pwm_num[1]);
    }
#endif
}


void aiio_blue_led_triggle(void)
{
    static bool status = false;

    if(status)
    {
        // aiio_hal_gpio_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, 0);
        // aiio_cloud_led_pwm_set(2, 1);
        aiio_pwm_output_close(pwm_num[2]);
        status = false;
    }
    else
    {
        // aiio_hal_gpio_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, 1);
        aiio_cloud_led_pwm_set(2, 100);
        aiio_pwm_start(pwm_num[2]);
        status = true;
    }
    blue_led_status = status;
#ifdef CONFIG_CLOUD_CONTROL_LED_MODULE
    aiio_set_powerstate_status(blue_led_status);
#endif
}


void aiio_blue_led_status(bool status)
{
    if(status)
    {
        // aiio_hal_gpio_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, 1);
        aiio_cloud_led_pwm_set(2, 100);
        aiio_pwm_start(pwm_num[2]);
    }
    else
    {
        // aiio_hal_gpio_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, 0);
        // aiio_cloud_led_pwm_set(2, 1);
        aiio_pwm_output_close(pwm_num[2]);
    }
    blue_led_status = status;
#ifdef CONFIG_CLOUD_CONTROL_LED_MODULE
    aiio_set_powerstate_status(blue_led_status);
#endif
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
    if(led_status_change_enable)
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

