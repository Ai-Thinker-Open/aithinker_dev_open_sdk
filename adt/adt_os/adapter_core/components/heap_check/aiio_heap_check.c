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

#include "aiio_heap_check.h"

#include "aiio_log.h"

#if CONFIG_AIIO_HEAP_LOG_EN
aiio_os_function_return_t aiio_heap_check(void *parmas)
{
    while (1)
    {
        aiio_log_i("rtos free heap size:%dk", aiio_os_get_free_heap_size() / 1024);
        aiio_os_tick_dealy(aiio_os_ms2tick(5000));
    }

    return NULL;
}

#endif
