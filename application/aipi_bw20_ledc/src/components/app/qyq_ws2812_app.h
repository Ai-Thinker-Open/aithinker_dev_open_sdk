#ifndef __QYQ_WS2812_APP_H_
#define __QYQ_WS2812_APP_H_
#include "chip_include.h"
#include "app_include.h"
#include "drive_include.h"
#include "interface_include.h"
#include "algorithm_include.h"
#ifdef __QYQ_WS2812_APP_C_
#define QYQ_WS2812_APP_EXT
#else
#define QYQ_WS2812_APP_EXT extern
#endif

typedef struct
{
    // 电源开关模式
    uint32_t ws2812_power_status;
    // 工作模式：0 颜色控制模式  1 流水灯模式  2 呼吸渐变模式  3 音乐灯模式
    uint32_t ws2812_mode;       // 用于区分模式
    uint32_t ws2812_work_mode;  // 用于区分哪个界面
    // 灯珠数量
    int32_t led_num;
    // 正常模式
    int32_t normal_color; // 范围限制：0-360
    int32_t normal_contrast;
    int32_t normal_luminance;
    // 音乐灯模式
    int32_t music_luminance;
    int32_t music_color;
    // 呼吸渐变亮度和速度
    int32_t color_luminance;
    int32_t color_delay;
    // 流水灯亮度和速度
    int32_t water_luminance;
    int32_t water_delay;
} qyq_ws2812_app_type_t;

QYQ_WS2812_APP_EXT void bw20_flash_save(void);
QYQ_WS2812_APP_EXT void qyq_ws2812_app_receive_task(void *para);
QYQ_WS2812_APP_EXT void qyq_ws2812_app_task(void *para);
#endif
