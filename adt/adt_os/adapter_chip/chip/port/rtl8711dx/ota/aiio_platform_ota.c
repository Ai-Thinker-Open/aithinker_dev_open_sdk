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

#include <stdint.h>

#include "aiio_type.h"
#include "aiio_error.h"
#include "aiio_log.h"
#include "aiio_platform_ota.h"

#include "sys_api.h"
// #include "device_lock.h"
#include "ameba_soc.h"
#include "ameba_ota.h"
#include "flash_api.h"
// #include "rtl8721d_ota.h"
// #include "rtl8721d_boot.h"

extern u32 IMG_ADDR[OTA_IMGID_MAX][2];

static uint8_t rtl_ota_inited=0;
// static uint8_t erase_flag = 0;
static uint8_t platform_ota_is_start = 0;
static uint32_t s_fw_len = 0;
extern ota_context *ctx;


int aiio_platform_ota_start(void)
{
    if(platform_ota_is_start)
    {
        aiio_log_e("platform ota already start\r\n");
        return AIIO_ERROR;
    }

    // erase_flag = 0;
    s_fw_len = 0;

    platform_ota_is_start = 1;

    return AIIO_ERROR;
}

int aiio_platform_ota_stop(void)
{
    if(!platform_ota_is_start)
    {
        aiio_log_e("platform ota already stop\r\n");
        return AIIO_ERROR;
    }

    platform_ota_is_start = 0;

    return AIIO_ERROR;
}

int aiio_platform_ota_install(uint8_t *data, uint32_t data_len, uint32_t data_total_len)
{
    if(!platform_ota_is_start)
    {
        aiio_log_e("platform ota stop \r\n");
        return AIIO_ERROR;
    }

    // if (!erase_flag)
    // {
    //     partition_erase(0, (data_total_len / 4096 + 1) * 4096);
    //     erase_flag = 1;
    // }

    if (aiio_partition_write_ota_farmware(s_fw_len, data, data_len) != 0)
    {
        goto __install_err;
    }

    s_fw_len += data_len;
    if (s_fw_len == data_total_len)
    {
        //对固件进行校验，校验成功后切换分区
        if (verify_ota_checksum(ctx->otaTargetHdr, ctx->otactrl->targetIdx, ctx->otactrl->index))
        {
            if (!ota_update_manifest(ctx->otaTargetHdr, ctx->otactrl->targetIdx, ctx->otactrl->index))
            {
                aiio_log_e("Change signature failed\r\n");
                goto __install_err;
            }
            aiio_log_i("--------------SUCCES------------------\r\n"); 
            return AIIO_OTA_INSTALL_SUCCESS;
        }

        // 固件校验失败
        aiio_log_e("verify_ota_checksum() error\r\n");
        goto __install_err;
    }

    return AIIO_OK;

__install_err:
    aiio_platform_ota_stop();
    return AIIO_ERROR;
}


int aiio_platform_get_ota_cache_addr(uint32_t need_size, uint32_t *out_addr)
{
    (void)need_size;

    u32 cur_idx = ota_get_cur_index(OTA_IMGID_APP);
    u32 target_idx = (cur_idx == OTA_INDEX_1) ? OTA_INDEX_2 : OTA_INDEX_1;

    *out_addr = IMG_ADDR[OTA_IMGID_APP][target_idx] - SPI_FLASH_BASE;
    return AIIO_OK;
}











