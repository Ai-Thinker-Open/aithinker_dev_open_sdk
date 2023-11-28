/** @brief      AT error code.
 *
 *  @file       aiio_chip_init.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/23      <td>1.0.0       <td>lins        <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_ERRCODE_H__
#define __AIIO_AT_ERRCODE_H__

#define AIIO_AT_ERR_SYS_BASE                             0
#define AIIO_AT_ERR_SYS_SUCCESS                          (AIIO_AT_ERR_SYS_BASE + 0)
#define AIIO_AT_ERR_SYS_CMD_UNSUPPORTED                  (AIIO_AT_ERR_SYS_BASE + 1)
#define AIIO_AT_ERR_SYS_OPT_UNSUPPORTED                  (AIIO_AT_ERR_SYS_BASE + 2)
#define AIIO_AT_ERR_SYS_PARAM_FORMAT_INVALID             (AIIO_AT_ERR_SYS_BASE + 3)
#define AIIO_AT_ERR_SYS_PARAM_INVALID                    (AIIO_AT_ERR_SYS_BASE + 4)
#define AIIO_AT_ERR_SYS_PARAM_TOO_LONG                   (AIIO_AT_ERR_SYS_BASE + 5)
#define AIIO_AT_ERR_SYS_BUSY                             (AIIO_AT_ERR_SYS_BASE + 31)

#define AIIO_AT_ERR_COM_BASE                             32
#define AIIO_AT_ERR_COM_UNKNOWN                          (AIIO_AT_ERR_COM_BASE + 0)
#define AIIO_AT_ERR_COM_MALLOC_FAIL                      (AIIO_AT_ERR_COM_BASE + 1)
#define AIIO_AT_ERR_COM_READ_FAIL                        (AIIO_AT_ERR_COM_BASE + 1)
#define AIIO_AT_ERR_COM_WRITE_FAIL                       (AIIO_AT_ERR_COM_BASE + 2)
#define AIIO_AT_ERR_COM_CONFIG_INVALID                   (AIIO_AT_ERR_COM_BASE + 4)
#define AIIO_AT_ERR_COM_TASK_CREATE_FAIL                 (AIIO_AT_ERR_COM_BASE + 5)
#define AIIO_AT_ERR_COM_FLASH_FAIL                       (AIIO_AT_ERR_COM_BASE + 6)
#define AIIO_AT_ERR_COM_UART_BAUD_RETE_UNSUPPORTED       (AIIO_AT_ERR_COM_BASE + 7)
#define AIIO_AT_ERR_COM_UART_DATA_BITS_UNSUPPORTED       (AIIO_AT_ERR_COM_BASE + 8)
#define AIIO_AT_ERR_COM_UART_STOP_BITS_UNSUPPORTED       (AIIO_AT_ERR_COM_BASE + 9)
#define AIIO_AT_ERR_COM_UART_PARITY_UNSUPPORTED          (AIIO_AT_ERR_COM_BASE + 10)
#define AIIO_AT_ERR_COM_UART_FLOW_CTRL_UNSUPPORTED       (AIIO_AT_ERR_COM_BASE + 11)
#define AIIO_AT_ERR_COM_UART_CONFIG_FAIL                 (AIIO_AT_ERR_COM_BASE + 12)
#define AIIO_AT_ERR_COM_USERNAME_PASSWORD_INVALID        (AIIO_AT_ERR_COM_BASE + 13)
#define AIIO_AT_ERR_COM_LOW_POWER_INVALID                (AIIO_AT_ERR_COM_BASE + 14)
#define AIIO_AT_ERR_COM_RET_CONFIG_NOT_SET               (AIIO_AT_ERR_COM_BASE + 15)
#define AIIO_AT_ERR_COM_COMMON_INVALID                   (AIIO_AT_ERR_COM_BASE + 31)

#define AIIO_AT_ERR_WIFI_BASE                            64
#define AIIO_AT_ERR_WIFI_INIT_FAIL                       (AIIO_AT_ERR_WIFI_BASE + 0)
#define AIIO_AT_ERR_WIFI_MODE_INVAILD                    (AIIO_AT_ERR_WIFI_BASE + 1)
#define AIIO_AT_ERR_WIFI_CONNECT_FAIL                    (AIIO_AT_ERR_WIFI_BASE + 2)
#define AIIO_AT_ERR_WIFI_DHCP_FAIL                       (AIIO_AT_ERR_WIFI_BASE + 3)
#define AIIO_AT_ERR_WIFI_GET_AUTH_FAIL                   (AIIO_AT_ERR_WIFI_BASE + 4)
#define AIIO_AT_ERR_WIFI_SACN_NO_AP                      (AIIO_AT_ERR_WIFI_BASE + 5)
#define AIIO_AT_ERR_WIFI_SCAN_START_FAIL                 (AIIO_AT_ERR_WIFI_BASE + 6)
#define AIIO_AT_ERR_WIFI_SCAN_TIMEOUT                    (AIIO_AT_ERR_WIFI_BASE + 7)
#define AIIO_AT_ERR_WIFI_AP_START_FAIL                   (AIIO_AT_ERR_WIFI_BASE + 8)
#define AIIO_AT_ERR_WIFI_GET_INFO_FAIL                   (AIIO_AT_ERR_WIFI_BASE + 9)
#define AIIO_AT_ERR_WIFI_NETWORK_INTERFACE_NOT_RUNNING   (AIIO_AT_ERR_WIFI_BASE + 10)
#define AIIO_AT_ERR_WIFI_COUNTRY_CODE_INVALID            (AIIO_AT_ERR_WIFI_BASE + 11)
#define AIIO_AT_ERR_WIFI_NETCONFIG_INVALID               (AIIO_AT_ERR_WIFI_BASE + 12)
#define AIIO_AT_ERR_WIFI_UNKNOWN                         (AIIO_AT_ERR_WIFI_BASE + 31)

#define AIIO_AT_ERR_SOCKET_BASE                          96
#define AIIO_AT_ERR_SOCKET_CREATE_FAIL                   (AIIO_AT_ERR_SOCKET_BASE + 0)
#define AIIO_AT_ERR_SOCKET_CONNECT_FAIL                  (AIIO_AT_ERR_SOCKET_BASE + 1)
#define AIIO_AT_ERR_SOCKET_DNS_FAIL                      (AIIO_AT_ERR_SOCKET_BASE + 2)
#define AIIO_AT_ERR_SOCKET_STATE_INVALID                 (AIIO_AT_ERR_SOCKET_BASE + 3)
#define AIIO_AT_ERR_SOCKET_TYPE_INVALID                  (AIIO_AT_ERR_SOCKET_BASE + 4)
#define AIIO_AT_ERR_SOCKET_SEND_FAIL                     (AIIO_AT_ERR_SOCKET_BASE + 5)
#define AIIO_AT_ERR_SOCKET_RECV_FAIL                     (AIIO_AT_ERR_SOCKET_BASE + 6)
#define AIIO_AT_ERR_SOCKET_TASK_CREATE_FAIL              (AIIO_AT_ERR_SOCKET_BASE + 7)
#define AIIO_AT_ERR_SOCKET_BIND_FAIL                     (AIIO_AT_ERR_SOCKET_BASE + 8)
#define AIIO_AT_ERR_SOCKET_TT_ENTER_FAIL                 (AIIO_AT_ERR_SOCKET_BASE + 9)
#define AIIO_AT_ERR_SOCKET_PING_FAIL                     (AIIO_AT_ERR_SOCKET_BASE + 10)
#define AIIO_AT_ERR_SOCKET_PING_LOSS                     (AIIO_AT_ERR_SOCKET_BASE + 11)
#define AIIO_AT_ERR_SOCKET_SSL_CONFIG_FAIL               (AIIO_AT_ERR_SOCKET_BASE + 12)
#define AIIO_AT_ERR_SOCKET_SSL_VERIFY_FAIL               (AIIO_AT_ERR_SOCKET_BASE + 13)
#define AIIO_AT_ERR_SOCKET_CONID_FAIL                    (AIIO_AT_ERR_SOCKET_BASE + 14)
#define AIIO_AT_ERR_SOCKET_UNKNOWN                      (AIIO_AT_ERR_SOCKET_BASE + 31)

#define AIIO_AT_ERR_GPIO_BASE                            128
#define AIIO_AT_ERR_GPIO_MODE_INVALID                    (AIIO_AT_ERR_GPIO_BASE + 0)
#define AIIO_AT_ERR_GPIO_IOMAP_INVALID                   (AIIO_AT_ERR_GPIO_BASE + 1)
#define AIIO_AT_ERR_GPIO_NC                              (AIIO_AT_ERR_GPIO_BASE + 2)
#define AIIO_AT_ERR_GPIO_MODE_SET_FAIL                   (AIIO_AT_ERR_GPIO_BASE + 3)
#define AIIO_AT_ERR_GPIO_PULL_MODE_INVALID               (AIIO_AT_ERR_GPIO_BASE + 4)
#define AIIO_AT_ERR_GPIO_PWM_PERIOD_UNSUPPORTED          (AIIO_AT_ERR_GPIO_BASE + 5)
#define AIIO_AT_ERR_GPIO_UNKNOWN                         (AIIO_AT_ERR_GPIO_BASE + 31)

#define AIIO_AT_ERR_HTTP_BASE                            160
#define AIIO_AT_ERR_HTTP_RESP_HEAD_INVALID               (AIIO_AT_ERR_HTTP_BASE + 0)
#define AIIO_AT_ERR_HTTP_UNKNOWN                         (AIIO_AT_ERR_HTTP_BASE + 31)

#define AIIO_AT_ERR_MQTT_BASE                            192
#define AIIO_AT_ERR_MQTT_MODE_INVALID                    (AIIO_AT_ERR_MQTT_BASE + 0)
#define AIIO_AT_ERR_MQTT_CONNECT_FAIL                    (AIIO_AT_ERR_MQTT_BASE + 1)
#define AIIO_AT_ERR_MQTT_CONFIG_FAIL                     (AIIO_AT_ERR_MQTT_BASE + 2)
#define AIIO_AT_ERR_MQTT_PUB_FAIL                        (AIIO_AT_ERR_MQTT_BASE + 3)
#define AIIO_AT_ERR_MQTT_SUB_LIST_FULL                   (AIIO_AT_ERR_MQTT_BASE + 4)
#define AIIO_AT_ERR_MQTT_NO_RESP_RECEIVED                (AIIO_AT_ERR_MQTT_BASE + 6)
#define AIIO_AT_ERR_MQTT_TIMEOUT                         (AIIO_AT_ERR_MQTT_BASE + 7)
#define AIIO_AT_ERR_MQTT_SUB_FAIL                        (AIIO_AT_ERR_MQTT_BASE + 8)
#define AIIO_AT_ERR_MQTT_UNSUB_FAIL                      (AIIO_AT_ERR_MQTT_BASE + 9)
#define AIIO_AT_ERR_MQTT_UNKNOWN                         (AIIO_AT_ERR_MQTT_BASE + 32)

#define AIIO_AT_ERR_BLE_BASE                             225
#define AIIO_AT_ERR_BLE_INIT_DEINIT_FAIL                 (AIIO_AT_ERR_BLE_BASE + 0)
#define AIIO_AT_ERR_BLE_MAC_SET_FAIL                     (AIIO_AT_ERR_BLE_BASE + 1)
#define AIIO_AT_ERR_BLE_STATE_INVALID                    (AIIO_AT_ERR_BLE_BASE + 2)
#define AIIO_AT_ERR_BLE_DISCONNECT_FAIL                  (AIIO_AT_ERR_BLE_BASE + 3)
#define AIIO_AT_ERR_BLE_MTU_SET_FAIL                     (AIIO_AT_ERR_BLE_BASE + 4)
#define AIIO_AT_ERR_BLE_SEND_FAIL                        (AIIO_AT_ERR_BLE_BASE + 5)
#define AIIO_AT_ERR_BLE_ADV_DATA_FAIL                    (AIIO_AT_ERR_BLE_BASE + 6)
#define AIIO_AT_ERR_BLE_ADV_STATE_FAIL                   (AIIO_AT_ERR_BLE_BASE + 7)
#define AIIO_AT_ERR_BLE_SCAN_NO_TARGET                   (AIIO_AT_ERR_BLE_BASE + 8)
#define AIIO_AT_ERR_BLE_CONNECT_FAIL                     (AIIO_AT_ERR_BLE_BASE + 9)
#define AIIO_AT_ERR_BLE_MTU_GET_FAIL                     (AIIO_AT_ERR_BLE_BASE + 10)
#define AIIO_AT_ERR_BLE_MAC_GET_FAIL                     (AIIO_AT_ERR_BLE_BASE + 11)
#define AIIO_AT_ERR_BLE_UNKNOWN                          (AIIO_AT_ERR_BLE_BASE + 30)

#endif
