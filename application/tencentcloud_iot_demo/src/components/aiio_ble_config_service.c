/**
 * @brief   Define the application interface of ble distribution network
 * 
 * @file    aiio_ble_config.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note        Achieve logic function of ble distribution network by the protocol interface of it
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-15          <td>1.0.0            <td>zhuolm             <td> The application interface of ble distribute network
 */
#include "aiio_ble_protocol.h"
#include "aiio_ble_config_service.h"
#include "utils_sha1.h"
#include "aiio_common.h"
#include "utils_md5.h"
#include "aiio_protocol_code.h"
#include "aiio_error.h"
#include "aiio_wifi.h"
#include "aiio_ble.h"
#include "aiio_log.h"
#include "platform_bl.h"
#include "aiio_stdio.h"
#include "aiio_os_port.h"




#if CONFIG_BLE_DISTRIBUTION_NETWORK_ENABLE


#define BLE_MIN_INTV    200
#define BLE_MAX_INTV    220
#define BLE_LATENCY     0
#define BLE_TIMEOUT     400

#define BLE_ADV_MIN    160//20
#define BLE_ADV_MAX    180//40


#define ADV_FLAG_LEN_BYTES                  (1)
#define ADV_FLAG_TYPE_BYTES                 (1)
#define ADV_FLAG_VALUE_BYTES                (1)
#define ADV_NAME_LEN_BYTES                  (1)
#define ADV_NAME_TYPE_BYTES                 (1)


#define ADV_DATA_MAX_LEN                    (31)

#define QUEUE_MAX_SIZE                      (5)


typedef enum
{
    ADV_NAME_LEN_POSITIONM = 0,
    ADV_NAME_TYPE_POSITION,
    ADV_NAME_VALUE_POSITION
}aiio_adv_data_structure_t;

typedef enum 
{
    QUEUE_INVAILD_EVENT = 0,
    QUEUE_REV_BLE_DISCONNECTED,
    QUEUE_REV_BLE_DATA_EVENT
}aiio_queue_event_t;

static aiio_ble_default_handle_t srv_handle;

static aiio_ble_config_t ble_config = {
    .conn_intv = {
        .min_interval = BLE_MIN_INTV,
        .max_interval = BLE_MAX_INTV,
        .latency = BLE_LATENCY,
        .timeout = BLE_TIMEOUT,
    },
};

// static aiio_ble_default_server_t ble_server = {
//     .server_uuid[0] = AIIO_BLE_UUID_DECLARE_128(0x00, 0x00, 0xff, 0xf0, 0x65, 0xd0, 0x4e, 0x20, 0xb5, 0x6a, 0xe4, 0x93, 0x54, 0x1b, 0xa4, 0xe2),
//     .tx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_128(0x00, 0x00, 0xff, 0xe3,  0x65,  0xd0, 0x4e, 0x20, 0xb5, 0x6a, 0xe4, 0x93, 0x54, 0x1b, 0xa4, 0xe2),
//     .rx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_128(0x00, 0x00, 0xff, 0xe1, 0x65,  0xd0, 0x4e, 0x20, 0xb5, 0x6a, 0xe4, 0x93, 0x54, 0x1b, 0xa4, 0xe2),

//     .server_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x01),
//     .tx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x02),
//     .rx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x03),
// };

typedef struct
{
    aiio_ble_uuid_128_t srv_tt_uuid;
    aiio_ble_uuid_128_t tt_tx_uuid;
    aiio_ble_uuid_128_t tt_rx_uuid;
    aiio_ble_uuid_128_t srv_cmd_uuid;
    aiio_ble_uuid_128_t cmd_tx_uuid;
    aiio_ble_uuid_128_t cmd_rx_uuid;
} ten_ble_config_t;

ten_ble_config_t tencent_ble_config = {
    .srv_tt_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_128, .val = {0x00, 0x00, 0xff, 0xf0, 0x65, 0xd0, 0x4e, 0x20, 0xb5, 0x6a, 0xe4, 0x93, 0x54, 0x1b, 0xa4, 0xe2}},
    .tt_tx_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_128, .val = {0x00, 0x00, 0xff, 0xe3,  0x65,  0xd0, 0x4e, 0x20, 0xb5, 0x6a, 0xe4, 0x93, 0x54, 0x1b, 0xa4, 0xe2}},
    .tt_rx_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_128, .val = {0x00, 0x00, 0xff, 0xe1, 0x65,  0xd0, 0x4e, 0x20, 0xb5, 0x6a, 0xe4, 0x93, 0x54, 0x1b, 0xa4, 0xe2}},

    .srv_cmd_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_16, .val = {0xb0, 0x01}},
    .cmd_tx_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_16, .val = {0xb0, 0x02}},
    .cmd_rx_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_16, .val = {0xb0, 0x03}},
};

