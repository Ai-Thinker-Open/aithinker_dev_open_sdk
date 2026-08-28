/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __ALI_IOT_EXPORTS_STATE_H__
#define __ALI_IOT_EXPORTS_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

/****************** SDK state code: used for device diagnosis *********************/

#define STATE_BASE                                  (0x0000)
#define STATE_CODE_SYSTEM_RESET                     (STATE_BASE + 0x0001)
#define STATE_CODE_ENTER_FACTORY_MODE               (STATE_BASE + 0x0002)

/* General: 0x0000 ~ 0x00FF */
/* API works as expected and returns sucess */
/* API 接口按预期完成工作, 并返回成功 */
#define STATE_SUCCESS                               (STATE_BASE - 0x0000)
#define STATE_FAIL                                  (STATE_BASE - 0x0001)
#define SUB_ERRCODE_AWSS_SSID_PWD_PARSE_FAIL        (STATE_BASE - 0x0002)
#define SUB_ERRCODE_AWSS_SSID_PWD_GET_TIMEOUT       (STATE_BASE - 0x0003)

/* User Input: 0x0100 ~ 0x01FF */
#define SUB_ERRCODE_USER_INPUT_BASE                       (-0x0100)

/* User input parameters contain unacceptable NULL pointer */
/* 用户传递给API的参数中含有不合理的空指针参数 */
#define SUB_ERRCODE_USER_INPUT_NULL_POINTER               (SUB_ERRCODE_USER_INPUT_BASE - 0x0001)
/* Some user input parameter(s) has value out of acceptable range */
/* 用户传递给API的参数中含有超出合理范围的取值 */
#define SUB_ERRCODE_USER_INPUT_OUT_RANGE                  (SUB_ERRCODE_USER_INPUT_BASE - 0x0002)
/* User input parameters contain unacceptable productKey */
/* 用户传递给API的参数中含有不合法的产品标识(productKey) */
#define SUB_ERRCODE_USER_INPUT_PK                         (SUB_ERRCODE_USER_INPUT_BASE - 0x0003)
/* User input parameters contain unacceptable productSecret */
/* 用户传递给API的参数中含有不合法的产品密钥(productSecret) */
#define SUB_ERRCODE_USER_INPUT_PS                         (SUB_ERRCODE_USER_INPUT_BASE - 0x0004)
/* User input parameters contain unacceptable deviceName */
/* 用户传递给API的参数中含有不合法的设备名称(deviceName) */
#define SUB_ERRCODE_USER_INPUT_DN                         (SUB_ERRCODE_USER_INPUT_BASE - 0x0005)
/* User input parameters contain unacceptable deviceSecret */
/* 用户传递给API的参数中含有不合法的设备密钥(deviceSecret) */
#define SUB_ERRCODE_USER_INPUT_DS                         (SUB_ERRCODE_USER_INPUT_BASE - 0x0006)
/* User input parameters contain unacceptable HTTP domain name */
/* 用户传递给API的参数中含有不合理的域名, SDK无法对其完成HTTP连接 */
#define SUB_ERRCODE_USER_INPUT_HTTP_DOMAIN                (SUB_ERRCODE_USER_INPUT_BASE - 0x0007)
/* User input parameters contain unacceptable MQTT domain name */
/* 用户传递给API的参数中含有不合理的域名, SDK无法对其完成MQTT连接 */
#define SUB_ERRCODE_USER_INPUT_MQTT_DOMAIN                (SUB_ERRCODE_USER_INPUT_BASE - 0x0008)


/* System: 0x0200 ~ 0x02FF */
#define SUB_ERRCODE_SYS_DEPEND_BASE                       (-0x0200)

/* SDK run into exception when invoking HAL_Malloc() */
/* SDK调用的系统适配接口 HAL_Malloc() 返回异常, 未能成功分配内存 */
#define SUB_ERRCODE_SYS_DEPEND_MALLOC                     (SUB_ERRCODE_SYS_DEPEND_BASE - 0x0001)
/* SDK run into exception when invoking HAL_Kv_Get() */
/* SDK调用的系统适配接口 HAL_Kv_Get() 返回异常, 未能成功根据Key获取Value */
#define SUB_ERRCODE_SYS_DEPEND_KV_GET                     (SUB_ERRCODE_SYS_DEPEND_BASE - 0x0002)
/* SDK run into exception when invoking HAL_Kv_Set() */
/* SDK调用的系统适配接口 HAL_Kv_Set() 返回异常, 未能成功根据Key写入Value */
#define SUB_ERRCODE_SYS_DEPEND_KV_SET                     (SUB_ERRCODE_SYS_DEPEND_BASE - 0x0003)
/* SDK run into exception when invoking HAL_Kv_Del() */
/* SDK调用的系统适配接口 HAL_Kv_Del() 返回异常, 未能成功根据Key删除KV对 */
#define SUB_ERRCODE_SYS_DEPEND_KV_DELETE                  (SUB_ERRCODE_SYS_DEPEND_BASE - 0x0004)
/* SDK run into exception when invoking HAL_MutexCreate() */
/* SDK调用的系统适配接口 HAL_MutexCreate() 返回异常, 未能创建一个互斥锁 */
#define SUB_ERRCODE_SYS_DEPEND_MUTEX_CREATE               (SUB_ERRCODE_SYS_DEPEND_BASE - 0x0005)
/* SDK run into exception when invoking HAL_MutexLock() */
/* SDK调用的系统适配接口 HAL_MutexLock() 返回异常, 未能成功申请互斥锁 */
#define SUB_ERRCODE_SYS_DEPEND_MUTEX_LOCK                 (SUB_ERRCODE_SYS_DEPEND_BASE - 0x0006)
/* SDK run into exception when invoking HAL_MutexUnlock() */
/* SDK调用的系统适配接口 HAL_MutexUnlock() 返回异常, 未能成功释放互斥锁 */
#define SUB_ERRCODE_SYS_DEPEND_MUTEX_UNLOCK               (SUB_ERRCODE_SYS_DEPEND_BASE - 0x0007)

