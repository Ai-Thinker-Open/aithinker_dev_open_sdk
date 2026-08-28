/**
 * @brief   声明一些公共的变量、结构体
 * 
 * @file    aiio_common.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * 
 * 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Authorhor           <th>Notes
 * <tr><td>2023-06-12          <td>1.0.0            <td>Ai-Thinker             <td>
 */

#ifndef __AIIO_COMMON_H_
#define __AIIO_COMMON_H_
#include "aiio_adapter_include.h"
#include "utils_param_check.h"
#include "aiio_cloud_key.h"
#include "aiio_cloud_led.h"

#define AIIO_AT_UART                             CONFIG_AT_UART
#define AIIO_AT_BAUDRATE                         CONFIG_AT_BAUDRATE
#define AIIO_AT_TX_PIN                           CONFIG_AT_TX_PIN
#define AIIO_AT_RX_PIN                           CONFIG_AT_RX_PIN
#define AIIO_AT_RECEIVE_BUFFER_MAX_LEN           CONFIG_AT_RECEIVE_BUFFER_MAX_LEN


#define AIIO_AT_DATABIT                           (8)
#define AIIO_AT_STOPBIT                           (1)
#define AIIO_AT_PARITY                            (0)

#define FLASH_RW_START_ADDR                 (CONFIG_FLASH_RW_START_ADDR)
#define FLASH_RW_WIFI_DATA_ADDR              (FLASH_RW_START_ADDR)
#define FLASH_RW_DEVICE_INFO_ADDR              (CONFIG_FLASH_RW_START_ADDR + CONFIG_FLASH_REGION_SIZE)
#define FLASH_RW_OTA_INFO_ADDR              (CONFIG_FLASH_RW_START_ADDR + CONFIG_FLASH_REGION_SIZE * 2)


typedef enum
{
    REV_CONFIG_DATA_EVENT,                          /*!< Get  config data event in  distribution network, it will callback when device got  config data*/
    REV_CONFIG_START_EVENT,                         /*!< Start event in  distribution network,it will callback when key is pressed*/
    REV_CONFIG_STOP_EVENT,                          /*!< Stop event in  distribution network,it will callback when key is pressed*/
    REV_BLE_DISCONNECTED_EVENT,                         /*!< BLE disconnect event, it will callback when device disconnected with APP*/
    REV_CONFIG_OK_EVENT,                            /*!< Success event in  distribution network，it will callback when device is online*/
    REV_CONFIG_FAIL_EVENT,                          /*!< FAIL event in  distribution network，it will callback when device is fail in  distribution network*/
    REV_WIFI_CONNECTED_EVENT,                           /*!< Wifi connected event，it will callback when device connected wifi*/
    REV_CLOUD_CONTRL_DATA_EVENT,                        /*!< Received to be control attribute event，it will callback when device received data from cloud*/
    REV_CLOUD_ONLINE_EVENT,                              /*!< Online event，it will callback when device report online status to cloud */
    REV_CLOUD_CONNECTED_EVENT,                          /*!< Cloud connectedc event, it will callback when device connected cloud by mqtt*/
    REV_CLOUD_DISCONNECTED,                              /*!< Cloud disconnected event, it will callback when device disconnected cloud by mqtt*/
    REV_CLOUD_OTA_INFO_EVENT,                           /*!< recevice ota information, ota information need to be stored in flash, the ota version needs to be used to determine whether the firmware upgrade is successful*/
    REV_CLOUD_OTA_FAIL_EVENT,                                 /*!< ota data download fail*/
    REV_CLOUD_OTA_SUCCESS_EVENT,                              /*!< ota data download success*/
}aiio_common_event_t;

typedef struct
{
    char ssid[33];                              /*!< The ssid of rout*/
    char passwd[65];                             /*!< The password of rout */
}wifi_config_data_t;

typedef struct 
{
    char *queue_data;                      
    uint16_t queue_data_len;                
    aiio_common_event_t common_event;      
}aiio_rev_queue_t;


/* MAX size of client ID */
#define MAX_SIZE_OF_CLIENT_ID (80)

