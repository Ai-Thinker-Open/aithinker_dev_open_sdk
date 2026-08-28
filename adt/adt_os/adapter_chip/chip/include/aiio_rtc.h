/** @brief      rtc interface implementation.
 *
 *  @file       aiio_rtc.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       Chip initialization interface implementation.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/05/22      <td>1.0.0       <td>wusen      <td>Define file
 *  </table>
 *
 */
#ifndef __AIIO_RTC_H__
#define __AIIO_RTC_H__

#include <stdio.h>
#include <stdint.h>
#include "aiio_type.h"

/** @brief RTC init.
 *
 */
CHIP_API int32_t aiio_hal_rtc_init(void);

/** @brief RTC deinit.
 * 
 */
CHIP_API int32_t aiio_hal_rtc_deinit(void);

/** @brief Get current rtc time (unit:s)
 *
 */
CHIP_API int32_t aiio_hal_rtc_get_time(uint64_t *time_stamp);

/** @brief Set current rtc time (unit:s)
 *
 */
CHIP_API int32_t aiio_hal_rtc_set_time(uint64_t *time_stamp);

#endif
