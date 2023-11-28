/** @brief      AT mqtt command.
 *
 *  @file       aiio_at_mqtt.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/03/01      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_MQTT_H__
#define __AIIO_AT_MQTT_H__
#include <stdint.h>
#include "aiio_type.h"

typedef enum{
    AIIO_AT_MQTT_HOST_TYPE=1,
    AIIO_AT_MQTT_PORT_TYPE=2,
    AIIO_AT_MQTT_SCHEME_TYPE=3,
    AIIO_AT_MQTT_CLIENT_ID_TYPE=4,
    AIIO_AT_MQTT_USERNAME_TYPE=5,
    AIIO_AT_MQTT_PASSWORD_TYPE=6,
    AIIO_AT_MQTT_LWT_TYPE=7,
}aiio_at_mqtt_keytype_t;


typedef enum{
    AIIO_AT_MQTT_QOS_LEVEL_0=0,     //At most once
    AIIO_AT_MQTT_QOS_LEVEL_1=1,     //At least one time
    AIIO_AT_MQTT_QOS_LEVEL_2=2,     //Only once
}aiio_at_mqtt_qos_level_t;

typedef enum{
    AIIO_AT_MQTT_SCHEME_TCP=1,      //TCP
    AIIO_AT_MQTT_SCHEME_SSL=2,      //SSL
}aiio_at_mqtt_scheme_type_t;

typedef enum{
    /*When you reconnect to the MQTT service,
    you do not need to receive the latest messages from this topic. retained to false.*/
    AIIO_AT_MQTT_RETAIN_FALSE=0,

    /*When you reconnect to the MQTT service,
    you need to receive the latest messages from this topic. Set to true to retained*/
    AIIO_AT_MQTT_RETAIN_TURE=1,
}aiio_at_mqtt_retained_t;

CORE_API int32_t aiio_at_mqtt_cmd_regist(void);

#endif
