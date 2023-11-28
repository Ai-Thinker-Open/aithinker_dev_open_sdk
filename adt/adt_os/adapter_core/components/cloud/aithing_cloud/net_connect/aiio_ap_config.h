/**
 * @brief   Declare AP distribution network logic interface
 * 
 * @file    aiio_ap_config.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-13          <td>1.0.0            <td>zhuolm             <td> The AP distribution network logic interface
 */

#ifndef __AIIO_AP_CONFIG_H_
#define __AIIO_AP_CONFIG_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "aiio_protocol_comm.h"
#include "aiio_ap_protocol.h"


#define AIIO_AP_SSID_LEN                    (33)
#define AIIO_AP_PASSWORD_LEN                (66)


/**
 * @brief   The event of AP distribute network, it will inform AP distribution network state by callbacked those event in aiio_ap_config_iot_event_t struction
 */
typedef enum
{
    AIIO_AP_CONFIG_FAIL,
    AIIO_AP_CONFIG_OK
}aiio_ap_config_iot_event_t;


/**
 * @brief   The information of the AP distribution network, it will transfer the AP distribution network data through the callback interface.
 */
typedef struct 
{
    char *entrypt_key;
    aiio_ap_data_t  *ap_data;
    aiio_ap_config_iot_event_t event;
}aiio_ap_iot_data_t;


typedef enum
{
    AIIO_WIFI_SOFT_AP_START_EVENT,
    AIIO_WIFI_SOFT_AP_STOP_EVENT,
}aiio_wifi_ap_state_event_t;

typedef struct 
{
    uint8_t ssid[AIIO_AP_SSID_LEN];                 /**< SSID of  soft-AP. */
    uint8_t password[AIIO_AP_PASSWORD_LEN];         /**< Password of  soft-AP. */
}aiio_wifi_ap_info_config_t;



/**
 * @brief   The configuration information of AP distribution network, it must be configured before AP distribution network starting 
 */
typedef struct 
{
    char                *deviceid;
    char                *productid;
    char                *wifi_flag;    
    uint32_t            task_prio;                                                       /*!< The task priority in AP distribution network, it will use default the task priority when the element is not set*/
    uint32_t            task_stack;                                                      /*!< The task stack in AP distribution network, it will use default the task stack when the element is not set*/
    uint16_t            udp_port;                                                       /*!< The port number of udp service*/
    uint32_t            rev_buffer_max_len;                                             /*!< The length of recevie buffer */
    void (*aiio_ap_iot_config_cb)(aiio_ap_iot_data_t *ap_iot_data);                 /*!< The callback function that is receiving AP distribution network data*/
    void (*aiio_wifi_ap_start)(aiio_wifi_ap_info_config_t *wifi_ap_info_config);          /*!< The callback function that is starting soft-AP*/
    void (*aiio_wifi_ap_stop)(void);                                            /*!< The callback function that is stoping soft-AP*/
}aiio_ap_iot_config_t;




/**
 * @brief   Initialize the AP Distribution network
 * @note    Set the wifi operating mode as soft-AP
 * 
 * @param[in]   ap_config                 The configuration information of AP distribution network
 * 
 * @return  int  Reture all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_ap_config_init(aiio_ap_iot_config_t *ap_config);



/**
 * @brief   Deinitialize AP Distribution network
 * @note    Stop soft-AP, and release related resource
 * @return  int  Reture all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_ap_config_deinit(void);

/**
 * @brief   Set soft-AP event
 * @note    When the soft-AP state changes, record the state change through 
 *          the function aiio_wifi_ap_event_notify() to notify the AP distribution network to stop or start,
 *          if set event is AIIO_WIFI_SOFT_AP_STOP_EVENT, and then delete udp service and udp client.
 * 
 * @param[in]   event       The soft-AP state event
 * 
 */
void aiio_wifi_ap_event_notify(aiio_wifi_ap_state_event_t event);




#endif
