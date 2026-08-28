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



void aiio_cloud_led_init(void);
void aiio_red_led_triggle(void);
void aiio_red_led_status(bool status);
void aiio_green_led_triggle(void);
void aiio_green_led_status(bool status);
void aiio_blue_led_triggle(void);
void aiio_blue_led_status(bool status);
void aiio_all_led_triggle(void);

bool aiio_get_blue_led_status(void);

void aiio_turn_on_led_status(aiio_led_status_t led_status);
void aiio_led_status_process(void);


// 初始化
void aiio_cloud_receptacle_led_init(void);

// 用于绿灯和黄灯闪烁
void aiio_cloud_receptacle_led_status_process(void);

// 用于确定闪烁使能
void aiio_cloud_receptacle_led_enable(uint8_t status);

// 获取使能状态
uint8_t aiio_cloud_receptacle_led_get_enable(void);

// 获取绿色灯的状态
bool aiio_get_green_led_status(void);

// 设置LED的状态
void aiio_cloud_receptacle_set_led_status(aiio_led_status_t led_status);


uint8_t aiio_cloud_receptacle_led_get_triggle(void);
#endif
