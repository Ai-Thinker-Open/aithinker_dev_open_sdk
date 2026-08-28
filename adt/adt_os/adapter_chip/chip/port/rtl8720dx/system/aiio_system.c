/** @brief      System-on-Chip Interface.
 *
 *  @file       aiio_system.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       System-on-Chip Interface
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2024/03/21      <td>1.0.0       <td>hongjz      <td>Define the first version of the system interface
 *  </table>
 *
 */
#include "aiio_system.h"

#include "sys_api.h"
#include "osdep_service.h"

void aiio_restart(void)
{
    sys_reset();
}

int aiio_random(void)
{
    uint32_t random;
    rtw_get_random_bytes(&random, sizeof(random));
    return random;
}
