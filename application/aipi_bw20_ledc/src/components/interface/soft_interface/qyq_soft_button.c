#define __QYQ_SOFT_BUTTON_C_
#include "qyq_soft_button.h"

static qyq_soft_button_control_block_t *button_master_block_list = null;
static uint8_t button_master_size = 0;

/**
 * @brief 初始化按键控制块
 *
 * 该函数用于初始化按键控制块，根据提供的配置初始化多个按键。
 *
 * @param block_list 按键控制块数组指针
 * @param number 按键数量
 * @return int8_t 返回状态码，0 表示成功，非0 表示失败
 */
int8_t qyq_soft_button_init(qyq_soft_button_control_block_t *block_list, uint8_t number)
{
    if (block_list == null)
    {
        return -1;
    }

    for (uint8_t i = 0; i < number; i++)
    {
        block_list[i].current_status = 0;
        block_list[i].last_status = 0;
        block_list[i].process_flag = 0;
        block_list[i].pres_flag = 0;
        block_list[i].contimer_count = 0;
        block_list[i].longtimer_count = 0;
    }

    button_master_block_list = block_list;
    button_master_size = number;

    return 0;
}

/**
 * @brief 获取指定按键的值
 *
 * 该函数用于获取指定按键的当前状态值。
 *
 * @param button_id 按键ID
 * @param value 按键值的指针，用于存储读取的按键状态
 * @return int8_t 返回状态码，0 表示成功，非0 表示失败
 */
int8_t qyq_soft_button_get_buttonvalue(uint8_t button_id, uint16_t *value)
{
    if ((button_id >= button_master_size) || (value == null) || (button_master_block_list == null))
    {
        return -1;
    }

    // 判断状态是否有变化
    static uint8_t reservestatus = 0;
    uint16_t timercountvalue = 0;

    *value = 0;

    button_master_block_list[button_id].current_status = button_master_block_list[button_id].statusin();
    // printf("status value is %d\r\n", button_master_block_list[button_id].current_status);

    if (button_master_block_list[button_id].last_status != button_master_block_list[button_id].current_status)
    {
        button_master_block_list[button_id].last_status = button_master_block_list[button_id].current_status;
        button_master_block_list[button_id].longtimer_count = 0;
        return 0;
    }
    // 去抖动
    if (button_master_block_list[button_id].longtimer_count < 2)
    {
        button_master_block_list[button_id].longtimer_count++;
        return 0;
    }
    if (button_master_block_list[button_id].current_status) // 判断当前状态
    {
        reservestatus = button_master_block_list[button_id].last_status;
        if (button_master_block_list[button_id].longtimer_value != null)
        {
            timercountvalue = button_master_block_list[button_id].longtimer_value[reservestatus - 1];
        }
        else
        {
            timercountvalue = 10000;
        }
        if (button_master_block_list[button_id].longtimer_count < timercountvalue)
        {
            button_master_block_list[button_id].longtimer_count++;
        }

        if (button_master_block_list[button_id].longtimer_count >= timercountvalue)
        {
            // 返回长按按键
            if (!button_master_block_list[button_id].process_flag)
            {
                button_master_block_list[button_id].pres_flag = 0;
                button_master_block_list[button_id].process_flag = 1;
                *value = (2 * button_master_block_list[button_id].button_number + reservestatus);
                return 0;
            }
        }
        else
        {
            if (button_master_block_list[button_id].pres_flag)
            {
                // 设置双击按键标志
                if (button_master_block_list[button_id].contimer_value != null)
                {
                    timercountvalue = button_master_block_list[button_id].contimer_value[reservestatus - 1];
                }
                else
                {
                    timercountvalue = 0;
                }
                if ((button_master_block_list[button_id].contimer_count != 0) &&
                    (button_master_block_list[button_id].contimer_count < timercountvalue))
                {
                    if (!button_master_block_list[button_id].process_flag)
                    {
                        button_master_block_list[button_id].process_flag = 1;
                        *value = (1 * button_master_block_list[button_id].button_number + reservestatus);
                        return 0;
                    }
                }
            }
            else
            {
                button_master_block_list[button_id].pres_flag = 1;
                button_master_block_list[button_id].contimer_count = 0;
            }
        }
    }
    else
    {
        // 等待双击按键是否执行
        button_master_block_list[button_id].longtimer_count = 0;
        if (button_master_block_list[button_id].contimer_value != null)
        {
            if (reservestatus != 0)
            {
                timercountvalue = button_master_block_list[button_id].contimer_value[reservestatus - 1];
            }
        }
        else
        {
            timercountvalue = 0;
        }
        if (button_master_block_list[button_id].contimer_count < timercountvalue)
        {
            button_master_block_list[button_id].contimer_count++;
            return 0;
        }
        // 返回单击按键并清除设置状态
        if ((button_master_block_list[button_id].pres_flag == 1) && (button_master_block_list[button_id].process_flag == 0))
        {
            button_master_block_list[button_id].pres_flag = 0;
            button_master_block_list[button_id].process_flag = 1;
            *value = reservestatus;
            return 0;
        }
        else
        {
            button_master_block_list[button_id].pres_flag = 0;
            button_master_block_list[button_id].process_flag = 0;
        }
    }

    return 0;
}
