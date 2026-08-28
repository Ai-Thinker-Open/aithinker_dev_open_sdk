/**
 * @brief      NVS application interface.
 *
 * @file       aiio_nvs.c
 * @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       NVS application interface.
 * @par        Change Logs:
 * <table>
 * <tr><th>Date            <th>Version     <th>Author      <th>Notes
 * <tr><td>2024/03/26      <td>1.0.0       <td>hongjz      <td>add nvs api
 * </table>
 *
 */

#include <stdio.h>

#include "littlefs_adapter.h"
#include "kv.h"

#include "aiio_nvs.h"

//LFS_FLASH_BASE_ADDR 为起始地址: 0x3F0000(可用64k)
#define AIIO_LFS_SIZE       (1024 * 64)
#define AIIO_LOOKHEAD_SIZE  (8)

int32_t aiio_nvs_init (void)
{
    g_lfs_cfg.block_count = AIIO_LFS_SIZE / 4096;
    g_lfs_cfg.lookahead_size = AIIO_LOOKHEAD_SIZE;
    rt_kv_init();

    return AIIO_OK;
}

int32_t aiio_nvs_deinit(void)
{
    return AIIO_OK;
}

aiio_nvs_err_code_t aiio_nvs_erase_key(const char *key)
{
    if (rt_kv_delete(key) != 0)
    {
        return AIIO_EF_ERASE_ERR;
    }

    return AIIO_EF_NO_ERR;
}

aiio_nvs_err_code_t aiio_nvs_erase_all(void)
{
    aiio_log_e("This operation is not supported!");
    return AIIO_EF_ERASE_ERR;
}

size_t aiio_nvs_get_blob(const char *key, void *value_buf, size_t buf_len, size_t *saved_value_len)
{
    int32_t read_len = rt_kv_get(key, value_buf, buf_len);
    if (read_len != buf_len)
    {
        return 0;
    }

    *saved_value_len = read_len;

    return read_len;
}

aiio_nvs_err_code_t aiio_nvs_set_blob(const char *key, const void *value_buf, size_t buf_len)
{
    if (rt_kv_set(key, value_buf, buf_len) != buf_len)
    {
        return AIIO_EF_WRITE_ERR;
    }

    return AIIO_EF_NO_ERR;
}

