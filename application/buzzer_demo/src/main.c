#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

static aiio_os_thread_handle_t *aiio_buzzer_thread = NULL;
static aiio_os_thread_handle_t *aiio_buzzer_thread1 = NULL;

#define AIIO_GPIO_BUZZEE_PORT CONFIG_GPIO_BUZZEE_PORT
#define AIIO_GPIO_BUZZEE_PIN CONFIG_GPIO_BUZZEE_PIN

void aiio_buzzer_function(void)
{
    // aiio_pwm_init_t init = {
    //     .freq = 1000,
    //     .duty = 50,
    //     .pwm_channel_num = AIIO_PWM_CHA_0,
    //     .gpio_port = AIIO_PWM_PORTB,
    //     .gpio_pin = AIIO_GPIO_NUM_5,
    // };
    // aiio_pwm_init(&init);
    // aiio_pwm_start(AIIO_PWM_CHA_0);
    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

void aiio_buzzer_function1(void)
{
    aiio_hal_gpio_init(AIIO_GPIO_BUZZEE_PORT, AIIO_GPIO_BUZZEE_PIN);

    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_BUZZEE_PORT, AIIO_GPIO_BUZZEE_PIN, AIIO_GPIO_OUTPUT);

    while (1)
    {
        aiio_hal_gpio_set(AIIO_GPIO_BUZZEE_PORT, AIIO_GPIO_BUZZEE_PIN, 0);
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
        aiio_hal_gpio_set(AIIO_GPIO_BUZZEE_PORT, AIIO_GPIO_BUZZEE_PIN, 1);
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

void aiio_main(void *params)
{
    uint8_t status = 2;
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    switch (status)
    {
    case 1:
        if (aiio_os_thread_create(&aiio_buzzer_thread, "aiio_buzzer_thread", aiio_buzzer_function, 4096, NULL, __AIIO_OS_PRIORITY_DEFAULT) == AIIO_ERROR)
        {
            return;
        }
        break;

    case 2:
        if (aiio_os_thread_create(&aiio_buzzer_thread1, "aiio_buzzer_thread1", aiio_buzzer_function1, 4096, NULL, __AIIO_OS_PRIORITY_DEFAULT) == AIIO_ERROR)
        {
            return;
        }
        break;

    default:
        break;
    }

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
