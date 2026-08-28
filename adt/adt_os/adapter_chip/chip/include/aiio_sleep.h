/** @brief      sleep application interface.
 *
 *  @file       aiio_sleep.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       adc application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2024/06/14      <td>1.0.0       <td>chenc         <td>sleep Init
 *  </table>
 *
 */
#ifndef __AIIO_SLEEP_H_
#define __AIIO_SLEEP_H_

#include "aiio_log.h"
#include "aiio_type.h"
#include "aiio_error.h"
#include "aiio_gpio.h"

typedef enum {
	AIIO_LOW_POWER_LIGHT_SLEEP=0,	//浅睡眠模式(可以保持连接和数据收发，且上电不会自动进入浅睡眠)
	AIIO_LOW_POWER_LIGHT_SLEEP_AUTO=1,	//浅睡眠模式(可以保持连接和数据收发，且上电就会自动进入浅睡眠)
	AIIO_LOW_POWER_DEEP_SLEEP=2,	//深睡眠模式(连接断开，需要可以串口唤醒)
	AIIO_LOW_POWER_NONE=3,	//正常工作模式
}aiio_lower_power_mode_t;

typedef enum {
	AIIO_LOW_POWER_WAKE_UP_TIMER=0,	//使用定时器唤醒
	AIIO_LOW_POWER_WAKE_UP_GPIO=2,	//使用GPIO唤醒
}aiio_wakeup_source_mode_t;

typedef enum {
	AIIO_LOW_POWER_WAKE_UP_LOW_LEVEL=0,//低电平唤醒
	AIIO_LOW_POWER_WAKE_UP_HIGH_LEVEL,//高电平唤醒
    AIIO_LOW_POWER_WAKE_UP_FALLING_EDGE,//下降沿唤醒
    AIIO_LOW_POWER_WAKE_UP_RISING_EDGE,//上升沿唤醒
    AIIO_LOW_POWER_WAKE_UP_RISING_FALLING_EDGE,//上升沿或下降沿唤醒
}aiio_wakeup_edge_mode_t;



typedef struct __AiWakeUpCfg{
	aiio_lower_power_mode_t lowerPowerMode;	//低功耗模式
	aiio_wakeup_source_mode_t wakeUpSourceMode;	//唤醒源设置
	//定时器唤醒参数
	uint32_t wakeUpMs;	//使用定时器唤醒模式时的定时时间，单位ms
	//GPIO唤醒参数
	uint8_t wakeUpPin;	//使用GPIO唤醒时的唤醒脚序号(这个是映射后的序号，对应的引脚编号(引脚从0开始)为 ai_AtStatus.saveConfigInRam.ioMap[wakeUpPin])
	uint8_t gpioWakeUpLeve;	//使用GPIO唤醒时的唤醒电平，0：低电平唤醒；其它：高电平唤醒
}aiio_wakeup_cfg_t;

//设置低功耗模式
//参数
//    ch：当前通道，注意这个只是给 beforSleepCallback 回调用的，hal层不要向AT口打印数据
//    beforSleepCallback：这个表示进入低功耗之前的回调函数，需要在进入低功耗之前调用 beforSleepCallback(ch);
//返回值
//    0：成功
//    1：失败
//    2：mode错误(不支持的mode，或者当前模式错误)
//    3：参数错误
//    4：引脚不支持唤醒
//    128：不支持的操作(或者没有适配)
CHIP_API uint8_t aiio_set_low_power_mode(aiio_wakeup_cfg_t *wakeup_cfg);

#endif