aiio_ble_default_server_t ble_server = {
        .server_uuid[0] = (aiio_ble_uuid_t *)&tencent_ble_config.srv_tt_uuid,
        .tx_char_uuid[0] = (aiio_ble_uuid_t *)&tencent_ble_config.tt_tx_uuid,
        .rx_char_uuid[0] = (aiio_ble_uuid_t *)&tencent_ble_config.tt_rx_uuid,

        .server_uuid[1] = (aiio_ble_uuid_t *)&tencent_ble_config.srv_cmd_uuid,
        .tx_char_uuid[1] = (aiio_ble_uuid_t *)&tencent_ble_config.cmd_tx_uuid,
        .rx_char_uuid[1] = (aiio_ble_uuid_t *)&tencent_ble_config.cmd_rx_uuid,
    };



static aiio_ble_adv_param_t param = {
    .conn_mode = AIIO_BLE_CONN_MODE_UND,
    /* BL602 unsupport */
    .disc_mode = AIIO_BLE_DISC_MODE_GEN,
    .interval_min = BLE_ADV_MIN,
    .interval_max = BLE_ADV_MAX,
};

typedef struct 
{
    char *queue_data;
    uint16_t queue_data_len;
    aiio_queue_event_t evt;
}aiio_ble_config_queue_t;




static uint8_t adv_structure_remain_len = (ADV_NAME_LEN_BYTES +  ADV_NAME_TYPE_BYTES);
static aiio_ble_iot_config_t  ble_iot_config = {0};
static uint8_t last_frame = 0xff;
static aiio_ble_data_t  ble_config_data = {0};
static aiio_entrypt_type_t entrypt_type = PROTOCOL_ENTRYPT_UNKNOW;
static char             *entrypt_key = NULL;
static aiio_ble_conn_t  ble_conn = NULL;
static aiio_os_thread_handle_t            ble_config_task_handle;
static aiio_os_queue_handle_t             ble_config_queue_handle = NULL;
static bool         ble_config_init = false;
static bool         ble_config_task_run = false;

static uint8_t set_wifi_token_flag = 0;

static void ble_salve_init(void);
static void aiio_ble_iot_destroy(void);


static uint8_t raw_adv_data[] = {
    /* flags */
    0x02,
    0x01,
    0x06,
    /* service uuid */
    0x03,
    0x03,
    0xF0,
    0xFF,
};

publish_token_info_t sg_publish_token_info;

static void aiio_HexPrint(char *str, uint8_t *data, uint16_t data_len)
{
    if(str)
    {
        aiio_log_d("\n%s: \r\n", str);
    }
    for(uint16_t n = 1; n <= data_len; n++)
    {
        aiio_log_d("%02x ", data[n-1]);
        if(n  % 16 == 0)
        {
            aiio_log_d("\r\n");
        }
    }
    aiio_log_d("\r\n");
    aiio_log_d("\r\n");
}

