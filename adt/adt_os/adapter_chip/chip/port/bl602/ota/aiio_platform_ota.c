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
#include <utils_sha256.h>
#include <hal_boot2.h>
#include <hal_sys.h>
#include <bl_mtd.h>


static bl_mtd_handle_t handle = NULL;
static iot_sha256_context ctx;
static uint8_t sha256_result[32];
static uint8_t sha256_img[32];
static HALPartition_Entry_Config ptEntry = {0};
static uint32_t ota_addr = 0;
static uint32_t bin_size = 0;
static uint32_t bin_max_size = 0;
static uint32_t part_size = 0;
static uint32_t ota_progress = 0;
static bool platform_ota_is_start = false;
static uint8_t  *ota_recv_buffer = NULL;
static uint32_t  flash_addr_offset = 0;

typedef struct ota_header {
    union {
        struct {
            uint8_t header[16];

            uint8_t type[4];//RAW XZ
            uint32_t len;//body len
            uint8_t pad0[8];

            uint8_t ver_hardware[16];
            uint8_t ver_software[16];

            uint8_t sha256[32];
        } s;
        uint8_t _pad[512];
    } u;
} ota_header_t;

#define OTA_HEADER_SIZE (sizeof(ota_header_t))


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


int aiio_platform_ota_start(void)
{
    uint8_t activeID;
    uint8_t ret = 0;

    if(platform_ota_is_start)
    {
        aiio_log_e("platform ota already start\r\n");
        return AIIO_ERROR;
    }

    memset(&ctx, 0, sizeof(ctx));
    memset(&sha256_result, 0, sizeof(sha256_result));
    memset(&sha256_img, 0, sizeof(sha256_img));
    memset(&ptEntry, 0, sizeof(ptEntry));
    ota_addr = 0;
    bin_size = 0;
    bin_max_size = 0;
    part_size = 0;

    ret = bl_mtd_open(BL_MTD_PARTITION_NAME_FW_DEFAULT, &handle, BL_MTD_OPEN_FLAG_BACKUP);
    if (ret) 
    {
        aiio_log_e("Open Default FW partition failed\r\n");
        return AIIO_ERROR;
    }

    // bl_mtd_erase_all(handle); 
    activeID = hal_boot2_get_active_partition();                                    
    aiio_log_i("activeID =  %d \r\n", activeID);
    if (hal_boot2_get_active_entries(BOOT2_PARTITION_TYPE_FW, &ptEntry))           
    {
        aiio_log_e("hal_boot2_get_active_entries fail\r\n");
        bl_mtd_close(handle);
        return AIIO_ERROR;
    }

    ota_addr = ptEntry.Address[!ptEntry.activeIndex];
    bin_max_size = ptEntry.maxLen[!ptEntry.activeIndex];
    part_size = ptEntry.maxLen[!ptEntry.activeIndex];

    aiio_log_i("ota_addr =  0x%08lx \r\n", ota_addr);
    aiio_log_i("code addr =  0x%08lx \r\n", ptEntry.Address[ptEntry.activeIndex]);
    aiio_log_i("bin_size =  0x%02lx \r\n", bin_size);
    aiio_log_i("bin_max_size =  0x%02lx \r\n", bin_max_size);
    aiio_log_i("part_size =  0x%02lx \r\n", part_size);
    aiio_log_i("ptEntry.activeIndex =  %d \r\n", ptEntry.activeIndex);

    // bl_mtd_erase(handle, 0, bin_max_size);
    bl_mtd_erase_all(handle); 
    ota_recv_buffer = aiio_os_malloc(OTA_HEADER_SIZE + 1);
    if(ota_recv_buffer == NULL)
    {
        aiio_log_e("malloc fail\r\n");
        aiio_platform_ota_stop();
        return AIIO_ERROR;
    }
    aiio_memset(ota_recv_buffer, 0, OTA_HEADER_SIZE + 1);

    aiio_log_i("Done\r\n");
    utils_sha256_init(&ctx);
    utils_sha256_starts(&ctx);

    platform_ota_is_start = true;

    return AIIO_OK;
}



int aiio_platform_ota_stop(void)
{

    if(!platform_ota_is_start)
    {
        aiio_log_e("platform ota already stop\r\n");
        return AIIO_ERROR;
    }

    utils_sha256_free(&ctx);
    bl_mtd_close(handle);

    if(ota_recv_buffer)
    {
        aiio_os_free(ota_recv_buffer);
        ota_recv_buffer = NULL;
    }

    aiio_memset(sha256_result, 0, sizeof(sha256_result));
    aiio_memset(sha256_img, 0, sizeof(sha256_img));
    ota_progress = 0;
    ota_addr = 0;
    bin_size = 0;
    bin_max_size = 0;
    part_size = 0;
    flash_addr_offset = 0;
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
    if (strstr(str, "XZ")) 
    {
        *use_xz = 1;
    } 
    else if(strstr(str, "RAW"))
    {
        *use_xz = 0;
    }
    else
    {
        aiio_log_e("ota file format err\r\n");
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

            if(bin_size > bin_max_size)
            {
                aiio_log_e("ota file too big \r\n");
                aiio_platform_ota_stop();
                return AIIO_ERROR;
            }

            // aiio_HexPrint("ota_recv_buffer", data, data_len);
            utils_sha256_update(&ctx, data, data_len);
            bl_mtd_write(handle, flash_addr_offset, data_len, data);
            flash_addr_offset += data_len;

        }
        else
        {
            // aiio_HexPrint("ota_recv_buffer", data, data_len);
            if(ota_progress + data_len > OTA_HEADER_SIZE)
            {
                aiio_strncpy((char *)&ota_recv_buffer[ota_progress], (char *)data, (OTA_HEADER_SIZE - ota_progress));
                utils_sha256_update(&ctx, &data[OTA_HEADER_SIZE - ota_progress], (data_len - (OTA_HEADER_SIZE - ota_progress)));
                bl_mtd_write(handle, flash_addr_offset, (data_len - (OTA_HEADER_SIZE - ota_progress)), &data[OTA_HEADER_SIZE - ota_progress]);
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
        utils_sha256_update(&ctx, data, data_len);
        bl_mtd_write(handle, flash_addr_offset, data_len, data);
        flash_addr_offset += data_len;
    }
    
    ota_progress += data_len;
    if(ota_progress == data_total_len)
    {
        utils_sha256_finish(&ctx, sha256_result);//加密完成
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
            bl_mtd_erase_all(handle); 
            aiio_platform_ota_stop();
            return AIIO_ERROR;
        }

        ptEntry.len = bin_size;
        hal_boot2_update_ptable(&ptEntry);//结束。
        aiio_log_i("--------------SUCCES------------------\r\n"); 
        return AIIO_OTA_INSTALL_SUCCESS;
    }

    return AIIO_OK;
}











