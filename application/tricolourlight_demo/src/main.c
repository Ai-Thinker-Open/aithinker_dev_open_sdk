#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

static aiio_os_thread_handle_t *aiio_tricolourlight_thread = NULL;

#define AIIO_GPIO_RED_PORT CONFIG_GPIO_RED_PORT
#define AIIO_GPIO_YELLOW_PORT CONFIG_GPIO_YELLOW_PORT
#define AIIO_GPIO_BLUE_PORT CONFIG_GPIO_BLUE_PORT
#define AIIO_GPIO_RED_PIN CONFIG_GPIO_RED_PIN
#define AIIO_GPIO_YELLOW_PIN CONFIG_GPIO_YELLOW_PIN
#define AIIO_GPIO_BLUE_PIN CONFIG_GPIO_BLUE_PIN


void aiio_tricolourlight_function(void)
{
    uint8_t status = 0;

    aiio_hal_gpio_init(AIIO_GPIO_RED_PORT, AIIO_GPIO_RED_PIN);
    aiio_hal_gpio_init(AIIO_GPIO_YELLOW_PORT, AIIO_GPIO_YELLOW_PIN);
    aiio_hal_gpio_init(AIIO_GPIO_BLUE_PORT, AIIO_GPIO_BLUE_PIN);

    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_RED_PORT, AIIO_GPIO_RED_PIN, AIIO_GPIO_OUTPUT);
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_YELLOW_PORT, AIIO_GPIO_YELLOW_PIN, AIIO_GPIO_OUTPUT);
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_BLUE_PORT, AIIO_GPIO_BLUE_PIN, AIIO_GPIO_OUTPUT);

    while (1)
    {
        switch (status)
        {
        case 0:
            aiio_hal_gpio_set(AIIO_GPIO_RED_PORT, AIIO_GPIO_RED_PIN, 0);
            aiio_hal_gpio_set(AIIO_GPIO_YELLOW_PORT, AIIO_GPIO_YELLOW_PIN, 0);
            aiio_hal_gpio_set(AIIO_GPIO_BLUE_PORT, AIIO_GPIO_BLUE_PIN, 0);
            break;
        case 1:
            aiio_hal_gpio_set(AIIO_GPIO_RED_PORT, AIIO_GPIO_RED_PIN, 0);
            aiio_hal_gpio_set(AIIO_GPIO_YELLOW_PORT, AIIO_GPIO_YELLOW_PIN, 0);
            aiio_hal_gpio_set(AIIO_GPIO_BLUE_PORT, AIIO_GPIO_BLUE_PIN, 1);
            break;
        case 2:
            aiio_hal_gpio_set(AIIO_GPIO_RED_PORT, AIIO_GPIO_RED_PIN, 0);
            aiio_hal_gpio_set(AIIO_GPIO_YELLOW_PORT, AIIO_GPIO_YELLOW_PIN, 1);
            aiio_hal_gpio_set(AIIO_GPIO_BLUE_PORT, AIIO_GPIO_BLUE_PIN, 0);
            break;
        case 3:
            aiio_hal_gpio_set(AIIO_GPIO_RED_PORT, AIIO_GPIO_RED_PIN, 0);
            aiio_hal_gpio_set(AIIO_GPIO_YELLOW_PORT, AIIO_GPIO_YELLOW_PIN, 1);
            aiio_hal_gpio_set(AIIO_GPIO_BLUE_PORT, AIIO_GPIO_BLUE_PIN, 1);
            break;
        case 4:
            aiio_hal_gpio_set(AIIO_GPIO_RED_PORT, AIIO_GPIO_RED_PIN, 1);
            aiio_hal_gpio_set(AIIO_GPIO_YELLOW_PORT, AIIO_GPIO_YELLOW_PIN, 0);
            aiio_hal_gpio_set(AIIO_GPIO_BLUE_PORT, AIIO_GPIO_BLUE_PIN, 0);
            break;
        case 5:
            aiio_hal_gpio_set(AIIO_GPIO_RED_PORT, AIIO_GPIO_RED_PIN, 1);
            aiio_hal_gpio_set(AIIO_GPIO_YELLOW_PORT, AIIO_GPIO_YELLOW_PIN, 0);
            aiio_hal_gpio_set(AIIO_GPIO_BLUE_PORT, AIIO_GPIO_BLUE_PIN, 1);
            break;
        case 6:
            aiio_hal_gpio_set(AIIO_GPIO_RED_PORT, AIIO_GPIO_RED_PIN, 1);
            aiio_hal_gpio_set(AIIO_GPIO_YELLOW_PORT, AIIO_GPIO_YELLOW_PIN, 1);
            aiio_hal_gpio_set(AIIO_GPIO_BLUE_PORT, AIIO_GPIO_BLUE_PIN, 0);
            break;
        case 7:
            aiio_hal_gpio_set(AIIO_GPIO_RED_PORT, AIIO_GPIO_RED_PIN, 1);
            aiio_hal_gpio_set(AIIO_GPIO_YELLOW_PORT, AIIO_GPIO_YELLOW_PIN, 1);
            aiio_hal_gpio_set(AIIO_GPIO_BLUE_PORT, AIIO_GPIO_BLUE_PIN, 1);
            break;
        default:
            break;
        }
        status++;
        if (status > 7)
        {
            status = 0;
        }
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    if (aiio_os_thread_create(&aiio_tricolourlight_thread, "aiio_tricolourlight_thread", aiio_tricolourlight_function, 4096, NULL, __AIIO_OS_PRIORITY_DEFAULT) == AIIO_ERROR)
    {
        return;
    }

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

