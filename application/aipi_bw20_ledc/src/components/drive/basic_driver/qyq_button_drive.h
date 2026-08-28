#ifndef __QYQ_BUTTON_DRIVE_H_
#define __QYQ_BUTTON_DRIVE_H_
#include "chip_include.h"
#include "interface_include.h"
#ifdef __QYQ_BUTTON_DRIVE_C_
#define QYQ_BUTTON_DRIVE_EXT
#else
#define QYQ_BUTTON_DRIVE_EXT extern
#endif

/**
 * @brief 按键回调函数类型定义
 * @param button_id 按键ID
 * @param button_value 按键值，表示当前按键的状态
 */
typedef void (*qyq_button_drive_callback_t)(uint8_t button_id, uint16_t button_value);

/**
 * @brief 按键控制块
 * 该结构体用于存储按键的状态和计时器信息
 */
typedef struct
{
    uint8_t status;     /**< 按键状态，0为未按下，1为按下 */
    uint16_t contimer;  /**< 单击与双击的延迟间隔，单位ms */
    uint16_t longtimer; /**< 长按按键的时间阈值，单位ms */
} qyq_button_control_block_t;

/**
 * @brief 初始化按键，配置相关硬件资源
 *
 * @param block_list 按键控制块数组，用于存储按键状态和定时器信息
 * @param number 按键数量
 * @return int8_t 成功返回0，失败返回负值
 */
QYQ_BUTTON_DRIVE_EXT int8_t qyq_button_drive_init(qyq_button_control_block_t *block_list, uint8_t number);

/**
 * @brief 读取所有按键的状态
 *
 * @param button_value_list 存储所有按键值的数组，按键状态将写入此数组
 * @param button_number 按键数量
 * @return int8_t 成功返回0，失败返回负值
 */
QYQ_BUTTON_DRIVE_EXT int8_t qyq_button_drive_read_all(uint8_t *button_value_list, uint8_t button_number);

/**
 * @brief 读取指定按键的状态
 *
 * @param button_id 指定要读取状态的按键ID
 * @param button_value 按键状态值，0为未按下，1为按下
 * @return int8_t 成功返回0，失败返回负值
 */
QYQ_BUTTON_DRIVE_EXT int8_t qyq_button_drive_read(uint8_t button_id, uint16_t *button_value);

/**
 * @brief 注册按键回调函数
 *
 * @param callback 回调函数指针，按键按下或释放时调用
 * @return int8_t 成功返回0，失败返回负值
 * @note 注册的回调函数会在按键状态变化时触发
 */
QYQ_BUTTON_DRIVE_EXT int8_t qyq_button_drive_register_callback(qyq_button_drive_callback_t callback);

/**
 * @brief 取消按键回调函数注册
 *
 * @return int8_t 成功返回0，失败返回负值
 */
QYQ_BUTTON_DRIVE_EXT int8_t qyq_button_drive_unregister_callback(void);

#endif // __QYQ_BUTTON_DRIVE_H_
