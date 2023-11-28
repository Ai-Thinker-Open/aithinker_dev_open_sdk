/*
 * Tencent is pleased to support the open source community by making IoT Hub
 available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file
 except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software
 distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 KIND,
 * either express or implied. See the License for the specific language
 governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lite-utils.h"
#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "utils_getopt.h"
#include "utils_timer.h"

#include "qcloud_wifi_config.h"
#include "qcloud_wifi_config_internal.h"

static bool wifi_config_result_success = false;

#define KV_QC_LOCAL_FW_INFO    ((const char *)"qcloud_ota_fw_info")

#define FW_RUNNING_MCU_VERSION    "mcu_v1.0.0"
#define FW_RUNNING_MODULE_VERSION "module_v1.0.0"

#define KEY_VER  "version"
#define KEY_SIZE "downloaded_size"

#define FW_VERSION_MAX_LEN        32
#define FW_FILE_PATH_MAX_LEN      128
#define OTA_BUF_LEN               (2 * 1024)
#define LOCAL_FW_INFO_LEN         128
#define OTA_HTTP_MAX_FETCHED_SIZE (2 * 1024)

#define MAX_OTA_RETRY_CNT 5

typedef struct _OTAContextData {
    void *ota_handle;
    void *mqtt_client;

    // remote_version means version for the FW in the cloud and to be downloaded
    char          remote_version[FW_VERSION_MAX_LEN];
    uint32_t      fw_file_size;
    IOT_OTAFWType fw_type; /* fw type */

    // for resuming download
    /* local_version means downloading but not running */
    char local_version[FW_VERSION_MAX_LEN];
    int  downloaded_size;

    // to make sure report is acked
    bool     report_pub_ack;
    int      report_packet_id;
    uint32_t ota_fail_cnt;
} OTAContextData;

static DeviceInfo sg_devInfo;

