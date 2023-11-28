/**
 * @brief   Define bl602 ota data proccess interface
 * 
 * @file    aiio_platform_ota.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-08-04          <td>1.0.0            <td>zhuolm             <td>
 */

#include "aiio_adapter_include.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "bflb_mtimer.h"
#include "bflb_flash.h"
#include "bflb_l1c.h"
#include "partition.h"


#define SW_SHA256 0
#define HW_SHA256 1
#define SHA256_EN HW_SHA256

#if SHA256_EN
#include "bflb_sec_sha.h"
static ATTR_NOCACHE_NOINIT_RAM_SECTION struct bflb_sha256_ctx_s ctx_sha256;
static struct bflb_device_s *sha_handle;
#else
#include "utils_sha256.h"
#endif


typedef struct ota_header {
    union {
        struct {
            uint8_t header[16];

            uint8_t type[4]; //RAW XZ
            uint32_t len;    //body len
            uint8_t pad0[8];

            uint8_t ver_hardware[16];
            uint8_t ver_software[16];

            uint8_t sha256[32];
        } s;
        uint8_t _pad[512];
    } u;
} ota_header_t;

#define OTA_PROGRAM_SIZE              (512)
#define OTA_HEADER_SIZE               (sizeof(ota_header_t))
#define OTA_PARTITION_NAME_TYPE_FW    "FW"


static pt_table_stuff_config pt_table_stuff[2];
static pt_table_id_type active_id;
static unsigned int buffer_offset = 0, flash_offset = 0;
static uint32_t bin_size; // part_size;
static pt_table_entry_config pt_fw_entry;
static uint8_t sha256_img[32];
static uint8_t sha256_result[32];
static uint8_t *ota_recv_buffer = NULL;
static bool platform_ota_is_start = false;
static uint32_t ota_progress = 0;
static uint32_t bin_area_size = 0;
static uint32_t ota_addr = 0;
static uint32_t  flash_addr_offset = 0;


// static void aiio_HexPrint(char *str, uint8_t *data, uint16_t data_len)
// {
//     if(str)
//     {
//         aiio_log_i("\n%s: \r\n", str);
//     }
//     for(uint16_t n = 1; n <= data_len; n++)
//     {
//         printf("%02x ", data[n-1]);
//         if(n  % 16 == 0)
//         {
//             printf("\r\n");
//         }
//     }
//     printf("\r\n");
//     printf("\r\n");
// }


// extern void my_dump_partition(pt_table_stuff_config *pt_stuff);
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


int aiio_platform_ota_start(void)
{
    int status = 0;

    if(platform_ota_is_start)
    {
        aiio_log_e("platform ota already start \r\n");
        return AIIO_ERROR;
    }

    /* Set flash operation function, read via xip */
    pt_table_set_flash_operation(bflb_flash_erase, bflb_flash_write, bflb_flash_read);

    active_id = pt_table_get_active_partition_need_lock(pt_table_stuff);
    if (PT_TABLE_ID_INVALID == active_id) 
    {
        aiio_log_e("No valid PT\r\n");
        return AIIO_ERROR;
    }
    aiio_log_d("Active PT:%d,Age %d\r\n", active_id, pt_table_stuff[active_id].pt_table.age);

    my_dump_partition(&pt_table_stuff[active_id]);

    aiio_log_d("[OTA] [TEST] active_id is %u\r\n", active_id);

    if (pt_table_get_active_entries_by_name(&pt_table_stuff[active_id], (uint8_t *)OTA_PARTITION_NAME_TYPE_FW, &pt_fw_entry)) 
    {
        aiio_log_e("PtTable get active entry fail!\r\n");
        return AIIO_ERROR;
    }

    ota_addr = pt_fw_entry.start_address[!pt_fw_entry.active_index];
    bin_area_size = pt_fw_entry.max_len[!pt_fw_entry.active_index];

    aiio_log_d("Starting OTA test. OTA size is %lu\r\n", bin_area_size);

    aiio_log_d("[OTA] [TEST] activeIndex is %u, use OTA address=%08x\r\n", pt_fw_entry.active_index, (unsigned int)ota_addr);

    status = bflb_flash_erase(ota_addr, bin_area_size); /* erase flash */
    if (status != 0) 
    {
        aiio_log_e("flash erase fail! %d\r\n", status);
        return AIIO_ERROR;
    }
    aiio_log_d("Done\r\n");

    /* init sha */
#if SHA256_EN
    sha_handle = bflb_device_get_by_name("sha");
    bflb_group0_request_sha_access(sha_handle);
    bflb_sha_init(sha_handle, SHA_MODE_SHA256);
    bflb_sha256_start(sha_handle, &ctx_sha256);
#else
    sha256_context ctx_sha256;
    utils_sha256_init(&ctx_sha256);
    utils_sha256_starts(&ctx_sha256);
#endif
    memset(sha256_result, 0, sizeof(sha256_result));

    ota_recv_buffer = aiio_os_malloc(OTA_HEADER_SIZE + 1);
    if(ota_recv_buffer == NULL)
    {
        aiio_log_e("malloc fail\r\n");
        aiio_platform_ota_stop();
        return AIIO_ERROR;
    }
    aiio_memset(ota_recv_buffer, 0, OTA_HEADER_SIZE + 1);
    platform_ota_is_start = true;

    return AIIO_OK;
}