#define SUB_ERRCODE_SYS_DEPEND_SEMAPHORE_CREATE           (SUB_ERRCODE_SYS_DEPEND_BASE - 0x000C)
/* SDK run into exception when invoking HAL_SemaphoreWait() */
/* SDK调用的系统适配接口 HAL_SemaphoreWait() 返回异常, 未能成功在信号量上睡眠 */
#define SUB_ERRCODE_SYS_DEPEND_SEMAPHORE_WAIT             (SUB_ERRCODE_SYS_DEPEND_BASE - 0x000D)

/* SDK run into exception when invoking HAL_Timer_Create() */
/* SDK调用的系统适配接口 HAL_Timer_Create() 返回异常, 未能成功创建timer */
#define SUB_ERRCODE_SYS_DEPEND_TIMER_CREATE               (SUB_ERRCODE_SYS_DEPEND_BASE - 0x000E)
#define SUB_ERRCODE_SYS_DEPEND_TIMER_START                (SUB_ERRCODE_SYS_DEPEND_BASE - 0x000F)
#define SUB_ERRCODE_SYS_DEPEND_TIMER_STOP                 (SUB_ERRCODE_SYS_DEPEND_BASE - 0x0010)
#define SUB_ERRCODE_SYS_DEPEND_TIMER_DELETE               (SUB_ERRCODE_SYS_DEPEND_BASE - 0x0011)

#define SUB_ERRCODE_SYS_DEPEND_THREAD_CREATE              (SUB_ERRCODE_SYS_DEPEND_BASE - 0x0012)

/* System: 0x0200 ~ 0x02FF */

/* MQTT: 0x0300 ~ 0x03FF */
#define SUB_ERRCODE_MQTT_BASE                             (-0x0300)

/* Deserialized CONNACK from MQTT server says protocol version is unacceptable */
/* 设备发出MQTT建连请求后, 服务器应答的报文表示不能接受请求中的MQTT协议版本 */
#define SUB_ERRCODE_MQTT_CONNACK_VERSION_UNACCEPT         (SUB_ERRCODE_MQTT_BASE - 0x0001)
/* Deserialized CONNACK from MQTT server says identifier is rejected */
/* 设备发出MQTT建连请求后, 服务器应答的报文表示不能接受请求中的设备标识符 */
#define SUB_ERRCODE_MQTT_CONNACK_IDENT_REJECT             (SUB_ERRCODE_MQTT_BASE - 0x0002)
/* Deserialized CONNACK from MQTT server says service is not available */
/* 设备发出MQTT建连请求后, 服务器应答的报文表示当前MQTT服务不可用 */
#define SUB_ERRCODE_MQTT_CONNACK_SERVICE_NA               (SUB_ERRCODE_MQTT_BASE - 0x0003)
/* Deserialized CONNACK from MQTT server says it failed to authorize */
/* 设备发出MQTT建连请求后, 服务器应答的报文表示当前对请求设备未能完成鉴权 */
#define SUB_ERRCODE_MQTT_CONNACK_NOT_AUTHORIZED           (SUB_ERRCODE_MQTT_BASE - 0x0004)
/* Deserialized CONNACK from MQTT server says username/password is invalid */
/* 设备发出MQTT建连请求后, 服务器应答的报文表示对设备身份鉴权不通过 */
#define SUB_ERRCODE_MQTT_CONNACK_BAD_USERDATA             (SUB_ERRCODE_MQTT_BASE - 0x0005)
/* MQTT init fail */
/* MQTT调用的TCP或TLS建连过程中，证书过期验证失败 */
#define SUB_ERRCODE_MQTT_INIT_FAIL                        (SUB_ERRCODE_MQTT_BASE - 0x0006)
/* TCP or TLS connection certificate is expired */
/* MQTT调用的TCP或TLS建连过程中，证书过期验证失败 */
#define SUB_ERRCODE_MQTT_CERT_VERIFY_FAIL                 (SUB_ERRCODE_MQTT_BASE - 0x0007)
/* TCP or TLS connection network error occured */
/* MQTT调用的TCP或TLS建连过程中，底层网络出错 */
#define SUB_ERRCODE_MQTT_NETWORK_CONNECT_ERROR            (SUB_ERRCODE_MQTT_BASE - 0x0008)
/* MQTT connection packet serilize fail or connect packet send fail */
/* MQTT调用的TCP或TLS建连过程中，底层网络发送连接包失败 */
#define SUB_ERRCODE_MQTT_CONNECT_PKT_SEND_FAIL            (SUB_ERRCODE_MQTT_BASE - 0x0009)
/* MQTT connection failed, because of unknown error */
/* MQTT调用的TCP或TLS建连过程中，出现未知错误导致MQTT建连失败 */
#define SUB_ERRCODE_MQTT_CONNECT_UNKNOWN_FAIL             (SUB_ERRCODE_MQTT_BASE - 0x000A)


