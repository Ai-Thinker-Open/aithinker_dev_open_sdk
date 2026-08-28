/**
 * @brief   Apply BLE distribution network protocol to define some interfaces applying BLE distribution network function
 * 
 * @file    aiio_ble_config_wifi.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note        This file is mainly describing applying ble distribution network function
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-16          <td>1.0.0            <td>zhuolm             <td> The interface of applying ble distribution network function
 */
#include "aiio_ble_config_wifi.h"
#include "aiio_ble_config.h"
#include "aiio_log.h"
#include "aiio_common.h"
#include "aiio_adapter_include.h"
#include "aiio_smartliving_demo.h"
#include "ali_breeze_export.h"

#define COMBO_EVT_CODE_AP_INFO         0x0001
#define COMBO_EVT_CODE_RESTART_ADV     0x0002

breeze_apinfo_t apinfo;
uint8_t g_ble_state = 0;
typedef struct combo_user_bind_s {
    uint8_t bind_state;
} combo_user_bind_t;
static combo_user_bind_t g_combo_bind = { 0 };

static char awss_report_id = 0;

static void combo_status_change_cb(breeze_event_t event)
{
   switch (event) {
        case CONNECTED:
            aiio_log_d("BLE Connected");
            g_ble_state = 1;
            break;

        case DISCONNECTED:
            aiio_log_d("BLE Disconnected");
            g_ble_state = 0;
            // aos_post_event(EV_BZ_COMBO, COMBO_EVT_CODE_RESTART_ADV, 0);
            break;

        case AUTHENTICATED:
            aiio_log_d("BLE Authenticated");
            g_combo_bind.bind_state = 0;
            break;

        case TX_DONE:
            aiio_log_d("BLE Payload TX Done");
            break;
        default:
            break;
    }
}

static void combo_set_dev_status_cb(uint8_t *buffer, uint32_t length)
{
    if (NULL == buffer || 0 == length) {
        return;
    }
    aiio_log_d("BLE_SET:%.*s", length, buffer);
}

static void combo_get_dev_status_cb(uint8_t *buffer, uint32_t length)
{
    if (NULL == buffer || 0 == length) {
        return;
    }
    aiio_log_d("BLE_QUE:%.*s", length, buffer);
}

static void combo_apinfo_rx_cb(breeze_apinfo_t * ap)
{
    aiio_rev_queue_t  rev_queue = {0};
    if (!ap) {
        aiio_log_d("combo apinfo rx null!!");
        return;
    }

    memcpy(&apinfo, ap, sizeof(apinfo));
    // aos_post_event(EV_BZ_COMBO, COMBO_EVT_CODE_AP_INFO, (unsigned long)&apinfo);
    aiio_log_d("apinfo.ssid:%s\r\n",apinfo.ssid);
    aiio_log_d("apinfo.pw:%s\r\n",apinfo.pw);
    aiio_log_d("apinfo.region_mqtturl:%s\r\n",apinfo.region_mqtturl);
    aiio_log_d("apinfo.region_type:%d\r\n",apinfo.region_type);
    aiio_log_d("apinfo.region_id:%d\r\n",apinfo.region_id);

    awss_set_token(apinfo.apptoken, apinfo.token_type);

    // get region information
    if (apinfo.region_type == REGION_TYPE_ID) {
        aiio_log_d("info.region_id: %d", apinfo.region_id);
        iotx_guider_set_dynamic_region(apinfo.region_id);
    } else if (apinfo.region_type == REGION_TYPE_MQTTURL) {
        aiio_log_d("info.region_mqtturl: %s", apinfo.region_mqtturl);
        // iotx_guider_set_dynamic_mqtt_url(info->region_mqtturl);
    } else {
        aiio_log_d("REGION TYPE not supported");
        iotx_guider_set_dynamic_region(IOTX_CLOUD_REGION_INVALID);
    }
    memset(wifi_config_data.ssid, 0, sizeof(wifi_config_data.ssid));
    memset(wifi_config_data.passwd, 0, sizeof(wifi_config_data.passwd));
    memset(wifi_config_data.region_mqtturl, 0, sizeof(wifi_config_data.region_mqtturl));
    memcpy(wifi_config_data.ssid, apinfo.ssid, sizeof(apinfo.ssid));
    memcpy(wifi_config_data.passwd, apinfo.pw, sizeof(apinfo.pw));
    memcpy(wifi_config_data.region_mqtturl, apinfo.region_mqtturl, sizeof(apinfo.region_mqtturl));
    aiio_flash_save_wifi_config_data(&wifi_config_data);
    rev_queue.common_event = REV_WIFI_CONNECTED_EVENT;
    if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
    {
        aiio_log_i("queue send failed\r\n");
    }
    memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
}

