#define __AIIO_RECEPTACLE_REPORT_C_
#include "aiio_receptacle_report.h"

static uint8_t receptacle_report_status = 0;
aiio_os_thread_handle_t *aiio_receptacle_report_thread = NULL;           /*!< handle of task thread*/
aiio_os_thread_handle_t *aiio_receptacle_power_thread = NULL;            /*!< handle of task thread*/
aiio_os_thread_handle_t *aiio_receptacle_voltage_sampling_thread = NULL; /*!< handle of task thread*/

extern uint8_t powerstatus;
extern uint8_t status;
extern uint8_t ledstatus;
extern double powerconsumption; // 总耗电量(只上报（ro）)
extern double electriccurrent;  // 电流(只上报（ro）)
extern double voltage;          // 电压(只上报（ro）)
extern double electricpower;    // 功率(只上报（ro）)

void aiio_receptacle_report_status_set(void)
{
    receptacle_report_status = 1;
}

void aiio_receptacle_report_status_clr(void)
{
    receptacle_report_status = 0;
}

void aiio_receptacle_report_task(void *msg)
{
    while (1)
    {
        if (receptacle_report_status)
        {
            aiio_report_all_attibute_status(NULL, NULL);
        }
        aiio_log_e("aiio_receptacle_report_task\r\n");
        aiio_os_tick_dealy(aiio_os_ms2tick(5000));
    }
}

void aiio_receptacle_power_task(void *msg)
{
    aiio_hal_gpio_init(AIIO_GPIO_A, AIIO_GPIO_NUM_5);
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_A, AIIO_GPIO_NUM_5, AIIO_GPIO_OUTPUT);
    aiio_hal_gpio_pin_pull_set(AIIO_GPIO_A, AIIO_GPIO_NUM_5, AIIO_GPIO_PULL_DOWN);

    while (1)
    {
        if (powerstatus)
        {
            aiio_hal_gpio_set(AIIO_GPIO_A, AIIO_GPIO_NUM_5, 1);
        }
        else
        {
            aiio_hal_gpio_set(AIIO_GPIO_A, AIIO_GPIO_NUM_5, 0);
        }
        aiio_os_tick_dealy(aiio_os_ms2tick(1));
    }
}

double v_bench_value = 0.1158;
double i_bench_value = 9.2;
double p_bench_value = 0;

uint16_t cf1_count = 0;
uint16_t cf_count = 0;

void aiio_hal_gpio_init_fun_cb_1(int32_t irq, void *param)
{
    // aiio_log_a("aiio_hal_gpio_init_fun_cb!");
    cf1_count++;
}

void aiio_hal_gpio_init_fun_cb_2(int32_t irq, void *param)
{
    // aiio_log_a("aiio_hal_gpio_init_fun_cb!");
    cf_count++;
}

void aiio_bl0937_v_start(void)
{
    uint16_t value = 0;
    aiio_hal_gpio_set(AIIO_GPIO_A, AIIO_GPIO_NUM_21, 1);
    aiio_os_tick_dealy(aiio_os_ms2tick(10));

    aiio_hal_gpio_int_enable(AIIO_GPIO_A, AIIO_GPIO_NUM_20, true);

    cf1_count = 0;
    aiio_os_tick_dealy(aiio_os_ms2tick(1000));

    aiio_hal_gpio_int_enable(AIIO_GPIO_A, AIIO_GPIO_NUM_20, false);

    value = (uint16_t)(cf1_count * v_bench_value);

    voltage = value;

    aiio_log_a("v value:%d\r\n", cf1_count);
}

void aiio_bl0937_i_start(void)
{
    uint16_t value = 0;
    aiio_hal_gpio_set(AIIO_GPIO_A, AIIO_GPIO_NUM_21, 0);
    aiio_os_tick_dealy(aiio_os_ms2tick(10));

    aiio_hal_gpio_int_enable(AIIO_GPIO_A, AIIO_GPIO_NUM_20, true);

    cf1_count = 0;
    aiio_os_tick_dealy(aiio_os_ms2tick(1000));

    aiio_hal_gpio_int_enable(AIIO_GPIO_A, AIIO_GPIO_NUM_20, false);

    value = (uint16_t)(cf1_count * i_bench_value);

    electriccurrent = value;
    electricpower = (uint16_t)(electriccurrent*voltage/1000);

    aiio_log_a("i value:%d\r\n", cf1_count);
}

void aiio_bl0937_power_start(void)
{
    // uint16_t value = 0;
    aiio_hal_gpio_int_enable(AIIO_GPIO_A, AIIO_GPIO_NUM_22, true);

    cf_count = 0;
    aiio_os_tick_dealy(aiio_os_ms2tick(1000));

    aiio_hal_gpio_int_enable(AIIO_GPIO_A, AIIO_GPIO_NUM_22, false);

    // value = (uint16_t)(cf_count * p_bench_value);

    // electricpower = value;

    aiio_log_a("power value:%d\r\n", cf_count);
}

// 电压采样
void aiio_receptacle_voltage_sampling_task(void *msg)
{
    aiio_hal_gpio_init(AIIO_GPIO_A, AIIO_GPIO_NUM_20);
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_A, AIIO_GPIO_NUM_20, AIIO_GPIO_INPUT);
    aiio_hal_gpio_pin_pull_set(AIIO_GPIO_A, AIIO_GPIO_NUM_20, AIIO_GPIO_PULL_UP);
    aiio_hal_gpio_init(AIIO_GPIO_A, AIIO_GPIO_NUM_22);
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_A, AIIO_GPIO_NUM_22, AIIO_GPIO_INPUT);
    aiio_hal_gpio_pin_pull_set(AIIO_GPIO_A, AIIO_GPIO_NUM_22, AIIO_GPIO_PULL_UP);
    aiio_hal_gpio_init(AIIO_GPIO_A, AIIO_GPIO_NUM_21);
    aiio_hal_gpio_pin_direction_set(AIIO_GPIO_A, AIIO_GPIO_NUM_21, AIIO_GPIO_OUTPUT);
    aiio_hal_gpio_pin_pull_set(AIIO_GPIO_A, AIIO_GPIO_NUM_21, AIIO_GPIO_PULL_UP);

    aiio_hal_gpio_int_register(AIIO_GPIO_A, AIIO_GPIO_NUM_20, aiio_hal_gpio_init_fun_cb_1);
    aiio_hal_gpio_int_register(AIIO_GPIO_A, AIIO_GPIO_NUM_22, aiio_hal_gpio_init_fun_cb_2);

    while (1)
    {
        aiio_bl0937_v_start();
        aiio_bl0937_i_start();
        aiio_bl0937_power_start();
        aiio_os_tick_dealy(aiio_os_ms2tick(1));
    }
}