/* MQTT: 0x0300 ~ 0x03FF */
#define STATE_CODE_MQTT_BASE                              (0x0300)
#define STATE_CODE_MQTT_CONNECT                           (STATE_CODE_MQTT_BASE + 0x0001)
#define STATE_CODE_MQTT_CONNECT_SUCCESS                   (STATE_CODE_MQTT_BASE + 0x0002)
#define STATE_CODE_MQTT_CONNECT_FAIL                      (STATE_CODE_MQTT_BASE + 0x0003)
#define STATE_CODE_MQTT_RECONNECT                         (STATE_CODE_MQTT_BASE + 0x0004)
#define STATE_CODE_MQTT_RECONNECT_SUCCESS                 (STATE_CODE_MQTT_BASE + 0x0005)
#define STATE_CODE_MQTT_RECONNECT_FAIL                    (STATE_CODE_MQTT_BASE + 0x0006)

/* WiFi Provision: 0x0400 ~ 0x04FF */
#define SUB_ERRCODE_WIFI_BASE                             (-0x0400)
/* Should not start new round WiFi provision since last round in progress */
/* 不应启动新的一轮WiFi配网, 因为上一轮仍未结束 */
#define SUB_ERRCODE_WIFI_IN_PROGRESS                      (SUB_ERRCODE_WIFI_BASE - 0x0001)
/* WiFi provision ran into mismatch CRC */
/* WiFi配网中发现报文的校验和不符合预期 */
#define SUB_ERRCODE_WIFI_CRC_ERROR                        (SUB_ERRCODE_WIFI_BASE - 0x0002)
/* WiFi provision failed to decrypt WiFi password */
/* WiFi配网中解密WiFi热点的密码失败 */
#define SUB_ERRCODE_WIFI_PASSWD_DECODE_FAILED             (SUB_ERRCODE_WIFI_BASE - 0x0003)
/* Unexpected version info from cellphone when wifi-provision */
/* 手机端发来的配网协议的版本号不符合预期 */
#define SUB_ERRCODE_WIFI_UNEXP_PROT_VERSION               (SUB_ERRCODE_WIFI_BASE - 0x0004)
/* Debug info during broadcast wifi-provision */
/* WiFi包长编码的一键配网中的调试信息 */
#define SUB_ERRCODE_WIFI_BCAST_UNSUPPORT                      (SUB_ERRCODE_WIFI_BASE - 0x0005)
/* The params from registar in zeroconfig Wifi Configuration is invalid */
/* WiFi零配过程中的主配方发来的参数有误 */
#define SUB_ERRCODE_WIFI_ZCONFIG_REGISTAR_PARAMS_ERROR    (SUB_ERRCODE_WIFI_BASE - 0x0006)
/* WiFi provision claiming other information */
/* WiFi配网中的其它过程信息 */
#define SUB_ERRCODE_WIFI_OTHERS                           (SUB_ERRCODE_WIFI_BASE - 0x0007)
/* CoAP init failed */
/* WiFi配网中 CoAP 初始化失败, CoAP用于所有配网模式 */
#define SUB_ERRCODE_WIFI_COAP_INIT_FAILED                 (SUB_ERRCODE_WIFI_BASE - 0x0008)
/* Got an invalid coap resp msg */
/* WiFi配网中 收到CoAP非法应答消息 */
#define SUB_ERRCODE_WIFI_COAP_RSP_INVALID                 (SUB_ERRCODE_WIFI_BASE - 0x0009)
/* Recieve devinfo query message */
/* WiFi配网中收到查询设备信息的包(手机热点) */
#define SUB_ERRCODE_WIFI_GOT_DEVINFO_QUERY                (SUB_ERRCODE_WIFI_BASE - 0x000A)

