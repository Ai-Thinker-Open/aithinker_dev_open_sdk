/** @brief      sleep application interface.
 *
 *  @file       aiio_sleep.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       adc application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2024/06/14      <td>1.0.0       <td>chenc         <td>sleep Init
 *  </table>
 *
 */
#include "aiio_sleep.h"


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
uint8_t aiio_set_low_power_mode(aiio_wakeup_cfg_t *wakeup_cfg)
{
	return 2;
}