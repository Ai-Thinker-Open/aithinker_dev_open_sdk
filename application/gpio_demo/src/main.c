/** @brief gpio demo
 *
 *  @file        main.c
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/02/10          <td>V1.0.0          <td>hewm            <td>demo
 *  <table>
 *
 */

#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

#include "aiio_chip_spec.h"
#include "aiio_gpio.h"

#define GPIO_INOUT_TEST_PORT  CONFIG_TEST_PORT_INOUT
#define GPIO_INOUT_TEST_GPIO  CONFIG_TEST_INOUT_GPIO
#define GPIO_OUT_TEST_PORT  CONFIG_TEST_PORT_OUT
#define GPIO_OUT_TEST_GPIO  CONFIG_TEST_OUT_GPIO


void test_gpio_function(void *params)
{
    aiio_hal_gpio_init(GPIO_INOUT_TEST_PORT, GPIO_INOUT_TEST_GPIO);
    aiio_hal_gpio_init(GPIO_OUT_TEST_PORT, GPIO_OUT_TEST_GPIO);

    aiio_hal_gpio_pin_pull_set(GPIO_INOUT_TEST_PORT, GPIO_INOUT_TEST_GPIO, AIIO_GPIO_PULL_UP);
    aiio_hal_gpio_pin_pull_set(GPIO_OUT_TEST_PORT, GPIO_OUT_TEST_GPIO, AIIO_GPIO_PULL_UP);

    aiio_hal_gpio_pin_direction_set(GPIO_INOUT_TEST_PORT, GPIO_INOUT_TEST_GPIO, AIIO_GPIO_OUTPUT);
    aiio_hal_gpio_pin_direction_set(GPIO_OUT_TEST_PORT, GPIO_OUT_TEST_GPIO, AIIO_GPIO_OUTPUT);
    int j = 0, i=0;
    while (1)
    {
        if (i++ >= 30)
        {
            aiio_hal_gpio_set(GPIO_INOUT_TEST_PORT, GPIO_INOUT_TEST_GPIO, 0);
            aiio_hal_gpio_set(GPIO_OUT_TEST_PORT, GPIO_OUT_TEST_GPIO, 0);

            aiio_hal_gpio_pin_direction_set(GPIO_INOUT_TEST_PORT, GPIO_INOUT_TEST_GPIO, AIIO_GPIO_INPUT);
            aiio_hal_gpio_pin_pull_set(GPIO_INOUT_TEST_PORT, GPIO_INOUT_TEST_GPIO, AIIO_GPIO_PULL_DOWN);
            aiio_os_tick_dealy(aiio_os_ms2tick(500));

            while (0 == aiio_hal_gpio_get(GPIO_INOUT_TEST_PORT, GPIO_INOUT_TEST_GPIO))
            {
                aiio_log_i("read io val:0");
                aiio_os_tick_dealy(aiio_os_ms2tick(500));
            }
            aiio_log_i("read io val:1");
            aiio_hal_gpio_set(GPIO_OUT_TEST_PORT, GPIO_OUT_TEST_GPIO, 1);
            i = 0;
        }else{
            aiio_hal_gpio_set(GPIO_INOUT_TEST_PORT, GPIO_INOUT_TEST_GPIO, ((j % 2) == 1 ? 0 : 1));
            aiio_hal_gpio_set(GPIO_OUT_TEST_PORT, GPIO_OUT_TEST_GPIO, ((j++ % 2) == 1 ? 1 : 0));
        }
        aiio_log_i("io test %d", i);
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }

}
void test_gpio_func(void)
{
    static aiio_os_thread_handle_t *aiio_gpio_thread = NULL;

    aiio_os_thread_create(&aiio_gpio_thread, "test_gpio", test_gpio_function, 4096, NULL, 3);
    aiio_log_i("create gpio task!");
}

void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger! a ");
    aiio_log_e("Hello EasyLogger! e ");
    aiio_log_w("Hello EasyLogger! w ");
    aiio_log_i("Hello EasyLogger! i ");
    aiio_log_d("Hello EasyLogger! d ");
    aiio_log_v("Hello EasyLogger! v ");

    test_gpio_func();

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