/* dev-ap start failed */
/* WiFi 设备热点开启失败 */
#define SUB_ERRCODE_WIFI_DEV_AP_START_FAIL                (SUB_ERRCODE_WIFI_BASE - 0x000B)
/* dev-ap receive message not in dev-ap awss mode */
/* WiFi 设备热点并未开启，但接收到了对端设备热点配网消息 */
#define SUB_ERRCODE_WIFI_DEV_AP_RECV_IN_WRONG_STATE       (SUB_ERRCODE_WIFI_BASE - 0x000C)
/* dev-ap received awss packet invalid */
#define SUB_ERRCODE_WIFI_DEV_AP_RECV_PKT_INVALID          (SUB_ERRCODE_WIFI_BASE - 0x000D)
/* dev-ap received awss packet invalid */
#define SUB_ERRCODE_WIFI_DEV_AP_PARSE_PKT_FAIL            (SUB_ERRCODE_WIFI_BASE - 0x000E)
/* dev-ap failed to decrypt WiFi password */
/* WiFi 热备热点解密WiFi热点的密码失败 */
#define SUB_ERRCODE_WIFI_DEV_AP_PASSWD_DECODE_FAILED      (SUB_ERRCODE_WIFI_BASE - 0x000F)
/* Send a coap msg failed */
/* WiFi配网中 发送CoAP消息失败 */
#define SUB_ERRCODE_WIFI_DEV_AP_SEND_PKT_FAIL             (SUB_ERRCODE_WIFI_BASE - 0x0010)
/* dev-ap stop failed */
/* WiFi 设备热点关闭失败 */
#define SUB_ERRCODE_WIFI_DEV_AP_CLOSE_FAIL                (SUB_ERRCODE_WIFI_BASE - 0x0011)
/* device send connect ap notify reach max count(timeout, send fail or recv no resp) */
#define SUB_ERRCODE_WIFI_SENT_CONNECTAP_NOTI_TIMEOUT      (SUB_ERRCODE_WIFI_BASE - 0x0012)
/* device can not find the AP to connect */
#define SUB_ERRCODE_WIFI_AP_DISCOVER_FAIL                 (SUB_ERRCODE_WIFI_BASE - 0x0013)
/* device find the AP rssi too low to connect */
#define SUB_ERRCODE_WIFI_AP_RSSI_TOO_LOW                  (SUB_ERRCODE_WIFI_BASE - 0x0014)
/* device connect the AP auth fail */
#define SUB_ERRCODE_WIFI_AP_CONN_PASSWD_ERR               (SUB_ERRCODE_WIFI_BASE - 0x0015)
/* device connect the AP IP Address get fail */
#define SUB_ERRCODE_WIFI_AP_CONN_IP_GET_FAIL              (SUB_ERRCODE_WIFI_BASE - 0x0016)
/* device connect the AP others fail */
#define SUB_ERRCODE_WIFI_AP_CONN_OTHERS_FAIL              (SUB_ERRCODE_WIFI_BASE - 0x0017)


#define STATE_CODE_WIFI_BASE                             (0x0400)
#define STATE_CODE_WIFI_AWSS                             (STATE_CODE_WIFI_BASE + 0x0001)
#define STATE_CODE_WIFI_GET_AP_INFO                      (STATE_CODE_WIFI_BASE + 0x0002)
#define STATE_CODE_WIFI_CONNECT_AP                       (STATE_CODE_WIFI_BASE + 0x0003)
#define STATE_CODE_WIFI_RECONNECT_AP                     (STATE_CODE_WIFI_BASE + 0x0004)
#define STATE_CODE_WIFI_GOT_IP                           (STATE_CODE_WIFI_BASE + 0x0005)
#define STATE_CODE_WIFI_CHAN_SCAN                        (STATE_CODE_WIFI_BASE + 0x0006)
#define STATE_CODE_WIFI_ZCONFIG_DESTROY                  (STATE_CODE_WIFI_BASE + 0x0007)
#define STATE_CODE_WIFI_FORCE_STOPPED                    (STATE_CODE_WIFI_BASE + 0x0008)
#define STATE_CODE_WIFI_PROCESS_FRAME                    (STATE_CODE_WIFI_BASE + 0x0009)
#define STATE_CODE_WIFI_CONNECT_AP_SUCCESS               (STATE_CODE_WIFI_BASE - 0x000A)
/* WiFi provision decrypt WiFi password successfully */
/* WiFi配网中解密WiFi热点的密码成功 */
#define STATE_CODE_WIFI_PASSWD_DECODE_SUCCESS            (STATE_CODE_WIFI_BASE + 0x000B)
/* WiFi configuration statistics report, including overall time, pack number, pack lens */
/* WiFi配网中的统计信息, 比如收到包的数量, 配网时间, 收包报文的总长度*/
#define STATE_CODE_WIFI_STATISTIC                        (STATE_CODE_WIFI_BASE + 0x000C)
/* Debug info during multicast wifi-provision */
/* WiFi组播配网中的调试信息 */
#define STATE_CODE_WIFI_MCAST_DEBUG                      (STATE_CODE_WIFI_BASE + 0x000D)
/* Debug info of p2p mode smartconfig */
/* WiFi p2p配网过程中的调试信息 */
#define STATE_CODE_WIFI_P2P_DEBUG                        (STATE_CODE_WIFI_BASE + 0x000E)
/* Notify connectap event */
/* WiFi配网中广播通知连接ap事件 */
#define STATE_CODE_WIFI_SENT_CONNECTAP_NOTIFY            (STATE_CODE_WIFI_BASE + 0x000F)
/* Dev AP config */
/* dev-ap already running */
/* WiFi 设备热点已经开启过, 本次为重复开启 */
#define STATE_CODE_WIFI_DEV_AP_ALREADY_RUN               (STATE_CODE_WIFI_BASE + 0x0010)


