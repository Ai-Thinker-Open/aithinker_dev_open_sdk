#ifndef __AIIO_MCU_PROTOCOL_H_
#define __AIIO_MCU_PROTOCOL_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief   初始化MCU通信处理任务
 * 
 * @author  hly
 * @date    2023-09-14
 */
void aiio_mcu_process_init(void);

/**
 * @brief   重新设置向云端发送数据的超时时间
 * 
 * @param[in]   SetTimeout      超时时间
 * 
 * @author  hly
 * @date    2023-09-14
 */
void SetPublicDataTimeout(uint32_t SetTimeout);

/**
 * @brief   重新设备发送心跳包的间隔时间
 * 
 * @param[in]   SetTimeout   设置发送心跳包的间隔时间
 * 
 * @author  hly
 * @date    2023-09-14
 */
void  SetHeartBeatTimeout(uint32_t SetTimeout);

#endif