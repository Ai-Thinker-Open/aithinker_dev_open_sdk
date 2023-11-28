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




#define AIIO_GPIO_R_LED_PORT CONFIG_GPIO_R_LED_PORT
#define AIIO_GPIO_R_LED_PIN CONFIG_GPIO_R_LED_PIN

#define AIIO_GPIO_G_LED_PORT CONFIG_GPIO_G_LED_PORT
#define AIIO_GPIO_G_LED_PIN CONFIG_GPIO_G_LED_PIN

#define AIIO_GPIO_B_LED_PORT CONFIG_GPIO_B_LED_PORT
#define AIIO_GPIO_B_LED_PIN CONFIG_GPIO_B_LED_PIN


static bool led_status_change_enable = false;                               /*!< switch variable of led twinkling*/
static uint8_t led_triggle_status = ALL_LED_TRIGGLE_STATUS;

void aiio_cloud_led_init(void)
{
    aiio_hal_gpio_init(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN);
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, AIIO_GPIO_OUTPUT);
    aiio_hal_gpio_pin_pull_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, AIIO_GPIO_PULL_DOWN);

    aiio_hal_gpio_init(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN);
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, AIIO_GPIO_OUTPUT);
    aiio_hal_gpio_pin_pull_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, AIIO_GPIO_PULL_DOWN);

    aiio_hal_gpio_init(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN);
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, AIIO_GPIO_OUTPUT);
    aiio_hal_gpio_pin_pull_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, AIIO_GPIO_PULL_DOWN);
}


void aiio_red_led_triggle(void)
{
    static bool status = false;

    if(status)
    {
        aiio_hal_gpio_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, 0);
        status = false;
    }
    else
    {
        aiio_hal_gpio_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, 1);
        status = true;
    }
}


void aiio_red_led_status(bool status)
{
    if(status)
    {
        aiio_hal_gpio_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, 1);
    }
    else
    {
        aiio_hal_gpio_set(AIIO_GPIO_R_LED_PORT, AIIO_GPIO_R_LED_PIN, 0);
    }
}



void aiio_green_led_triggle(void)
{
    static bool status = false;

    if(status)
    {
        aiio_hal_gpio_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, 0);
        status = false;
    }
    else
    {
        aiio_hal_gpio_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, 1);
        status = true;
    }
}


void aiio_green_led_status(bool status)
{
    if(status)
    {
        aiio_hal_gpio_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, 1);
    }
    else
    {
        aiio_hal_gpio_set(AIIO_GPIO_G_LED_PORT, AIIO_GPIO_G_LED_PIN, 0);
    }
}


void aiio_blue_led_triggle(void)
{
    static bool status = false;

    if(status)
    {
        aiio_hal_gpio_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, 0);
        status = false;
    }
    else
    {
        aiio_hal_gpio_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, 1);
        status = true;
    }
}


void aiio_blue_led_status(bool status)
{
    if(status)
    {
        aiio_hal_gpio_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, 1);
    }
    else
    {
        aiio_hal_gpio_set(AIIO_GPIO_B_LED_PORT, AIIO_GPIO_B_LED_PIN, 0);
    }
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

