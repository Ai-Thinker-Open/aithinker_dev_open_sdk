/** @brief      Chip initialization interface implementation.
 *
 *  @file       aiio_chip_init.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       Chip initialization interface implementation.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/01/30      <td>1.0.0       <td>chenzf      <td>Define file
 *  <tr><td>2023/02/01      <td>1.0.1       <td>chenzf      <td>Update aiio_chip_init
 *  </table>
 *
 */
#ifndef __AIIO_CHIP_INIT_H__
#define __AIIO_CHIP_INIT_H__


#include "aiio_type.h"

/** @brief The chip initialization function is called in the main function.
 *
 *  @return             Return the operation status. When the return value is AIIO_OK, the initialization is successful.
 *  @retval             AIIO_OK         Init successful.
 *  @retval             AIIO_ERROR      Init error.
 *  @note               This function needs to be adapted according to different platforms, and the system engineer should complete the adaptation.
 *                      The main function is to initialize the chip, including clock initialization, register initialization, etc., must call this function to run other interfaces.
 *  @see
 */
CHIP_API int32_t aiio_chip_init(void);

#endif
