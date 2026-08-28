/** @brief PWM intermediate device port adaptation
 *  
 *  @file        aiio_pwm.c
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note 
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2024/07/26          <td>V1.0.0          <td>specter          <td>PWM device port adaptation
 *  <table>
 *  
 */
#include "aiio_chip_spec.h"
#include "aiio_error.h"
#include "aiio_log.h"
#include "aiio_pwm.h"

//SDK
#include "device.h"
#include "pwmout_api.h"
#include "os_wrapper.h"
#include <stdio.h>

#define RTL8711_PIN_CALCULATE(a, b) ((a)<<5|(b))
//表示PWM信息的结构体
typedef struct __aiio_rtl_pwm_info{
	//IO功能(都是0表示不能控制，例如EN，串口，vcc等引脚)
	uint8_t	inited:1;	//该引脚是否已经初始化成了PWM引脚(0：没有初始化；1：已经初始化了)
	PinName pinName;	//引脚名称(SDK中的名字)
	pwmout_t pwmObj;	//pwm初始化句柄
}rtl8711_pwm_info;

//IO引脚信息
static rtl8711_pwm_info ai_rtlPwmInfoArray[]={
		{.inited=0,.pwmObj.pwm_idx=0},
		{.inited=0,.pwmObj.pwm_idx=1},
		{.inited=0,.pwmObj.pwm_idx=2},
		{.inited=0,.pwmObj.pwm_idx=3},
		{.inited=0,.pwmObj.pwm_idx=4},
		{.inited=0,.pwmObj.pwm_idx=5},
		{.inited=0,.pwmObj.pwm_idx=6},
		{.inited=0,.pwmObj.pwm_idx=7},
	};

static int rtl8711GetPwmInfo(rtl8711_pwm_info **ppinfo,PinName pinName);

int aiio_pwm_init(aiio_pwm_init_t *init)
{
    uint8_t index;
    aiio_log_e("gpio_port:%d gpio_pin:%d freq:%ld duty:%f",init->gpio_port,init->gpio_pin,init->freq,init->duty);
    PinName rtl_pin=RTL8711_PIN_CALCULATE(init->gpio_port,init->gpio_pin);
    rtl8711_pwm_info *pinfo=NULL;
    int i_ret=rtl8711GetPwmInfo(&pinfo,rtl_pin);
    if(0==i_ret){
        aiio_log_e("no free PWM");
        return AIIO_PWM_NO_FREE_PWM;
    }
    aiio_log_d("pwm_idx:%d",pinfo->pwmObj.pwm_idx);
    if(1==pinfo->inited){
        aiio_log_w("port:%d pin:%d inited",init->gpio_port,init->gpio_pin);
        return pinfo->pwmObj.pwm_idx;
    }
    sys_jtag_off();	//JTAG enable pin is disabled
    pwmout_init(&pinfo->pwmObj,rtl_pin);
    int us_count=1000000/(init->freq);
    if(0==us_count){
        us_count=1;
    }
    aiio_log_d("us_count:%ld",us_count);
    pwmout_period_us(&pinfo->pwmObj,us_count);

    float pwm_duty = init->duty / 100;
    aiio_log_d("pwm_duty:%f",pwm_duty);
    pwmout_write(&pinfo->pwmObj, pwm_duty);

    pwmout_stop(&pinfo->pwmObj);

    pinfo->inited = 1;

    return pinfo->pwmObj.pwm_idx;
}

int aiio_pwm_start(aiio_pwm_chann_t pwm_channel_num)
{
    if(1!=ai_rtlPwmInfoArray[pwm_channel_num].inited){
        aiio_log_e("PWM %d not init",pwm_channel_num);
        return AIIO_NOT_INIT;
    }

    pwmout_start(&ai_rtlPwmInfoArray[pwm_channel_num].pwmObj);

    return AIIO_OK;
}