static void _event_handler(void *pclient, void *handle_context, MQTTEventMsg *msg)
{
    uintptr_t       packet_id = (uintptr_t)msg->msg;
    OTAContextData *ota_ctx   = (OTAContextData *)handle_context;

    switch (msg->event_type) {
        case MQTT_EVENT_UNDEF:
            Log_i("undefined event occur.");
            break;

        case MQTT_EVENT_DISCONNECT:
            Log_i("MQTT disconnect.");
            break;

        case MQTT_EVENT_RECONNECT:
            Log_i("MQTT reconnect.");
            break;

        case MQTT_EVENT_SUBCRIBE_SUCCESS:
            Log_i("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_SUBCRIBE_TIMEOUT:
            Log_i("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_SUBCRIBE_NACK:
            Log_i("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_PUBLISH_SUCCESS:
            Log_i("publish success, packet-id=%u", (unsigned int)packet_id);
            if (ota_ctx->report_packet_id == packet_id)
                ota_ctx->report_pub_ack = true;
            break;

        case MQTT_EVENT_PUBLISH_TIMEOUT:
            Log_i("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_PUBLISH_NACK:
            Log_i("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;
        default:
            Log_i("Should NOT arrive here.");
            break;
    }
}

static int _setup_connect_init_params(MQTTInitParams *initParams, void *ota_ctx, DeviceInfo *device_info)
{
    initParams->region      = device_info->region;
    initParams->product_id  = device_info->product_id;
    initParams->device_name = device_info->device_name;

#ifdef AUTH_MODE_CERT
    char  certs_dir[16] = "certs";
    char  current_path[128];
    char *cwd = getcwd(current_path, sizeof(current_path));

    if (cwd == NULL) {
        Log_e("getcwd return NULL");
        return QCLOUD_ERR_FAILURE;
    }

    HAL_Snprintf(initParams->cert_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir, device_info->dev_cert_file_name);
    HAL_Snprintf(initParams->key_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir, device_info->dev_key_file_name);
#else
    initParams->device_secret = device_info->device_secret;
#endif

    initParams->command_timeout        = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
    initParams->keep_alive_interval_ms = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;

    initParams->auto_connect_enable  = 1;
    initParams->event_handle.h_fp    = _event_handler;
    initParams->event_handle.context = ota_ctx;

    return QCLOUD_RET_SUCCESS;
}

static void _wait_for_pub_ack(OTAContextData *ota_ctx, int packet_id)
{
    int wait_cnt              = 10;
    ota_ctx->report_pub_ack   = false;
    ota_ctx->report_packet_id = packet_id;

    while (!ota_ctx->report_pub_ack) {
        HAL_SleepMs(500);
        IOT_MQTT_Yield(ota_ctx->mqtt_client, 500);
        if (wait_cnt-- == 0) {
            Log_e("wait report pub ack timeout!");
            break;
        }
    }
    ota_ctx->report_pub_ack = false;
    return;
}

/**********************************************************************************
 * OTA file operations START
 * these are platform-dependant functions
 * POSIX FILE is used in this sample code
 **********************************************************************************/
#include "flash_partition_table.h"
#include "ln_nvds.h"
#include "ln_kv_api.h"
#include "hal/hal_flash.h"

//#define KV_QC_LOCAL_FW_INFO

//#include "ota_port.h"
//#include "ota_image.h"
//#include "ota_types.h"

int _read_ln882h_fw(void *dst_buf, uint32_t read_size_bytes, uint32_t fetched_bytes, OTAContextData *ota_ctx)
{
    if (fetched_bytes % 4 || read_size_bytes % 4) {
        Log_e("fetched size: %u and read bytes: %u should be word aligned", fetched_bytes, read_size_bytes);
        return -1;
    }

    uint32_t src_addr = OTA_SPACE_OFFSET + fetched_bytes;
    hal_flash_read_by_cache(src_addr, read_size_bytes, dst_buf);

    return 0;
}

// calculate left MD5 for resuming download from break point
static int _cal_exist_fw_md5(OTAContextData *ota_ctx)
{
    size_t rlen = 0, total_read = 0;
    int    ret = QCLOUD_RET_SUCCESS;

    ret = IOT_OTA_ResetClientMD5(ota_ctx->ota_handle);
    if (ret) {
        Log_e("reset MD5 failed: %d", ret);
        return QCLOUD_ERR_FAILURE;
    }

    char *buff = HAL_Malloc(OTA_BUF_LEN);
    if (buff == NULL) {
        Log_e("malloc ota buffer failed");
        return QCLOUD_ERR_MALLOC;
    }

    size_t size = ota_ctx->downloaded_size;

    while (total_read < ota_ctx->downloaded_size)
    {
        rlen = (size > OTA_BUF_LEN) ? OTA_BUF_LEN : size;

        if (0 != _read_ln882h_fw(buff, rlen, total_read, ota_ctx)) {
            Log_e("read data from flash error");
            ret = QCLOUD_ERR_FAILURE;
            break;
        }
        IOT_OTA_UpdateClientMd5(ota_ctx->ota_handle, buff, rlen);
        size -= rlen;
        total_read += rlen;
    }

    HAL_Free(buff);
    Log_d("total read: %d", total_read);
    return ret;
}

/* update local firmware info for resuming download from break point */
static int _update_local_fw_info(OTAContextData *ota_ctx)
{
    char  data_buf[LOCAL_FW_INFO_LEN] = {0};

    HAL_Snprintf(data_buf, sizeof(data_buf), "{\"%s\":\"%s\", \"%s\":%d}", KEY_VER, ota_ctx->remote_version, KEY_SIZE, ota_ctx->downloaded_size);

    int ret = ln_kv_set(KV_QC_LOCAL_FW_INFO, (const void *)data_buf, strlen(data_buf));
    if (ret != KV_ERR_NONE) {
        return QCLOUD_ERR_FAILURE;
    }

    return QCLOUD_RET_SUCCESS;
}

static int _get_local_fw_info(char *local_version)
{
    char json_doc[LOCAL_FW_INFO_LEN] = {0};
    size_t value_len = 0;

    int ret = ln_kv_get(KV_QC_LOCAL_FW_INFO, json_doc, sizeof(json_doc), &value_len);
    if (ret != KV_ERR_NONE) {
        return 0;
    }

    char *version = LITE_json_value_of(KEY_VER, json_doc);
    char *size    = LITE_json_value_of(KEY_SIZE, json_doc);

    if ((NULL == version) || (NULL == size)) {
        if (version)
            HAL_Free(version);
        if (size)
            HAL_Free(size);
        return 0;
    }

    int local_size = atoi(size);
    HAL_Free(size);

    if (local_size <= 0) {
        Log_w("local info offset invalid: %d", local_size);
        local_size = 0;
    } else {
        strncpy(local_version, version, FW_VERSION_MAX_LEN);
    }
    HAL_Free(version);

    return local_size;
}

/* get local firmware offset for resuming download from break point */
static int _update_fw_downloaded_size(OTAContextData *ota_ctx)
{
    int local_size = _get_local_fw_info(ota_ctx->local_version);
    if (local_size == 0) {
        ota_ctx->downloaded_size = 0;
        return 0;
    }

    if ((0 != strcmp(ota_ctx->local_version, ota_ctx->remote_version)) ||
        (ota_ctx->downloaded_size > ota_ctx->fw_file_size)) {
        ota_ctx->downloaded_size = 0;
        return 0;
    }

    ota_ctx->downloaded_size = local_size;
    Log_i("calc MD5 for resuming download from offset: %d", ota_ctx->downloaded_size);

    int ret = _cal_exist_fw_md5(ota_ctx);
    if (ret) {
        Log_e("regen OTA MD5 error: %d", ret);
        ota_ctx->downloaded_size = 0;
        return 0;
    }

    Log_d("local MD5 update done!");
    return local_size;
}

static int _save_fw_data_to_file(uint32_t offset, char *buf, int len)
{
    // void *fp;
    // if (offset > 0) {
    //     if (NULL == (fp = HAL_FileOpen(file_name, "ab+"))) {
    //         Log_e("open file failed");
    //         return QCLOUD_ERR_FAILURE;
    //     }
    // } else {
    //     if (NULL == (fp = HAL_FileOpen(file_name, "wb+"))) {
    //         Log_e("open file failed");
    //         return QCLOUD_ERR_FAILURE;
    //     }
    // }

    // HAL_FileSeek(fp, offset, SEEK_SET);

    // if (1 != HAL_FileWrite(buf, len, 1, fp)) {
    //     Log_e("write data to file failed");
    //     HAL_FileClose(fp);
    //     return QCLOUD_ERR_FAILURE;
    // }
    // HAL_FileFlush(fp);
    // HAL_FileClose(fp);

    return 0;
}

/**********************************************************************************
 * OTA file operations END
 **********************************************************************************/

// main OTA cycle
bool process_ota(OTAContextData *ota_ctx)
{
    bool  download_finished     = false;
    bool  upgrade_fetch_success = true;
    char  buf_ota[OTA_BUF_LEN]  = {0};
    int   rc                    = 0;
    void *h_ota                 = ota_ctx->ota_handle;
    int   packet_id             = 0;
    int   local_offset          = 0;
    int   last_downloaded_size  = 0;

    /* Must report version first */
    if (0 > IOT_OTA_ReportVersion(h_ota, IOT_OTA_FWTYPE_MCU, FW_RUNNING_MCU_VERSION)) {
        Log_e("report OTA (MCU)version failed");
        upgrade_fetch_success = false;
        goto end_of_ota;
    }
    if (0 > IOT_OTA_ReportVersion(h_ota, IOT_OTA_FWTYPE_MODULE, FW_RUNNING_MODULE_VERSION)) {
        Log_e("report OTA (Module)version failed");
        upgrade_fetch_success = false;
        goto end_of_ota;
    }

    do {
        IOT_MQTT_Yield(ota_ctx->mqtt_client, 200);

        Log_i("wait for ota upgrade command...");

    begin_of_ota:
        // recv the upgrade cmd
        if (IOT_OTA_IsFetching(h_ota))
        {
            IOT_OTA_Ioctl(h_ota, IOT_OTAG_FILE_SIZE, &ota_ctx->fw_file_size, 4);
            IOT_OTA_Ioctl(h_ota, IOT_OTAG_VERSION, ota_ctx->remote_version, FW_VERSION_MAX_LEN);
            IOT_OTA_Ioctl(h_ota, IOT_OTAG_FWTYPE, &ota_ctx->fw_type, 4);

            /* check if pre-downloading finished or not */
            /* if local FW downloaded size (ota_ctx->downloaded_size) is not zero, it will do resuming download */
            local_offset = _update_fw_downloaded_size(ota_ctx);
            if (ota_ctx->fw_file_size == local_offset) {  // have already downloaded
                upgrade_fetch_success = true;
                goto end_of_download;
            }

            /*set offset and start http connect*/
            rc = IOT_OTA_StartDownload(h_ota, ota_ctx->downloaded_size, ota_ctx->fw_file_size, OTA_HTTP_MAX_FETCHED_SIZE);
            if (QCLOUD_RET_SUCCESS != rc) {
                Log_e("OTA download start err,rc:%d", rc);
                upgrade_fetch_success = false;
                goto end_of_ota;
            }
            Log_d("remote fw type : %d %s", ota_ctx->fw_type, ota_ctx->fw_type == IOT_OTA_FWTYPE_MCU ? IOT_OTA_FWTYPE_MCU_STR : IOT_OTA_FWTYPE_MODULE_STR);

            // download and save the fw
            do {
                int len = IOT_OTA_FetchYield(h_ota, buf_ota, OTA_BUF_LEN, 1);
                if (len > 0) {
                    _save_fw_data_to_file(ota_ctx->downloaded_size, buf_ota, len);
                } else {
                    Log_e("download fail rc=%d", len);
                    upgrade_fetch_success = false;

                    if (len == IOT_OTA_ERR_FETCH_AUTH_FAIL || len == IOT_OTA_ERR_FETCH_NOT_EXIST) {
                        ota_ctx->ota_fail_cnt = MAX_OTA_RETRY_CNT;
                    }
                    goto end_of_ota;
                }

                /* get OTA information and update local info */
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_FETCHED_SIZE, &ota_ctx->downloaded_size, 4);
                rc = _update_local_fw_info(ota_ctx);
                if (QCLOUD_RET_SUCCESS != rc) {
                    Log_e("update local fw info err,rc:%d", rc);
                }

                // quit ota process as something wrong with mqtt
                rc = IOT_MQTT_Yield(ota_ctx->mqtt_client, 100);
                if (rc != QCLOUD_RET_SUCCESS && rc != QCLOUD_RET_MQTT_RECONNECTED) {
                    Log_e("MQTT error: %d", rc);
                    upgrade_fetch_success = false;
                    goto end_of_ota;
                }

            } while (!IOT_OTA_IsFetchFinish(h_ota));

        /* Must check MD5 match or not */
        end_of_download:
            if (upgrade_fetch_success)
            {
                // download is finished, delete the fw info file
                uint32_t firmware_valid;
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_CHECK_FIRMWARE, &firmware_valid, 4);
                if (0 == firmware_valid) {
                    Log_e("The firmware is invalid");
                    upgrade_fetch_success = false;
                    goto end_of_ota;
                } else {
                    Log_i("The firmware is valid");
                    upgrade_fetch_success = true;
                }
            }

            download_finished = true;
        }

        if (!download_finished)
            HAL_SleepMs(1000);
    } while (!download_finished);

    // do some post-download stuff for your need

end_of_ota:
    if (!upgrade_fetch_success) {
        // retry again
        if (IOT_MQTT_IsConnected(ota_ctx->mqtt_client)) {
            if ((ota_ctx->downloaded_size - last_downloaded_size) != OTA_HTTP_MAX_FETCHED_SIZE) {
                ota_ctx->ota_fail_cnt++;
            }
            if (ota_ctx->ota_fail_cnt <= MAX_OTA_RETRY_CNT) {
                upgrade_fetch_success = true;
                last_downloaded_size  = ota_ctx->downloaded_size;
                Log_e("OTA failed, retry %drd time!", ota_ctx->ota_fail_cnt);
                HAL_SleepMs(1000);
                goto begin_of_ota;
            } else {
                ota_ctx->ota_fail_cnt = 0;
                Log_e("report download fail!");
                packet_id = IOT_OTA_ReportUpgradeFail(h_ota, NULL);
            }
        }
    } else if (upgrade_fetch_success) {
        packet_id             = IOT_OTA_ReportUpgradeSuccess(h_ota, NULL);
        ota_ctx->ota_fail_cnt = 0;
    }

    _wait_for_pub_ack(ota_ctx, packet_id);

    return upgrade_fetch_success;
}

static bool sg_loop_test = true;

static void _wifi_config_result_cb(eWiFiConfigResult event, void *usr_data)
{
    Log_d("entry...");
    qiot_wifi_config_stop();
    switch (event) {
        case RESULT_WIFI_CONFIG_SUCCESS:
            Log_i("WiFi is ready, to do Qcloud IoT demo");
            Log_d("timestamp now:%d", HAL_Timer_current_sec());
            wifi_config_result_success = true;
            break;

        case RESULT_WIFI_CONFIG_TIMEOUT:
            Log_e("wifi config timeout!");
            wifi_config_result_success = false;
            break;

        case RESULT_WIFI_CONFIG_FAILED:
            Log_e("wifi config failed!");
            qiot_wifi_config_send_log();
            wifi_config_result_success = false;
            break;

        default:
            break;
    }
}

static bool qcloud_wifi_config_proc()
{
    int   rc;
    Timer timer;

#if WIFI_PROV_BT_COMBO_CONFIG_ENABLE
    rc = qiot_wifi_config_start(WIFI_CONFIG_TYPE_BT_COMBO, NULL, _wifi_config_result_cb);
    countdown(&timer, 500);
    while ((rc == QCLOUD_RET_SUCCESS) && (false == wifi_config_result_success) && !expired(&timer)) {
        Log_d("wait wifi config result...");
        HAL_SleepMs(1000);
    }
    qiot_wifi_config_stop();
    if (true == wifi_config_result_success) {
        return true;
    }
#endif  // WIFI_PROV_BT_COMBO_CONFIG_ENABLE

    return false;
}

int ota_mqtt_sample(char *argv)
{
    int             rc;
    OTAContextData *ota_ctx     = NULL;
    void           *mqtt_client = NULL;
    void           *h_ota       = NULL;
    MQTTInitParams init_params  = DEFAULT_MQTTINIT_PARAMS;
    
    IOT_Log_Set_Level(eLOG_DEBUG);

    if (false == qcloud_wifi_config_proc()) {
        Log_d("wifi config failed");
    }
    
    ota_ctx = (OTAContextData *)HAL_Malloc(sizeof(OTAContextData));
    if (ota_ctx == NULL) {
        Log_e("malloc failed");
        goto exit;
    }
    memset(ota_ctx, 0, sizeof(OTAContextData));

    rc = HAL_GetDevInfo(&sg_devInfo);
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("get device info failed: %d", rc);
        goto exit;
    }

    // setup MQTT init params
//    init_params = DEFAULT_MQTTINIT_PARAMS;
    rc          = _setup_connect_init_params(&init_params, ota_ctx, &sg_devInfo);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("init params err,rc=%d", rc);
        return rc;
    }

    // create MQTT mqtt_client and connect to server
    mqtt_client = IOT_MQTT_Construct(&init_params);
    if (mqtt_client != NULL) {
        Log_i("Cloud Device Construct Success");
    } else {
        Log_e("Cloud Device Construct Failed");
        return QCLOUD_ERR_FAILURE;
    }

    // init OTA handle
    h_ota = IOT_OTA_Init(sg_devInfo.product_id, sg_devInfo.device_name, mqtt_client);
    if (NULL == h_ota) {
        Log_e("initialize OTA failed");
        goto exit;
    }

    ota_ctx->ota_handle  = h_ota;
    ota_ctx->mqtt_client = mqtt_client;

    bool ota_success;
    do {
        // mqtt should be ready first
        rc = IOT_MQTT_Yield(mqtt_client, 500);
        if (rc == QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT)
        {
            HAL_SleepMs(1000);
            continue;
        } 
        else if (rc != QCLOUD_RET_SUCCESS && rc != QCLOUD_RET_MQTT_RECONNECTED)
        {
            if (rc == QCLOUD_ERR_MQTT_RECONNECT_TIMEOUT) {
                Log_e( "exit. mqtt reconnect timeout! Please check the network connection, or try to increase "
                    "MAX_RECONNECT_WAIT_INTERVAL(%d)", MAX_RECONNECT_WAIT_INTERVAL);
            } else {
                Log_e("exit with error: %d", rc);
            }
            break;
        }

        // OTA process
        ota_success = process_ota(ota_ctx);
        if (!ota_success) {
            Log_e("OTA failed! Do it again");
            HAL_SleepMs(2000);
        }
    } while (!ota_success);

exit:

    if (NULL != ota_ctx)
        HAL_Free(ota_ctx);

    if (NULL != h_ota)
        IOT_OTA_Destroy(h_ota);

    IOT_MQTT_Destroy(&mqtt_client);

    return 0;
}
