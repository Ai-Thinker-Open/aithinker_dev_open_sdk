#include "app_include.h"

QueueHandle_t xled_queue;

void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    xled_queue = xQueueCreate(5, sizeof(uint8_t));

    bl616_rgbled_init();
    
    mqtt_app_init();

    bl616_uart_init();

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
