/**
 * @brief   Declare ota launch interface and ota data transmit callback interface
 * 
 * @file    aiio_ota_transmit.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-08-04          <td>1.0.0            <td>zhuolm             <td> The ota launch interface and ota data transmit callback interface
 */
#ifndef __AIIO_OTA_TRANSMIT_H_
#define __AIIO_OTA_TRANSMIT_H_
#include "aiio_common.h"


int aiio_ota_transmit_start(uint8_t *ota_info);
int aiio_ota_transmit_stop(void);




#endif