ble_qiot_ret_status_t ble_event_notify2(uint8_t type, uint8_t length_flag, uint8_t *header, uint8_t header_len,
                                        const char *buf, uint16_t buf_len)
{
    char *   p              = (char *)buf;
    uint16_t left_len       = buf_len;
    uint16_t send_len       = 0;
    uint16_t mtu_size       = 0;
    uint8_t  slice_state    = BLE_QIOT_EVENT_NO_SLICE;
    uint16_t send_buf_index = 0;
    uint16_t tmp_len        = 0;

    uint8_t send_buf[BLE_QIOT_EVENT_BUF_SIZE] = {0};
    int err = 0;
    aiio_ble_send_t ble_send_data = {0};


    do {
        memset(send_buf, 0, sizeof(send_buf));
        send_buf_index = 0;
        // send_len       = left_len > mtu_size ? mtu_size : left_len;
        send_len       = left_len;

        send_buf[send_buf_index++] = type;
        if (NULL != buf) {
            tmp_len = HTONS(send_len + header_len);
            memcpy(send_buf + send_buf_index, &tmp_len, sizeof(uint16_t));
            send_buf_index += sizeof(uint16_t);
            if (NULL != header) {
                memcpy(send_buf + send_buf_index, header, header_len);
                send_buf_index += header_len;
            }
            memcpy(send_buf + send_buf_index, p, send_len);
            send_buf_index += send_len;

            p += send_len;
            left_len -= send_len;
            send_len += (BLE_QIOT_EVENT_FIXED_HEADER_LEN + header_len);

            if (0 == left_len) {
                slice_state =
                    (BLE_QIOT_EVENT_NO_SLICE == slice_state) ? BLE_QIOT_EVENT_NO_SLICE : BLE_QIOT_EVENT_SLICE_FOOT;
            } else {
                slice_state =
                    (BLE_QIOT_EVENT_NO_SLICE == slice_state) ? BLE_QIOT_EVENT_SLICE_HEAD : BLE_QIOT_EVENT_SLICE_BODY;
            }
            // the high 2 bits means slice state, and the left 14 bits is data length
            send_buf[1] |= slice_state << 6;
            send_buf[1] |= length_flag;
        } else {
            send_len = send_buf_index;
        }

        aiio_HexPrint("post data", send_buf, send_len);

        ble_send_data.type = AIIO_BLE_NOTIFY;
        ble_send_data.data = (uint8_t *)send_buf;
        ble_send_data.length = send_len;
        ble_send_data.handle = srv_handle.tx_char_handle[0];
        ble_send_data.conn = ble_conn;

        err = aiio_ble_send(&ble_send_data);
        if(err != AIIO_OK)
        {
            aiio_log_e("ble send data fail \r\n");
        }

    } while (left_len != 0);

    return BLE_QIOT_RS_OK;
}

ble_qiot_ret_status_t ble_event_notify(uint8_t type, uint8_t *header, uint8_t header_len, const char *buf,
                                       uint16_t buf_len)
{
    return ble_event_notify2(type, 0, header, header_len, buf, buf_len);
}

ble_qiot_ret_status_t ble_event_report_wifi_mode(uint8_t result)
{
    aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);
    return ble_event_notify(BLE_QIOT_EVENT_UP_WIFI_MODE, NULL, 0, (const char *)&result, sizeof(uint8_t));
}

ble_qiot_ret_status_t ble_event_report_wifi_info(uint8_t result)
{
    return ble_event_notify(BLE_QIOT_EVENT_UP_WIFI_INFO, NULL, 0, (const char *)&result, sizeof(uint8_t));
}


void ble_combo_wifi_info_set(const char *ssid, uint8_t ssid_len, const char *passwd, uint8_t passwd_len)
{
    aiio_rev_queue_t  rev_queue = {0};
    memset(wifi_config_data.ssid, 0, sizeof(wifi_config_data.ssid));
    memset(wifi_config_data.passwd, 0, sizeof(wifi_config_data.passwd));
    memcpy(wifi_config_data.ssid, ssid, ssid_len);
    memcpy(wifi_config_data.passwd, passwd, passwd_len);
    aiio_flash_save_wifi_config_data(&wifi_config_data);
    ble_event_report_wifi_info(0);
    rev_queue.common_event = REV_WIFI_CONNECTED_EVENT;
    if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
    {
        aiio_log_i("queue send failed\r\n");
    }
    memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
}

ble_qiot_ret_status_t ble_event_report_wifi_connect(BLE_WIFI_MODE mode, BLE_WIFI_STATE state, uint8_t ssid_len,
                                                    const char *ssid)
{
    char    buf[40] = {0};
    uint8_t pos     = 0;

    buf[pos++] = mode;
    buf[pos++] = state;
    buf[pos++] = 0;
    buf[pos++] = ssid_len;
    memcpy(buf + pos, ssid, ssid_len);

    return ble_event_notify(BLE_QIOT_EVENT_UP_WIFI_CONNECT, NULL, 0, (const char *)buf, pos + ssid_len);
}

void bt_combo_report_wificonn_success()
{
    ble_event_report_wifi_connect(BLE_WIFI_MODE_STA, BLE_WIFI_STATE_CONNECT, (uint8_t)strlen((const char *)wifi_config_data.ssid), (const char *)wifi_config_data.ssid);
}

