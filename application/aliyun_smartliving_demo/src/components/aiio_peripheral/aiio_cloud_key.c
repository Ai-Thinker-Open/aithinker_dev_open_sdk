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
    int cnt = 0;
    uint8_t lev = 0;
    uint8_t flag = 0;
    while (1)
    {
        lev = aiio_hal_gpio_get(AIIO_GPIO_BUTTON_PORT, AIIO_GPIO_BUTTON_PIN);
        if (lev==1)
        {
            cnt++;
            if (cnt>=100)
            {
                if(flag == 0){
                    aiio_log_d("Hold down a button!");
                    aiio_flash_clear_config_data();
                    aiio_turn_on_led_status(GREEN_RED_LED_TRIGGLE_STATUS);
                    flag=1;
                }
            }
        }
        else
        {
            if(cnt>1 && cnt<20){
                aiio_log_d("click button!");
                aiio_all_led_triggle();
            }
            if(flag == 1){
                aiio_restart();
            }
            cnt = 0;
            flag = 0;
        }
        aiio_os_tick_dealy(aiio_os_ms2tick(30));
    }

}


