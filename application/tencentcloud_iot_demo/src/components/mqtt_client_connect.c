/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or
 *initial documentation
 *******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>
#include <string.h>

#include "utils_param_check.h"
#include "utils_hmac.h"
#include "aiio_common.h"
#include "qcloud_iot_export_error.h"
#include "aiio_adapter_include.h"

#define MQTT_CONNECT_FLAG_USERNAME    0x80
#define MQTT_CONNECT_FLAG_PASSWORD    0x40
#define MQTT_CONNECT_FLAG_WILL_RETAIN 0x20
#define MQTT_CONNECT_FLAG_WILL_QOS2   0x18
#define MQTT_CONNECT_FLAG_WILL_QOS1   0x08
#define MQTT_CONNECT_FLAG_WILL_QOS0   0x00
#define MQTT_CONNECT_FLAG_WILL_FLAG   0x04
#define MQTT_CONNECT_FLAG_CLEAN_SES   0x02

#define MQTT_CONNACK_FLAG_SES_PRE 0x01

/**
 * Connect return code
 */
typedef enum {
    CONNACK_CONNECTION_ACCEPTED                 = 0,  // connection accepted
    CONANCK_UNACCEPTABLE_PROTOCOL_VERSION_ERROR = 1,  // connection refused: unaccpeted protocol verison
    CONNACK_IDENTIFIER_REJECTED_ERROR           = 2,  // connection refused: identifier rejected
    CONNACK_SERVER_UNAVAILABLE_ERROR            = 3,  // connection refused: server unavailable
    CONNACK_BAD_USERDATA_ERROR                  = 4,  // connection refused: bad user name or password
    CONNACK_NOT_AUTHORIZED_ERROR                = 5   // connection refused: not authorized
} MQTTConnackReturnCodes;


static void _copy_connect_params(MQTTConnectParams *destination, MQTTConnectParams *source)
{
    POINTER_SANITY_CHECK_RTN(destination);
    POINTER_SANITY_CHECK_RTN(source);

    /* In case of reconnecting, source == destination */
    if (source == destination) {
        return;
    }

    destination->mqtt_version        = source->mqtt_version;
    destination->client_id           = source->client_id;
    destination->username            = source->username;
    destination->keep_alive_interval = source->keep_alive_interval;
    destination->clean_session       = source->clean_session;
    destination->auto_connect_enable = source->auto_connect_enable;
#ifdef AUTH_WITH_NOTLS
    destination->device_secret     = source->device_secret;
    destination->device_secret_len = source->device_secret_len;
#endif
}

/**
 * Serializes the connect options into the buffer.
 * @param buf the buffer into which the packet will be serialized
 * @param len the length in bytes of the supplied buffer
 * @param options the options to be used to build the connect packet
 * @param serialized length
 * @return int indicating function execution status
 */
static int _serialize_connect_packet(unsigned char *buf, size_t buf_len, MQTTConnectParams *options,
                                     uint32_t *serialized_len)
{
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(options, QCLOUD_ERR_INVAL);
    STRING_PTR_SANITY_CHECK(options->client_id, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(serialized_len, QCLOUD_ERR_INVAL);

    unsigned char *ptr     = buf;
    unsigned char  header  = 0;
    unsigned char  flags   = 0;
    uint32_t       rem_len = 0;
    int            rc;

    long cur_timesec = HAL_Timer_current_sec() + MAX_ACCESS_EXPIRE_TIMEOUT / 1000;
    if (cur_timesec <= 0 || MAX_ACCESS_EXPIRE_TIMEOUT <= 0) {
        cur_timesec = LONG_MAX;
    }

    // 20 for timestampe length & delimiter
    int username_len  = strlen(options->client_id) + QCLOUD_IOT_DEVICE_SDK_APPID_LEN + MAX_CONN_ID_LEN + 20;
    options->username = (char *)malloc(username_len);
    if (options->username == NULL) {
        aiio_log_i("malloc username failed!");
        rc = QCLOUD_ERR_MALLOC;
        goto err_exit;
    }

    get_next_conn_id(options->conn_id);
    snprintf(options->username, username_len, "%s;%s;%s;%ld", options->client_id, QCLOUD_IOT_DEVICE_SDK_APPID,
                 options->conn_id, cur_timesec);

#if defined(AUTH_WITH_NOTLS) && defined(AUTH_MODE_KEY)
    if (options->device_secret != NULL && options->username != NULL) {
        char sign[41] = {0};
        utils_hmac_sha1(options->username, strlen(options->username), sign, options->device_secret,
                        options->device_secret_len);
        options->password = (char *)malloc(51);
        if (options->password == NULL) {
            aiio_log_i("malloc password failed!");
            rc = QCLOUD_ERR_MALLOC;
            goto err_exit;
        }
        snprintf(options->password, 51, "%s;hmacsha1", sign);
        aiio_log_i("222 @@@@ options->client_id:%s\r\n",options->client_id);
        aiio_log_i("222 @@@@ options->username:%s\r\n",options->username);
        aiio_log_i("222 @@@@ options->password:%s\r\n",options->password);
        aiio_log_i("222 @@@@ options->device_secret:%s\r\n",options->device_secret);

        mqtt_cfg.client_id = options->client_id;
        mqtt_cfg.username = options->username;
        mqtt_cfg.password = options->password;
    }
#endif

//     rem_len = _get_packet_connect_rem_len(options);
//     if (get_mqtt_packet_len(rem_len) > buf_len) {
//         Log_e("get_mqtt_packet_len failed!");
//         rc = QCLOUD_ERR_BUF_TOO_SHORT;
//         goto err_exit;
//     }

//     rc = mqtt_init_packet_header(&header, CONNECT, QOS0, 0, 0);
//     if (QCLOUD_RET_SUCCESS != rc) {
//         Log_e("mqtt_init_packet_header failed!");
//         goto err_exit;
//     }

//     // 1st byte in fixed header
//     mqtt_write_char(&ptr, header);

//     // remaining length
//     ptr += mqtt_write_packet_rem_len(ptr, rem_len);

//     // MQTT protocol name and version in variable header
//     if (4 == options->mqtt_version) {
//         mqtt_write_utf8_string(&ptr, "MQTT");
//         mqtt_write_char(&ptr, (unsigned char)4);
//     } else {
//         mqtt_write_utf8_string(&ptr, "MQIsdp");
//         mqtt_write_char(&ptr, (unsigned char)3);
//     }

//     // flags in variable header
//     flags |= (options->clean_session) ? MQTT_CONNECT_FLAG_CLEAN_SES : 0;
//     flags |= (options->username != NULL) ? MQTT_CONNECT_FLAG_USERNAME : 0;

// #if defined(AUTH_WITH_NOTLS) && defined(AUTH_MODE_KEY)
//     flags |= MQTT_CONNECT_FLAG_PASSWORD;
// #endif

//     mqtt_write_char(&ptr, flags);

//     // keep alive interval (unit:ms) in variable header
//     mqtt_write_uint_16(&ptr, options->keep_alive_interval);

//     // client id
//     mqtt_write_utf8_string(&ptr, options->client_id);

//     if ((flags & MQTT_CONNECT_FLAG_USERNAME) && options->username != NULL) {
//         mqtt_write_utf8_string(&ptr, options->username);
//         HAL_Free(options->username);
//         options->username = NULL;
//     }

//     if ((flags & MQTT_CONNECT_FLAG_PASSWORD) && options->password != NULL) {
//         mqtt_write_utf8_string(&ptr, options->password);
//         HAL_Free(options->password);
//         options->password = NULL;
//     }

//     *serialized_len = (uint32_t)(ptr - buf);

    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);

err_exit:
    free(options->username);
    options->username = NULL;

    free(options->password);
    options->password = NULL;

    IOT_FUNC_EXIT_RC(rc);
}

