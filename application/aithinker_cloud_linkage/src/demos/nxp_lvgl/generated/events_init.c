/*
 * Copyright 2023 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */

#include "aiio_adapter_include.h"
#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

enum
{
    SPRAY_MODE = 0x01,
    LAMP_BRIGHTNESS = 0x02,
    LAMP_SWITCH,
    POWER_STATE,
    CPICKER_RGB,
};

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} lv_usr_rgb_t;

typedef struct
{
    uint16_t powerstate : 1;       /**< 开关*/
    uint16_t light_switch : 1;     /**< 灯光开关*/
    uint16_t countdown_switch : 1; /**< 倒计时开关*/
    uint16_t clean_tip : 1;        /**< 清洗提示*/
    uint16_t water_tip : 1;        /**< 缺水提示*/
    uint16_t light_builtin : 1;    /**< 灯光内置模式*/
    uint16_t reserved : 10;        /**< 保留*/

    uint8_t spray_mode;         /**< 档位设定*/
    uint8_t light_brightness;   /**< 灯光亮度*/
    uint8_t light_mode;         /**< 灯光模式*/
    uint8_t clean_time;         /**< 清洗时长*/
    uint8_t remain_electricity; /**< 剩余电量比率*/
    uint8_t remain_water;       /**< 剩余水量比率*/
    uint16_t countdown;         /**< 倒计时设置*/
    uint16_t countdown_remain;  /**< 倒计时剩余时间*/

    lv_usr_rgb_t rgb;
} lv_usr_info_t;

lv_usr_info_t dev_info;

static void cpicker_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        uint8_t addr = (uint8_t)lv_event_get_user_data(e);
        switch (addr)
        {

        case CPICKER_RGB:
        {

            lv_obj_t *obj_colorwheel = lv_event_get_current_target(e);
            if (obj_colorwheel != NULL)
            {
                lv_color_t color = lv_colorwheel_get_rgb(obj_colorwheel);
#if LV_COLOR_16_SWAP == 0
                dev_info.rgb.r = color.ch.red << 3;
                dev_info.rgb.g = color.ch.green << 2;
                dev_info.rgb.b = color.ch.blue << 3;
#else
                dev_info.rgb.r = color.ch.red << 3;
                dev_info.rgb.g = (color.ch.green_h << 3 | green_l) << 3;
                dev_info.rgb.b = color.ch.blue << 3;
#endif
                aiio_log_i("Red:%d\n\nGreen:%d\n\nBlue:%d", dev_info.rgb.r, dev_info.rgb.g, dev_info.rgb.b);
            }
        }
        break;
        default:
            break;
        }
    }
}
static void slider_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    uint8_t addr = (uint8_t)lv_event_get_user_data(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        int duty = (int)lv_slider_get_value(obj);
        switch (addr)
        {
        case SPRAY_MODE:
        {
            dev_info.spray_mode = duty;
            aiio_log_i("SPRAY_MODE val:%d-0x%02x\r\n", dev_info.spray_mode, addr);
        }
        break;
        case LAMP_BRIGHTNESS:
        {
            dev_info.light_brightness = duty;
            aiio_log_i("LAMP_BRIGHTNESS val:%d-0x%02x\r\n", dev_info.light_brightness, addr);
        }
        break;
        case LAMP_SWITCH:
        {
            dev_info.light_switch = lv_obj_has_state(obj, LV_STATE_CHECKED) ? 1 : 0;
            aiio_log_i("LAMP_SWITCH val:%d  (%s)\r\n", dev_info.light_switch, lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
        }
        break;
        case POWER_STATE:
        {
            dev_info.powerstate = lv_obj_has_state(obj, LV_STATE_CHECKED) ? 1 : 0;
            aiio_log_i("POWER_STATE val:%d (%s)\r\n", dev_info.powerstate, lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
        }
        break;
        default:
            break;
        }
    }
}

void events_init(lv_ui *ui)
{

    lv_obj_add_event_cb(ui->screen_slider_1, slider_event_cb, LV_EVENT_VALUE_CHANGED, (void*)SPRAY_MODE);
    lv_obj_add_event_cb(ui->screen_slider_2, slider_event_cb, LV_EVENT_VALUE_CHANGED, (void*)LAMP_BRIGHTNESS);
    lv_obj_add_event_cb(ui->screen_sw_1, slider_event_cb, LV_EVENT_VALUE_CHANGED, (void*)LAMP_SWITCH);
    lv_obj_add_event_cb(ui->screen_sw_2, slider_event_cb, LV_EVENT_VALUE_CHANGED, (void*)POWER_STATE);
    lv_obj_add_event_cb(ui->screen_cpicker_1, cpicker_event_cb, LV_EVENT_VALUE_CHANGED, (void *)CPICKER_RGB);
}
