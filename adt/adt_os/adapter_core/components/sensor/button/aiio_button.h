/** @brief      button application interface.
 *
 *  @file       aiio_button.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       button application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>button Init
 *  </table>
 *
 */
#ifndef __AIIO_BUTTON_H_
#define __AIIO_BUTTON_H_
#include "aiio_stdio.h"

typedef uint8_t (*aiio_button_statusin_t)(void);

/** @brief pwm output close
 *
 *  @param[in]      pwm_channel_num Set the channel number
 *  @param[in]      limit           limit number
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 *  @author         liq
 */
CHIP_API int aiio_button_init(aiio_button_statusin_t aiio_button_statusin, uint16_t aiio_button_number, uint16_t *aiio_button_contimer_list, uint16_t *aiio_button_longtimer_list);

/** @brief pwm output close
 *
 *  @param[in]      pwm_channel_num Set the channel number
 *  @param[in]      limit           limit number
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 *  @author         liq
 */
CHIP_API uint16_t aiio_button_get_buttonvalue(void);

#endif
