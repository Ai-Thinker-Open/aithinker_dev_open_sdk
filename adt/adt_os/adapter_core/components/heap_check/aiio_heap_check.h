/** @brief      Heap check interface.
 *
 *  @file       aiio_chip_init.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       Heap check interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/01/30      <td>1.0.0       <td>chenzf      <td>Define file
 *  </table>
 *
 */

#ifndef __AIIO_HEAP_CHECK_H__
#define __AIIO_HEAP_CHECK_H__

#include "aiio_type.h"
#include "aiio_error.h"
#include "aiio_os_port.h"

#if CONFIG_AIIO_HEAP_LOG_EN
aiio_os_function_return_t aiio_heap_check(void *parmas);
#endif

#endif /* __AIIO_HEAP_CHECK_H__ */
