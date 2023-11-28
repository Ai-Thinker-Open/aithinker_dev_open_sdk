
#include "flash_partition_table.h"
#include "hal/hal_flash.h"
#include "netif/ethernetif.h"

#include "ota_port.h"
#include "ota_image.h"
#include "ota_types.h"
// #include "ln_kv_api.h"
#include "ln_nvds.h"

#include "utils/reboot_trace/reboot_trace.h"
#include "utils/debug/ln_assert.h"
#include "utils/debug/log.h"
#include "osal/osal.h"

#include "httpsclient.h"
#include "httpsclient_wrapper.h"
#include "httpsclient_debug.h"

#include "ln_at.h"
#include "usr_ctrl.h"

#define KV_OTA_UPG_STATE           ("kv_ota_upg_state")
#define HTTP_OTA_DEMO_STACK_SIZE   (1024 * 16)

#define SECTOR_SIZE_4KB            (1024 * 4)

struct ota_status_sem g_ota_status_sem = {NULL};

static char g_http_uri_buff[512] = "http://192.168.31.21/flashimage-ota-xz-v1.1.bin";

// a block to save http data.
static char *temp4K_buf    = NULL;
static int   temp4k_offset = 0;

// where to save OTA data in flash.
static int32_t flash_ota_start_addr  = OTA_SPACE_OFFSET;
static int32_t flash_ota_offset      = 0;
static uint8_t is_persistent_started = LN_FALSE;
static uint8_t is_ready_to_verify    = LN_FALSE;
static uint8_t is_precheck_ok        = LN_FALSE;
static OS_Thread_t g_httpota_thread;
static uint8_t httpc_ota_started = 0;

static ln_at_err_t ln_at_set_http_ota(uint8_t para_num, const char *name);
static ln_at_err_t ln_at_help_http_ota(const char *name);

/**
 * @brief Pre-check the image file to be downloaded.
 *
 * @attention None
 *
 * @param[in]  app_offset  The offset of the APP partition in Flash.
 * @param[in]  ota_hdr     pointer to ota partition info struct.
 *
 * @return  whether the check is successful.
 * @retval  #LN_TRUE     successful.
 * @retval  #LN_FALSE    failed.
 */
static int ota_download_precheck(uint32_t app_offset, image_hdr_t * ota_hdr)
{
    image_hdr_t *app_hdr = NULL;
    if (NULL == (app_hdr = OS_Malloc(sizeof(image_hdr_t)))) {
        LOG(LOG_LVL_ERROR, "[%s:%d] malloc failed.\r\n", __func__, __LINE__);
        return LN_FALSE;
    }

    if ( OTA_ERR_NONE != image_header_fast_read(app_offset, app_hdr) ) {
        LOG(LOG_LVL_ERROR, "failed to read app header.\r\n");
        goto ret_err;
    }

    if ((ota_hdr->image_type == IMAGE_TYPE_ORIGINAL) || \
        (ota_hdr->image_type == IMAGE_TYPE_ORIGINAL_XZ)) 
    {
        // check version
        if (((ota_hdr->ver.ver_major << 8) + ota_hdr->ver.ver_minor) == \
            ((app_hdr->ver.ver_major << 8) + app_hdr->ver.ver_minor)) {
            LOG(LOG_LVL_ERROR, "[%s:%d] same version, do not upgrade!\r\n",
                    __func__, __LINE__);
            goto ret_err;
        }

        // check file size
        if (((ota_hdr->img_size_orig + sizeof(image_hdr_t)) > APP_SPACE_SIZE) || \
            ((ota_hdr->img_size_orig_xz + sizeof(image_hdr_t)) > OTA_SPACE_SIZE)) {
            LOG(LOG_LVL_ERROR, "[%s:%d] size check failed.\r\n", __func__, __LINE__);
            goto ret_err;
        }
    } else {
        //image type not support!
        goto ret_err;
    }

    OS_Free(app_hdr);
    return LN_TRUE;
    
    ret_err:
    OS_Free(app_hdr);
    return LN_FALSE;
}

