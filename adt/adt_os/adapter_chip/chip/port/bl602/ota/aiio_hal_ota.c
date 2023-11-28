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

#include <stdio.h>
#include <string.h>

#include "bl_sys.h"
#include "FreeRTOS.h"
#include "task.h"

#include <hal_boot2.h>
#include <bl_mtd.h>
#include "aiio_system.h"
#include "aiio_os_port.h"

static uint8_t activeID;
static HALPartition_Entry_Config otaEntry;
static bl_mtd_handle_t otaHandle = NULL;
static uint8_t ota_init_state;
static uint32_t ota_addr;
static uint32_t part_size;
static uint32_t fw_size;
static uint32_t last_offset;


int32_t aiio_init_ota_partition(void)
{
    int32_t ret;

    if (ota_init_state == 0) {
        ret = bl_mtd_open(BL_MTD_PARTITION_NAME_FW_DEFAULT, &otaHandle, BL_MTD_OPEN_FLAG_BACKUP);
        if (ret) {
            aiio_log_d("Open Default FW partition failed\r\n");
            return AIIO_ERROR;
        }

        activeID = hal_boot2_get_active_partition();
        aiio_log_d("[OTA] activeID:%d \r\n", activeID);

        if (hal_boot2_get_active_entries(BOOT2_PARTITION_TYPE_FW, &otaEntry)) {
            aiio_log_d("PtTable_Get_Active_Entries fail\r\n");
            return AIIO_ERROR;
        }
        ota_addr = otaEntry.Address[!otaEntry.activeIndex];
        part_size = otaEntry.maxLen[!otaEntry.activeIndex];
        hal_update_mfg_ptable();
        // bl_mtd_erase_all(otaHandle);
        aiio_log_d("aiio_init_ota_partition success, ota_addr:0x%x, part_size:0x%x \r\n", ota_addr, part_size);

        ota_init_state = 1;
    }
    
    return AIIO_OK;
}


int32_t aiio_partition_write_ota_farmware(int32_t dst_offset, const void *src, int32_t size)
{
    int32_t ret;

    if (aiio_init_ota_partition() != 0) {
        return AIIO_ERROR;
    }

    if (dst_offset - last_offset > 0x4000) {
        last_offset = dst_offset;
        aiio_log_d("[OTA] write dst_offset:0x%0x \r\n", last_offset);
    }

    bl_mtd_write(otaHandle, dst_offset, size, src);
    fw_size = dst_offset + size;

    return AIIO_OK;
}


int32_t aiio_partition_erase(int32_t start_addr, int32_t size)
{
    if (aiio_init_ota_partition() != 0) {
        return AIIO_ERROR;
    }
    if (start_addr + size > part_size) {
        aiio_log_d("[OTA] erase out of area, part_size:0x%0x \r\n", part_size);
        return AIIO_ERROR;
    }

    aiio_log_d("[OTA] erase start_addr:0x%0x size:0x%0x \r\n", start_addr, size);
    bl_mtd_erase(otaHandle, start_addr, size);

    return AIIO_OK;
}


void aiio_set_boot_partition(void)
{
    uint8_t buff[256];

    otaEntry.len = fw_size;
    aiio_log_d("[OTA] ota size:0x%0x \r\n", fw_size);
    hal_boot2_update_ptable(&otaEntry);
}


