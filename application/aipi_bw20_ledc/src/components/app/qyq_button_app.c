#define __QYQ_BUTTON_APP_C_
#include "qyq_button_app.h"

qyq_button_control_block_t button_block[3];
static qyq_button_app_type_t key_value;
extern rtos_queue_t button_xqueue;

void qyq_button_drive_callback(uint8_t button_id, uint16_t button_value)
{
    key_value.button_id = button_id;
    key_value.button_value = button_value;
    rtos_queue_send(button_xqueue, &key_value, 0xffffffffUL);
}

static void bw20_button_init(void)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        button_block[i].contimer = 0;
        button_block[i].longtimer = 300;
    }

    button_block[1].contimer = 20;

    // 初始化BUTTON驱动
    qyq_button_drive_init(button_block, sizeof(button_block) / sizeof(qyq_button_control_block_t));

    // 注册按键回调
    qyq_button_drive_register_callback(qyq_button_drive_callback);
}

void qyq_button_app_task(void *para)
{
    // 按键初始化
    bw20_button_init();

    while (1)
    {
        vTaskDelay(100);
    }
}