static int ota_persistent_start(void)
{
    if ( NULL == temp4K_buf ) {
        temp4K_buf = OS_Malloc(SECTOR_SIZE_4KB);
        if ( NULL == temp4K_buf) {
            LOG_EMSG("failed to alloc 4KB!!!\r\n");
            return LN_FALSE;
        }
        memset(temp4K_buf, 0, SECTOR_SIZE_4KB);
    }

    temp4k_offset = 0;
    flash_ota_start_addr = OTA_SPACE_OFFSET;
    flash_ota_offset = 0;
    is_persistent_started = LN_TRUE;
    return LN_TRUE;
}

/**
 * @brief Save block to flash.
 *
 * @param buf
 * @param buf_len
 * @return return LN_TRUE on success, LN_FALSE on failure.
 */
static int ota_persistent_write(const char *buf, const int32_t buf_len)
{
    int part_len = 0; // [0, 1, 2, ..., 4K-1], 0, 1, 2, ..., (part_len-1)

    if (!is_persistent_started) {
        return LN_TRUE;
    }

    if (temp4k_offset + buf_len < SECTOR_SIZE_4KB) {
        // just copy all buf data to temp4K_buf
        memcpy(temp4K_buf + temp4k_offset, buf, buf_len);
        temp4k_offset += buf_len;
        part_len = 0;
    } else {
        // just copy part of buf to temp4K_buf
        part_len = temp4k_offset + buf_len - SECTOR_SIZE_4KB;
        memcpy(temp4K_buf + temp4k_offset, buf, buf_len - part_len);
        temp4k_offset += buf_len - part_len;
    }

    if (temp4k_offset >= (SECTOR_SIZE_4KB - 1) ) {
        // write to flash
        LOG(LOG_LVL_INFO, "write at flash: 0x%08x\r\n", flash_ota_start_addr + flash_ota_offset);
        
        if(flash_ota_offset == 0) {
            if (LN_TRUE != ota_download_precheck(APP_SPACE_OFFSET, (image_hdr_t *) temp4K_buf)) {
                LOG(LOG_LVL_INFO, "ota download precheck failed!\r\n");
                is_precheck_ok = LN_FALSE;
                return LN_FALSE;
            }
            is_precheck_ok = LN_TRUE;
        }
        
        hal_flash_erase(flash_ota_start_addr + flash_ota_offset, SECTOR_SIZE_4KB);
        hal_flash_program(flash_ota_start_addr + flash_ota_offset, SECTOR_SIZE_4KB, (uint8_t *)temp4K_buf);

        flash_ota_offset += SECTOR_SIZE_4KB;
        memset(temp4K_buf, 0, SECTOR_SIZE_4KB);
        temp4k_offset = 0;
    }

    if (part_len > 0) {
        memcpy(temp4K_buf + temp4k_offset, buf + (buf_len - part_len), part_len);
        temp4k_offset += part_len;
    }

    return LN_TRUE;
}

/**
 * @brief save last block and clear flags.
 * @return return LN_TRUE on success, LN_FALSE on failure.
 */
static int ota_persistent_finish(void)
{
    if (!is_persistent_started) {
        return LN_FALSE;
    }

    // write to flash
    LOG(LOG_LVL_INFO, "write at flash: 0x%08x\r\n", flash_ota_start_addr + flash_ota_offset);
    hal_flash_erase(flash_ota_start_addr + flash_ota_offset, SECTOR_SIZE_4KB);
    hal_flash_program(flash_ota_start_addr + flash_ota_offset, SECTOR_SIZE_4KB, (uint8_t *)temp4K_buf);

    OS_Free(temp4K_buf);
    temp4K_buf = NULL;
    temp4k_offset = 0;

    flash_ota_offset = 0;
    is_persistent_started = LN_FALSE;
    return LN_TRUE;
}

static int http_get_callback(const char *recvbuf, int32_t recvbuf_len, int32_t total_bytes, int8_t is_recv_finished)
{
    if (!is_recv_finished) {
        if (LN_TRUE != ota_persistent_write(recvbuf, recvbuf_len)) {
            return -1;
        }
    } else {
        ota_persistent_finish();
        is_ready_to_verify = LN_TRUE;
        LOG_I("cb info: recv %d finished, no more data to deal with.\r\n", total_bytes);
    }
    return recvbuf_len;
}

