#include "aiio_gpio.h"
#include "aiio_error.h"
#include "aiio_log.h"
#include "gpio_api.h"

#define RTL8720_PIN_CALCULATE(a, b) ((a)<<5|(b))

//表示io引脚信息的结构体
typedef struct __aiio_rtl_io_info{
	//IO功能(都是0表示不能控制，例如EN，串口，vcc等引脚)
	uint8_t	asGpio:1;	//该引脚是否可以设置为GPIO
	uint8_t	asPwm:1;	//该引脚是否可以设置为PWM引脚
	//pin引脚
	uint8_t pinIndex;	//引脚编号(芯片手册上引脚的序号)
	PinName pinName;	//引脚名称(代码中控制引脚用的名字)
	gpio_t pinObj;	//引脚初始化句柄
	// pwmout_t pwmObj;	//pwm初始化句柄
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


aiio_ret_t aiio_at_get_gpio_by_map(uint8_t at_pin,uint32_t *p_port, uint16_t *p_pin){

    PinName pin;

      for(uint8_t u8_i=0;u8_i<(sizeof(ai_rtlIoInfoArray)/sizeof(aiio_rtl_io_info));u8_i++){
            if(at_pin==ai_rtlIoInfoArray[u8_i].pinIndex){
            pin= ai_rtlIoInfoArray[u8_i].pinName;
            break;
        }
      }

    *p_port=pin/32;
    *p_pin=pin%32;

    return AIIO_OK;
}
//------------------------------------------------ 内部调用 -----------------------------------------------------------------
static aiio_rtl_io_info *aiio_get_rtl_io_info_by_pinindex(uint32_t port, uint16_t pin){
    PinName rtl_pin=RTL8720_PIN_CALCULATE(port,pin);

	for(uint8_t u8_i=0;u8_i<(sizeof(ai_rtlIoInfoArray)/sizeof(aiio_rtl_io_info));u8_i++){
		if(rtl_pin==ai_rtlIoInfoArray[u8_i].pinName){
			return &ai_rtlIoInfoArray[u8_i];
		}
	}
    aiio_log_w("port:%d pin:%d PinName:%d not found",port,pin,rtl_pin);

	return NULL;	//没有找到对应的pin脚信息
}


aiio_ret_t aiio_hal_gpio_init(uint32_t port, uint16_t pin)
{
    sys_jtag_off();	//JTAG enable pin is disabled
    aiio_rtl_io_info *pIoInfo=aiio_get_rtl_io_info_by_pinindex(port,pin);
    gpio_init(&pIoInfo->pinObj, pIoInfo->pinName);
    return AIIO_OK;
}

aiio_ret_t aiio_hal_gpio_deinit(uint32_t port, uint16_t pin)
{
    aiio_rtl_io_info *pIoInfo=aiio_get_rtl_io_info_by_pinindex(port,pin);
    gpio_deinit(&pIoInfo->pinObj, pIoInfo->pinName);

    return AIIO_OK;
}

int32_t aiio_hal_gpio_pin_direction_set(uint32_t port, uint16_t pin, aiio_gpio_direction_t dir)
{
    aiio_rtl_io_info *pIoInfo=aiio_get_rtl_io_info_by_pinindex(port,pin);
    gpio_dir(&pIoInfo->pinObj, dir);

    return AIIO_OK;
}

int32_t aiio_hal_gpio_pin_pull_set(uint32_t port, uint16_t pin, aiio_gpio_pull_t pull)
{
    gpio_t gpio_cfg;
    PinMode mode;
    aiio_rtl_io_info *pIoInfo=aiio_get_rtl_io_info_by_pinindex(port,pin);
    gpio_cfg.pin = pIoInfo->pinName;

    switch (pull)
    {
    case AIIO_GPIO_PULL_DOWN:
        mode = PullDown;
        break;
    case AIIO_GPIO_PULL_UP:
        mode = PullUp;
        break;
    case AIIO_GPIO_PULL_NONE:
        mode = PullNone;
        break;
    default:
        mode = PullNone;
        break;
    }

    gpio_mode(&gpio_cfg, mode);

    return AIIO_OK;
}

aiio_ret_t aiio_hal_gpio_set(uint32_t port, uint16_t pin, uint8_t OnOff)
{
    gpio_t gpio_cfg;
    aiio_rtl_io_info *pIoInfo=aiio_get_rtl_io_info_by_pinindex(port,pin);
    gpio_cfg.pin = pIoInfo->pinName;

    gpio_write(&gpio_cfg, (int)OnOff);

    return AIIO_OK;
}

uint8_t aiio_hal_gpio_get(uint32_t port, uint16_t pin)
{
    gpio_t gpio_cfg;
    aiio_rtl_io_info *pIoInfo=aiio_get_rtl_io_info_by_pinindex(port,pin);
    gpio_cfg.pin = pIoInfo->pinName;

    return (uint8_t)gpio_read(&gpio_cfg);
}
