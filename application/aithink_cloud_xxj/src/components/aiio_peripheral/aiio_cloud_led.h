/**
 * @brief   Declare interface of led application function
 * 
 * @file    aiio_cloud_led.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note     This file is mainly describing led application
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-16          <td>1.0.0            <td>zhuolm             <td> The interface of led application
 */
#ifndef __AIIO_CLOUD_LED_H_
#define __AIIO_CLOUD_LED_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


typedef enum
{
    ONLY_GREEN_LED_TRIGGLE_STATUS,
    ONLY_BLUE_LED_TRIGGLE_STATUS,
    ONLY_RED_LED_TRIGGLE_STATUS,
    ONLY_GREEN_LED_TURN_ON,
    ONLY_BLUE_LED_TURN_ON,
    ONLY_RED_LED_TURN_ON,
    GREEN_RED_LED_TRIGGLE_STATUS,
    BLUE_RED_LED_TRIGGLE_STATUS,
    GREEN_BLUE_LED_TRIGGLE_STATUS,
    ALL_LED_TRIGGLE_STATUS,
    GREEN_RED_LED_TURN_ON,
    BLUE_RED_LED_TURN_ON,
    BLUE_GREEN_TURN_ON,
    ALL_LED_TURN_ON,
    ALL_LED_TURN_OFF
}aiio_led_status_t;

typedef enum
{
    XXJ_SIZE_ONE=0,
    XXJ_SIZE_TWO,
    XXJ_SIZE_THREE,
    XXJ_SIZE_OFF
}aiio_xxj_size_t;

void aiio_gpio_init(void);
void aiio_pwm_control(uint8_t pwm_duty);
void aiio_red_led_triggle(void);
void aiio_red_led_status(bool status);
void aiio_blue_led_triggle(void);
void aiio_blue_led_status(bool status);
void aiio_all_led_triggle(void);

void aiio_turn_on_led_status(aiio_led_status_t led_status);
void aiio_led_status_process(void);
void aiio_spray_set(int spray);

#endif