/**
 * @brief check ota image header, body.
 * @return return LN_TRUE on success, LN_FALSE on failure.
 */
static int ota_verify_download(void)
{
    image_hdr_t ota_header;

    if ( OTA_ERR_NONE != image_header_fast_read(OTA_SPACE_OFFSET, &ota_header) ) {
        LOG(LOG_LVL_ERROR, "failed to read ota header.\r\n");
        return LN_FALSE;
    }

    if ( OTA_ERR_NONE != image_header_verify(&ota_header) ) {
        LOG(LOG_LVL_ERROR, "failed to verify ota header.\r\n");
        return LN_FALSE;
    }

    if ( OTA_ERR_NONE != image_body_verify(OTA_SPACE_OFFSET, &ota_header)) {
        LOG(LOG_LVL_ERROR, "failed to verify ota body.\r\n");
        return LN_FALSE;
    }

    LOG(LOG_LVL_INFO, "Succeed to verify OTA image content.\r\n");
    return LN_TRUE;
}

static int update_ota_state(void)
{
    upg_state_t state = UPG_STATE_DOWNLOAD_OK;
    ln_nvds_set_ota_upg_state(state);
    return LN_TRUE;
}

static void http_ota_entry(void *params)
{
    int ret_code = 0;
    HTTP_INFO *http_info = NULL;
    char *url_str = NULL;
    int recv_total = 0;
    ota_port_ctx_t * ota_port = NULL;
    
    ota_port = ota_get_port_ctx();

    while(!netdev_got_ip()){
        OS_MsDelay(500);
    }
    
    LOG_IMSG("http start.!!!\r\n");
    while(1) {

        if (NULL == (http_info = httpinfo_new())) {
            LOG_EMSG("malloc for @http_info failed.\r\n");
            ret_code = -1;
            goto flag_exit;
        }

        if (0 != http_init(&http_info, FALSE)) {
            LOG_EMSG("http init failed.!!!\r\n");
            ret_code = -1;
            goto flag_err;
        }

        url_str = g_http_uri_buff;

        if (LN_TRUE == ota_persistent_start()) {
            recv_total = http_get_with_callback(http_info, url_str, http_get_callback);
        }

        if (recv_total > 0) {
            LOG_I("total received %d bytes.\r\n", recv_total);
            ret_code = 0;
        } 
        else if((recv_total < 0) && (is_precheck_ok != LN_TRUE)) {
            ret_code = -2;
        } 
        else {
            ret_code = -1;
            LOG_EMSG("http_get error!!!\r\n");
        }

        http_close(http_info);

flag_err:
        httpinfo_delete(http_info);

flag_exit:

        OS_SecDelay(5);
        if (ret_code != 0) {
            if (ret_code == -2) {
                break;
            } else {
                // retry if something is wrong.
                LOG_EMSG("http client is restart...\r\n");
            }
        } else {
            // just download once if everything is ok.
            break;
        }
    }

    LOG_IMSG("http client job done, exit...\r\n");
    if (LN_TRUE == is_precheck_ok)
    {
        if ( (LN_TRUE == is_ready_to_verify) && (LN_TRUE == ota_verify_download()) ) {
            update_ota_state();
            //update done
            OS_SemaphoreRelease(&g_ota_status_sem.ota_update_done_sem);
            //return at cmd
            while(1);
            ln_chip_reboot();
        }
    }

    httpc_ota_started = 0;
    OS_ThreadDelete(NULL);
}

