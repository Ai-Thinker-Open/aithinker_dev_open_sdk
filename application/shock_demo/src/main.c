#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

#define AIIO_GPIO_SHOCK_PORT CONFIG_GPIO_SHOCK_PORT
#define AIIO_GPIO_SHOCK_PIN CONFIG_GPIO_SHOCK_PIN

void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");
    aiio_hal_gpio_init(AIIO_GPIO_SHOCK_PORT, AIIO_GPIO_SHOCK_PIN);
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_SHOCK_PORT, AIIO_GPIO_SHOCK_PIN, AIIO_GPIO_INPUT);
    aiio_hal_gpio_pin_pull_set(AIIO_GPIO_SHOCK_PORT, AIIO_GPIO_SHOCK_PIN, AIIO_GPIO_PULL_DOWN);

    while (1)
    {
        if (aiio_hal_gpio_get(AIIO_GPIO_SHOCK_PORT, AIIO_GPIO_SHOCK_PIN) == 1)
        {
            aiio_log_w("shock on!");
        }
        aiio_os_tick_dealy(aiio_os_ms2tick(1));
    }
}

