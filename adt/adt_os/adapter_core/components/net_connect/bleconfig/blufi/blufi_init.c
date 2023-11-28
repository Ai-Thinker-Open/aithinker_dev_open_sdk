/** @brief      blufi btc.
 *
 *  @file       blufi_init.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/05/18      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#include "aiio_blufi_api.h"
#include "aiio_blufi.h"
#include "aiio_blufi_export_import.h"
#include "aiio_log.h"

int32_t aiio_blufi_host_init(void)
{
    aiio_hal_blufi_server_create_start();
    return AIIO_OK;
}


int32_t aiio_blufi_gap_register_callback(void)
{
    return aiio_blufi_profile_init();
}

int32_t aiio_blufi_host_and_cb_init(aiio_blufi_callbacks_t *example_callbacks)
{
    int32_t ret = 0;

    ret = aiio_blufi_register_callbacks(example_callbacks);
    if (ret)
    {
        aiio_log_e("[BLUFI] %s blufi register failed, error code = %x\n", __func__, ret);
        return ret;
    }

    ret = aiio_blufi_host_init();
    if (ret)
    {
        aiio_log_e("[BLUFI] %s initialise host failed: %d\n", __func__, ret);
        return ret;
    }

    ret = aiio_blufi_gap_register_callback();
    if (ret)
    {
        aiio_log_e("[BLUFI] %s gap register failed, error code = %x\n", __func__, ret);
        return ret;
    }

    return ret;
}