int aiio_platform_ota_stop(void)
{
    if(!platform_ota_is_start)
    {
        aiio_log_e("platform ota already stop \r\n");
        return AIIO_ERROR;
    }
    memset(pt_table_stuff, 0, sizeof(pt_table_stuff));
    memset(sha256_img, 0, sizeof(sha256_img));
    memset(sha256_result, 0, sizeof(sha256_result));
    memset(&pt_fw_entry, 0, sizeof(pt_table_entry_config));

    buffer_offset = 0;
    flash_offset = 0;
    bin_size = 0;
    ota_progress = 0;
    bin_area_size = 0;
    ota_addr = 0;
    flash_addr_offset = 0;

    if(ota_recv_buffer)
    {
        aiio_os_free(ota_recv_buffer);
        ota_recv_buffer = NULL;
    }

    platform_ota_is_start = false;

    return AIIO_OK;
}

static int aiio_ota_package_header_check(ota_header_t *ota_header, uint32_t *ota_len, int16_t *use_xz)
{
    char str[33] = {0};//assume max segment size
    int i = 0;

    aiio_strncpy(str, (char *)ota_header->u.s.header, sizeof(ota_header->u.s.header));
    str[sizeof(ota_header->u.s.header)] = '\0';
    aiio_log_i("[OTA] [HEADER] ota header is : %s \r\n", str);

    aiio_strncpy(str, (char *)ota_header->u.s.type, sizeof(ota_header->u.s.type));
    str[sizeof(ota_header->u.s.type)] = '\0';
    aiio_log_i("[OTA] [HEADER] ota type is : %s \r\n", str);

    if (strstr(str, "XZ")) {
        *use_xz = 1;
    } else {
        *use_xz = 0;
    }

    if ((!strcmp(str, "RAW")) && (*use_xz != 1)) 
    {
        return AIIO_ERROR;
    }

    aiio_strncpy((char *)ota_len, (char *)&(ota_header->u.s.len), 4);
    aiio_log_i("[OTA] [HEADER] file length (exclude ota header) is %lu\r\n", *ota_len);

    aiio_strncpy(str, (char *)ota_header->u.s.ver_hardware, sizeof(ota_header->u.s.ver_hardware));
    str[sizeof(ota_header->u.s.ver_hardware)] = '\0';
    aiio_log_i("[OTA] [HEADER] ver_hardware is : %s \r\n", str);

    aiio_strncpy(str, (char *)ota_header->u.s.ver_software, sizeof(ota_header->u.s.ver_software));
    str[sizeof(ota_header->u.s.ver_software)] = '\0';
    aiio_log_i("[OTA] [HEADER] ver_software is : %s \r\n", str);

    aiio_strncpy(str, (char *)ota_header->u.s.sha256, sizeof(ota_header->u.s.sha256));
    str[sizeof(ota_header->u.s.sha256)] = '\0';
    aiio_log_i("[OTA] [HEADER] sha256 is : \r\n");
    for (i = 0; i < sizeof(ota_header->u.s.sha256); i++) {
        printf("%02X", str[i]);
    }
    printf("\r\n");

    return AIIO_OK;
}

