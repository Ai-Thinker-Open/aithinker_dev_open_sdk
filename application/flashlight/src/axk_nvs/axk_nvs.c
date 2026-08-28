#include "axk_nvs.h"
#include "aiio_adapter_include.h"

static int32_t axk_nvs_init(void);
static int32_t axk_nvs_deinit(void);
static aiio_nvs_err_code_t axk_nvs_erase_key(const char *key);
static aiio_nvs_err_code_t axk_nvs_erase_all(void);
static size_t axk_nvs_get_blob(const char *key, void *value_buf, size_t buf_len, size_t *saved_value_len);
static aiio_nvs_err_code_t axk_nvs_set_blob(const char *key, const void *value_buf, size_t buf_len);

axk_nvs_t g_axk_nvs = 
{
    .init = axk_nvs_init,
    .deinit = axk_nvs_deinit,
    .erase_key = axk_nvs_erase_key,
    .erase_all = axk_nvs_erase_all,
    .get_blob = axk_nvs_get_blob,
    .set_blob = axk_nvs_set_blob,
};

static int32_t axk_nvs_init(void)
{
    return aiio_nvs_init();
}

static int32_t axk_nvs_deinit(void)
{
    return aiio_nvs_deinit();
}

static aiio_nvs_err_code_t axk_nvs_erase_key(const char *key)
{
    return aiio_nvs_erase_key(key);
}

static aiio_nvs_err_code_t axk_nvs_erase_all(void)
{
    return aiio_nvs_erase_all();
}

//If the return value is greater than 0, the read succeeds
static size_t axk_nvs_get_blob(const char *key, void *value_buf, size_t buf_len, size_t *saved_value_len)
{
    return aiio_nvs_get_blob(key, value_buf, buf_len, saved_value_len);
}

static aiio_nvs_err_code_t axk_nvs_set_blob(const char *key, const void *value_buf, size_t buf_len)
{
    return aiio_nvs_set_blob(key, value_buf, buf_len);
}