/* MAX size of product ID */
#define MAX_SIZE_OF_PRODUCT_ID (10)

/* MAX size of device name */
#define MAX_SIZE_OF_DEVICE_NAME (48)

/* MAX size of device secret */
#define MAX_SIZE_OF_DEVICE_SECRET (64)

/* Max size of conn Id  */
#define MAX_CONN_ID_LEN (6)

#define QCLOUD_IOT_DEVICE_SDK_APPID     "21010406"
#define QCLOUD_IOT_DEVICE_SDK_APPID_LEN (sizeof(QCLOUD_IOT_DEVICE_SDK_APPID) - 1)

/* MAX valid time when connect to MQTT server. 0: always valid */
/* Use this only if the device has accurate UTC time. Otherwise, set to 0 */
#define MAX_ACCESS_EXPIRE_TIMEOUT (0)

/* Max size of base64 encoded PSK = 64, after decode: 64/4*3 = 48*/
#define DECODE_PSK_LENGTH 48

/* default MQTT Tx buffer size, MAX: 16*1024 */
#define QCLOUD_IOT_MQTT_TX_BUF_LEN (2048)

typedef struct 
{
    char product_id[MAX_SIZE_OF_PRODUCT_ID + 1];
    char device_name[MAX_SIZE_OF_DEVICE_NAME + 1];
    char device_secret[MAX_SIZE_OF_DEVICE_SECRET + 1];
}aiio_device_info;


// /* The structure of MQTT event handle */
// typedef struct {
//     MQTTEventHandleFun h_fp;
//     void              *context;
// } MQTTEventHandler;

/* The structure of MQTT init parameters */
typedef struct {
    char *region;  // region

    /* device info */
    char *product_id;   // product ID
    char *device_name;  // device name

#ifdef AUTH_MODE_CERT
    char *cert_file;  // cert file path
    char *key_file;   // key file path
#else
    char *device_secret;  // device secret
#endif

    uint32_t command_timeout;         // timeout value (unit: ms) for MQTT
                                      // connect/pub/sub/yield
    uint32_t keep_alive_interval_ms;  // MQTT keep alive time interval in millisecond

    uint8_t clean_session;  // flag of clean session, 1 clean, 0 not clean

    uint8_t auto_connect_enable;  // flag of auto reconnection, 1 is enable and
                                  // recommended

    // MQTTEventHandler event_handle;  // event callback

} MQTTInitParams;

/**
 * @brief define MQTT connect parameters structure
 *
 */
typedef struct {
    char *client_id;  // unique client id
    char *username;   // user name
    char *password;   // passwrod

    char conn_id[MAX_CONN_ID_LEN];

    char    struct_id[4];    // The eyecatcher for this structure.  must be MQTC.
    uint8_t struct_version;  // struct version = 0
    uint8_t mqtt_version;    // MQTT protocol version: 4 = 3.1.1

    uint16_t keep_alive_interval;  // keep alive interval, unit: second
    uint8_t  clean_session;        // flag of clean session, refer to MQTT spec 3.1.2.4

    uint8_t auto_connect_enable;  // enable auto connection or not

#ifdef AUTH_WITH_NOTLS
    char *device_secret;      // PSK
    int   device_secret_len;  // length of PSK
#endif

} MQTTConnectParams;

/**
 * @brief MQTT QCloud IoT Client structure
 */
