/** @brief pwm demo
 *
 *  @file        main.c
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/02/10          <td>V1.0.0          <td>hewm            <td>demo
 *  <tr><th>2023/02/10          <td>V1.0.1          <td>liq             <td>demo
 *  <table>
 *
 */

#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"
#include "aiio_pwm.h"
#include "aiio_gpio.h"
#include "aiio_autoconf.h"

#define TEST_PWM_IO0 CONFIG_TEST_PWM0
#define TEST_PWM_IO2 CONFIG_TEST_PWM2
#define TEST_PWM_PORT0 CONFIG_TEST_PWM0_PORT
#define TEST_PWM_PORT2 CONFIG_TEST_PWM2_PORT

void test_pwm_function(void *params)
{
    uint32_t pwm_duty = 0;
    aiio_pwm_init_t init = {
        .freq = 10000,
        .duty = 20,
        .gpio_port = TEST_PWM_PORT0,
        .gpio_pin = TEST_PWM_IO0,
    };
    aiio_pwm_init_t init_chann2 = {
        .freq = 10000,
        .duty = 80,
        .gpio_port = TEST_PWM_PORT2,
        .gpio_pin = TEST_PWM_IO2,
    };
    int pwm_num0 = aiio_pwm_init(&init);           // 初始化PWM,设置频率为10K,占空比为20%
    int pwm_num2 = aiio_pwm_init(&init_chann2);    // 初始化PWM

    int ret = aiio_pwm_start(pwm_num0); // 开始产生PWM
    ret = aiio_pwm_start(pwm_num2); // 开始产生PWM

    while (1)
    {
        pwm_duty++;
        if (pwm_duty > 100)
            pwm_duty = 0;

        aiio_pwm_set_duty(pwm_num0, pwm_duty); // 配置占空比
        aiio_pwm_set_duty(pwm_num2, pwm_duty);

        aiio_log_a("PWM Duty = %f\n", aiio_pwm_get_duty(pwm_num0));
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

void test_pwm_func(void)
{
    static aiio_os_thread_handle_t *aiio_pwm_thread = NULL;
    aiio_os_thread_create(&aiio_pwm_thread, "test_pwm", test_pwm_function, 4096, NULL, 15);
    aiio_log_a("create pwm task!");
}
void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    test_pwm_func();
    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
