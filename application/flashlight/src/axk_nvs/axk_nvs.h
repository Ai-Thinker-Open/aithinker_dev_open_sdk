#ifndef _AXK_NVS_H_
#define _AXK_NVS_H_

#include <stdint.h>
#include "aiio_adapter_include.h"

typedef struct axk_nvs
{
    int32_t (*init)(void);
    int32_t (*deinit)(void);
    aiio_nvs_err_code_t (*erase_key)(const char *key);
    aiio_nvs_err_code_t (*erase_all)(void);
    size_t (*get_blob)(const char *key, void *value_buf, size_t buf_len, size_t *saved_value_len);
    aiio_nvs_err_code_t (*set_blob)(const char *key, const void *value_buf, size_t buf_len);
} axk_nvs_t;

extern axk_nvs_t g_axk_nvs;

#endif //_AXK_NVS_H_
