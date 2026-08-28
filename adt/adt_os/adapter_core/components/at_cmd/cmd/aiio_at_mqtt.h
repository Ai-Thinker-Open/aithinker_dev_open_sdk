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
#include "aiio_at_basic.h"

/**
 * @brief MQTT AT 命令参数类型枚举
 *
 * 用于区分不同配置项的类型标识
 */
typedef enum{

    AIIO_AT_MQTT_HOST_TYPE = 1,       /**< MQTT服务器地址 */
    AIIO_AT_MQTT_PORT_TYPE = 2,       /**< MQTT服务器端口 */
    AIIO_AT_MQTT_SCHEME_TYPE = 3,     /**< 连接方式（TCP/SSL） */
    AIIO_AT_MQTT_CLIENT_ID_TYPE = 4,  /**< 客户端ID */
    AIIO_AT_MQTT_USERNAME_TYPE = 5,   /**< 用户名 */
    AIIO_AT_MQTT_PASSWORD_TYPE = 6,   /**< 密码 */
    AIIO_AT_MQTT_LWT_TYPE = 7,        /**< 遗嘱消息配置 */
    AIIO_AT_MQTT_KEEP_ALIVE_TYPE = 8, /**< 心跳时间 */

} aiio_at_mqtt_keytype_t;


/**
 * @brief MQTT QoS 等级定义
 *
 * @note QoS（Quality of Service）用于定义消息传输可靠性
 */
typedef enum{

    AIIO_AT_MQTT_QOS_LEVEL_0 = 0, /**< 最多一次（At most once） */
    AIIO_AT_MQTT_QOS_LEVEL_1 = 1, /**< 至少一次（At least once） */
    AIIO_AT_MQTT_QOS_LEVEL_2 = 2, /**< 仅一次（Exactly once） */

} aiio_at_mqtt_qos_level_t;


/**
 * @brief MQTT 连接方式
 */
typedef enum{

    AIIO_AT_MQTT_SCHEME_TCP = 1, /**< 使用 TCP 连接 */
    AIIO_AT_MQTT_SCHEME_SSL = 2, /**< 使用 SSL/TLS 连接 */

} aiio_at_mqtt_scheme_type_t;


/**
 * @brief MQTT Retained（保留消息）标志
 *
 * @note
 * - Retained = 0：服务器不会保存该消息
 * - Retained = 1：服务器会保存该消息，新订阅者可立即收到最后一条消息
 */
typedef enum{

    /**
     * @brief 不保留消息
     *
     * 重连或新订阅时不会收到历史消息
     */
    AIIO_AT_MQTT_RETAIN_FALSE = 0,

    /**
     * @brief 保留消息
     *
     * 重连或新订阅时会收到该主题最后一条消息
     */
    AIIO_AT_MQTT_RETAIN_TURE = 1,

} aiio_at_mqtt_retained_t;


/**
 * @brief 注册 MQTT AT 命令
 *
 * @details
 * 将 MQTT 相关 AT 指令注册到 AT 命令框架中，
 * 调用后系统即可解析和执行 MQTT 相关 AT 指令。
 *
 * @return int32_t
 * @retval 0      注册成功
 * @retval 非0    注册失败
 */
CORE_API int32_t aiio_at_mqtt_cmd_regist(void);


#endif