int aiio_pwm_set_duty(aiio_pwm_chann_t pwm_channel_num, float duty)
{
    if(1!=ai_rtlPwmInfoArray[pwm_channel_num].inited){
        aiio_log_e("PWM %d not init",pwm_channel_num);
        return AIIO_NOT_INIT;
    }

    pwmout_write(&ai_rtlPwmInfoArray[pwm_channel_num].pwmObj, duty / 100.0);

    return AIIO_OK;
}

float aiio_pwm_get_duty(aiio_pwm_chann_t pwm_channel_num)
{
    float duty = pwmout_read(&ai_rtlPwmInfoArray[pwm_channel_num].pwmObj);
    return (duty * 100);
}

int aiio_pwm_set_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t period)
{
    if(1!=ai_rtlPwmInfoArray[pwm_channel_num].inited){
        aiio_log_e("PWM %d not init",pwm_channel_num);
        return AIIO_NOT_INIT;
    }
    int us_count=1000000/period;
    if(0==us_count){
        us_count=1;
    }
    pwmout_period_us(&ai_rtlPwmInfoArray[pwm_channel_num].pwmObj, us_count);

    return AIIO_OK;
}

int32_t aiio_pwm_get_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t *period)
{

    *period = ai_rtlPwmInfoArray[pwm_channel_num].pwmObj.period;
    
    return AIIO_OK;
}

int32_t aiio_pwm_output_close(aiio_pwm_chann_t pwm_channel_num)
{
    if(0==ai_rtlPwmInfoArray[pwm_channel_num].inited){
        aiio_log_e("PWM %d closed",pwm_channel_num);
        return AIIO_NOT_INIT;
    }

    pwmout_stop(&ai_rtlPwmInfoArray[pwm_channel_num].pwmObj);
    ai_rtlPwmInfoArray[pwm_channel_num].inited=0;

    return AIIO_OK;
}
//----------------------------------------- hal底层调用 ---------------------------------------------
int aiio_rtl8711d_pwm_close(uint32_t port, uint16_t pin){
    PinName rtl_pin=RTL8711_PIN_CALCULATE(port,pin);
    int count=sizeof(ai_rtlPwmInfoArray)/sizeof(ai_rtlPwmInfoArray[0]);

    for(int i=0;i<count;i++){
        if(ai_rtlPwmInfoArray[i].pinName==rtl_pin){
            if(ai_rtlPwmInfoArray[i].inited){
                aiio_log_d("port:%d pin:%d set pwm,and inited,close pwm %d",port,pin,aiio_pwm_output_close(ai_rtlPwmInfoArray[i].pwmObj.pwm_idx));
                return 1;
            }else{
                aiio_log_d("port:%d pin:%d set pwm,but not init",port,pin);
                return 0;
            }
        }
    }
    aiio_log_d("port:%d pin:%d not set pwm",port,pin);
    return 0;
}
//----------------------------------------- 内部调用 ---------------------------------
//获取pwm信息结构体
//参数
//    pinfo(输出)：获取到的pwm信息
//    pinName(输入)：要配置的pin
//返回值
//    1：获取成功（如果当前pinName已经有信息了那么返回对应信息，如果没有则找一个空闲(inited==0)的pwm返回）
//    0：当前没有空闲的pwm了
static int rtl8711GetPwmInfo(rtl8711_pwm_info **ppinfo,PinName pinName){
    int count=sizeof(ai_rtlPwmInfoArray)/sizeof(ai_rtlPwmInfoArray[0]);

    for(int i=0;i<count;i++){
        if(ai_rtlPwmInfoArray[i].pinName==pinName){
            *ppinfo=&(ai_rtlPwmInfoArray[i]);
            return 1;
        }
    }
    for(int i=0;i<count;i++){
        if(0==ai_rtlPwmInfoArray[i].inited){
            ai_rtlPwmInfoArray[i].pinName=pinName;
            *ppinfo=&(ai_rtlPwmInfoArray[i]);
            return 1;
        }
    }
    return 0;
}
