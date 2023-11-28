/**
 * @file     bootram_file_mode.c
 * @author   BSP Team
 * @brief
 * @version  0.0.0.1
 * @date     2020-12-31
 *
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 *
 */
#include "proj_config.h"
#include "ln882h.h"
#include "mode_ctrl.h"
#include "file_mode.h"

/****************************************  variables  *********************************************/
static uint8_t sg_sector_4k_buffer[SIZE_4KB] = {0};

/****************************************  functions  *********************************************/
/**
 * @brief
 *
 * @param upgrade_ctrl
 * @return int return 0 on success, -1 on failure.
 */
static int flush_to_storage_aligned(upgrade_ctrl_t* ctrlA, int isLastWrite)
{
    uint32_t write_len = ctrlA->cache_buffer_pos;
    bootram_flash_write(ctrlA->flash_write_offset, write_len, ctrlA->cache_buffer_arr);

#if (defined(FLASH_VERIFY_ENABLE) && (1 == FLASH_VERIFY_ENABLE))
    {
        bootram_flash_read(ctrlA->flash_write_offset, write_len, ctrlA->verify_buffer_arr);
        if (memcmp(ctrlA->cache_buffer_arr, ctrlA->verify_buffer_arr, write_len) != 0) {
            // verify failed, try erase/program/verify again.
            bootram_flash_erase(ctrlA->flash_write_offset, CACHE_BUFFER_SIZE);
            bootram_flash_write(ctrlA->flash_write_offset, CACHE_BUFFER_SIZE, ctrlA->cache_buffer_arr);
            bootram_flash_read(ctrlA->flash_write_offset, CACHE_BUFFER_SIZE, ctrlA->verify_buffer_arr);

            if (memcmp(ctrlA->cache_buffer_arr, ctrlA->verify_buffer_arr, write_len) != 0) {
                // verify failed again, no more rescure.
                return -1;
            }
        }
        else {
            // OK
        }
    }
#endif

    ctrlA->flash_write_offset += ctrlA->cache_buffer_pos;
    ctrlA->cache_buffer_pos = 0;

    if (isLastWrite) {
        // write cache_tailing_buf to storage.
        bootram_flash_write(ctrlA->flash_write_offset, ctrlA->cache_tailing_pos, ctrlA->cache_tailing_buf);

#if (defined(FLASH_VERIFY_ENABLE) && (1 == FLASH_VERIFY_ENABLE))
        {
            bootram_flash_read(ctrlA->flash_write_offset, ctrlA->cache_tailing_pos, ctrlA->verify_buffer_arr);
            if (memcmp(ctrlA->cache_tailing_buf, ctrlA->verify_buffer_arr, ctrlA->cache_tailing_pos) != 0) {
                // error.
                return -2;
            }
            else {
                // OK
            }
        }
#endif

        ctrlA->flash_write_offset += ctrlA->cache_tailing_pos;
    }
    else {
        // copy cache_tailing_buf to cache_buffer_arr.
        if (ctrlA->cache_tailing_pos > 0) {
            memcpy(ctrlA->cache_buffer_arr + ctrlA->cache_buffer_pos, ctrlA->cache_tailing_buf, ctrlA->cache_tailing_pos);
            ctrlA->cache_buffer_pos += ctrlA->cache_tailing_pos;
            ctrlA->cache_tailing_pos = 0;
        }
    }

    return 0;
}

