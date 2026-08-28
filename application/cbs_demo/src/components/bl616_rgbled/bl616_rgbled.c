#define __BL616_RGBLED_C_
#include "bl616_rgbled.h"

#define BL616_GREEN_LED GPIO_PIN_30 // 绿灯
#define BL616_RED_LED GPIO_PIN_1    // 红灯
#define BL616_BLUE_LED GPIO_PIN_0   // 蓝灯

static TaskHandle_t bl618_rgbled_handle;
struct bflb_device_s *gpio;
extern QueueHandle_t xled_queue;

// 红灯显示
void bl616_rgbled_red_disp(void)
{
    bflb_gpio_reset(gpio, BL616_GREEN_LED);
    bflb_gpio_set(gpio, BL616_RED_LED);
    bflb_gpio_reset(gpio, BL616_BLUE_LED);
}

// 绿灯显示
void bl616_rgbled_green_disp(void)
{
    bflb_gpio_set(gpio, BL616_GREEN_LED);
    bflb_gpio_reset(gpio, BL616_RED_LED);
    bflb_gpio_reset(gpio, BL616_BLUE_LED);
}

// 蓝灯显示
void bl616_rgbled_blue_disp(void)
{
    bflb_gpio_reset(gpio, BL616_GREEN_LED);
    bflb_gpio_reset(gpio, BL616_RED_LED);
    bflb_gpio_set(gpio, BL616_BLUE_LED);
}

// led 关闭
void bl616_rgbled_off(void)
{
    bflb_gpio_set(gpio, BL616_GREEN_LED);
    bflb_gpio_set(gpio, BL616_RED_LED);
    bflb_gpio_set(gpio, BL616_BLUE_LED);
}

// 流水灯
void bl616_regled_water(void)
{
    bl616_rgbled_red_disp();
    vTaskDelay(pdMS_TO_TICKS(500));
    bl616_rgbled_green_disp();
    vTaskDelay(pdMS_TO_TICKS(500));
    bl616_rgbled_blue_disp();
    vTaskDelay(pdMS_TO_TICKS(500));
    bl616_rgbled_red_disp();
    vTaskDelay(pdMS_TO_TICKS(500));
    bl616_rgbled_green_disp();
    vTaskDelay(pdMS_TO_TICKS(500));
    bl616_rgbled_blue_disp();
    vTaskDelay(pdMS_TO_TICKS(500));
}

// rgbled 任务处理
void bl616_rgbled_task(void *msg)
{
    uint8_t led_value = 0;

    bl616_regled_water();
    bl616_rgbled_red_disp();

    while (1)
    {
        if (xQueueReceive(xled_queue, &led_value, portMAX_DELAY) == pdPASS)
        {
            // 成功接收到数据
            if (led_value == 1)
            {
                bl616_rgbled_green_disp();
            }
            else
            {
                bl616_rgbled_red_disp();
            }
        }
    }
}

void bl616_rgbled_init(void)
{
    gpio = bflb_device_get_by_name("gpio");
    printf("gpio output\r\n");
    bflb_gpio_init(gpio, BL616_GREEN_LED, GPIO_OUTPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_0);
    bflb_gpio_init(gpio, BL616_RED_LED, GPIO_OUTPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_0);
    bflb_gpio_init(gpio, BL616_BLUE_LED, GPIO_OUTPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_0);
    bflb_gpio_init(gpio, GPIO_PIN_3, GPIO_OUTPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_0);

    bl616_rgbled_off();

    xTaskCreate(bl616_rgbled_task, (char *)"bl616_rgbled_task", 512, NULL, 18, &bl618_rgbled_handle);
}