// publish MQTT msg
static int publish_token_msg(void *client,  publish_token_info_t *info)
{
    char topic_name[128] = {0};

    int size = snprintf(topic_name, sizeof(topic_name), "$thing/up/service/%s/%s", DeviceInfo_info.product_id,
                            DeviceInfo_info.device_name);
    if (size < 0 || size > sizeof(topic_name) - 1) {
        aiio_log_e("topic content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topic_name));
    }

    PublishParams pub_params = DEFAULT_PUB_PARAMS;
    pub_params.qos           = QOS1;

    char topic_content[128]     = {0};
    // printf("%s %s %s %s \r\n",info->pairTime.type,info->pairTime.start,info->pairTime.getSSID,info->pairTime.wifiConnected);
    info->pairTime.tokenPublish = HAL_GetTimeMs();
         size                        = snprintf(
                               topic_content, sizeof(topic_content),
                               "{\"method\":\"app_bind_token\",\"clientToken\":\"%s-%u\",\"params\": "
                                                      "{\"token\":\"%s\"}}",DeviceInfo_info.device_name, sg_publish_token_info.pairTime.getToken, sg_publish_token_info.token_str);
   
    if (size < 0 || size > sizeof(topic_content) - 1) {
        aiio_log_e("payload content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topic_content));
    }
    aiio_log_i("topic_content:%s\r\n",topic_content);
    pub_params.payload     = topic_content;
    pub_params.payload_len = strlen(topic_content);
    aiio_log_i("pub_params.payload :%s\r\n",pub_params.payload);

    return aiio_mqtt_client_publish(client, topic_name, pub_params.payload, 0, 0, 0);
}

// send mqtt token to iot explorer
void _mqtt_send_token(void)
{
    publish_token_msg(client, &sg_publish_token_info);
}

ble_qiot_ret_status_t ble_event_report_wifi_token(uint8_t result)
{
    return ble_event_notify(BLE_QIOT_EVENT_UP_WIFI_TOKEN, NULL, 0, (const char *)&result, sizeof(uint8_t));
}

void qiot_device_bind_set_token(const char *token)
{
    sg_publish_token_info.token_received    = true;
    sg_publish_token_info.pairTime.getToken = HAL_GetTimeMs();
    strncpy(sg_publish_token_info.token_str, token, MAX_TOKEN_LENGTH);
}

ble_qiot_ret_status_t ble_combo_wifi_token_set(const char *token, uint16_t len)
{
    qiot_device_bind_set_token(token);
    return 0;
}

uint16_t ble_get_user_data_mtu_size(void)
{
    return 128;
}

// report device info
ble_qiot_ret_status_t ble_event_report_device_info(void)
{
    char     device_info[56] = {0};  // 1 byte llsync proto version + 2 bytes mtu size + 1 byte length of develop version
    uint16_t mtu_size       = 0;

    mtu_size = LLSYNC_MTU_SET_MASK;
    mtu_size |= ble_get_user_data_mtu_size();
    mtu_size       = HTONS(mtu_size);
    device_info[0] = BLE_QIOT_LLSYNC_PROTOCOL_VERSION;
    memcpy(&device_info[1], &mtu_size, sizeof(mtu_size));
    device_info[3] = (char)strlen(BLE_QIOT_USER_DEVELOPER_VERSION);
    memcpy(&device_info[4], BLE_QIOT_USER_DEVELOPER_VERSION, device_info[3]);
    return ble_event_notify(BLE_QIOT_EVENT_UP_REPORT_MTU, NULL, 0, device_info, 4 + device_info[3]);
}

static int sg_bind_reply_code = -1;
char *token_str = NULL;
uint8_t token_len = 0;

static void aiio_ble_config_task(void *pv)
{
    aiio_ble_config_queue_t ble_config_queue_data = {0};
    aiio_rev_queue_t  rev_queue = {0};

    ble_config_task_run = true;

    char *   p_ssid      = NULL;
    char *   p_passwd    = NULL;
    while (ble_config_task_run)
    {
        if(aiio_os_queue_recv(ble_config_queue_handle, &ble_config_queue_data, 100) != AIIO_OK)
        {
            aiio_os_tick_dealy(aiio_os_ms2tick(50));
            continue;
        }

        switch (ble_config_queue_data.evt)
        {
            case QUEUE_INVAILD_EVENT:
            {
                aiio_log_d("QUEUE_INVAILD_EVENT \r\n");
            }
            break;

            case QUEUE_REV_BLE_DISCONNECTED:
            {
                aiio_log_d("QUEUE_REV_BLE_DISCONNECTED \r\n");
                aiio_ble_iot_data_t config_iot_data = {
                    .event = AIIO_BLE_DISCONNECED,
                };
                if(ble_iot_config.aiio_ble_iot_config_cb)
                {
                    ble_iot_config.aiio_ble_iot_config_cb(&config_iot_data);
                }

            }
            break;

            case QUEUE_REV_BLE_DATA_EVENT:
            {
                aiio_log_d("QUEUE_REV_BLE_DATA_EVENT \r\n");
                if(ble_config_queue_data.queue_data)
                {
                    aiio_log_d("queue_data = %s \r\n", ble_config_queue_data.queue_data);
                    aiio_HexPrint("receive queue data", (uint8_t *)ble_config_queue_data.queue_data, ble_config_queue_data.queue_data_len);
                    aiio_log_d("queue_data_len = %d \r\n", ble_config_queue_data.queue_data_len);
                    aiio_log_d("queue_data[0] = %02x \r\n", ble_config_queue_data.queue_data[0]);
                    aiio_log_d("queue_data[0] = %d \r\n", ble_config_queue_data.queue_data[0]);
                    switch(ble_config_queue_data.queue_data[0]){
                        case E_DEV_MSG_GET_DEV_INFO:
                            ble_event_report_device_info();
                            break;
                        case E_DEV_MSG_SET_WIFI_MODE:
                            ble_event_report_wifi_mode(0);
                            break;
                        case E_DEV_MSG_SET_WIFI_INFO:
                        {
                            // 1 byte ssid len + N bytes ssid + 1 byte pwd len + N bytes pwd
                            p_ssid = &ble_config_queue_data.queue_data[3];
                            p_passwd = &p_ssid[p_ssid[0] + 1];
                            ble_combo_wifi_info_set((const char *)&p_ssid[1], p_ssid[0], (const char *)&p_passwd[1], p_passwd[0]);
                        }
                        break; 
                        case E_DEV_MSG_SET_WIFI_TOKEN:
                        if (token_str == NULL) {
                            token_str = (char *)malloc(MAX_TOKEN_LENGTH);
                            if (token_str == NULL) {
                                aiio_log_i("Failed to allocate memory for token_str\r\n");
                                break; 
                            }
                            memset(token_str, 0, MAX_TOKEN_LENGTH); // 初始化内存为0
                        }
                        if(set_wifi_token_flag == 1){
                            memcpy(token_str+token_len, ble_config_queue_data.queue_data + 3, ble_config_queue_data.queue_data_len - 3);
                            // ble_combo_wifi_token_set(ble_config_queue_data.queue_data + 3, ble_config_queue_data.queue_data_len - 3);
                            ble_combo_wifi_token_set(token_str, MAX_TOKEN_LENGTH);
                            ble_event_report_wifi_token(0);
                            rev_queue.common_event = REV_CLOUD_CONNECTED_EVENT;
                            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                            {
                                aiio_log_i("queue send failed\r\n");
                            }
                            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
                            if (token_str != NULL) {
                                free(token_str);
                                token_str = NULL; 
                            }
                        }
                        if(set_wifi_token_flag == 0){
                            memcpy(token_str, ble_config_queue_data.queue_data + 3, ble_config_queue_data.queue_data_len - 3);
                            token_len = ble_config_queue_data.queue_data_len - 3;
                            set_wifi_token_flag = 1;
                        }
                        break;
                     default:
                        break;    
                    }
                    aiio_os_free(ble_config_queue_data.queue_data);
                    aiio_memset(&ble_config_queue_data, 0, sizeof(aiio_ble_config_queue_t));
                }
            }
            break;
            
            default:
                aiio_log_e("not find evt \r\n");
                break;
        }
    }


    aiio_os_queue_delete(ble_config_queue_handle);
    ble_config_queue_handle = NULL;

    aiio_log_d("delete ble config task \r\n");
    aiio_os_thread_delete(ble_config_task_handle);
}


static void aiio_ble_cb(aiio_ble_evt_t *event)
{
    switch (event->type) 
    {
        case AIIO_BLE_EVT_DISCONNECT:
        {
            aiio_log_w("ble disconn");
            aiio_ble_config_queue_t ble_config_queue_data = {0};
            ble_config_queue_data.evt = QUEUE_REV_BLE_DISCONNECTED;
            if(ble_config_queue_handle && ble_config_task_run)
            {
                if (aiio_os_queue_send(ble_config_queue_handle, &ble_config_queue_data, 0) != AIIO_OK)
                {
                    aiio_log_i("queue send failed\r\n");
                }
            }
            ble_conn = event->disconnect.conn;
            aiio_log_i("ble_conn = %p \r\n", ble_conn);
            ble_conn = NULL;
        }
        break;

        case AIIO_BLE_EVT_CONNECT:
        {
            aiio_log_i("ble conn");
            ble_conn = event->connect.conn;
            aiio_log_i("ble_conn = %p \r\n", ble_conn);
        }
        break;

        case AIIO_BLE_EVT_MTU:
        {
            aiio_log_i("mtu update:%d", event->mtu.mtu);
            ble_conn = event->mtu.conn;
            ble_event_notify(BLE_QIOT_EVENT_UP_SYNC_MTU, NULL, 0, (const char*)&event->mtu.mtu, sizeof(uint16_t));
            aiio_log_i("ble_conn = %p \r\n", ble_conn);
        }
        break;

        case AIIO_BLE_EVT_DATA:
        {
            aiio_log_i("AIIO_BLE_EVT_DATA\r\n");
            ble_conn = event->data.conn;
            aiio_log_i("ble_conn = %p \r\n", ble_conn);
            aiio_log_i("data down handle:%d len:%d", event->data.handle, event->data.length);
            // aiio_HexPrint("ble receive data", (uint8_t *)event->data.data, event->data.length);
            aiio_ble_config_queue_t ble_config_queue_data = {0};
            ble_config_queue_data.queue_data = aiio_os_malloc(event->data.length + 1);
            if(ble_config_queue_data.queue_data)
            {
                aiio_memset(ble_config_queue_data.queue_data, 0, event->data.length + 1);
                aiio_strncpy(ble_config_queue_data.queue_data, (char *)event->data.data, event->data.length);
                ble_config_queue_data.queue_data_len = event->data.length;
                ble_config_queue_data.evt = QUEUE_REV_BLE_DATA_EVENT;

                // aiio_HexPrint("ble queue data", (uint8_t *)ble_config_queue_data.queue_data, ble_config_queue_data.queue_data_len);
                if(ble_config_queue_handle && ble_config_task_run)
                {
                    if (aiio_os_queue_send(ble_config_queue_handle, &ble_config_queue_data, 0) != AIIO_OK)
                    {
                        aiio_log_i("queue send failed\r\n");
                        aiio_os_free(ble_config_queue_data.queue_data);
                    }
                }
                else
                {
                    aiio_os_free(ble_config_queue_data.queue_data);
                }
            }
        }
        break;

        default:
            aiio_log_w("undef event!");
            break;
    }
}


static void ble_print_mac(void)
{
    uint8_t mac[6];

    aiio_ble_get_mac(mac);
    aiio_log_i("ble mac:%02x:%02x:%02x:%02x:%02x:%02x",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}


int aiio_ble_config_response_status_tencent(int8_t error_code)
{
    char *packet_data = NULL;
    int err = 0;
    aiio_ble_send_t ble_send_data = {0};

    if(ble_iot_config.deviceid == NULL)
    {
        aiio_log_e("ble_conn is NULL, or device is NULL \r\n");
        return AIIO_ERROR;
    }

    packet_data = aiio_os_malloc(AIIO_BLE_PROTOCOL_PACKET_MAX_LEN);
    AIIO_MEM_CHECK(packet_data, return AIIO_ERROR);
    aiio_memset(packet_data, 0, AIIO_BLE_PROTOCOL_PACKET_MAX_LEN);

    last_frame += 1;
    err = aiio_PacketBleResponseDivideData(packet_data, AIIO_BLE_PROTOCOL_PACKET_MAX_LEN, last_frame, entrypt_type, entrypt_key, ble_iot_config.deviceid, error_code);
    if(err < 0)
    {
        aiio_log_e("packet ble protocol fail \r\n");
        aiio_os_free(packet_data);
        return AIIO_ERROR;
    }
    aiio_log_i("packet data len = %d \r\n", err);
    // aiio_HexPrint("ble response divide data", (uint8_t *)packet_data, err);

    ble_send_data.type = AIIO_BLE_NOTIFY;
    ble_send_data.data = (uint8_t *)packet_data;
    ble_send_data.length = err;
    ble_send_data.handle = srv_handle.tx_char_handle[0];
    ble_send_data.conn = ble_conn;

    err = aiio_ble_send(&ble_send_data);
    if(err != AIIO_OK)
    {
        aiio_log_e("ble send data fail \r\n");
        aiio_os_free(packet_data);
        return AIIO_ERROR;
    }

    aiio_os_free(packet_data);

    return AIIO_OK;
}


static void ble_salve_init(void)
{
    aiio_ble_init(AIIO_BLE_MODE_PERIPHERAL, &ble_config);
    ble_gatts_add_default_svcs(&ble_server, &srv_handle);

    aiio_ble_register_event_cb(aiio_ble_cb);

    ble_print_mac();
}


static void aiio_ble_iot_destroy(void)
{
    aiio_ble_config_queue_t ble_config_queue_data = {0};

    if(ble_iot_config.deviceid)
    {
        aiio_os_free(ble_iot_config.deviceid);
    }

    if(ble_iot_config.productid)
    {
        aiio_os_free(ble_iot_config.productid);
    }

    if(ble_iot_config.wifi_flag)
    {
        aiio_os_free(ble_iot_config.wifi_flag);
    }

    if(entrypt_key)
    {
        aiio_os_free(entrypt_key);
        entrypt_key = NULL;
    }

    aiio_memset(&ble_iot_config, 0, sizeof(ble_iot_config));
}

static e_llsync_bind_state       sg_llsync_bind_state;        // llsync bind state in used

void llsync_bind_state_set(e_llsync_bind_state new_state)
{
    aiio_log_i("bind state: %d ---> %d", sg_llsync_bind_state, new_state);
    sg_llsync_bind_state = new_state;
}

e_llsync_bind_state llsync_bind_state_get(void)
{
    return sg_llsync_bind_state;
}

// [1byte bind state] + [6 bytes mac] + [8bytes identify string]/[10 bytes product id]
int ble_get_my_broadcast_data(char *out_buf, int buf_len)
{
    uint8_t mac[6];
    aiio_wifi_sta_mac_get(mac);
    aiio_log_i("ble mac:%02x:%02x:%02x:%02x:%02x:%02x",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // POINTER_SANITY_CHECK(out_buf, BLE_QIOT_RS_ERR_PARA);
    // BUFF_LEN_SANITY_CHECK(buf_len, BLE_BIND_IDENTIFY_STR_LEN + BLE_QIOT_MAC_LEN + 1, BLE_QIOT_RS_ERR_PARA);
    int     ret_len                      = 0;
#if BLE_QIOT_LLSYNC_STANDARD
    int     i                            = 0;
    uint8_t md5_in_buf[128]              = {0};
    uint8_t md5_in_len                   = 0;
    uint8_t md5_out_buf[MD5_DIGEST_SIZE] = {0};

    out_buf[ret_len] = llsync_bind_state_get() | (BLE_QIOT_LLSYNC_PROTOCOL_VERSION << LLSYNC_PROTO_VER_BIT);

    ret_len++;
    llsync_bind_state_set(E_LLSYNC_BIND_WAIT);
#endif //BLE_QIOT_LLSYNC_STANDARD
    // 1 bytes state + 6 bytes mac + 10 bytes product id
    memcpy(out_buf + ret_len, mac, BLE_QIOT_MAC_LEN);
    ret_len += BLE_QIOT_MAC_LEN;
    memcpy(out_buf + ret_len, DeviceInfo_info.product_id, MAX_SIZE_OF_PRODUCT_ID);
    ret_len += MAX_SIZE_OF_PRODUCT_ID;

    return ret_len;
}

ble_qiot_ret_status_t ble_ten_advertising_start(manufacturer_data_s *adv)
{
    uint8_t usr_adv_data[31] = {0};
    uint8_t len              = 0;
    uint8_t index            = 0;
    int i = 0;
    uint8_t usr_adv_name[31] = {0};

    memcpy(usr_adv_data, &adv->company_identifier, sizeof(uint16_t));
    len = sizeof(uint16_t);
    memcpy(usr_adv_data + len, adv->adv_data, adv->adv_data_len);
    len += adv->adv_data_len;
    aiio_log_d("usr_adv_data: ");
    for (i = 0; i < len; i++) {  
        aiio_log_d("%02X ", usr_adv_data[i]);  
    }  
    aiio_log_d("\n"); 

    index                 = 7;
    raw_adv_data[index++] = len + 1;
    raw_adv_data[index++] = 0xFF;
    memcpy(raw_adv_data + index, usr_adv_data, len);
    index += len;

    usr_adv_name[0] = strlen(SAMPLE_DEVICE_NAME) + 1;
    usr_adv_name[1] = 0x09;
    memcpy(usr_adv_name + 2, SAMPLE_DEVICE_NAME, strlen(SAMPLE_DEVICE_NAME));

    aiio_log_i("adv = %s\r\n", raw_adv_data);
    aiio_log_d("index is %d\r\n",index);
    for (i = 0; i < index; i++) {  
        aiio_log_d("%02X ", raw_adv_data[i]);  
    }  
    aiio_log_d("\n");  
    aiio_ble_adv_start(&param, raw_adv_data, index, usr_adv_name, strlen(SAMPLE_DEVICE_NAME) + 2);

    return 0;
}

ble_qiot_ret_status_t ble_qiot_advertising_start(void)
{
    manufacturer_data_s my_adv_info;
    uint8_t    adv_data[32] = {0};
    uint8_t    adv_data_len = 0;

    adv_data_len = ble_get_my_broadcast_data((char *)adv_data, sizeof(adv_data));
    my_adv_info.company_identifier = TENCENT_COMPANY_IDENTIFIER;
    my_adv_info.adv_data           = adv_data;
    my_adv_info.adv_data_len       = adv_data_len;
    ble_ten_advertising_start(&my_adv_info);

    return BLE_QIOT_RS_OK;
}


int aiio_ble_config_init_tencent(aiio_ble_iot_config_t *ble_congfig)
{
    int err = 0;

    if(ble_config_init)
    {
        aiio_log_e("ble config already init \r\n");
        return AIIO_ERROR;
    }

    AIIO_MEM_CHECK(ble_congfig, goto config_fail);
    AIIO_MEM_CHECK(ble_congfig->aiio_ble_iot_config_cb, goto config_fail);

    ble_iot_config.aiio_ble_iot_config_cb = ble_congfig->aiio_ble_iot_config_cb;


    aiio_os_queue_create(&ble_config_queue_handle, QUEUE_MAX_SIZE, sizeof(aiio_ble_config_queue_t));
    if (!ble_config_queue_handle)
    {
        aiio_log_e("create queue fail \r\n");
        goto config_fail;
    }


    ble_iot_config.task_prio = ble_congfig->task_prio;
    if (ble_iot_config.task_prio <= 0) {
        ble_iot_config.task_prio = AIIO_IOT_BLE_CONFIG_TASK_PRIORITY;
    }

    ble_iot_config.task_stack = ble_congfig->task_stack;
    if (ble_iot_config.task_stack <= 0) {
        ble_iot_config.task_stack = AIIO_IOT_BLE_CONFIG_TASK_STACK;
    }

    aiio_log_i("create ble config task \r\n");
    if (aiio_os_thread_create(&ble_config_task_handle, "ble_config_task", aiio_ble_config_task, ble_iot_config.task_stack, NULL, ble_iot_config.task_prio) != AIIO_OK) 
    {
        aiio_log_e("create ble config task fail \r\n");
        goto config_fail;
    }

    ble_salve_init();

    err = ble_qiot_advertising_start();
    if(err != BLE_QIOT_RS_OK)
    {
        aiio_log_e("ble adv start fail \r\n");
        aiio_ble_deinit();
        goto config_fail;
    }

    ble_config_init = true;

    return AIIO_OK;

config_fail:
    aiio_ble_iot_destroy();
    return AIIO_ERROR;
}



int aiio_ble_config_deinit_tencent(void)
{
    if(!ble_config_init)
    {
        aiio_log_e("ble config already deinit \r\n");
        return AIIO_ERROR;
    }

    aiio_ble_iot_destroy();
    aiio_ble_deinit();

    ble_config_init = false;
    ble_config_task_run = false;

    return AIIO_OK;
}


#endif

