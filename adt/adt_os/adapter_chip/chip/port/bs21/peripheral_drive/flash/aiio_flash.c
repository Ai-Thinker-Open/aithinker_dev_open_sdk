/**
 * @brief      flash application interface.
 *
 * @file       aiio_flash.c
 * @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       flash application interface.
 * @par        Change Logs:
 * <table>
 * <tr><th>Date            <th>Version     <th>Author      <th>Notes
 * <tr><td>2023/06/27      <td>1.0.0       <td>zhuolm      <td>Define flash API
 * </table>
 *
 */

#include "aiio_type.h"
#include "aiio_error.h"
#include "aiio_log.h"
#include "soc_osal.h"
#include "securec.h"
#include "sfc.h"
#include "sfc_porting.h"
#include "app_init.h"
#include "memory_config_common.h"

int32_t aiio_flash_init(void)
{

    return AIIO_OK;
}

int32_t aiio_flash_deinit(void)
{

    return AIIO_OK;
}

int32_t aiio_flash_write(uint32_t write_addr, const char *write_data, uint16_t write_length)
{
    /* Erase User space */
    errcode_t ret = uapi_sfc_reg_write(write_addr,(uint8_t *)write_data, write_length);
    if (ret != ERRCODE_SUCC) {
        aiio_log_w("flash write failed! ret = %x\r\n", ret);
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_flash_read(uint32_t read_addr, uint8_t *data, uint16_t read_length)
{
    /* Erase User space */
    errcode_t ret = uapi_sfc_reg_read(read_addr,(uint8_t *)data, read_length);
    if (ret != ERRCODE_SUCC) {
        aiio_log_w("flash read failed! ret = %x\r\n", ret);
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_flash_erase(uint32_t erase_addr, uint16_t erase_length)
{
    /* Erase User space */
    errcode_t ret = uapi_sfc_reg_erase(erase_addr, erase_length);
    if (ret != ERRCODE_SUCC) {
        aiio_log_w("flash erase failed! ret = %x\r\n", ret);
        return AIIO_ERROR;
    }

    return AIIO_OK;
}
