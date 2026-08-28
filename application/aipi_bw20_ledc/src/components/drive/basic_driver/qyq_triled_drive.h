#ifndef __QYQ_TRILED_DRIVE_H_
#define __QYQ_TRILED_DRIVE_H_
#include "ameba_soc.h"
#include "os_wrapper.h"
#include <stdio.h>
#include "platform_autoconf.h"
#include "ameba_ledc.h"
#include "math.h"
#ifdef __QYQ_TRILED_DRIVE_C_
#define QYQ_TRILED_DRIVE_EXT
#else
#define QYQ_TRILED_DRIVE_EXT extern
#endif

// TRILED 控制块
typedef struct
{
    uint8_t status; /**< TRILED状态：1表示打开，0表示关闭 */
    uint8_t red;    /**< 红色分量：范围为0-255 */
    uint8_t green;  /**< 绿色分量：范围为0-255 */
    uint8_t blue;   /**< 蓝色分量：范围为0-255 */
} qyq_triled_control_block_t;

/**
 * @brief 初始化TRILED驱动
 * @param triledlist 指向TRITRILED控制块数组的指针，用于存储多个TRILED的配置信息
 * @param trilednumber 需要初始化的TRILED数量
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_init(qyq_triled_control_block_t *triledlist, uint8_t trilednumber);

/**
 * @brief 打开指定的TRILED
 * @param triled_id 需要打开的TRILED的ID
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_turn_on(uint8_t triled_id);

/**
 * @brief 打开所有的TRILED
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_turn_onall(void);

/**
 * @brief 关闭指定的TRILED
 * @param triled_id 需要关闭的TRILED的ID
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_turn_off(uint8_t triled_id);

/**
 * @brief 关闭所有的TRILED
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_turn_offall(void);

/**
 * @brief 切换指定的TRILED状态（打开->关闭或关闭->打开）
 * @param triled_id 需要切换状态的TRILED的ID
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_toggle(uint8_t triled_id);

/**
 * @brief 获取指定TRILED的当前状态
 * @param triled_id 需要查询的TRILED的ID
 * @return int8_t 返回TRILED的当前状态，1表示打开，0表示关闭，负数表示错误
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_get_state(uint8_t triled_id, qyq_triled_control_block_t *triledstatus);

/**
 * @brief 设置指定TRILED的颜色
 * @param triled_id 需要设置颜色的TRILED的ID
 * @param red 红色分量值（0-255）
 * @param green 绿色分量值（0-255）
 * @param blue 蓝色分量值（0-255）
 * @return int8_t 返回状态码，0表示成功，非0表示失败
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_set_color(uint8_t triled_id, uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief 设置三色LED的颜色。
 *
 * 该函数通过指定红、绿、蓝三个颜色分量的强度值来设置三色LED的颜色。
 *
 * @param red   红色分量的强度值（0-255）。
 * @param green 绿色分量的强度值（0-255）。
 * @param blue  蓝色分量的强度值（0-255）。
 *
 * @return int8_t 返回0表示成功，返回负值表示失败。
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_set_allcolor(uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief 设置指定三色LED的颜色（HSV格式）。
 *
 * 该函数通过指定色调 (Hue)、饱和度 (Saturation) 和亮度 (Value)，
 * 将颜色设置到指定的三色LED上。
 *
 * @param triled_id 指定的三色LED的ID。
 * @param hue       色调，范围为 0-360，表示颜色的基本类型（如红、绿、蓝）。
 * @param sat       饱和度，范围为 0.0-1.0，表示颜色的纯度，0 表示灰色，1 表示最纯的颜色。
 * @param value     亮度，范围为 0.0-1.0，表示颜色的明暗程度，0 表示黑色，1 表示最亮。
 *
 * @return int8_t 返回 0 表示成功，负值表示失败。
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_set_hsv_color(uint8_t triled_id, float hue, float sat, float value);

/**
 * @brief 设置指定三色LED的颜色（HSV格式）。
 *
 * 该函数通过指定色调 (Hue)、饱和度 (Saturation) 和亮度 (Value)，
 * 将颜色设置到指定的三色LED上。
 *
 * @param num 前面多少盏灯。
 * @param hue       色调，范围为 0-360，表示颜色的基本类型（如红、绿、蓝）。
 * @param sat       饱和度，范围为 0.0-1.0，表示颜色的纯度，0 表示灰色，1 表示最纯的颜色。
 * @param value     亮度，范围为 0.0-1.0，表示颜色的明暗程度，0 表示黑色，1 表示最亮。
 *
 * @return int8_t 返回 0 表示成功，负值表示失败。
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_set_hsv_numcolor(uint32_t num, float hue, float sat, float value);

/**
 * @brief 设置指定三色LED的颜色（HSV格式）。
 *
 * 该函数通过指定色调 (Hue)、饱和度 (Saturation) 和亮度 (Value)，
 * 将颜色设置到指定的三色LED上。
 *
 * @param num 前面多少盏灯。
 * @param r_color       色调，范围为 0-360，表示颜色的基本类型（如红、绿、蓝）。
 * @param g_color       饱和度，范围为 0.0-1.0，表示颜色的纯度，0 表示灰色，1 表示最纯的颜色。
 * @param b_color     亮度，范围为 0.0-1.0，表示颜色的明暗程度，0 表示黑色，1 表示最亮。
 *
 * @return int8_t 返回 0 表示成功，负值表示失败。
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_set_rgb_numcolor(uint32_t num, uint8_t r_color, uint8_t g_color, uint8_t b_color);

/**
 * @brief 设置所有三色LED的颜色（HSV格式）。
 *
 * 该函数通过指定色调 (Hue)、饱和度 (Saturation) 和亮度 (Value)，
 * 将颜色设置到所有三色LED上。
 *
 * @param hue   色调，范围为 0-360，表示颜色的基本类型（如红、绿、蓝）。
 * @param sat   饱和度，范围为 0.0-1.0，表示颜色的纯度，0 表示灰色，1 表示最纯的颜色。
 * @param value 亮度，范围为 0.0-1.0，表示颜色的明暗程度，0 表示黑色，1 表示最亮。
 *
 * @return int8_t 返回 0 表示成功，负值表示失败。
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_set_hsv_allcolor(float hue, float sat, float value);

/**
 * @brief 设置指定三色LED的颜色（HSV格式）。
 *
 * 该函数通过指定色调 (Hue)、饱和度 (Saturation) 和亮度 (Value)，
 * 将颜色设置到指定的三色LED上。
 *
 * @param num 前面多少盏灯。
 * @param hue       色调，范围为 0-360，表示颜色的基本类型（如红、绿、蓝）。
 * @param sat       饱和度，范围为 0.0-1.0，表示颜色的纯度，0 表示灰色，1 表示最纯的颜色。
 * @param value     亮度，范围为 0.0-1.0，表示颜色的明暗程度，0 表示黑色，1 表示最亮。
 *
 * @return int8_t 返回 0 表示成功，负值表示失败。
 */
QYQ_TRILED_DRIVE_EXT int8_t qyq_triled_drive_set_hsv_numintervals_color(uint32_t start, uint32_t end, float hue, float sat, float value);



QYQ_TRILED_DRIVE_EXT void qyq_triled_drive_rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b, float *h, float *s, float *v);

QYQ_TRILED_DRIVE_EXT void qyq_triled_drive_hsv_to_rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b);

#endif // __QYQ_TRILED_DRIVE_H_
