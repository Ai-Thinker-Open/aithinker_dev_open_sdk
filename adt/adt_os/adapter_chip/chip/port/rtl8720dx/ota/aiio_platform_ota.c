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
#include "device_lock.h"
#include "ameba_soc.h"
#include "flash_api.h"
#include "rtl8721d_ota.h"
#include "rtl8721d_boot.h"

extern const u32 IMG_ADDR[MAX_IMG_NUM][2];

static uint8_t erase_flag = 0;
static uint8_t platform_ota_is_start = 0;
static uint32_t s_fw_len = 0;
static update_ota_target_hdr OtaTargetHdr;

static int partition_erase(int start_addr, int size)
{
    uint32_t NewFWAddr = 0;
    uint32_t NewFWBlkSize = 0;
    uint32_t ota_target_index;
    flash_t flash_ota;

    ota_target_index = !ota_get_cur_index();

    //------------------------step4: erase flash space for new firmware------------------------
    // 擦除flash
    NewFWAddr = IMG_ADDR[0][ota_target_index]; // 根据镜像位置确定flash擦除起始未知
    NewFWBlkSize = ((size - 1) / 4096) + 2;    // 根据长度计算要擦除的扇区大小
    aiio_log_i("erase size=%d bytes, NewFWBlkSize=%d\r\n", size, NewFWBlkSize);

    device_mutex_lock(RT_DEV_LOCK_FLASH);
    for (int i = 0; i < NewFWBlkSize; i++)
    {
        flash_erase_sector(&flash_ota, NewFWAddr + i * 4096);
    }
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
}

static int partition_write_ota_farmware(int dst_offset, const void *src, int size)
{
    unsigned char ota_header[32];
    static uint32_t NewFWAddr = 0;
    uint32_t ota_target_index;
    flash_t flash_ota;

    if (0 == dst_offset)
    {
        if (size < sizeof(ota_header) + 8)
        {
            aiio_log_e("first buf size %d<%d\r\n", size, sizeof(ota_header) + 8);
            return -1; // 首次数据必须大于32字节
        }
        memset(ota_header, 0, sizeof(ota_header));
        memset(&OtaTargetHdr, 0, sizeof(update_ota_target_hdr));

        //-------------------step2:获取固件头---------------------
        ota_target_index = !ota_get_cur_index();

        memcpy(ota_header, src, sizeof(ota_header));
        OtaTargetHdr.FileHdr.FwVer = ota_header[0] | ota_header[1] << 8 | ota_header[2] << 16 | ota_header[3] << 24;
        OtaTargetHdr.FileHdr.HdrNum = ota_header[4] | ota_header[5] << 8 | ota_header[6] << 16 | ota_header[7] << 24;

        //--------------step3: parse firmware file header and get the target OTA image header------------
        if (!get_ota_tartget_header(ota_header, 32, &OtaTargetHdr, ota_target_index))
        {
            aiio_log_e("get OTA header failed\r\n");
            return -1;
        }

        // 收到的数据偏移过头部32字节
        src += sizeof(ota_header);
        dst_offset += sizeof(ota_header);
        size -= sizeof(ota_header);

        memcpy(OtaTargetHdr.Sign[0], src, 8);
        // 收到的数据偏移OtaTargetHdr.Sign[0]的8字节
        src += 8;
        dst_offset += 8;
        size -= 8;

        NewFWAddr = OtaTargetHdr.FileImgHdr->FlashAddr;
    }
    //-------------------------step5: copy image to flash-------------------------
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    if (flash_burst_write(&flash_ota, NewFWAddr + 8 + (dst_offset - sizeof(ota_header) - 8) - SPI_FLASH_BASE, size, src) < 0)
    {
        device_mutex_unlock(RT_DEV_LOCK_FLASH);
        aiio_log_e("write flash error\r\n");
        return -1;
    }
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    return 0;
}

int aiio_platform_ota_start(void)
{
    if(platform_ota_is_start)
    {
        aiio_log_e("platform ota already start\r\n");
        return AIIO_ERROR;
    }

    erase_flag = 0;
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

    if (!erase_flag)
    {
        partition_erase(0, (data_total_len / 4096 + 1) * 4096);
        erase_flag = 1;
    }

    if (partition_write_ota_farmware(s_fw_len, data, data_len) != 0)
    {
        goto __install_err;
    }

    s_fw_len += data_len;
    if (s_fw_len == data_total_len)
    {
        // 对固件进行校验，校验成功后切换分区
        if (verify_ota_checksum(&OtaTargetHdr))
        {
            if (!change_ota_signature(&OtaTargetHdr, !ota_get_cur_index()))
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

    uint32_t target_idx = ota_get_cur_index() == OTA_INDEX_1 ? OTA_INDEX_2 : OTA_INDEX_1;

    *out_addr = IMG_ADDR[0][target_idx] - SPI_FLASH_BASE;
    return AIIO_OK;
}