int combo_net_init()
{
    breeze_dev_info_t dinfo = { 0 };
    uint8_t combo_adv_mac[6] = {0};
    uint8_t temp; 
    aiio_wifi_sta_mac_get(combo_adv_mac);
    combo_adv_mac[5]+=1;
    aiio_log_d("000 wifi mac = %02x%02x%02x%02x%02x%02x!!",combo_adv_mac[0],combo_adv_mac[1],combo_adv_mac[2],combo_adv_mac[3],combo_adv_mac[4],combo_adv_mac[5]);  
    temp = combo_adv_mac[0]; combo_adv_mac[0] = combo_adv_mac[5]; combo_adv_mac[5] = temp;  
    temp = combo_adv_mac[1]; combo_adv_mac[1] = combo_adv_mac[4]; combo_adv_mac[4] = temp;  
    temp = combo_adv_mac[2]; combo_adv_mac[2] = combo_adv_mac[3]; combo_adv_mac[3] = temp;  
    aiio_log_d("111 mac = %02x%02x%02x%02x%02x%02x!!",combo_adv_mac[0],combo_adv_mac[1],combo_adv_mac[2],combo_adv_mac[3],combo_adv_mac[4],combo_adv_mac[5]);
    dinfo.product_id = DeviceInfo_info.product_Id;
    dinfo.product_key = DeviceInfo_info.product_key;
    dinfo.product_secret =  DeviceInfo_info.product_Secret;
    dinfo.device_name = DeviceInfo_info.device_name;
    dinfo.device_secret = DeviceInfo_info.device_secret;
    dinfo.dev_adv_mac = combo_adv_mac;
    breeze_awss_init(&dinfo,
                        combo_status_change_cb,
                        combo_set_dev_status_cb,
                        combo_get_dev_status_cb,
                        combo_apinfo_rx_cb,
                        NULL);
    breeze_awss_start();
    return 0;
}

void combo_ap_conn_notify(void)
{
    uint8_t rsp[] = {0x01, 0x01, 0x01};
    // if (g_ble_state) {
        breeze_post(rsp, sizeof(rsp));
    // }
}

void combo_token_report_notify(void)
{
    uint8_t rsp[] = { 0x01, 0x01, 0x03 };
    // if (g_ble_state) {
        breeze_post(rsp, sizeof(rsp));
    // }
}

int awss_build_packet(int type, void *id, void *ver, void *method, void *data, int code, void *packet, int *packet_len)
{

    if (packet_len == NULL || data == NULL || packet == NULL) {
        return -1;
    }

    int len = *packet_len;
    if (len <= 0) {
        return -1;
    }

    if (type == AWSS_CMP_PKT_TYPE_REQ) {
        if (ver == NULL || method == NULL) {
            return -1;
        }

        len = snprintf(packet, len - 1, AWSS_REQ_FMT, (char *)id, (char *)ver, (char *)method, (char *)data);
        return 0;
    } else if (type == AWSS_CMP_PKT_TYPE_RSP) {
        len = snprintf(packet, len - 1, AWSS_ACK_FMT, (char *)id, code, (char *)data);
        return 0;
    }
    return -1;
}

const char *awss_build_topic(const char *topic_fmt, char *topic, uint32_t tlen)
{
    if (topic == NULL || topic_fmt == NULL || tlen == 0) {
        return NULL;
    }

    snprintf(topic, tlen - 1, topic_fmt, DeviceInfo_info.product_key, DeviceInfo_info.device_name);

    return topic;
}

int awss_report_token_to_cloud(void)
{
#define REPORT_TOKEN_PARAM_LEN  (64)
#define REPORT_TOKEN_STATE_MSG_LEN  (64)
    char token_state_msg[REPORT_TOKEN_STATE_MSG_LEN];
    char param[REPORT_TOKEN_PARAM_LEN] = {0};
    char id_str[MSG_REQ_ID_LEN] = {0};
    bind_token_type_t token_type;
    unsigned char token_str[RANDOM_STR_MAX_LEN] = {0};

    int packet_len = AWSS_REPORT_LEN_MAX;

    char *packet = malloc(packet_len + 1);
    if (packet == NULL) {
        aiio_log_e("alloc mem(%d) failed", packet_len);
        return -1;
    }

    snprintf(id_str, MSG_REQ_ID_LEN - 1, "\"%u\"", awss_report_id ++);

    awss_get_token(token_str, RANDOM_STR_MAX_LEN, &token_type);
    aiio_log_d("report token token_str:%s\r\n", token_str);
    snprintf(param, REPORT_TOKEN_PARAM_LEN - 1, "{\"token\":\"%s\"}", token_str);
    aiio_log_d("report token param:%s\r\n", param);
    awss_build_packet(AWSS_CMP_PKT_TYPE_REQ, id_str, ILOP_VER, METHOD_MATCH_REPORT, param, 0, packet, &packet_len);

    snprintf(token_state_msg, REPORT_TOKEN_STATE_MSG_LEN, "report token:%s to cloud", token_str);

    aiio_log_d("report token:%s\r\n", packet);
    char topic[TOPIC_LEN_MAX] = {0};
    awss_build_topic(TOPIC_MATCH_REPORT, topic, TOPIC_LEN_MAX);

    aiio_log_d("report token topic:%s\r\n", topic);

    // int ret = awss_cmp_mqtt_send(topic, packet, packet_len, 1);

    int ret = aiio_mqtt_client_publish(client, topic, packet, 0, 1, 0);

    aiio_log_d("report token res:%d\r\n", ret);

    return ret;
}


