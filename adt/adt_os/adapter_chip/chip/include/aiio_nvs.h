/**
 * @brief      NVS application interface.
 *
 * @file       aiio_nvs.h
 * @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       NVS application interface.
 * @par        Change Logs:
 * <table>
 * <tr><th>Date            <th>Version     <th>Author      <th>Notes
 * <tr><td>2023/02/19      <td>1.0.0       <td>yanch       <td>Define NVS API
 * <tr><td>2023/04/24      <td>1.0.1       <td>wusen       <td>add nvs errorcode & get/set env_blob
 * </table>
 *
 */

#ifndef __AIIO_NVS_H__
#define __AIIO_NVS_H__

#include "aiio_type.h"
#include "stdio.h"
#include "stdlib.h"
#include "aiio_chip_spec.h"
#include "aiio_os_port.h"
#include "aiio_error.h"
#include "aiio_log.h"
#include "aiio_autoconf.h"
/**
 * Opaque pointer type representing non-volatile storage handle
 */
typedef uint32_t nvs_handle_t;

#define AIIO_EF_STR_ENV_VALUE_MAX_SIZE  (3979)

typedef enum {
    AIIO_EF_NO_ERR,
    AIIO_EF_ERASE_ERR,
    AIIO_EF_READ_ERR,
    AIIO_EF_WRITE_ERR,
    AIIO_EF_ENV_NAME_ERR,
    AIIO_EF_ENV_NAME_EXIST,
    AIIO_EF_ENV_FULL,
    AIIO_EF_ENV_INIT_FAILED,
    AIIO_EF_ENV_ARG_ERR,
}aiio_nvs_err_code_t;

CHIP_API int32_t aiio_nvs_init(void);

CHIP_API int32_t aiio_nvs_deinit(void);

CHIP_API aiio_nvs_err_code_t aiio_nvs_erase_key(const char *key);

CHIP_API aiio_nvs_err_code_t aiio_nvs_erase_all(void);

CHIP_API void aiio_nvs_dump(void);

CHIP_API size_t aiio_nvs_get_blob(const char *key, void *value_buf, size_t buf_len, size_t *saved_value_len);

CHIP_API aiio_nvs_err_code_t aiio_nvs_set_blob(const char *key, const void *value_buf, size_t buf_len);

#endif // !__AIIO_NVS_H__
