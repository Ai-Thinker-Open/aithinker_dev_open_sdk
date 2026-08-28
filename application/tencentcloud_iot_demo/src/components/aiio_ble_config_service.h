/**
 * @brief   Declare the application interface of ble distribution network
 * 
 * @file    aiio_ble_config.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note        Achieve logic function of ble distribution network by the protocol interface of it
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-15          <td>1.0.0            <td>zhuolm             <td> The application interface of ble distribute network
 */
#ifndef __AIIO_BLE_CONFIG_H_
#define __AIIO_BLE_CONFIG_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "aiio_protocol_comm.h"
#include "aiio_ble_protocol.h"
#include "aiio_protocol_code.h"


#define     AIIO_IOT_BLE_CONFIG_TASK_PRIORITY                           (5)                     /*!< The default task priority in ble distribution network*/
#define     AIIO_IOT_BLE_CONFIG_TASK_STACK                              (4 * 1024)              /*!< The default task stack in ble distribution network*/


#define SAMPLE_DEVICE_NAME "l"
#define TENCENT_COMPANY_IDENTIFIER  0xFEE7  // Tencent Company ID, another is 0xFEBA
#define LLSYNC_PROTO_VER_BIT         0x04
#define BLE_QIOT_MAC_LEN         (6)   // fixed length of mac

#define BLE_QIOT_LLSYNC_STANDARD    1   // support llsync standard
#define BLE_QIOT_LLSYNC_PROTOCOL_VERSION (2)  // llsync protocol version, equal or less than 15
#define BLE_QIOT_USER_DEVELOPER_VERSION "0.0.4"
#define LLSYNC_MTU_SET_MASK          0x8000

// advertise manufacture specific data, use advertising type 0xFF
typedef struct {
    uint16_t company_identifier;
    uint8_t *adv_data;
    uint8_t  adv_data_len;
} manufacturer_data_s;

typedef enum {
    E_LLSYNC_BIND_IDLE = 0,  // no bind
    E_LLSYNC_BIND_WAIT,      // wait bind, return idle state if no bind in the period
    E_LLSYNC_BIND_SUCC,      // bound
} e_llsync_bind_state;

/**
 * @brief   The event of ble distribute network, it will inform ble distribution network state by callbacked those event in aiio_ble_config_iot_event_t struction
 */
typedef enum
{
    AIIO_BLE_CONFIG_FAIL,
    AIIO_BLE_DISCONNECED,
    AIIO_BLE_CONFIG_OK
}aiio_ble_config_iot_event_t;


/**
 * @brief   The information of the ble distribution network, it will transfer the ble distribution network data through the callback interface.
 */
typedef struct 
{
    char *entrypt_key;
    aiio_ble_data_t  *ble_data;
    aiio_ble_config_iot_event_t event;
}aiio_ble_iot_data_t;


/**
 * @brief   The configuration information of ble distribution network, it must be configured before ble distribution network starting 
 */
typedef struct 
{
    char *deviceid;
    char *productid;
    char *wifi_flag;    
    int  task_prio;                                                       /*!< The task priority in ble distribution network, it will use default the task priority when the element is not set*/
    int  task_stack;                                                      /*!< The task stack in ble distribution network, it will use default the task stack when the element is not set*/
    void (*aiio_ble_iot_config_cb)(aiio_ble_iot_data_t *ble_data);
}aiio_ble_iot_config_t;

typedef enum {
    E_DEV_MSG_SYNC_TIME = 0,  // sync info before bind
    E_DEV_MSG_CONN_VALID,     // connect request
    E_DEV_MSG_BIND_SUCC,      // inform bind success
    E_DEV_MSG_BIND_FAIL,      // inform bind failed
    E_DEV_MSG_UNBIND,         // unbind request
    E_DEV_MSG_CONN_SUCC,      // inform connect result
    E_DEV_MSG_CONN_FAIL,
    E_DEV_MSG_UNBIND_SUCC,  // inform unbind result
    E_DEV_MSG_UNBIND_FAIL,
    E_DEV_MSG_SET_MTU_RESULT,  // inform set mtu result
    E_DEV_MSG_BIND_TIMEOUT,    // inform bind timeout
    E_DEV_MSG_GET_DEV_INFO = 0xE0,  // configure network start
    E_DEV_MSG_SET_WIFI_MODE,
    E_DEV_MSG_SET_WIFI_INFO,
    E_DEV_MSG_SET_WIFI_CONNECT,
    E_DEV_MSG_SET_WIFI_TOKEN,
    E_DEV_MSG_GET_DEV_LOG,
    E_DEV_MSG_MSG_BUTT,
} e_dev_info_msg_type;

