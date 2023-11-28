#ifndef _LN_AT_DRIVE_PORT_H_
#define _LN_AT_DRIVE_PORT_H_

#include "ln_at.h"

#include "hal/hal_adc.h"
#include "hal/hal_efuse.h"

#include "hal/hal_common.h"
#include "hal/hal_adv_timer.h"
#include "hal/hal_gpio.h"
#include "hal/hal_clock.h"


//general
//********************************************************************************************
typedef void (*manager_init_func)(void);
//********************************************************************************************


//gpio
//********************************************************************************************
#define IS_AT_GPIO(IO)     ((IO) >= AT_GPIO_0 && (IO) <= AT_GPIO_17)

typedef enum
{
	AT_GPIO_BASE_A = 0,
	AT_GPIO_BASE_B,
}at_gpio_base_t;

typedef enum
{
	//GPIO_BASE_A
	AT_GPIO_0 = 0,	
	AT_GPIO_1 = 1,
	AT_GPIO_2 = 4,
	AT_GPIO_3 = 5,
	AT_GPIO_4 = 6,
	AT_GPIO_5 = 7,
	AT_GPIO_6 = 8,
	AT_GPIO_7 = 9,
	AT_GPIO_8 = 10,
	AT_GPIO_9 = 11,
	AT_GPIO_10 = 12,
	//GPIO_BASE_B
	AT_GPIO_11 = 13,
	AT_GPIO_12 = 14,
	AT_GPIO_13 = 15,
	AT_GPIO_14 = 16,
	AT_GPIO_15 = 17,
	AT_GPIO_16 = 18,
	AT_GPIO_17 = 19,
}at_gpio_t;

typedef struct 
{
	at_gpio_base_t gpio_base;
	gpio_pin_t gpio_pin;
}ln_at_gpio_t;

typedef ln_at_gpio_t* (*get_gpio_info_func)(at_gpio_t gpio);

typedef struct
{
	uint16_t gpio_base_a_mask;
	uint16_t gpio_base_b_mask;
	
	manager_init_func _cfg_init;
	get_gpio_info_func _get_gpio;
}ln_at_drv_gpio_item_t;

ln_at_drv_gpio_item_t* ln_at_drv_gpio_handle(void);
extern ln_at_drv_gpio_item_t* ln_at_drv_gpio_handle(void);
//********************************************************************************************


//pwm
//********************************************************************************************
#define PWM_CHANNEL_NUM		4
#define PWM_FREQ_MIN			100
#define PWM_FREQ_MAX			8000000
#define PWM_DUTY_RES_MIN	0
#define PWM_DUTY_RES_MAX	16

typedef enum
{
	PWM_CHA_0 = 0,
	PWM_CHA_1 = 1,
	
	PWM_CHA_2 = 2,
	PWM_CHA_3 = 3,
}ln_at_pwm_channel_t;

typedef struct
{
	uint8_t duty_res;
	uint32_t freq;
	ln_at_gpio_t gpio;
}ln_at_pwm_channel_info_t;

typedef int (*init_drv_pwm_func)(uint32_t freq, uint8_t duty_res, uint8_t channel_port , ln_at_pwm_channel_t pwm_channel);
typedef int (*set_drv_pwm_duty_func)(uint16_t duty, uint8_t channel);

typedef struct
{
	uint8_t channel_init_mask;
	ln_at_pwm_channel_info_t idx[PWM_CHANNEL_NUM];
	
	manager_init_func		_cfg_init;
	init_drv_pwm_func		_drv_init;
	set_drv_pwm_duty_func		_set_duty;
}ln_at_drv_pwm_item_t;

ln_at_drv_pwm_item_t* ln_at_drv_pwm_handle(void);
extern ln_at_drv_pwm_item_t* ln_at_drv_pwm_handle(void);
//********************************************************************************************

//I2C
//********************************************************************************************
#define I2C_MAX_CHANNEL 1

typedef struct
{
	bool is_init;
	
	uint32_t clock;
	ln_at_gpio_t scl_io;
	ln_at_gpio_t sda_io;
}ln_at_i2c_item_t;

typedef struct
{
	bool bSendFlag;
	
	uint16_t send_len;
	uint16_t recv_len;
	uint16_t send_offset;
	uint16_t recv_offset;
	ln_at_i2c_item_t i2c_idx[I2C_MAX_CHANNEL];
	
}ln_at_drv_i2c_manager_t;

#endif
