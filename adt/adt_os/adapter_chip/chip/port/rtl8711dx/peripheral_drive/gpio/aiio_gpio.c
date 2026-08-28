 //中间件相关
#include "aiio_error.h"
#include "aiio_log.h"
#include "aiio_gpio.h"
#include "aiio_pwm.h"

//SDK相关
#include "ameba_soc.h"
#include "os_wrapper.h"
#include "device.h"
#include "gpio_api.h"   // mbed

#define RTL8711_PIN_CALCULATE(a, b) ((a)<<5|(b))

//表示io引脚信息的结构体
typedef struct __aiio_rtl_io_info{
	//pin引脚
	//uint8_t pinIndex;	//引脚编号(芯片手册上引脚的序号)，这个对应关系放在at框架中
	PinName pinName;	//引脚名称(代码中控制引脚用的名字)
	gpio_t pinObj;	//引脚初始化句柄
	//pwmout_t pwmObj;	//pwm初始化句柄
}aiio_rtl_io_info;
//IO引脚信息(顺序是根据IC芯片引脚编号排布的BW16使用的是RTL8720DN 48pin的芯片，注意：框架中编号是从)
static aiio_rtl_io_info ai_rtlIoInfoArray[]={   //WAIT_TODO 待填充
		{.pinName=_PA_8},
        {.pinName=_PA_12},
        {.pinName=_PA_13},
        {.pinName=_PA_14},
        {.pinName=_PA_15},
        {.pinName=_PA_16},
        {.pinName=_PA_17},
        {.pinName=_PA_18},
        {.pinName=_PA_26},
        {.pinName=_PA_27},
        {.pinName=_PA_28},
        {.pinName=_PA_29},
        {.pinName=_PA_30},
        {.pinName=_PA_31},
        {.pinName=_PB_17},
        {.pinName=_PB_18},
        {.pinName=_PB_19},
        {.pinName=_PB_20},
        {.pinName=_PB_21},
	};

static aiio_rtl_io_info *aiio_get_rtl_io_info_by_pinindex(uint32_t port, uint16_t pin);
int aiio_rtl8711d_pwm_close(uint32_t port, uint16_t pin);

aiio_ret_t aiio_hal_gpio_init(uint32_t port, uint16_t pin)
{
    aiio_rtl8711d_pwm_close(port,pin);

    sys_jtag_off();	//JTAG enable pin is disabled
    aiio_rtl_io_info *pIoInfo=aiio_get_rtl_io_info_by_pinindex(port,pin);
    gpio_init(&pIoInfo->pinObj, pIoInfo->pinName);
    return AIIO_OK;
}

aiio_ret_t aiio_hal_gpio_deinit(uint32_t port, uint16_t pin)
{
    aiio_rtl_io_info *pIoInfo=aiio_get_rtl_io_info_by_pinindex(port,pin);
    gpio_deinit(&pIoInfo->pinObj);

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
    PinMode mode;
    aiio_rtl_io_info *pIoInfo=aiio_get_rtl_io_info_by_pinindex(port,pin);

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

    gpio_mode(&pIoInfo->pinObj, mode);

    return AIIO_OK;
}

aiio_ret_t aiio_hal_gpio_set(uint32_t port, uint16_t pin, uint8_t OnOff)
{
    aiio_rtl_io_info *pIoInfo=aiio_get_rtl_io_info_by_pinindex(port,pin);

    gpio_write(&pIoInfo->pinObj, (int)OnOff);

    return AIIO_OK;
}

uint8_t aiio_hal_gpio_get(uint32_t port, uint16_t pin)
{
    aiio_rtl_io_info *pIoInfo=aiio_get_rtl_io_info_by_pinindex(port,pin);

    return (uint8_t)gpio_read(&pIoInfo->pinObj);
}

aiio_ret_t aiio_at_get_gpio_by_map(uint8_t at_pin,uint32_t *p_port, uint16_t *p_pin){
    *p_port=at_pin/32;
    *p_pin=at_pin%32;

    return AIIO_OK;
}
//------------------------------------------------ 内部调用 -----------------------------------------------------------------
static aiio_rtl_io_info *aiio_get_rtl_io_info_by_pinindex(uint32_t port, uint16_t pin){
    PinName rtl_pin=RTL8711_PIN_CALCULATE(port,pin);

	for(uint8_t u8_i=0;u8_i<(sizeof(ai_rtlIoInfoArray)/sizeof(aiio_rtl_io_info));u8_i++){
		if(rtl_pin==ai_rtlIoInfoArray[u8_i].pinName){
			return &ai_rtlIoInfoArray[u8_i];
		}
	}
    aiio_log_w("port:%d pin:%d PinName:%d not found",port,pin,rtl_pin);

	return NULL;	//没有找到对应的pin脚信息
}