// // #ifndef _PWM_PORT_H_
// // #define _PWM_PORT_H_

// // #include "hal/hal_interrupt.h"
// // #include "hal/hal_gpio.h"
// // #include "hal/hal_timer.h"
// // #include "hal/hal_adv_timer.h"
// // #include "hal/hal_clock.h"
// // #include "hal/hal_common.h"
// // #include "utils/debug/log.h"

// // typedef enum
// // {
// //     PWM_CHA_0 = 0,
// //     PWM_CHA_1 = 1,

// //     PWM_CHA_2 = 2,
// //     PWM_CHA_3 = 3,

// //     PWM_CHA_4 = 4,
// //     PWM_CHA_5 = 5,

// //     PWM_CHA_6 = 6,
// //     PWM_CHA_7 = 7,

// //     PWM_CHA_8 = 8,
// //     PWM_CHA_9 = 9,

// //     PWM_CHA_10 = 10,
// //     PWM_CHA_11 = 11,
// // } pwm_channel_t;

// // typedef enum
// // {
// //     GPIO_A = 0,
// //     GPIO_B = 1,
// // } gpio_port_t;

// // void pwm_init(uint32_t freq, float duty, pwm_channel_t pwm_channel_num, gpio_port_t gpio_port, gpio_pin_t gpio_pin);
// // void pwm_start(pwm_channel_t pwm_channel_num);
// // void pwm_set_duty(float duty, pwm_channel_t pwm_channel_num);
// // float pwm_get_duty(pwm_channel_t pwm_channel_num);
// // void pwm_set_freq(pwm_channel_t pwm_channel_num, uint32_t period);
// // void pwm_disable(pwm_channel_t pwm_channel_num);
// // uint32_t pwm_get_freq(pwm_channel_t pwm_channel_num);
// // void pwm_cnt_set(int cnt);

// // //**********************************************
// // typedef enum
// // {
// //     TIMER_CH_0,
// //     TIMER_CH_1,
// //     TIMER_CH_2,
// //     TIMER_CH_3, // ͨ��3��ʹ��wifi������»ᱻwifiռ�ã�������ʹ��
// // } timer_ch_t;

// // #define PWM_CAP_DATA_LEN 511
// // #define PWM_CAP_CNT_MAX ((PWM_CAP_DATA_LEN + 1) * 2)
// // #define TIME_T 1000000

// // typedef struct
// // {
// //     uint16_t freq;
// //     float duty;
// // } pwm_cap_t;

// // void app_pwm(void);
// // void pwm_cap_data_handle(void);

// // void timer_init(timer_ch_t timer_ch, uint32_t us, void (*timer_it_callback)(void));
// // uint32_t timer_get_timer_cnt_time(timer_ch_t timer_ch);

// // #endif

// #ifndef _PWM_PORT_H_
// #define _PWM_PORT_H_

// #include "hal/hal_interrupt.h"
// #include "hal/hal_gpio.h"
// #include "hal/hal_timer.h"
// #include "hal/hal_adv_timer.h"
// #include "hal/hal_clock.h"
// #include "hal/hal_common.h"
// #include "utils/debug/log.h"
// #include "aiio_adapter_include.h"

// typedef enum
// {
//     PWM_CHA_0 = 0,
//     PWM_CHA_1 = 1,
    
//     PWM_CHA_2 = 2,
//     PWM_CHA_3 = 3,
    
//     PWM_CHA_4 = 4,
//     PWM_CHA_5 = 5,

//     PWM_CHA_6 = 6,
//     PWM_CHA_7 = 7,
    
//     PWM_CHA_8 = 8,
//     PWM_CHA_9 = 9,
    
//     PWM_CHA_10 = 10,
//     PWM_CHA_11 = 11,
// }pwm_channel_t;

// typedef enum
// {
//     GPIO_A = 0,
//     GPIO_B = 1,
// }gpio_port_t;

// void     pwm_init(uint32_t freq, float duty,pwm_channel_t pwm_channel_num,gpio_port_t gpio_port,gpio_pin_t gpio_pin);
// void     pwm_cnt_set(int cnt, uint32_t freq, float duty);
// void     pwm_start(pwm_channel_t pwm_channel_num);
// void     pwm_set_duty(float duty, pwm_channel_t pwm_channel_num);
// float    pwm_get_duty(pwm_channel_t pwm_channel_num);
// void     pwm_set_freq(pwm_channel_t pwm_channel_num,uint32_t period);
// void     pwm_disable(pwm_channel_t pwm_channel_num);
// uint32_t 		 pwm_get_freq(pwm_channel_t pwm_channel_num);
// uint8_t get_start_flag_value(void);

// //**********************************************
// typedef enum
// {
//     TIMER_CH_0,
//     TIMER_CH_1,
//     TIMER_CH_2,
//     TIMER_CH_3,     //通道3在使用wifi的情况下会被wifi占用，请慎重使用
// }timer_ch_t;

// #define PWM_CAP_DATA_LEN 		256
// #define PWM_CAP_CNT_MAX 		((PWM_CAP_DATA_LEN+1)*2)
// #define TIME_T 							1000000 

// typedef struct
// {
// 	uint16_t freq;
// 	float duty;
// }pwm_cap_t;

// void app_pwm(void);
// void pwm_cap_data_handle(void);

// void        timer_init(timer_ch_t timer_ch,uint32_t us,void (*timer_it_callback)(void));
// uint32_t    timer_get_timer_cnt_time(timer_ch_t timer_ch);

// #endif