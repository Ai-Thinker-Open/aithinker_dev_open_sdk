#ifndef __QYQ_SOFT_BUTTON_H_
#define __QYQ_SOFT_BUTTON_H_
#include "chip_include.h"
#ifndef __QYQ_SOFT_BUTTON_C_
#define QYQ_SOFT_BUTTON_EXT extern
#else
#define QYQ_SOFT_BUTTON_EXT
#endif

/**
 * @brief 按键软件控制块结构体
 * 
 * 此结构体定义了按键软件控制块的相关配置和状态变量。
 */
typedef struct
{
    uint8_t current_status;    /**< 按键接口当前状态：0 表示未按下，1 表示按下 */
    uint8_t last_status;       /**< 按键接口上一个状态：用于检测按键状态变化 */
    uint8_t process_flag;      /**< 按键接口进程标志：用于按键状态处理流程的控制 */
    uint8_t pres_flag;         /**< 当前按键状态标志：0 表示未按下，1 表示按下 */
    uint16_t contimer_count;   /**< 连续按下计数器：用于检测短按或双击的时间间隔 */
    uint16_t longtimer_count;  /**< 长按时间计数器：用于检测按键是否达到长按的条件 */
    
    uint16_t *contimer_value;  /**< 短按与双击的连续时间：设置短按或双击的时间间隔阈值 */
    uint16_t *longtimer_value; /**< 长按时间值：设置长按的时间阈值 */
    uint16_t button_number;    /**< 按键数量：系统中配置的按键个数 */

    /**
     * @brief 按键输入引脚状态读取函数指针
     * @return uint8_t 返回按键输入引脚的电平状态，0 表示未按下，1 表示按下
     */
    uint8_t (*statusin)(void);
} qyq_soft_button_control_block_t;

/**
 * @brief 初始化按键控制块
 * 
 * 该函数用于初始化按键控制块，根据提供的配置初始化多个按键。
 * 
 * @param block_list 按键控制块数组指针
 * @param number 按键数量
 * @return int8_t 返回状态码，0 表示成功，非0 表示失败
 */
QYQ_SOFT_BUTTON_EXT int8_t qyq_soft_button_init(qyq_soft_button_control_block_t *block_list, uint8_t number);

/**
 * @brief 获取指定按键的值
 * 
 * 该函数用于获取指定按键的当前状态值。
 * 
 * @param button_id 按键ID
 * @param value 按键值的指针，用于存储读取的按键状态
 * @return int8_t 返回状态码，0 表示成功，非0 表示失败
 */
QYQ_SOFT_BUTTON_EXT int8_t qyq_soft_button_get_buttonvalue(uint8_t button_id, uint16_t *value);

#endif