/**
 * @brief Setup connection with MQTT server
 *
 * @param pClient
 * @param options
 * @return
 */
static int _mqtt_connect(Qcloud_IoT_Client *pClient, MQTTConnectParams *options)
{
    IOT_FUNC_ENTRY;

    // Timer    connect_timer;
    // int      connack_rc = QCLOUD_ERR_FAILURE, rc = QCLOUD_ERR_FAILURE;
    // uint8_t  sessionPresent = 0;
    uint32_t len            = 0;

    // InitTimer(&connect_timer);
    // countdown_ms(&connect_timer, pClient->command_timeout_ms);

    if (NULL != options) {
        _copy_connect_params(&(pClient->options), options);
    }

    // // TCP or TLS network connect
    // rc = pClient->network_stack.connect(&(pClient->network_stack));
    // if (QCLOUD_RET_SUCCESS != rc) {
    //     IOT_FUNC_EXIT_RC(rc);
    // }

    // HAL_MutexLock(pClient->lock_write_buf);
    // serialize CONNECT packet
    _serialize_connect_packet(pClient->write_buf, pClient->write_buf_size, &(pClient->options), &len);
    // if (QCLOUD_RET_SUCCESS != rc || 0 == len) {
    //     HAL_MutexUnlock(pClient->lock_write_buf);
    //     IOT_FUNC_EXIT_RC(rc);
    // }

    // send CONNECT packet
    // rc = send_mqtt_packet(pClient, len, &connect_timer);
    // HAL_MutexUnlock(pClient->lock_write_buf);
    // if (QCLOUD_RET_SUCCESS != rc) {
    //     IOT_FUNC_EXIT_RC(rc);
    // }

    // // wait for CONNACK
    // rc = wait_for_read(pClient, CONNACK, &connect_timer, QOS0);
    // if (QCLOUD_RET_SUCCESS != rc) {
    //     IOT_FUNC_EXIT_RC(rc);
    // }

    // // deserialize CONNACK and check reture code
    // rc = _deserialize_connack_packet(&sessionPresent, &connack_rc, pClient->read_buf, pClient->read_buf_size);
    // if (QCLOUD_RET_SUCCESS != rc) {
    //     IOT_FUNC_EXIT_RC(rc);
    // }

    // if (QCLOUD_RET_MQTT_CONNACK_CONNECTION_ACCEPTED != connack_rc) {
    //     IOT_FUNC_EXIT_RC(connack_rc);
    // }

    // set_client_conn_state(pClient, CONNECTED);
    // HAL_MutexLock(pClient->lock_generic);
    // pClient->was_manually_disconnected = 0;
    // pClient->is_ping_outstanding       = 0;
    // countdown(&pClient->ping_timer, pClient->options.keep_alive_interval);
    // HAL_MutexUnlock(pClient->lock_generic);

    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
}

int qcloud_iot_mqtt_connect(Qcloud_IoT_Client *pClient, MQTTConnectParams *pParams)
{
    // IOT_FUNC_ENTRY;
    int rc;
    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(pParams, QCLOUD_ERR_INVAL);

    // check connection state first
    // if (get_client_conn_state(pClient)) {
    //     IOT_FUNC_EXIT_RC(QCLOUD_RET_MQTT_ALREADY_CONNECTED);
    // }

    rc = _mqtt_connect(pClient, pParams);

    // disconnect network if connect fail
    // if (rc != QCLOUD_RET_SUCCESS) {
    //     pClient->network_stack.disconnect(&(pClient->network_stack));
    // }

    IOT_FUNC_EXIT_RC(rc);
}


