/**
 * @brief   
 * 
 * @file    aiio_ap_config_wifi.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-13          <td>1.0.0            <td>zhuolm             <td>
 */
#ifndef __AIIO_AP_CONFIG_WIFI_H_
#define __AIIO_AP_CONFIG_WIFI_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


void aiio_ap_config_start(void);
void aiio_ap_config_stop(void);
void aiio_ap_config_notify(uint8_t event);


#endif

