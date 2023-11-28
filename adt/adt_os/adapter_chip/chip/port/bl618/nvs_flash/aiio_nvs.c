/**
 * @brief      NVS application interface.
 *
 * @file       aiio_nvs.c
 * @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       NVS application interface.
 * @par        Change Logs:
 * <table>
 * <tr><th>Date            <th>Version     <th>Author      <th>Notes
 * <tr><td>2023/06/27      <td>1.0.0       <td>wusen       <td>add nvs api
 * </table>
 *
 */
#if (CONFIG_EASYFLASH4 && CONFIG_BFLB_MTD && CONFIG_PARTITION)
#include <stdio.h>
#include "bflb_mtd.h"
#include <easyflash.h>

#include "aiio_nvs.h"

int32_t aiio_nvs_init (void)
{
    bflb_mtd_init();
    easyflash_init();
    return AIIO_OK;
}

int32_t aiio_nvs_deinit(void)
{
    return AIIO_OK;
}
aiio_nvs_err_code_t aiio_nvs_erase_key(const char *key)
{
    return ef_del_env(key);
}

aiio_nvs_err_code_t aiio_nvs_erase_all(void)
{
    return ef_env_set_default();
}

size_t aiio_nvs_get_blob(const char *key, void *value_buf, size_t buf_len, size_t *saved_value_len)
{
    return ef_get_env_blob(key,value_buf,buf_len,saved_value_len);
}

aiio_nvs_err_code_t aiio_nvs_set_blob(const char *key, const void *value_buf, size_t buf_len)
{
    return ef_set_env_blob(key,value_buf,buf_len);
}

#endif