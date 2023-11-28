/** @brief      button application interface.
 *
 *  @file       aiio_button.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       button application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>button Init
 *  </table>
 *
 */
#include "aiio_button.h"

static struct
{
    uint16_t aiio_button_number;
    uint8_t aiio_button_current_status;    // 按键接口当前状态
    uint8_t aiio_button_last_status;       // 按键接口上一个状态
    uint8_t aiio_button_process_flag;      // 按键接口进程
    uint8_t aiio_button_pres_flag;         // 当前按键状态标志
    uint16_t aiio_button_contimer_count;   // 连续按下计数器
    uint16_t aiio_button_longtimer_count;  // 长按时间计数器
    uint16_t *aiio_button_contimer_value;  // 短按与双击的连续时间
    uint16_t *aiio_button_longtimer_value; // 长按时间值
} aiio_button;
static aiio_button_statusin_t p_aiio_button_statusin;
static uint8_t reservestatus = 0;

int aiio_button_init(aiio_button_statusin_t aiio_button_statusin, uint16_t aiio_button_number, uint16_t *aiio_button_contimer_list, uint16_t *aiio_button_longtimer_list)
{
    if (aiio_button_statusin == NULL)
    {
        return -1;
    }

    if (aiio_button_contimer_list == NULL)
    {
        return -1;
    }

    if (aiio_button_longtimer_list == NULL)
    {
        return -1;
    }

    p_aiio_button_statusin = aiio_button_statusin;
    aiio_button.aiio_button_current_status = 0;
    aiio_button.aiio_button_last_status = 0;
    aiio_button.aiio_button_process_flag = 0;
    aiio_button.aiio_button_pres_flag = 0;
    aiio_button.aiio_button_contimer_count = 0;
    aiio_button.aiio_button_longtimer_count = 0;
    aiio_button.aiio_button_number = aiio_button_number;
    aiio_button.aiio_button_contimer_value = aiio_button_contimer_list;
    aiio_button.aiio_button_longtimer_value = aiio_button_longtimer_list;
    return 0;
}

uint16_t aiio_button_get_buttonvalue(void)
{

    uint16_t timercountvalue = 0;

    aiio_button.aiio_button_current_status = p_aiio_button_statusin();

    if (aiio_button.aiio_button_last_status != aiio_button.aiio_button_current_status)
    {
        aiio_button.aiio_button_last_status = aiio_button.aiio_button_current_status;
        aiio_button.aiio_button_longtimer_count = 0;
        return 0;
    }

    if (aiio_button.aiio_button_longtimer_count < 2)
    {
        aiio_button.aiio_button_longtimer_count++;
        return 0;
    }
    
    if (aiio_button.aiio_button_current_status)
    {
        reservestatus = aiio_button.aiio_button_last_status;
        if (aiio_button.aiio_button_longtimer_value != NULL)
        {
            timercountvalue = aiio_button.aiio_button_longtimer_value[reservestatus - 1];
        }
        else
        {
            timercountvalue = 10000;
        }
        if (aiio_button.aiio_button_longtimer_count < timercountvalue)
        {
            aiio_button.aiio_button_longtimer_count++;
        }	

        if (aiio_button.aiio_button_longtimer_count >= timercountvalue)
        {
            if (!aiio_button.aiio_button_process_flag)
            {
                aiio_button.aiio_button_pres_flag = 0;
                aiio_button.aiio_button_process_flag = 1;
                return (2 * aiio_button.aiio_button_number + reservestatus);
            }
        }
        else
        {
            if (aiio_button.aiio_button_pres_flag)
            {

                if (aiio_button.aiio_button_contimer_value != NULL)
                {
                    timercountvalue = aiio_button.aiio_button_contimer_value[reservestatus - 1];
                }
                else
                {
                    timercountvalue = 0;
                }
                if ((aiio_button.aiio_button_contimer_count != 0) &&
                    (aiio_button.aiio_button_contimer_count < timercountvalue))
                {
                    if (!aiio_button.aiio_button_process_flag)
                    {
                        aiio_button.aiio_button_process_flag = 1;
                        return (1 * aiio_button.aiio_button_number + reservestatus);
                    }
                }
            }
            else
            {
                aiio_button.aiio_button_pres_flag = 1;
                aiio_button.aiio_button_contimer_count = 0;
            }
        }
    }
    else
    {

        aiio_button.aiio_button_longtimer_count = 0;
        if (aiio_button.aiio_button_contimer_value != NULL)
        {
            if (reservestatus != 0)
            {
                timercountvalue = aiio_button.aiio_button_contimer_value[reservestatus - 1];
            }
        }
        else
        {
            timercountvalue = 0;
        }
        if (aiio_button.aiio_button_contimer_count < timercountvalue)
        {
            aiio_button.aiio_button_contimer_count++;
            return 0;
        }

        if ((aiio_button.aiio_button_pres_flag == 1) && (aiio_button.aiio_button_process_flag == 0))
        {
            aiio_button.aiio_button_pres_flag = 0;
            aiio_button.aiio_button_process_flag = 1;
            return reservestatus;
        }
        else
        {
            aiio_button.aiio_button_pres_flag = 0;
            aiio_button.aiio_button_process_flag = 0;
        }
    }

    return 0;
}
