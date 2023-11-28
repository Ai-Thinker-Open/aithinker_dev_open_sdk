/** @brief      AT io interface.
 *
 *  @file       aiio_at_io.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/28      <td>1.0.0       <td>lins        <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_IO_H__
#define __AIIO_AT_IO_H__

#include "aiio_type.h"

/** @brief AT printf
 *
 *  @param[in]      fmt             C string that contains the text to be written to AT uart
 *  @param[in]      ...             Additional arguments.
 *  @return
 *  @note
 *  @see
 */
CORE_API void aiio_at_printf(char *fmt, ...);

/** @brief AT send data
 *
 *  @param[in]      data             Point to data.
 *  @param[in]      len              Data length.
 *  @return
 *  @note
 *  @see
 */
CORE_API void aiio_at_send_data(const uint8_t *data, uint32_t len);

/** @brief AT response OK
 *
 *  @return
 *  @note
 *  @see
 */
CORE_API void aiio_at_response_ok(void);

/** @brief AT response ERROR
 *
 *  @param[in]      name             Cmd name.
 *  @param[in]      errcode          Error code.
 *  @return
 *  @note
 *  @see
 */
CORE_API void aiio_at_response_error(const char *name, int errcode);

/** @brief Receive one byte from UART buffer
 *
 *  @param[out]     data             Output data.
 *  @return                          Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK          Successful.
 *  @retval         AIIO_ERROR       Error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_at_recv_one_byte(uint8_t *data);

/** @brief Init AT IO
 *
 *  @return
 *  @note
 *  @see
 */
CORE_API void aiio_at_io_init(void);

/** @brief Reinit AT IO
 *
 *  @return
 *  @note
 *  @see
 */
CORE_API void aiio_at_io_reinit(void);

#endif