static int flash_to_storage_notaligned(upgrade_ctrl_t* ctrlN, int isLastWrite)
{
    uint32_t sector4k_cnt  = ctrlN->flash_write_offset / SIZE_4KB;
    uint32_t sector4k_base = sector4k_cnt * SIZE_4KB;
    uint32_t head_length   = ctrlN->flash_write_offset - sector4k_base;
    uint32_t tail_length   = SIZE_4KB - head_length;

    // [0] flash 地址前一部分没有对齐
    bootram_flash_read(sector4k_base, SIZE_4KB, sg_sector_4k_buffer);
    memcpy(sg_sector_4k_buffer + head_length, ctrlN->cache_buffer_arr, tail_length);

    bootram_flash_erase(sector4k_base, SIZE_4KB);
    bootram_flash_write(sector4k_base, SIZE_4KB, sg_sector_4k_buffer);

#if (defined(FLASH_VERIFY_ENABLE) && (1 == FLASH_VERIFY_ENABLE))
    {
        bootram_flash_read(sector4k_base, SIZE_4KB, ctrlN->verify_buffer_arr);
        if (memcmp(ctrlN->verify_buffer_arr, sg_sector_4k_buffer, SIZE_4KB) != 0) {
            bootram_flash_erase(sector4k_base, SIZE_4KB);
            bootram_flash_write(sector4k_base, SIZE_4KB, sg_sector_4k_buffer);
            bootram_flash_read(sector4k_base, SIZE_4KB, ctrlN->verify_buffer_arr);
            if (memcmp(ctrlN->verify_buffer_arr, sg_sector_4k_buffer, SIZE_4KB) != 0) {
                return -1;
            }
        }
        else {
            // OK
        }
    }
#endif

    // [1] 中间对齐部分
    ctrlN->flash_write_offset = sector4k_base + SIZE_4KB;
    uint8_t* write_arr        = ctrlN->cache_buffer_arr + tail_length;
    uint32_t remain_length    = ctrlN->cache_buffer_pos - tail_length;
    uint32_t write_len_real   = remain_length & (~(SIZE_4KB - 1));  // 只写入 N 个 4KB 扇区
    bootram_flash_write(ctrlN->flash_write_offset, write_len_real, write_arr);

#if (defined(FLASH_VERIFY_ENABLE) && (1 == FLASH_VERIFY_ENABLE))
    {
        bootram_flash_read(ctrlN->flash_write_offset, write_len_real, ctrlN->verify_buffer_arr);
        if (memcmp(ctrlN->verify_buffer_arr, write_arr, write_len_real) != 0) {
            return -1;
        }
        else {
            // OK
        }
    }
#endif

    ctrlN->flash_write_offset += write_len_real;

    // [2] 后面小于 4KB 的尾巴
    const uint32_t copy_start = tail_length + write_len_real;
    const uint32_t copy_len   = ctrlN->cache_buffer_pos - copy_start;
    if (!isLastWrite) {
        // 拷贝到缓冲区头部
        memcpy(ctrlN->cache_buffer_arr, ctrlN->cache_buffer_arr + copy_start, copy_len);
        ctrlN->cache_buffer_pos = copy_len;

        if (ctrlN->cache_tailing_pos > 0) {
            memcpy(ctrlN->cache_buffer_arr + ctrlN->cache_buffer_pos, ctrlN->cache_tailing_buf, ctrlN->cache_tailing_pos);
            ctrlN->cache_buffer_pos += ctrlN->cache_tailing_pos;

            memset(ctrlN->cache_tailing_buf, 0, ctrlN->cache_tailing_pos);
            ctrlN->cache_tailing_pos = 0;
        }
    }
    else {
        // 写入到 flash
        bootram_flash_write(ctrlN->flash_write_offset, copy_len, ctrlN->cache_buffer_arr + copy_start);

#if (defined(FLASH_VERIFY_ENABLE) && (1 == FLASH_VERIFY_ENABLE))
        {
            bootram_flash_read(ctrlN->flash_write_offset, copy_len, ctrlN->verify_buffer_arr);
            if (memcmp(ctrlN->verify_buffer_arr, ctrlN->cache_buffer_arr + copy_start, copy_len) !=
                0) {
                // error
                return -2;
            }
            else {
                // OK
            }
        }
#endif

        ctrlN->flash_write_offset += copy_len;
    }

    return 0;
}

/**
 * @brief
 *
 * @param mblock
 * @return uint32_t return -1 on error, other value on success.
 */
