/** @brief ir demo
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

typedef struct
{
  uint32_t freq;
  uint32_t duty;
}ir_nec_t;

ir_nec_t nec_start = {
  .freq = 74,
  // .duty = 33,//占空比
  .duty = 66,//占空比
};

ir_nec_t nec_one = {
  .freq = 445,
  // .duty = 75,//占空比  
  .duty = 25,//占空比  
};

ir_nec_t nec_zero = {
  .freq = 892,
  .duty = 50,//占空比  
};

ir_nec_t nec_repet = {
  .freq = 89,
  // .duty = 20,//占空比  
  .duty = 80,//占空比  
};

void nec_send_dat(uint8_t dat)
{
  uint8_t tmp = dat;
  for(int i = 0;i < 8;i ++)
  {
    if((tmp&0x01) == 0x01)
    {
      // pwm_cnt_set(1,nec_one.freq,nec_one.duty);
      aiio_pwm_output(AIIO_PWM_CHA_0, 1,nec_one.freq,nec_one.duty, AIIO_PWM_PORTB, AIIO_GPIO_NUM_5);
    }
    else
    {
      // pwm_cnt_set(1,nec_zero.freq,nec_zero.duty);
      aiio_pwm_output(AIIO_PWM_CHA_0, 1,nec_zero.freq,nec_zero.duty, AIIO_PWM_PORTB, AIIO_GPIO_NUM_5);
    }
    tmp >>= 1;
  }
}

void aiio_liqi_test_function(void)
{
    // 测试PWM的输出
    // aiio_pwm_init_t init = {
    //     .freq = 1000,
    //     .duty = 20,
    //     .pwm_channel_num = AIIO_PWM_CHA_0,
    //     .gpio_port = AIIO_PWM_PORTB,
    //     .gpio_pin = AIIO_GPIO_NUM_5,
    // };

    // 测试PWM IR NEC功能输出
    aiio_os_tick_dealy(aiio_os_ms2tick(1000));
    aiio_pwm_output(AIIO_PWM_CHA_0, 1, 1000, 0, AIIO_PWM_PORTB, AIIO_GPIO_NUM_5);
    aiio_pwm_output(AIIO_PWM_CHA_0, 1,nec_start.freq,nec_start.duty, AIIO_PWM_PORTB, AIIO_GPIO_NUM_5);
    nec_send_dat(0x17);
    nec_send_dat(~0x17);
    nec_send_dat(0x25);
    nec_send_dat(~0x25);
}

void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    // test_pwm_func();
    aiio_liqi_test_function();
    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
