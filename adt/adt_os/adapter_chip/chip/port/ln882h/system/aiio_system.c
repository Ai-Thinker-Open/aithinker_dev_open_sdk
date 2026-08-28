/** @brief      System-on-Chip Interface.
 *
 *  @file       aiio_system.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       System-on-Chip Interface
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/04/13      <td>1.0.0       <td>wusen       <td>Define the first version of the system interface
 *  </table>
 *
 */
#include "aiio_system.h"
#include <stdlib.h>
#include "reboot_trace.h"

void aiio_restart(void)
{
   ln_chip_reboot();
}

int aiio_random(void)
{
   return rand();
}