/* COAP: 0x0500 ~ 0x05FF */
#define SUB_ERRCODE_COAP_BASE                             (-0x0500)

/* COAP: 0x0500 ~ 0x05FF */

/* HTTP: 0x0600 ~ 0x06FF */
#define SUB_ERRCODE_HTTP_BASE                             (-0x0600)
/* Got HTTP response from server, says dynamic register request failed */
/* 使用动态注册/一型一密功能时, 从服务端返回的报文表示, 申请密钥失败 */
#define SUB_ERRCODE_HTTP_DYNREG_FAIL_RESP                 (SUB_ERRCODE_HTTP_BASE - 0x0001)
/* Got HTTP response from server, which contains invalid deviceSecret */
/* 使用动态注册/一型一密功能时, 从服务端返回的报文中, 解析得到的密钥不合理 */
#define SUB_ERRCODE_HTTP_DYNREG_INVALID_DS                (SUB_ERRCODE_HTTP_BASE - 0x0002)
/* Reserved buffer is too short when compose pre-auth HTTP request */
/* 组装预认证消息准备发给服务端时, 为HTTP上行报文预留的缓冲区太短, 不足容纳待发送内容 */
#define SUB_ERRCODE_HTTP_PREAUTH_REQ_BUF_SHORT            (SUB_ERRCODE_HTTP_BASE - 0x0003)
/* Reserved buffer is too short when retrieve pre-auth HTTP response */
/* 收取预认证请求的应答报文时, 为HTTP回应报文预留的缓冲区太短, 不足容纳待接收内容 */
#define SUB_ERRCODE_HTTP_PREAUTH_INVALID_RESP             (SUB_ERRCODE_HTTP_BASE - 0x0004)
/* Content length field does not exist in HTTP response from server */
/* 从HTTP服务器返回的报文中, 没有找到 Content-Length 字段 */
#define SUB_ERRCODE_HTTP_RESP_MISSING_CONTENT_LEN         (SUB_ERRCODE_HTTP_BASE - 0x0005)
/* Report preauth relative HTTP request parameters */
/* 进行HTTP预认证的过程中, 上报预认证请求的相关信息 */
#define SUB_ERRCODE_HTTP_PREAUTH_REQ                      (SUB_ERRCODE_HTTP_BASE - 0x0006)
/* Report preauth relative HTTP respond parameters */
/* 进行HTTP预认证的过程中, 上报预认证应答的相关信息 */
#define SUB_ERRCODE_HTTP_PREAUTH_RSP                      (SUB_ERRCODE_HTTP_BASE - 0x0007)
/* Preauth relative HTTP DNS parse fail */
/* HTTP init fail */
#define SUB_ERRCODE_HTTP_NWK_INIT_FAIL                    (SUB_ERRCODE_HTTP_BASE - 0x0008)
/* Preauth relative HTTP DNS parse fail */
/* 进行HTTP预认证的过程中, 域名解析失败 */
#define SUB_ERRCODE_HTTP_PREAUTH_DNS_FAIL                 (SUB_ERRCODE_HTTP_BASE - 0x0009)
/* Preauth relative HTTP timeout fail */
/* 进行HTTP预认证的过程中, 认证超时失败 */
#define SUB_ERRCODE_HTTP_PREAUTH_TIMEOUT_FAIL             (SUB_ERRCODE_HTTP_BASE - 0x000A)
/* Preauth relative HTTP identify auth fail */
/* 进行HTTP预认证的过程中, 认证身份失败(三元组) */
#define SUB_ERRCODE_HTTP_PREAUTH_IDENT_AUTH_FAIL          (SUB_ERRCODE_HTTP_BASE - 0x000B)

/* HTTP: 0x0600 ~ 0x06FF */

/* OTA: 0x0700 ~ 0x07FF */
#define SUB_ERRCODE_OTA_BASE                              (-0x0700)
/* Read firmware version failed */
/* 在读取设备的固件版本号时失败 */
#define SUB_ERRCODE_OTA_FIRMWARE_VER_READ_FAIL            (SUB_ERRCODE_OTA_BASE - 0x0040)

/* OTA: 0x0700 ~ 0x07FF */

/* Bind: 0x0800 ~ 0x08FF */
#define SUB_ERRCODE_BIND_BASE                             (-0x0800)

