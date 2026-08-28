#define __MAIN_C_
#include "app_main.h"

// MIC消息队列创建
rtos_queue_t mic_xqueue;
rtos_queue_t button_xqueue;

void app_main(void *para)
{
	// 创建mic发送消息事件
	rtos_queue_create(&mic_xqueue, 10, sizeof(int32_t));
	rtos_queue_create(&button_xqueue, 10, sizeof(qyq_button_app_type_t));

	xTaskCreate(qyq_button_app_task, (char *)"qyq_button_app_task", 4096, NULL, 20, NULL);
	xTaskCreate(qyq_mic_app_task, (char *)"qyq_mic_app_task", 4096, NULL, 19, NULL);
	xTaskCreate(qyq_ws2812_app_task, (char *)"qyq_ws2812_app_task", 4096, NULL, 18, NULL);
	xTaskCreate(qyq_ws2812_app_receive_task, (char *)"qyq_ws2812_app_receive_task", 4096, NULL, 17, NULL);

	vTaskDelete(NULL); // 删除当前任务
}
