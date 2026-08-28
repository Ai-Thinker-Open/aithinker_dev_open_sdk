 //SDK相关
#include "ameba_soc.h"
#include "os_wrapper.h"
#include "kv.h"
#include "vfs.h"

//中间件相关
#include "aiio_log.h"
#include "aiio_nvs.h"

int32_t aiio_nvs_init (void)
{
    int ret = 0;
	vfs_init();
    //空间使用的是VFS1，地址在 component/soc/amebadplus/usrcfg/ameba_flashcfg.c 中定义的，已经修改为160k
	ret = vfs_user_register(VFS_PREFIX, VFS_LITTLEFS, VFS_INF_FLASH, VFS_REGION_1, VFS_RW);
	if (ret) {
		aiio_log_e("File System Init Fail");
        return AIIO_ERROR;
	}
    ret=vfs_kv_init();
    if(ret<0){
        aiio_log_e("File System Init Fail");
        return AIIO_ERROR;
    }
    aiio_log_i("File System Init success");

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
