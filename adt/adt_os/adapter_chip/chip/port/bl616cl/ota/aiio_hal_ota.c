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
#include "FreeRTOS.h"
#include "task.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "bflb_mtimer.h"
#include "bflb_flash.h"
#include "bflb_l1c.h"
#include "partition.h"
#include "bflb_boot2.h"

#include "bflb_mtd.h"
#include <bflb_flash.h>

#include "aiio_system.h"
#include "aiio_os_port.h"

static uint8_t activeID;
static pt_table_stuff_config pt_table_stuff[2];
static pt_table_id_type active_id;
static pt_table_entry_config pt_fw_entry;
static bflb_mtd_handle_t otaHandle = NULL;

static uint8_t ota_init_state;
static uint32_t ota_addr;
static uint32_t part_size;
static uint32_t fw_size;
static uint32_t last_offset;

#define OTA_PARTITION_NAME_TYPE_FW    "FW"

static void my_dump_partition(pt_table_stuff_config *pt_stuff)
{
    aiio_log_d("======= PtTable_Config @%p=======\r\n", pt_stuff);
    aiio_log_d("magicCode: 0x%08X;\r\n", (unsigned int)(pt_stuff->pt_table.magicCode));
    aiio_log_d("version: 0x%04X;\r\n", pt_stuff->pt_table.version);
    aiio_log_d("entryCnt %u;\r\n", pt_stuff->pt_table.entryCnt);
    aiio_log_d("age: %lu;\r\n", pt_stuff->pt_table.age);
    aiio_log_d("crc32 0x%08X\r\n", (unsigned int)pt_stuff->pt_table.crc32);
    printf("idx  type device active_index    name    address[0]   address[1]   length[0]    length[1]   age \r\n");
    for (uint32_t i = 0; i < pt_stuff->pt_table.entryCnt; i++) {
        printf("[%02d] ", i);
        printf("  %02u", (pt_stuff->pt_entries[i].type));
        printf("     %u", (pt_stuff->pt_entries[i].device));
        printf("        %u", (pt_stuff->pt_entries[i].active_index));
        printf("       %8s", (pt_stuff->pt_entries[i].name));
        printf("   0x%08lx", (pt_stuff->pt_entries[i].start_address[0]));
        printf("   0x%08lx", (pt_stuff->pt_entries[i].start_address[1]));
        printf("   0x%08lx", (pt_stuff->pt_entries[i].max_len[0]));
        printf("   0x%08lx", (pt_stuff->pt_entries[i].max_len[1]));
        printf("   %lu\r\n", (pt_stuff->pt_entries[i].age));
    }
}



int32_t aiio_init_ota_partition(void)
{
    int32_t ret;

    if (ota_init_state == 0) {

        /* Set flash operation function, read via xip */
        pt_table_set_flash_operation(bflb_flash_erase, bflb_flash_write, bflb_flash_read);

        active_id = pt_table_get_active_partition_need_lock(pt_table_stuff);

        if (PT_TABLE_ID_INVALID == active_id) {
            aiio_log_d("No valid PT\r\n");
            return AIIO_ERROR;
        }
        aiio_log_d("Active PT:%d,Age %d\r\n", active_id, pt_table_stuff[active_id].pt_table.age);

        if (pt_table_get_active_entries_by_name(&pt_table_stuff[active_id], (uint8_t *)OTA_PARTITION_NAME_TYPE_FW, &pt_fw_entry))
        {
            aiio_log_d("PtTable get active entry fail!\r\n");
            return AIIO_ERROR;
        }


        ret = bflb_mtd_open(OTA_PARTITION_NAME_TYPE_FW, &otaHandle, BFLB_MTD_OPEN_FLAG_BACKUP);
        if (ret) {
            aiio_log_d("Open Default FW partition failed\r\n");
            return AIIO_ERROR;
        }

        ota_addr = pt_fw_entry.start_address[!pt_fw_entry.active_index];
        part_size = pt_fw_entry.max_len[!pt_fw_entry.active_index];
        bflb_update_mfg_ptable();
        
        aiio_log_d("aiio_init_ota_partition success, ota_addr:0x%x, part_size:0x%x \r\n", ota_addr, part_size);

        ota_init_state = 1;
    }
    
    return AIIO_OK;
}


int32_t aiio_partition_write_ota_farmware(int32_t dst_offset, const void *src, int32_t size)
{
    if (aiio_init_ota_partition() != 0) {
        return AIIO_ERROR;
    }

    if (dst_offset - last_offset > 0x4000) {
        last_offset = dst_offset;
        aiio_log_d("[OTA] write dst_offset:0x%0x \r\n", last_offset);
    }

    bflb_mtd_write(otaHandle, dst_offset, size, src);
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
    bflb_mtd_erase(otaHandle, start_addr, size);

    return AIIO_OK;
}


void aiio_set_boot_partition(void)
{
    pt_fw_entry.len = fw_size;
    aiio_log_d("[OTA] ota size:0x%0x \r\n", fw_size);
    bflb_boot2_update_ptable(&pt_fw_entry);
}

