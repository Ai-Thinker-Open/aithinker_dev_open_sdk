/** @brief PWM intermediate device port adaptation
 *  
 *  @file        aiio_pwm.c
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note 
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2024/04/10          <td>V1.0.0          <td>hongjz          <td>PWM device port adaptation
 *  <table>
 *  
 */

#include "aiio_pwm.h"
#include "aiio_log.h"
#include "aiio_chip_spec.h"

#include "device.h"
#include "pwmout_api.h"
#include "pwmout_ex_api.h"
#include "PinNames.h"

//表示io引脚信息的结构体
typedef struct __aiio_rtl_io_info{
	//IO功能(都是0表示不能控制，例如EN，串口，vcc等引脚)
	uint8_t	asGpio:1;	//该引脚是否可以设置为GPIO
	uint8_t	asPwm:1;	//该引脚是否可以设置为PWM引脚
	//pin引脚
	uint8_t pinIndex;	//引脚编号(芯片手册上引脚的序号)
	PinName pinName;	//引脚名称(代码中控制引脚用的名字)
	gpio_t pinObj;	//引脚初始化句柄
	pwmout_t pwmObj;	//pwm初始化句柄
}aiio_rtl_io_info;
//IO引脚信息(顺序是根据IC芯片引脚编号排布的BW16使用的是RTL8720DN 48pin的芯片，注意：框架中编号是从)
static aiio_rtl_io_info ai_rtlIoInfoArray[]={
		{.asGpio=1,.asPwm=1,.pinIndex=20,.pinName=PA_12},
		{.asGpio=1,.asPwm=1,.pinIndex=21,.pinName=PA_13},
		{.asGpio=1,.asPwm=0,.pinIndex=22,.pinName=PA_14},
		{.asGpio=1,.asPwm=0,.pinIndex=23,.pinName=PA_15},
		{.asGpio=1,.asPwm=0,.pinIndex=26,.pinName=PA_27},
		{.asGpio=1,.asPwm=1,.pinIndex=27,.pinName=PA_30},
		{.asGpio=1,.asPwm=1,.pinIndex=29,.pinName=PA_26},
		{.asGpio=1,.asPwm=1,.pinIndex=30,.pinName=PA_25},
		{.asGpio=1,.asPwm=0,.pinIndex=34,.pinName=PB_3},
	};

//根据引脚序号获取IO信息
//参数
//    pinindex：RTL IO对应 ai_rtlIoInfoArray[]中的 pinindex
//返回值
//    成功返回IO信息的结构体指针
//    失败返回NULL
static aiio_rtl_io_info *aiio_get_rtl_io_info_by_pinindex(uint8_t pinindex){
	for(uint8_t u8_i=0;u8_i<(sizeof(ai_rtlIoInfoArray)/sizeof(aiio_rtl_io_info));u8_i++){
		if(pinindex==ai_rtlIoInfoArray[u8_i].pinIndex){
			return &ai_rtlIoInfoArray[u8_i];
		}
	}
	return NULL;	//没有找到对应的pin脚信息
}

typedef struct
{
    uint8_t inited : 1;
    uint8_t reserved : 7;
    pwmout_t pwm;
} aiio_pwm_dev_t;

static aiio_pwm_dev_t pwm_instance[2];

static int32_t pwm_param_check(aiio_pwm_chann_t pwm_channel_num)
{
    if (pwm_channel_num > 1)
    {
        aiio_log_e("pwm channle error!");
        return AIIO_ERROR;
    }

    if (!pwm_instance[pwm_channel_num].inited)
    {
        aiio_log_e("pwm is not inited!");
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int aiio_pwm_init(aiio_pwm_init_t *init)
{
    uint8_t index;
    aiio_rtl_io_info *pIoInfo=aiio_get_rtl_io_info_by_pinindex(init->gpio_pin);
    PinName pwm_pin=pIoInfo->pinName;

    switch (pwm_pin)
    {
    case PA_13:
        index = 0;
        break;
    case PA_30:
        index = 1;
        break;
    default:
        aiio_log_e("unknown pwm pin!");
        return AIIO_ERROR;
    }

    if (init->freq == 0)
    {
        aiio_log_e("pwm freq error!");
        return AIIO_ERROR;
    }

    if (init->duty < 0 || init->duty > 100)
    {
        aiio_log_e("pwm duty error!");
        return AIIO_ERROR;
    }

    pwmout_init(&pwm_instance[index].pwm, pwm_pin);

    float period = 1000000.0f / init->freq;
    pwmout_period_us(&pwm_instance[index].pwm, period);

    float pwm_duty = init->duty / 100;
    pwmout_write(&pwm_instance[index].pwm, pwm_duty);

    pwmout_stop(&pwm_instance[index].pwm);

    pwm_instance[index].inited = 1;

    return index;
}

int aiio_pwm_start(aiio_pwm_chann_t pwm_channel_num)
{
    if (pwm_param_check(pwm_channel_num) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    pwmout_t *pwm_dev = &pwm_instance[pwm_channel_num].pwm;
    pwmout_start(pwm_dev);

    return AIIO_OK;
}

int aiio_pwm_set_duty(aiio_pwm_chann_t pwm_channel_num, float duty)
{
    if (pwm_param_check(pwm_channel_num) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    pwmout_write(&pwm_instance[pwm_channel_num].pwm, duty / 100);

    return AIIO_OK;
}

float aiio_pwm_get_duty(aiio_pwm_chann_t pwm_channel_num)
{
    if (pwm_param_check(pwm_channel_num) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    float duty = pwmout_read(&pwm_instance[pwm_channel_num].pwm);
    return (duty * 100);
}

int aiio_pwm_set_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t period)
{
    if (pwm_param_check(pwm_channel_num) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    pwmout_period_us(&pwm_instance[pwm_channel_num].pwm, period);

    return AIIO_OK;
}

int32_t aiio_pwm_get_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t *period)
{
    if (pwm_param_check(pwm_channel_num) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    *period = pwm_instance[2].pwm.period;
    
    return AIIO_OK;
}

int32_t aiio_pwm_output_close(aiio_pwm_chann_t pwm_channel_num)
{
    if (pwm_param_check(pwm_channel_num) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    pwmout_stop(&pwm_instance[pwm_channel_num].pwm);

    return AIIO_OK;
}

int32_t aiio_pwm_output(aiio_pwm_chann_t pwm_channel_num,int cnt, uint32_t freq, float duty,uint32_t gpio_port, uint32_t gpio_pin)
{
    (void)pwm_channel_num;
    (void)cnt;
    (void)freq;
    (void)duty;
    (void)duty;
    (void)gpio_port;
    (void)gpio_pin;
    return AIIO_ERROR;
}


