/** @brief      AT io control command.
 *
 *  @file       aiio_at_io_control.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/03/01      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_IO_CONTROL_H__
#define __AIIO_AT_IO_CONTROL_H__
#include <stdint.h>
#include "aiio_type.h"
#include "aiio_gpio.h"

#define AIIO_CONFIG_MAX_IO_MAP_NUMBER	(48)	//Maximum number of I/O map stores

#define AIIO_GLB_GPIO_PIN_MAX  __AIIO_GPIO_NUM_MAX

typedef struct{
    uint32_t ioMapSetTag;	//用于判断是否写入过IO MAP(这个值为 CONFIG_SAVED_VALUE 的时候表示已经初始化过的)
	uint8_t ioMapNumber;	//IO映射表中的数据有多少个
	uint8_t ioMap[AIIO_CONFIG_MAX_IO_MAP_NUMBER];	//io映射表，例如ioMap[0]==10，那么表示模组的0号引脚对应IC的10号引脚，255表示不可用的引脚
}aiio_iomap_save_config_t;

//IO模式
typedef enum{
	AIIO_IO_MODE_DEFAULT=0,	//默认状态(刚上电，还没有设置过)
	AIIO_IO_MODE_GPIO_OUT=1,	//gpio输出模式
	AIIO_IO_MODE_GPIO_IN_NO_PULL=2,	//gpio输入模式，没有上下拉
	AIIO_IO_MODE_GPIO_IN_PULL_UP=3,	//gpio输入模式，上拉
	AIIO_IO_MODE_GPIO_IN_PULL_DOWN=4,	//gpio输入模式，下拉
	AIIO_IO_MODE_PWM=5,	//pwm模式
}aiio_iomode_type_t;

//pwm参数
typedef struct{
	float	duty;	//占空比0~100
	uint32_t	cycle;	//pwm周期(单位us)
}aiio_pwm_config_t;

//IO运行状态记录
typedef struct{
	aiio_iomode_type_t	ioMode;	//当前IO状态
	aiio_pwm_config_t   pwmCfg;	//pwm配置(当ioMode==AIIO_IO_MODE_PWM时有效)
}aiio_iostatus_config_t;

typedef struct{
    aiio_iostatus_config_t ioStatus[AIIO_CONFIG_MAX_IO_MAP_NUMBER];	//保存IO配置信息的数组
}aiio_io_status_config_t;


//GPIO对测分组，用来记录哪两个GPIO相连，使用的pin脚序号是根据模组封装上的引脚顺序
typedef struct{
	uint8_t pinA;
	uint8_t pinB;
}aiio_fac_gpiotest_grop_t;

typedef struct {
    //key
    aiio_hal_gpio_int_cb_t kitButTriggerCallback;
}aiio_at_key_callbacks_t;

extern aiio_at_key_callbacks_t aiio_key_Callbacks;

extern aiio_iomap_save_config_t aiio_iomap_save_config;
extern aiio_io_status_config_t aiio_io_status_config;


CORE_API int32_t aiio_at_io_control_cmd_regist(void);

#endif
