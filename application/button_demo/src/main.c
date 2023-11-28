#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

static uint16_t aiio_button_contimer_list[1] = {20};
static uint16_t aiio_button_longtimer_list[1] = {300};
static aiio_os_thread_handle_t *aiio_button_thread = NULL;

#define AIIO_GPIO_BUTTON_PORT CONFIG_GPIO_BUTTON_PORT
#define AIIO_GPIO_BUTTON_PIN CONFIG_GPIO_BUTTON_PIN

uint8_t aiio_button_statusin(void)
{
    if (aiio_hal_gpio_get(AIIO_GPIO_BUTTON_PORT, AIIO_GPIO_BUTTON_PIN) == 0)
    {
        return 1;
    }
    return 0;
}

void aiio_button_function(void)
{
    aiio_button_init(aiio_button_statusin, 1, aiio_button_contimer_list, aiio_button_longtimer_list);

    while (1)
    {
        switch (aiio_button_get_buttonvalue())
        {
        case 1:
            aiio_log_a("Click button!");
            break;
        case 2:
            aiio_log_a("Double click button!");
            break;
        case 3:
            aiio_log_a("Hold down a button!");
            break;
        default:
            break;
        }
        aiio_os_tick_dealy(aiio_os_ms2tick(10));
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

    aiio_hal_gpio_init(AIIO_GPIO_BUTTON_PORT, AIIO_GPIO_BUTTON_PIN);
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_BUTTON_PORT, AIIO_GPIO_BUTTON_PIN, AIIO_GPIO_INPUT);
    aiio_hal_gpio_pin_pull_set(AIIO_GPIO_BUTTON_PORT, AIIO_GPIO_BUTTON_PIN, AIIO_GPIO_PULL_UP);

    if (aiio_os_thread_create(&aiio_button_thread, "aiio_button_thread", aiio_button_function, 4096, NULL, __AIIO_OS_PRIORITY_DEFAULT) == AIIO_ERROR)
    {
        return;
    }

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
