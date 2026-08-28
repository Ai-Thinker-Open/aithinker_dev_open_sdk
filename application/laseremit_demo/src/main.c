#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

#define AIIO_GPIO_LASEREMIT_PORT CONFIG_GPIO_LASEREMIT_PORT
#define AIIO_GPIO_LASEREMIT_PIN CONFIG_GPIO_LASEREMIT_PIN

void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    aiio_hal_gpio_init(AIIO_GPIO_LASEREMIT_PORT, AIIO_GPIO_LASEREMIT_PIN);

    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_LASEREMIT_PORT, AIIO_GPIO_LASEREMIT_PIN, AIIO_GPIO_OUTPUT);

    aiio_hal_gpio_set(AIIO_GPIO_LASEREMIT_PORT, AIIO_GPIO_LASEREMIT_PIN, 0);
    while (1)
    {
        aiio_hal_gpio_set(AIIO_GPIO_LASEREMIT_PORT, AIIO_GPIO_LASEREMIT_PIN, 0);
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
        aiio_hal_gpio_set(AIIO_GPIO_LASEREMIT_PORT, AIIO_GPIO_LASEREMIT_PIN, 1);
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