// define message type that from device to server
enum {
    BLE_QIOT_EVENT_UP_PROPERTY_REPORT = 0,
    BLE_QIOT_EVENT_UP_CONTROL_REPLY,
    BLE_QIOT_EVENT_UP_GET_STATUS,
    BLE_QIOT_EVENT_UP_EVENT_POST,
    BLE_QIOT_EVENT_UP_ACTION_REPLY,
    BLE_QIOT_EVENT_UP_BIND_SIGN_RET,
    BLE_QIOT_EVENT_UP_CONN_SIGN_RET,
    BLE_QIOT_EVENT_UP_UNBIND_SIGN_RET,
    BLE_QIOT_EVENT_UP_REPORT_MTU,
    BLE_QIOT_EVENT_UP_REPLY_OTA_REPORT,
    BLE_QIOT_EVENT_UP_REPLY_OTA_DATA,
    BLE_QIOT_EVENT_UP_REPORT_CHECK_RESULT,
    BLE_QIOT_EVENT_UP_SYNC_MTU,
    BLE_QIOT_EVENT_UP_SYNC_WAIT_TIME,
    BLE_QIOT_EVENT_UP_WIFI_MODE = 0xE0,
    BLE_QIOT_EVENT_UP_WIFI_INFO,
    BLE_QIOT_EVENT_UP_WIFI_CONNECT,
    BLE_QIOT_EVENT_UP_WIFI_TOKEN,
    BLE_QIOT_EVENT_UP_WIFI_LOG,
    BLE_QIOT_EVENT_UP_BUTT,
};

// the minimum between BLE_QIOT_EVENT_MAX_SIZE and mtu
#define BLE_QIOT_EVENT_BUF_SIZE (128)
#define SWAP_16(x) ((((x)&0xFF00) >> 8) | (((x)&0x00FF) << 8))
#define HTONS(x) SWAP_16(x)
// 1 byte type + 2 bytes payload-length
#define BLE_QIOT_EVENT_FIXED_HEADER_LEN (3)

typedef struct {
    bool sub_ready;
    bool send_ready;
    bool reply_ready;
    int  reply_code;
} TokenHandleData;

#define MAX_TOKEN_LENGTH              32
#define MAX_TYPE_LENGTH               16
#define DEFAULT_PUB_PARAMS              \
    {                                   \
        QOS0, 0, 0, 0, NULL, 0, NULL, 0 \
    }
typedef struct {
    char token_str[MAX_TOKEN_LENGTH + 4];
    bool token_received;
    struct {
        char     type[MAX_TYPE_LENGTH + 1];
        uint32_t start;
        uint32_t getSSID;
        uint32_t wifiConnected;
        uint32_t getToken;
        uint32_t mqttStart;
        uint32_t mqttConnected;
        uint32_t tokenPublish;
    } pairTime;
} publish_token_info_t;

typedef enum _QoS {
    QOS0 = 0,  // At most once delivery
    QOS1 = 1,  // At least once delivery, PUBACK is required
    QOS2 = 2   // Exactly once delivery. NOT supported currently
} QoS;

/**
 * @brief MQTT message parameter for pub/sub
 */
typedef struct {
    QoS      qos;       // MQTT QoS level
    uint8_t  retained;  // RETAIN flag
    uint8_t  dup;       // DUP flag
    uint16_t id;        // MQTT Id

    const char *ptopic;     // MQTT topic
    size_t      topic_len;  // topic length

    void * payload;      // MQTT msg payload
    size_t payload_len;  // MQTT length of msg payload
} MQTTMessage;

typedef MQTTMessage PublishParams;

typedef enum {
    BLE_WIFI_MODE_NULL = 0,  // invalid mode
    BLE_WIFI_MODE_STA  = 1,  // station
    BLE_WIFI_MODE_AP   = 2,  // ap
} BLE_WIFI_MODE;

typedef enum {
    BLE_WIFI_STATE_CONNECT = 0,  // wifi connect
    BLE_WIFI_STATE_OTHER   = 1,  // other state
} BLE_WIFI_STATE;

enum {
    BLE_QIOT_EVENT_NO_SLICE   = 0,
    BLE_QIOT_EVENT_SLICE_HEAD = 1,
    BLE_QIOT_EVENT_SLICE_BODY = 2,
    BLE_QIOT_EVENT_SLICE_FOOT = 3,
};

/**
 * @brief   Initialize the ble protocol stack and start the ble broadcast
 * 
 * @param[in]   ble_congfig                 The configuration information of ble distribution network
 * 
 * @return  int  Reture all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_ble_config_init_tencent(aiio_ble_iot_config_t *ble_congfig);



/**
 * @brief   Deinitialize ble protocol stack, disconnect ble connection, turn off ble broadcast and release memory resources
 * @return  int  Reture all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_ble_config_deinit_tencent(void);

/**
 * @brief   Report the status of ble distribution network to app by this function when device is in ble distribution network
 * @note    The device needs to report those status that wifi connection, mqtt connection, report activaty to cloud and report online to cloud， which by this function.
 *          And device also must to report some error status when  the ble distribution network stops by abnormity error.
 * 
 * @param[in]   error_code  The error code that needs to response to app. all kind of error code that is defined in this file that is called aiio_protocol_code.h
 * 
 * @return  int  Reture all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_ble_config_response_status_tencent(int8_t error_code);



#endif