/* Failed to initialize CoAP server in LAN */
/* 绑定功能模块未能成功开启CoAP服务, 该服务用于局域网内的token传递 */
#define SUB_ERRCODE_BIND_COAP_INIT_FAIL                   (SUB_ERRCODE_BIND_BASE - 0x0002)
/* Sending bind token to cloud */
/* 正在将绑定token发送给服务端 */
#define SUB_ERRCODE_BIND_REPORT_TOKEN                     -(SUB_ERRCODE_BIND_BASE - 0x0003)
/* Sent bind token to cloud gets success response */
/* 绑定token发送给服务端后, 已得到服务端成功接收的回应 */
#define SUB_ERRCODE_BIND_REPORT_TOKEN_SUCCESS             -(SUB_ERRCODE_BIND_BASE - 0x0004)
/* Got token query request from external devices in same LAN */
/* 正在接收到同一局域网内来自其它电子设备的token查询请求 */
#define SUB_ERRCODE_BIND_RECV_TOKEN_QUERY                 -(SUB_ERRCODE_BIND_BASE - 0x0006)
/* Responding bind token in LAN */
/* 正在响应局域网内的token查询请求, 将token发送给其它电子设备 */
#define SUB_ERRCODE_BIND_SENT_TOKEN_RESP                  (SUB_ERRCODE_BIND_BASE - 0x0007)
/* Got invalid MQTT respond from server after msg report */
/* mqtt信息上报给服务端后, 接收到的回应报文不合法 */
#define SUB_ERRCODE_BIND_MQTT_RSP_INVALID                 (SUB_ERRCODE_BIND_BASE - 0x0008)
/* Got invalid MQTT msgid from server after token report */
/* 将绑定token上报给服务端后, 接收到的回应报文msgid不是最新的*/
#define SUB_ERRCODE_BIND_MQTT_MSGID_INVALID               (SUB_ERRCODE_BIND_BASE - 0x0009)

/* Bind token report to cloud and receive ack success */
#define SUB_ERRCODE_BIND_ALREADY_RESET                    -(SUB_ERRCODE_BIND_BASE - 0x0011)
/* Bind token report to cloud reach up threshould and still failed */
#define SUB_ERRCODE_BIND_REPORT_TOKEN_TIMEOUT             (SUB_ERRCODE_BIND_BASE - 0x0012)
/* Device reset in progress */
#define SUB_ERRCODE_BIND_RST_IN_PROGRESS                  -(SUB_ERRCODE_BIND_BASE - 0x0013)
/* Receive invalid mqtt message in awss state */
#define SUB_ERRCODE_BIND_MQTT_MSG_INVALID                 (SUB_ERRCODE_BIND_BASE - 0x0014)
/* Receive invalid coap message in awss state */
#define SUB_ERRCODE_BIND_COAP_MSG_INVALID                 (SUB_ERRCODE_BIND_BASE - 0x0015)
/* Not got external token configuration */
/* 绑定功能模块确认未接收到来自外部的Token传入 */
#define SUB_ERRCODE_BIND_NO_APP_TOKEN                     (SUB_ERRCODE_BIND_BASE - 0x0040)
/* Dev bind token already report to cloud, no need to report again */
/* 绑定功能模块确认已上报Token成功到云端，无需再重复上报 */
#define SUB_ERRCODE_BIND_ALREADY_REPORT                   -(SUB_ERRCODE_BIND_BASE - 0x0041)
/* Sending bind token to cloud failed */
/* 将绑定token发送给服务端时失败 */
#define SUB_ERRCODE_BIND_REPORT_TOKEN_FAIL                (SUB_ERRCODE_BIND_BASE - 0x0042)
/* Bind token get Response to App failed */
#define SUB_ERRCODE_BIND_APP_GET_TOKEN_RESP_FAIL          (SUB_ERRCODE_BIND_BASE - 0x0043)
/* Sent reset msg to cloud failed */
/*  */
#define SUB_ERRCODE_BIND_REPORT_RESET_FAIL                (SUB_ERRCODE_BIND_BASE - 0x0044)


/* Bind: 0x0800 ~ 0x08FF */
#define STATE_CODE_BIND_BASE                             (0x0800)
/* Got external token configuration */
/* 绑定功能模块接收到来自外部的Token传入 */
#define STATE_CODE_BIND_SET_APP_TOKEN                    (STATE_CODE_BIND_BASE + 0x0001)


/* Device Model: 0x0900 ~ 0x09FF */
#define SUB_ERRCODE_DEV_MODEL_BASE                        (-0x0900)
/* Got MQTT message from server but its JSON format is wrong */
/* 物模型模块中接收到来自服务端的MQTT下推消息, 但报文内容不是合理的JSON格式 */
#define SUB_ERRCODE_DEV_MODEL_WRONG_JSON_FORMAT           (SUB_ERRCODE_DEV_MODEL_BASE - 0x0011)

/* Device Model: 0x0900 ~ 0x09FF */

#define SUB_ERRCODE_BLE_BASE                              (-0x0A00)
#define SUB_ERRCODE_BLE_START_ADV_FAIL                    (SUB_ERRCODE_BLE_BASE - 0x0001)
#define SUB_ERRCODE_BLE_STOP_ADV_FAIL                     (SUB_ERRCODE_BLE_BASE - 0x0002)
#define SUB_ERRCODE_BLE_START_SCAN_FAIL                   (SUB_ERRCODE_BLE_BASE - 0x0003)
#define SUB_ERRCODE_BLE_STOP_SCAN_FAIL                    (SUB_ERRCODE_BLE_BASE - 0x0004)
#define SUB_ERRCODE_BLE_CONNECT_FAIL                      (SUB_ERRCODE_BLE_BASE - 0x0005)
#define SUB_ERRCODE_BLE_DISCONNECT_FAIL                   (SUB_ERRCODE_BLE_BASE - 0x0006)
#define SUB_ERRCODE_BLE_GATT_REGISTER_SERVICE_FAIL        (SUB_ERRCODE_BLE_BASE - 0x0007)