typedef struct Client {
    uint8_t is_connected;
    uint8_t was_manually_disconnected;
    uint8_t is_ping_outstanding;  // 1 = ping request is sent while ping response
                                  // not arrived yet

    uint16_t next_packet_id;      // MQTT random packet id
    uint32_t command_timeout_ms;  // MQTT command timeout, unit:ms

    uint32_t current_reconnect_wait_interval;  // unit:ms
    uint32_t counter_network_disconnected;     // number of disconnection

    size_t        write_buf_size;                         // size of MQTT write buffer
    size_t        read_buf_size;                          // size of MQTT read buffer
    unsigned char write_buf[QCLOUD_IOT_MQTT_TX_BUF_LEN];  // MQTT write buffer
    // unsigned char read_buf[QCLOUD_IOT_MQTT_RX_BUF_LEN];   // MQTT read buffer

    void *lock_generic;    // mutex/lock for this client struture
    void *lock_write_buf;  // mutex/lock for write buffer

    void *lock_list_pub;  // mutex/lock for puback waiting list
    void *lock_list_sub;  // mutex/lock for suback waiting list

    // List *list_pub_wait_ack;  // puback waiting list
    // List *list_sub_wait_ack;  // suback waiting list

    // MQTTEventHandler event_handle;  // callback for MQTT event

    MQTTConnectParams options;  // handle to connection parameters

    // Network network_stack;  // MQTT network stack

    // Timer ping_timer;             // MQTT ping timer
    // Timer reconnect_delay_timer;  // MQTT reconnect delay timer

    // DeviceInfo     device_info;
    // SubTopicHandle sub_handles[MAX_MESSAGE_HANDLERS];  // subscription handle array

    // char host_addr[HOST_STR_LENGTH];

#ifdef AUTH_MODE_CERT
    char cert_file_path[FILE_PATH_MAX_LEN];  // full path of device cert file
    char key_file_path[FILE_PATH_MAX_LEN];   // full path of device key file
#else
    unsigned char psk_decode[DECODE_PSK_LENGTH];
#endif

#ifdef MQTT_RMDUP_MSG_ENABLED
#define MQTT_MAX_REPEAT_BUF_LEN 10
    uint16_t     repeat_packet_id_buf[MQTT_MAX_REPEAT_BUF_LEN];
    unsigned int current_packet_id_cnt;
#endif

#ifdef MULTITHREAD_ENABLED
    bool yield_thread_exit;
    bool yield_thread_running;
    int  yield_thread_exit_code;
#endif

} Qcloud_IoT_Client;

typedef enum {
    BLE_QIOT_RS_OK             = 0,   // success
    BLE_QIOT_RS_ERR            = -1,  // normal error
    BLE_QIOT_RS_ERR_FLASH      = -2,  // flash error
    BLE_QIOT_RS_ERR_PARA       = -3,  // parameters error
    BLE_QIOT_RS_VALID_SIGN_ERR = -4,
} ble_qiot_ret_status_t;

/* Max size of a topic name */
#define MAX_SIZE_OF_CLOUD_TOPIC ((MAX_SIZE_OF_DEVICE_NAME) + (MAX_SIZE_OF_PRODUCT_ID) + 64 + 6)

#define DEFAULT_MQTTCONNECT_PARAMS                                            \
    {                                                                         \
        NULL, NULL, NULL, {0}, {'M', 'Q', 'T', 'C'}, 0, 4, 240, 1, 1, NULL, 0 \
    }
extern aiio_mqtt_client_config_t mqtt_cfg;
extern aiio_device_info DeviceInfo_info;
extern aiio_mqtt_client_handle_t client;
extern QueueHandle_t    cloud_rev_queue_handle;        /*!< The receive queue data handle*/
extern wifi_config_data_t wifi_config_data;

// void aiio_mqtt_connect_packet(void);
void IOT_MQTT_Construct(aiio_device_info *pParams);
uint64_t HAL_GetTimeMs(void); 
long HAL_Timer_current_sec(void);
void get_next_conn_id(char *conn_id);
int qcloud_iot_mqtt_connect(Qcloud_IoT_Client *pClient, MQTTConnectParams *pParams);
void bt_combo_report_wificonn_success();
void _mqtt_send_token(void);

int aiio_flash_get_device_info(aiio_device_info *device_info);
int aiio_flash_clear_device_info(aiio_device_info *device_info);
int aiio_flash_save_device_info(aiio_device_info *device_info);

int aiio_flash_get_wifi_config_data(wifi_config_data_t *config_data);
void aiio_flash_save_wifi_config_data(wifi_config_data_t *config_data);
void aiio_flash_clear_config_data(void);
#endif