uint32_t bootram_save_block_data_to_flash(ymodem_t* mblock)
{
    upgrade_ctrl_t*      upgrade_ctrl = NULL;
    bootram_file_ctrl_t* file_ctrl    = bootram_file_get_handle();
    int                  ret          = -1;

    if (mblock == NULL) {
        return ret;
    }

    upgrade_ctrl = &file_ctrl->upgrade_ctrl;
    if (upgrade_ctrl->flash_write_offset >= FLASH_MAX_SIZE_ON_CHIP) {
        return ret;
    }

    if (mblock->valid_data_len == 0) {  // block data length check (not likely meet this condition)
        return mblock->valid_data_len;
    }

    if (upgrade_ctrl->cache_buffer_pos + mblock->valid_data_len < CACHE_BUFFER_SIZE) {
        // 当前 ymodem block 数据长度 **还没有** 超过缓冲区剩余长度
        memcpy(upgrade_ctrl->cache_buffer_arr + upgrade_ctrl->cache_buffer_pos, mblock->data_buf, mblock->valid_data_len);
        upgrade_ctrl->cache_buffer_pos += mblock->valid_data_len;
    }
    else {  // 当前 ymodem block 数据长度 **超过了** 缓冲区剩余长度
        uint32_t copy_len = CACHE_BUFFER_SIZE - upgrade_ctrl->cache_buffer_pos;
        memcpy(upgrade_ctrl->cache_buffer_arr + upgrade_ctrl->cache_buffer_pos, mblock->data_buf, copy_len);
        upgrade_ctrl->cache_buffer_pos += copy_len;

        // 把 ymodem block 中剩余的数据拷贝到 cache_tailing_buf 中
        uint32_t data_start = copy_len;
        copy_len            = mblock->valid_data_len - copy_len;
        memcpy(upgrade_ctrl->cache_tailing_buf + upgrade_ctrl->cache_tailing_pos, mblock->data_buf + data_start, copy_len);
        upgrade_ctrl->cache_tailing_pos += copy_len;
    }

    if (mblock->rev_len < mblock->filelen) {  // file receive not complete
        if (upgrade_ctrl->cache_buffer_pos >=
            CACHE_BUFFER_SIZE) {  // buffer is full, write to flash.
            if (upgrade_ctrl->flash_write_offset % SIZE_4KB == 0) {  // aligned.
                ret = flush_to_storage_aligned(upgrade_ctrl, 0);
                if (0 != ret) {
                    return ret;
                }
            }
            else {  // not aligned.
                ret = flash_to_storage_notaligned(upgrade_ctrl, 0);
                if (0 != ret) {
                    return ret;
                }
            }
        }
    }
    else {                                                       // file receive complete
        if (upgrade_ctrl->flash_write_offset % SIZE_4KB == 0) {  // aligned.
            ret = flush_to_storage_aligned(upgrade_ctrl, 1);
            if (0 != ret) {
                return ret;
            }
        }
        else {  // not aligned.
            ret = flash_to_storage_notaligned(upgrade_ctrl, 1);
            if (0 != ret) {
                return ret;
            }
        }
    }

    return mblock->valid_data_len;
}

int bootram_enter_file_mode(void)
{
    bootram_file_ctrl_t* file_ctrl    = bootram_file_get_handle();
    upgrade_ctrl_t*      upgrade_ctrl = &(file_ctrl->upgrade_ctrl);
    ymodem_t*            ymodem_ctrl  = &(file_ctrl->ymodem_ctrl);

    // upgrade_ctrl init
    upgrade_ctrl->cache_buffer_pos = 0;

    // ymodem_ctrl init
    memset(ymodem_ctrl, 0, sizeof(ymodem_t));

    if (ymodem_recieve_loop(ymodem_ctrl, bootram_save_block_data_to_flash) == YMODEM_SUCCESS) {
        upgrade_ctrl->file_count++;
    }

    return 0;
}