#define STATE_CODE_BLE_BASE                              (0x0A00)
#define STATE_CODE_BLE_CONNECTED                         (STATE_CODE_BLE_BASE + 0x0001)
#define STATE_CODE_BLE_DISCONNECTED                      (STATE_CODE_BLE_BASE + 0x0002)

/****************** Device Diagnosis: used for device diagnosis *******************/

#define DEV_SUPPORT_NONE                    0x00000000    // Device support no optional features
#define DEV_SUPPORT_ERRCODE                 0x00000001    // Device errcode diagnosis
#define DEV_SUPPORT_OFFLINE_OTA             0x00000002    // Device offline OTA repair
#define DEV_SUPPORT_OFFLINE_LOG             0x00000004    // Device offline log diagnosis
#define DEV_SUPPORT_DEVAP_BATCH_PROV        0x00000008    // Device dev ap toc batch provision
#define DEV_SUPPORT_ALL_SERVICE             0xFFFFFFFF    // Device support all optional features

#ifdef DEV_ERRCODE_ENABLE
#define DEV_DIAGNOSIS_ERRCODE               DEV_SUPPORT_ERRCODE
#else
#define DEV_DIAGNOSIS_ERRCODE               DEV_SUPPORT_NONE
#endif

#ifdef DEV_OFFLINE_OTA_ENABLE
#define DEV_DIAGNOSIS_OFFLINE_OTA           DEV_SUPPORT_OFFLINE_OTA
#else
#define DEV_DIAGNOSIS_OFFLINE_OTA           DEV_SUPPORT_NONE
#endif

#ifdef DEV_OFFLINE_LOG_ENABLE
#define DEV_DIAGNOSIS_OFFLINE_LOG           DEV_SUPPORT_OFFLINE_LOG
#else
#define DEV_DIAGNOSIS_OFFLINE_LOG           DEV_SUPPORT_NONE
#endif

#ifdef AWSS_BATCH_DEVAP_ENABLE
#define DEV_DIAGNOSIS_DEVAP_BATCH_PROV      DEV_SUPPORT_DEVAP_BATCH_PROV
#else
#define DEV_DIAGNOSIS_DEVAP_BATCH_PROV      DEV_SUPPORT_NONE
#endif

#define DEV_DIAGNOSIS_SUPPORTED_SERVICE     (DEV_DIAGNOSIS_ERRCODE | DEV_DIAGNOSIS_OFFLINE_OTA \
                                           | DEV_DIAGNOSIS_OFFLINE_LOG | DEV_DIAGNOSIS_DEVAP_BATCH_PROV)

/****************** Device Errcode: used for device diagnosis *********************/

// Device errcode keys
#define DEV_ERRCODE_KEY                     "ErrCode"
#define DEV_ERRCODE_MSG_KEY                 "ErrCodeMsg"

// Device errcode global definitions
#define DEV_ERRCODE_VERSION                 1
#define DEV_ERRCODE_MSG_MAX_LEN             (64)
#define DEV_ERRCODE_TOPIC_RSP_MAX_LEN       (256)
#define DEV_ERRCODE_TOPIC_RSP_FMT           "\"codeVer\":\"%d\",\"state\":%d,\"code\":%d,\"errMsg\":\"%s\",\"signSecretType\":%d,\"sign\":\"%s\""

typedef enum
{
    DEV_ERRCODE_SIGN_TYPE_DS = 0,           // errcode sign type - device secret
    DEV_ERRCODE_SIGN_TYPE_PS = 1,           // errcode sign type - product secret
    DEV_ERRCODE_SIGN_INVALID
} dev_errcode_sign_type_t;

/****************** Device Offline OTA,used for device diagnosis *********************/
#define DEV_OFFLINE_OTA_VERSION                 1
#define DEV_OFFLINE_OTA_MSG_MAX_LEN             (64)
#define DEV_OFFLINE_OTA_TOPIC_RSP_MAX_LEN       (64)
#define DEV_OFFLINE_OTA_TOPIC_RSP_FMT           "\"id\":%s,\"code\":%d,\"data\":{}"

