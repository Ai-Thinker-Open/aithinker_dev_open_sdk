/*
  * AI-THINKER MIT License
  *
  * Copyright (c) 2017 <AI-THINKER SYSTEMS (SHENZHEN) PTE LTD>
  *
  * Permission is hereby granted for use on AI-THINKER SYSTEMS products only, in which case,
  * it is free of charge, to any person obtaining a copy of this software and associated
  * documentation files (the "Software"), to deal in the Software without restriction, including
  * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
  * to do so, subject to the following conditions:
  *
  *
  * The above copyright notice and this permission notice shall be included in all copies or
  * substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  *
  *
  */
#ifndef _M_LEDCDEVINFO_H_
#define _M_LEDCDEVINFO_H_

#ifdef __cplusplus
extern "C" {
#endif


// #include "driver/ledc.h"
// #include "driver/gpio.h"
#include "stdint.h"
#include "stdbool.h"

#define USER_COLOR_TEMPERATURE_MAX (70) //冷光色温
#define USER_COLOR_TEMPERATURE_MIN (20) //暖光色温
/**
 * @ 默认灯光灯颜色
 * 0:白光
 * 1:暖光
 * 2:红光
 * 3:绿光
 * 4:蓝光
 */
#define TEMPERATURE_CONNECT_BEFORE (1) //默认灯光灯光选择
//@设备渐变分辨率，单位:ms
#define LEDC_INDICATOR_TIMER_CONFIG (1)


// typedef struct
// {
//     bool isInit;
//     bool isStart;
//     int time_cycle;
//     uint32_t maxDutyCycle;
//     ledc_channel_config_t channel;
// } ledc_t;

typedef struct
{
    bool isInitLed;       //led是否初始化
    bool isStuLed;        //led当前状态
    int time;             //当前状态保持时间
    int temporaryTime;    //临时变量
//    gpio_num_t GPIO_PINx; //LED_GPIO
} led_t;

typedef enum
{

    READING = 1,        // 阅读
    MUSIC = 2,              //音乐
    SUNSET = 3,           //日落
    NIGHT_LIGHT = 4, // 夜灯
    CINEMA = 6,      // 影院
    LIVE = 8,        // 起床模式
    SOFT = 57,       // 护眼模式
    MANUAL = 353,    // 手动
} e_work_mode;

typedef enum
{
    B_COLOR = 255,      // B
    G_COLOR = 65280,    // G
    R_COLOR = 16711680, // R
} e_color;

typedef enum
{
    MONO = 0, // 白灯
    COLOR, // 彩灯
    SCENE, //情景模式
    MAXMODE
} e_light_mode;

typedef struct
{
    uint32_t Hue;
    uint32_t Saturation;
    uint32_t Value;
} dev_hsv;

typedef struct
{
    uint32_t Red;
    uint32_t Green;
    uint32_t Blue;
} dev_rgb;

typedef struct
{
    bool LightSwitch;//开关
    e_work_mode WorkMode;//情景模式
    e_light_mode LightMode;//彩灯白灯模式
    dev_hsv HSVColor;//色调，饱和度，明度
    dev_rgb RGBColor;//
    uint32_t Brightness;//亮度
    uint32_t ColorTemperature;//色温
    uint32_t ColorSpeed;
} json_info;

typedef struct
{
    uint32_t dev_switch;                //灯开关
    uint32_t dev_c;
    uint32_t dev_w;
    uint32_t dev_r;
    uint32_t dev_g;
    uint32_t dev_b;
    json_info dev_json_info;
    uint32_t dev_voice_ctrl;
} dev_info;

// extern SemaphoreHandle_t  xSemaphore_light;

void get_dev_now_info(dev_info * dev_info_get);
void dev_info_set(dev_info dev_info_t);
void dev_info_get(dev_info *dev_info_t);
void dev_info_parse(dev_info *dev_info_t);
uint32_t ledc_gradient_update_s(uint32_t *target_duty);
uint32_t ledc_gradient_update_st(dev_info dev_info_t);
void ledc_params_reset(dev_info *dev_info_t);
int ledcDevInitialize(void);
void light_off(void);

void set_dev_switch(unsigned int d);
void set_dev_Brightness(unsigned int d);
void set_dev_Hue(unsigned int d);
void set_dev_Saturation(unsigned int d);
void set_dev_Scene(unsigned int d);
void set_dev_Value(unsigned int d);
void set_dev_Temperature(unsigned int d);
void set_dev_LightMode(unsigned int d);
//更新完整属性
void updata_dev_info(void);
int ledcDevRestart(void);
int ledcDevRestore(void);
int start_open_ledc(void);

#ifdef __cplusplus
}
#endif

#endif  //_M_LEDCDEVINFO_H_