int aiio_platform_ota_install(uint8_t *data, uint32_t data_len, uint32_t data_total_len)
{
    ota_header_t *ota_header;
    int16_t use_xz = 0;
    int err = 0;
    uint32_t i = 0;
    int status = 0;

    if(data == NULL)
    {
        aiio_log_e("param err \r\n");
        return AIIO_ERROR;
    }

    if(!platform_ota_is_start)
    {
        aiio_log_e("platform ota stop \r\n");
        return AIIO_ERROR;
    }

    if(bin_size == 0)
    {
        if(ota_progress >= OTA_HEADER_SIZE)
        {
            // aiio_HexPrint("ota_recv_buffer", ota_recv_buffer, ota_progress);
            ota_header = (ota_header_t*)ota_recv_buffer;
            err = aiio_ota_package_header_check(ota_header, &bin_size, &use_xz);
            if((err != AIIO_OK) || (use_xz != 1))
            {
                aiio_log_e("ota package header check err \r\n");
                aiio_platform_ota_stop();
                return AIIO_ERROR;
            }

            aiio_log_i("bin_size =  0x%02lx \r\n", bin_size);
            aiio_strncpy((char *)sha256_img, (char *)ota_header->u.s.sha256, sizeof(sha256_img));
            aiio_log_i("[OTA] [TCP] Update bin_size to %lu, file status %s\r\n", bin_size, use_xz ? "XZ" : "RAW");
            aiio_log_i("sha256_img : \r\n");
            for (i = 0; i < sizeof(sha256_img); i++) 
            {
                printf("%02X", sha256_img[i]);
            }
            printf("\r\n");

            if(bin_size > bin_area_size)
            {
                aiio_log_e("ota file too big \r\n");
                aiio_platform_ota_stop();
                return AIIO_ERROR;
            }

            // aiio_HexPrint("ota_recv_buffer", data, data_len);
#if SHA256_EN
            bflb_l1c_dcache_clean_range(data, data_len);
            status = bflb_sha256_update(sha_handle, &ctx_sha256, data, data_len);
            if (status != 0) 
            {
                aiio_log_e("sha256 update fail! %d\r\n", status);
                aiio_platform_ota_stop();
                return AIIO_ERROR;
            }
#else
            utils_sha256_update(&ctx_sha256, data, data_len);
#endif
            status = bflb_flash_write((ota_addr + flash_addr_offset), data, data_len);
            if (status != 0) 
            {
                aiio_log_e("flash write fail! %d\r\n", status);
                aiio_log_e("flash write addr: 0x%08x\r\n", (ota_addr + flash_addr_offset));
                aiio_platform_ota_stop();
                return AIIO_ERROR;
            }
            flash_addr_offset += data_len;
        }
        else
        {
            // aiio_HexPrint("ota_recv_buffer", data, data_len);
            if(ota_progress + data_len > OTA_HEADER_SIZE)
            {
                aiio_strncpy((char *)&ota_recv_buffer[ota_progress], (char *)data, (OTA_HEADER_SIZE - ota_progress));
#if SHA256_EN
                bflb_l1c_dcache_clean_range(&data[OTA_HEADER_SIZE - ota_progress], (data_len - (OTA_HEADER_SIZE - ota_progress)));
                status = bflb_sha256_update(sha_handle, &ctx_sha256, &data[OTA_HEADER_SIZE - ota_progress], (data_len - (OTA_HEADER_SIZE - ota_progress)));
                if (status != 0) 
                {
                    aiio_log_e("sha256 update fail! %d\r\n", status);
                    aiio_platform_ota_stop();
                    return AIIO_ERROR;
                }
#else
                utils_sha256_update(&ctx_sha256, &data[OTA_HEADER_SIZE - ota_progress], (data_len - (OTA_HEADER_SIZE - ota_progress)));
#endif
                status = bflb_flash_write((ota_addr + flash_addr_offset), &data[OTA_HEADER_SIZE - ota_progress], (data_len - (OTA_HEADER_SIZE - ota_progress)));
                if (status != 0) 
                {
                    aiio_log_e("flash write fail! %d\r\n", status);
                    aiio_log_e("flash write addr: 0x%08x\r\n", (ota_addr + flash_addr_offset));
                    aiio_platform_ota_stop();
                    return AIIO_ERROR;
                }
                flash_addr_offset += (data_len - (OTA_HEADER_SIZE - ota_progress));
            }
            else
            {
                aiio_strncpy((char *)&ota_recv_buffer[ota_progress], (char *)data, data_len);
                
            }
        }
    }
    else
    {
        // aiio_HexPrint("ota_recv_buffer", data, data_len);
#if SHA256_EN
        bflb_l1c_dcache_clean_range(data, data_len);
        status = bflb_sha256_update(sha_handle, &ctx_sha256, data, data_len);
        if (status != 0) 
        {
            aiio_log_e("sha256 update fail! %d\r\n", status);
            aiio_platform_ota_stop();
            return AIIO_ERROR;
        }
#else
        utils_sha256_update(&ctx_sha256, data, data_len);
#endif
        status = bflb_flash_write((ota_addr + flash_addr_offset), data, data_len);
        if (status != 0) 
        {
            aiio_log_e("flash write fail! %d\r\n", status);
            aiio_log_e("flash write addr: 0x%08x\r\n", (ota_addr + flash_addr_offset));
            aiio_platform_ota_stop();
            return AIIO_ERROR;
        }
        flash_addr_offset += data_len;
    }
    
    ota_progress += data_len;
    if(ota_progress == data_total_len)
    {
#if SHA256_EN
        bflb_sha256_finish(sha_handle, &ctx_sha256, sha256_result);
#else
        utils_sha256_finish(&ctx_sha256, sha256_result);
#endif

        aiio_log_i("ota_progress = %d \r\n", ota_progress);
        aiio_log_i("data_total_len = %d \r\n", data_total_len);
        aiio_log_i("bin_size = %d \r\n", bin_size);
        aiio_log_i("flash_addr_offset = %d \r\n", flash_addr_offset);
        aiio_log_i("Calculated SHA256 Checksum: \r\n");
        for (i = 0; i < sizeof(sha256_result); i++) 
        {
            printf("%02X", sha256_result[i]);
        }
        printf("\r\n");
        aiio_log_i("Header SET SHA256 Checksum:");
        for (i = 0; i < sizeof(sha256_img); i++) 
        {
            printf("%02X", sha256_img[i]);
        }
        printf("\r\n");
        if (memcmp(sha256_img, sha256_result, sizeof(sha256_img))) 
        {
            /*Error found*/
            aiio_log_e("[OTA] [TCP] SHA256 NOT Correct\r\n");
            aiio_platform_ota_stop();
            return AIIO_ERROR;
        }

        aiio_log_i("[OTA] [TCP] prepare OTA partition info\r\n");
        pt_fw_entry.len = bin_size;
        aiio_log_i("[OTA] [TCP] Update PARTITION, partition len is %lu\r\n", pt_fw_entry.len);
        pt_fw_entry.active_index = !(pt_fw_entry.active_index & 0x01);
        pt_fw_entry.age++;
        status = pt_table_update_entry(!active_id, &pt_table_stuff[active_id], &pt_fw_entry);
        if (status != 0) 
        {
            printf("pt table update fail! %d\r\n", status);
            aiio_platform_ota_stop();
            return AIIO_ERROR;
        }
        aiio_log_i("[OTA] [TCP] Rebooting\r\n");
        aiio_log_i("--------------SUCCES------------------\r\n"); 
        return AIIO_OTA_INSTALL_SUCCESS;
    }

    return AIIO_OK;
}