static ln_at_err_t ln_at_help_http_ota(const char *name)
{
    ln_at_printf("%s=<uri>\r\n", name);
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_at_set_http_ota(uint8_t para_num, const char *name)
{
    uint16_t len = 0;
    uint8_t para_index = 1;
    bool is_default = false;
    char *in_uri = NULL;

    if (httpc_ota_started == 1)
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        ln_at_printf("%s The ota task has been started!\r\n", name);
        return LN_AT_ERR_COMMON;
    }
    httpc_ota_started = 1;

    LOG(LOG_LVL_INFO, "name:%s, pnum:%d\r\n", name, para_num);

    if (!name || (para_num > 2))
    {
        return LN_AT_ERR_PARAM;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &in_uri))
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_PARSE;
    }

    if (in_uri == NULL) {
        goto __exit;
    }

    memset(g_http_uri_buff, 0x0, sizeof(g_http_uri_buff));

    len = strlen(in_uri);
    if (len >= sizeof(g_http_uri_buff))
    {
        goto __exit;
    }

    LOG(LOG_LVL_INFO, "Input http url:%s; len:%d\r\n", in_uri, len);

    strncpy(g_http_uri_buff, in_uri, len);

    if (OS_OK != OS_ThreadCreate(&g_httpota_thread, "http_ota", http_ota_entry, NULL, OS_PRIORITY_NORMAL, HTTP_OTA_DEMO_STACK_SIZE))
    {
        goto __exit;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    httpc_ota_started = 0;
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

static ln_at_err_t ln_at_exec_http_ota(const char *name)
{
    uint16_t len = 0;
    uint8_t para_index = 1;
    bool is_default = false;
    char *in_uri = NULL;

    if (httpc_ota_started == 1)
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        ln_at_printf("%s The ota task has been started!\r\n", name);
        return LN_AT_ERR_COMMON;
    }
    httpc_ota_started = 1;
    
    //init ota status sem
    memset(&(g_ota_status_sem), 0, sizeof(struct ota_status_sem));
    if (OS_SemaphoreCreateBinary(&g_ota_status_sem.ota_find_server_sem) != OS_OK) {
        LOG(LOG_LVL_ERROR, "OS_SemaphoreCreateBinary ota_status_sem.\r\n");
        goto __exit;
    }
    if (OS_SemaphoreCreateBinary(&g_ota_status_sem.ota_connect_server_sem) != OS_OK) {
        LOG(LOG_LVL_ERROR, "OS_SemaphoreCreateBinary ota_connect_server_sem.\r\n");
        goto __exit;
    }
    if (OS_SemaphoreCreateBinary(&g_ota_status_sem.ota_get_version_sem) != OS_OK) {
        LOG(LOG_LVL_ERROR, "OS_SemaphoreCreateBinary ota_get_version_sem.\r\n");
        goto __exit;
    }
    if (OS_SemaphoreCreateBinary(&g_ota_status_sem.ota_update_done_sem) != OS_OK) {
        LOG(LOG_LVL_ERROR, "OS_SemaphoreCreateBinary ota_update_done_sem.\r\n");
        goto __exit;
    }

    if (OS_OK != OS_ThreadCreate(&g_httpota_thread, "http_ota", http_ota_entry, NULL, OS_PRIORITY_NORMAL, HTTP_OTA_DEMO_STACK_SIZE))
    {
        goto __exit;
    }

    ln_at_printf(LN_AT_RET_OK_STR);

    OS_SemaphoreWait(&g_ota_status_sem.ota_find_server_sem, OS_WAIT_FOREVER);
    ln_at_printf("%s:1\r\n",name);
    OS_SemaphoreWait(&g_ota_status_sem.ota_connect_server_sem, OS_WAIT_FOREVER);
    ln_at_printf("%s:2\r\n",name);
    OS_SemaphoreWait(&g_ota_status_sem.ota_get_version_sem, OS_WAIT_FOREVER);
    ln_at_printf("%s:3\r\n",name);
    OS_SemaphoreWait(&g_ota_status_sem.ota_update_done_sem, OS_WAIT_FOREVER);
    ln_at_printf("%s:4\r\n",name);

    OS_SemaphoreDelete(&g_ota_status_sem.ota_connect_server_sem);
    OS_SemaphoreDelete(&g_ota_status_sem.ota_find_server_sem);
    OS_SemaphoreDelete(&g_ota_status_sem.ota_get_version_sem);
    OS_SemaphoreDelete(&g_ota_status_sem.ota_update_done_sem);
    memset(&g_ota_status_sem, 0, sizeof(struct ota_status_sem));
    ln_chip_reboot();
    return LN_AT_ERR_NONE;

__exit:
    httpc_ota_started = 0;
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}



LN_AT_CMD_REG(CIUPDATE, NULL, NULL, NULL, ln_at_exec_http_ota);
