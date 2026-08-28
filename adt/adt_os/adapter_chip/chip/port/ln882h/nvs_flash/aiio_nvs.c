/**
 * @brief      NVS application interface.
 *
 * @file       aiio_nvs.c
 * @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       NVS application interface.
 * @par        Change Logs:
 * <table>
 * <tr><th>Date            <th>Version     <th>Author      <th>Notes
 * <tr><td>2023/05/09      <td>1.0.0       <td>wusen       <td>add nvs api
 * </table>
 *
 */

#include <stdio.h>
#include "ln_kv_api.h"
#include "flash_partition_table.h"
#include "aiio_nvs.h"

int32_t aiio_nvs_init(void)
{
    if (KV_ERR_NONE != ln_kv_port_init(KV_SPACE_OFFSET, (KV_SPACE_OFFSET + KV_SPACE_SIZE))) {
        aiio_log_e("KV init filed!\r\n");
        return AIIO_ERROR;
    }
    return AIIO_OK;
}

int32_t aiio_nvs_deinit(void)
{
    ln_kv_deinit();
    return AIIO_OK;
}
aiio_nvs_err_code_t aiio_nvs_erase_key(const char *key)
{
    kv_err_t ln_kv_err_code=KV_ERR_NONE;
    ln_kv_err_code =ln_kv_del(key);
    if(ln_kv_err_code==KV_ERR_NONE){
        return AIIO_EF_NO_ERR;
    }else if(ln_kv_err_code==KV_ERR_FLASH_ERASE_FAILED){
        return AIIO_EF_ERASE_ERR;
    }else if(ln_kv_err_code==KV_ERR_FLASH_READ_FAILED){
        return AIIO_EF_READ_ERR;
    }else if(ln_kv_err_code==KV_ERR_FLASH_WRITE_FAILED){
        return AIIO_EF_WRITE_ERR;
    }else{
        return AIIO_EF_ENV_ARG_ERR;
    }
}

aiio_nvs_err_code_t aiio_nvs_erase_all(void)
{
    aiio_log_e(" nonsupport please use aiio_nvs_erase_key");
    return AIIO_ERROR;
}

size_t aiio_nvs_get_blob(const char *key, void *value_buf, size_t buf_len, size_t *saved_value_len)
{
    
    kv_err_t ln_kv_err_code=KV_ERR_NONE;
    ln_kv_err_code =ln_kv_get(key,value_buf,buf_len,saved_value_len);
    
    return *saved_value_len;
}

aiio_nvs_err_code_t aiio_nvs_set_blob(const char *key, const void *value_buf, size_t buf_len)
{
    kv_err_t ln_kv_err_code=KV_ERR_NONE;
    ln_kv_err_code =ln_kv_set(key,value_buf,buf_len);
    if(ln_kv_err_code==KV_ERR_NONE){
        return AIIO_EF_NO_ERR;
    }else if(ln_kv_err_code==KV_ERR_FLASH_ERASE_FAILED){
        return AIIO_EF_ERASE_ERR;
    }else if(ln_kv_err_code==KV_ERR_FLASH_READ_FAILED){
        return AIIO_EF_READ_ERR;
    }else if(ln_kv_err_code==KV_ERR_FLASH_WRITE_FAILED){
        return AIIO_EF_WRITE_ERR;
    }else{
        return AIIO_EF_ENV_ARG_ERR;
    }
}

