/**
 * @brief   Declare interface of the LAN network communication
 * 
 * @file    aiio_lan_service.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-04          <td>1.0.0            <td>zhuolm             <td> The interface of the LAN network communication
 */
#ifndef __AIIO_LAN_SERVICE_H_
#define __AIIO_LAN_SERVICE_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>



#define     AIIO_LAN_SERVICE_DEFAULT_PORT                       (1314)
#define     AIIO_MDNS_DEFAULT_SERVICE_TYPE                      ("_iotaithings")


typedef enum
{
    AIIO_LAN_INVAILD_EVENT = 0,             /*!< invalid event*/
    AIIO_LAN_RESTORE_EVENT,                 /*!< it wil callback this event when it is received restore command from cloud. The device have to restore the facory when receives this event, which need to clean all running data in device*/
    AIIO_LAN_REBOOT_EVENT,                  /*!< it will callback this event when it is received reboot command from cloud. The device have to reboot when receives this event*/
    AIIO_LAN_INFO_EVENT,                    /*!< it will callback this event when it is received request-information command from cloud, which auto to publish device information to cloud*/
    AIIO_LAN_CONFIG_EVENT,                  /*!< it will callback this event when received configuration command from cloud. The device have to parse configuration command, and it will be update keepalive time, mqtt connection timeout time and auto to report attribute for cloud. And then the mqtt needs to reconnect by device.*/
    AIIO_LAN_UPGRADE_EVENT,                 /*!< it will callback this event when received OTA information command from cloud*/
    AIIO_LAN_QUERY_EVENT,                   /*!< it will callback this event when received request-attribute data from cloud, The device have to report all attribute status to cloud*/
    AIIO_LAN_CONTROL_EVENT,                 /*!< it will callback this event when received control-attribute status from cloud. The device have to change the status of attribute, and report control-attribute status to cloud*/
}aiio_lan_service_event_t;

typedef struct 
{
    char *data;                                 /*!< attribute data*/
    char *msgMid;                               /*!< The message ID of the attribute data or command that is received from cloud, which is different every time */
    char *from;                                 /*!< The source of attribute data or command, which is come from device, app, or cloud*/
    uint32_t data_len;                          /*!< The length of attribute data*/
    aiio_lan_service_event_t  lan_service_event;      /*!< The type of event, which is defined in structure that called aiio_lan_service_event_t in this file*/
}aiio_lan_service_data_t;



typedef void (*rev_lan_service_data_cb)(aiio_lan_service_data_t *lan_service_data);

typedef struct 
{
    char                        *device_id;                                     /*!< client ID data*/
    char                        *local_ip;                                      /*!< The local network address */
    uint16_t                    lan_port;                                       /*!< The port number of UDP service in lan */
    uint32_t                    task_prio;                                      /*!< Priority of the thread task*/
    uint32_t                    task_stack;                                     /*!< The stack for the thread task*/
    uint32_t                    rev_buffer_max_len;                             /*!< The length of recevie buffer */

    rev_lan_service_data_cb     lan_service_data_cb;     /*!< The callback function of receiving data in lan*/
}aiio_lan_service_config_t;


/**
 * @brief   Initialize lan network communication
 * @note    The LAN communication mainly adopts the UDP protocol
 * 
 * @param[in]   lan_service_config                  The paramter of lan netowrk configuration
 * 
 * @return  int             Reture all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_lan_service_init(aiio_lan_service_config_t *lan_service_config);


/**
 * @brief   Deinitialize LAN network communication, and release memory resource
 * 
 * @return  int  Reture all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_lan_service_deinit(void);


#endif