// Device offline OTA Resp definition
typedef enum
{
    DEV_OFFLINE_OTA_RSP_OK = 200,
    DEV_OFFLINE_OTA_RSP_INVALID_PARAM   = 10099,
    DEV_OFFLINE_OTA_RSP_SAME_VERSION    = 10010,
    DEV_OFFLINE_OTA_RSP_DOWNLOAD_FAILED = 10011,
    DEV_OFFLINE_OTA_RSP_VERIFY_FAILED   = 10012,
    DEV_OFFLINE_OTA_RSP_MAX
} dev_offline_ota_resp_t;
// Device state machine definition
typedef enum
{
    DEV_STATE_INIT = 0x00,
    DEV_STATE_WIFI_MONITOR = 0x01,
    DEV_STATE_AWSS = 0x02,
    DEV_STATE_CONNECT_AP = 0x03,
    DEV_STATE_CONNECT_CLOUD = 0x04,
    DEV_STATE_MAX
} dev_state_t;

// Device awss state machine definition
typedef enum
{
    AWSS_STATE_STOP = 0x00,
    AWSS_STATE_START = 0x01,
    AWSS_STATE_COLLECTING_SSID = 0x02,
    AWSS_STATE_SSID_GOT = 0x03,
    AWSS_STATE_MAX
} dev_awss_state_t;

// Device awss pattern
typedef enum
{
    AWSS_PATTERN_SMART_CONFIG = 0x00,
    AWSS_PATTERN_DEV_AP_CONFIG = 0x01,
    AWSS_PATTERN_ZERO_CONFIG = 0x02,
    AWSS_PATTERN_BLE_CONFIG = 0x03,
    AWSS_PATTERN_PHONE_AP_CONFIG = 0x04,
    AWSS_PATTERN_MAX
} dev_awss_pattern_t;

/*Device error code*/
#define DEV_ERRCODE_SYSTERM_ERR             0xC400

/* smart config awss */
#define DEV_ERRCODE_SC_SSID_PWD_GET_TIMEOUT 0xC440
#define DEV_ERRCODE_SC_SSID_PWD_PARSE_ERR   0xC441

/* dev ap awss */
#define DEV_ERRCODE_DA_DEV_AP_START_FAIL    0xC460
#define DEV_ERRCODE_DA_SSID_PWD_GET_TIMEOUT 0xC462
#define DEV_ERRCODE_DA_VERSION_ERR          0xC463
#define DEV_ERRCODE_DA_PKT_CHECK_ERR        0xC464
#define DEV_ERRCODE_DA_SSID_PWD_PARSE_ERR   0xC465
#define DEV_ERRCODE_DA_SWITCH_STA_FAIL      0xC466

/* zero config */
#define DEV_ERRCODE_ZC_SSID_PWD_GET_TIMEOUT 0xC480
#define DEV_ERRCODE_ZC_VERSION_ERR          0xC481
#define DEV_ERRCODE_ZC_PKT_CHECK_ERR        0xC482
#define DEV_ERRCODE_ZC_SSID_PWD_PARSE_ERR   0xC483

/* ble awss */
#define DEV_ERRCODE_BA_BLE_ADV_START_FAIL   0xC4A0
#define DEV_ERRCODE_BA_BLE_CONN_FAIL        0xC4A1
#define DEV_ERRCODE_BA_SSID_PWD_GET_TIMEOUT 0xC4A2
#define DEV_ERRCODE_BA_VERSION_ERR          0xC4A3
#define DEV_ERRCODE_BA_PKT_CHECK_ERR        0xC4A4
#define DEV_ERRCODE_BA_SSID_PWD_PARSE_ERR   0xC4A5

/* Router */
#define DEV_ERRCODE_AP_DISCOVER_FAIL        0xC4E0
#define DEV_ERRCODE_AP_RSSI_TOO_LOW         0xC4E1
#define DEV_ERRCODE_AP_CONN_PASSWD_ERR      0xC4E2
#define DEV_ERRCODE_AP_GET_IP_FAIL          0xC4E3
#define DEV_ERRCODE_AP_CONN_LOCAL_NOTI_FAIL 0xC4E4
#define DEV_ERRCODE_AP_CONN_OTHERS_FAIL     0xC4E5

// DEV_STATE_CONNECT_CLOUD
#define DEV_ERRCODE_HTTPS_INIT_FAIL         0xC500
#define DEV_ERRCODE_HTTPS_DNS_FAIL          0xC501
#define DEV_ERRCODE_HTTPS_PREAUTH_TIMEOUT   0xC502
#define DEV_ERRCODE_HTTPS_DEVAUTH_FAIL      0xC503
#define DEV_ERRCODE_MQTT_INIT_FAIL          0xC504
#define DEV_ERRCODE_MQTT_CONN_TIMEOUT       0xC505
#define DEV_ERRCODE_MQTT_AUTH_FAIL          0xC506
#define DEV_ERRCODE_COAP_INIT_FAIL          0xC507
#define DEV_ERRCODE_TOKEN_RPT_CLOUD_TIMEOUT 0xC508
#define DEV_ERRCODE_TOKEN_RPT_CLOUD_ACK_ERR 0xC509
#define DEV_ERRCODE_TOKEN_GET_LOCAL_PKT_ERR 0xC50A
#define DEV_ERRCODE_TOKEN_GET_LOCAL_RSP_ERR 0xC50B

#ifdef __cplusplus
}
#endif
#endif  /* __IOT_EXPORTS_STATE_H__ */
