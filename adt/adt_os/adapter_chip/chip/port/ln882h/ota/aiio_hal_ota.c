/** @brief      ota application interface.
 *
 *  @file       aiio_hal_ota.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       Wifi application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/11/23      <td>1.0.0       <td>wusen       <td>frist version
 *  </table>
 *
 */

#include "aiio_platform_ota.h"


int32_t aiio_init_ota_partition(void)
{
    
    return AIIO_OK;
}


int32_t aiio_partition_write_ota_farmware(int32_t dst_offset, const void *src, int32_t size)
{

    return AIIO_OK;
}


int32_t aiio_partition_erase(int32_t start_addr, int32_t size)
{

    return AIIO_OK;
}


void aiio_set_boot_partition(void)
{
 
}

