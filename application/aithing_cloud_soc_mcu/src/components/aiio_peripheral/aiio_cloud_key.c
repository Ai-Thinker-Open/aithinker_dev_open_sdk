/**
 * @brief   Define key applicaion interface
 * 
 * @file    aiio_cloud_key.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note        This file is mainly decribing key application
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-16          <td>1.0.0            <td>zhuolm             <td> The interface of key application
 */
#include "aiio_cloud_key.h"
#include "aiio_adapter_include.h"
#include "aiio_common.h"
#include "aiio_cloud_led.h"
#include "config.h"


#define AIIO_GPIO_BUTTON_PORT CONFIG_GPIO_BUTTON_PORT
#define AIIO_GPIO_BUTTON_PIN CONFIG_GPIO_BUTTON_PIN


static aiio_os_thread_handle_t *aiio_button_thread = NULL;
static uint16_t aiio_button_contimer_list[1] = {20};
static uint16_t aiio_button_longtimer_list[1] = {300};
static bool task_delete = false;


static void aiio_button_function(void *params);


void aiio_cloud_key_init(void)
{
    aiio_hal_gpio_init(AIIO_GPIO_BUTTON_PORT, AIIO_GPIO_BUTTON_PIN);
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_BUTTON_PORT, AIIO_GPIO_BUTTON_PIN, AIIO_GPIO_INPUT);
    aiio_hal_gpio_pin_pull_set(AIIO_GPIO_BUTTON_PORT, AIIO_GPIO_BUTTON_PIN, AIIO_GPIO_PULL_DOWN);

    if (aiio_os_thread_create(&aiio_button_thread, "aiio_button_thread", aiio_button_function, 4096, NULL, 10) == AIIO_ERROR)
    {
        aiio_log_e("create key task fail \r\n");
        return ;
    }
}


void aiio_cloud_key_deinit(void)
{
    task_delete = true;
}


static uint8_t aiio_button_statusin(void)
{
    if (aiio_hal_gpio_get(AIIO_GPIO_BUTTON_PORT, AIIO_GPIO_BUTTON_PIN) == 1)
    {
        return 1;
    }
    return 0;
}



static void aiio_button_function(void *params)
{
    aiio_rev_queue_t  rev_queue = {0};
    aiio_button_init(aiio_button_statusin, 1, aiio_button_contimer_list, aiio_button_longtimer_list);     

    task_delete = false;
    while (!task_delete)
    {
        switch (aiio_button_get_buttonvalue())                           
        {
            case 1:
            {
                aiio_log_w("click button!");
                aiio_all_led_triggle();
                
                aiio_report_bool_attibute_status(NULL, NULL, CMD_POWERSTATE, aiio_get_blue_led_status());
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
                rev_queue.common_event = REV_CONFIG_STOP_EVENT;
                if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                {
                    aiio_log_e("queue send failed\r\n");
                }
                memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
#else
                control_led_count++;
                if(control_led_count >= 10)
                {
                    aiio_entry_start_pair();
                    control_led_count = 0;
                }
            aiio_log_i("################# control_led_count = %d ###################\r\n", control_led_count);
#endif
            }
            break;

            case 2:
            {
                aiio_log_w("Double click button!");
            }
            break;

            case 3:
            {
                aiio_log_w("Hold down a button!");
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
                rev_queue.common_event = REV_CONFIG_START_EVENT;
                if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                {
                    aiio_log_e("queue send failed\r\n");
                }
                memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
#endif
            }
            break;
            
            default:
                break;
        }

        aiio_os_tick_dealy(aiio_os_ms2tick(10));
    }

    aiio_os_thread_delete(aiio_button_thread);
}


