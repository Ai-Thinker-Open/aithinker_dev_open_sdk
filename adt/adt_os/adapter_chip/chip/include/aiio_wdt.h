/** @brief      wdt application interface.
 *
 *  @file       aiio_wdt.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       wdt application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/09/08      <td>1.0.0       <td>hanly       <td>Define WDT API
 *  </table>
 *
 */

#ifndef __AIIO_WDT_H__
#define __AIIO_WDT_H__

#include "aiio_log.h"
#include "aiio_type.h"

/**
 * @brief API to init wdt
 */
void aiio_wdt_init(void);

/**
 * @brief API to deinit wdt
 */
void aiio_wdt_deinit(void);

/**
 * @brief API to reload wdt
 */
void aiio_wdt_reload(void);

#endif