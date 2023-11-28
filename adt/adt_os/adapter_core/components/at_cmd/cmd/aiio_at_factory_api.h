/** @brief      AT factory test command api.
 *
 *  @file       aiio_at_factory_api.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/11/21      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_FACTORY_API_H__
#define __AIIO_AT_FACTORY_API_H__
#include <stdint.h>
#include "aiio_type.h"
#include "aiio_at_io_control.h"

uint8_t aiio_KitButtonInit(uint8_t enable,void *param);
uint8_t aiio_GetLedShowGroup(char *modeName,uint8_t **p_ledArray,uint8_t *p_ledDriver);
uint8_t aiio_GetGpioTestGroup(char *modeName,aiio_fac_gpiotest_grop_t **pgpioTestArray);

#endif