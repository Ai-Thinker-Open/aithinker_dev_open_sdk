/**
 * @brief   Declare AP protocol packaging and parsing interface
 * 
 * @file    aiio_ap_protocol.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-11          <td>1.0.0            <td>zhuolm             <td> The AP protocol packaging and parsing interface
 */
#ifndef __AIIO_AP_PROTOCOL_H_
#define __AIIO_AP_PROTOCOL_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "aiio_protocol_comm.h"



#define AIIO_AP_PROCOTOL_VERSION             (1)           /**< AP protocol version */




/**
 * @brief The encryption type of the AP distribution network communication
*/
typedef enum 
{
    PROTOCOL_AP_ENTRYPT_NONE = 0,              /*!< The type that is not encrypted*/
    PROTOCOL_AP_ENTRYPT_AES_CBC_128,            /*!< The type that is using the AES CBC 128*/    
    PROTOCOL_AP_ENTRYPT_UNKNOW = 0xff          /*!< The type that is unknow */
}aiio_ap_entrypt_type_t;


/**
 * @brief   The information of AP distribution network structure
 */
typedef struct 
{
    char *ssid;                              /*!< ssid of rout */
    char *passwd;                             /*!< passwd of rout*/
    char *wificc;                             /*!< country code */
    char *mqttip;                             /*!< ip address or domain name */
    char *token;                             /*!< activaty information，it will be given from cloud by app */
    char *tz;                                /*!< time zone */
    uint32_t ts;                                /*!< time stamp */
    uint16_t  port;                          /*!< port number of remote service */
}aiio_ap_data_t;



/**
 * @brief   Parse protocol the event of the AP distribution network 
 */
typedef enum
{
    AIIO_AP_DATA_CRC_ERR_EVENT = 0,                        /*!< It will callback this event when an error during CRC verification on the data from the APP, it needs to packet protocol data that include crc error status by this function 
                                                                 that is named aiio_PacketAPResponseDivideData(), and then send to this protocol data for APP by call other function , and all kind of error code that is defined in this file that is called aiio_protocol_code.h*/
    AIIO_AP_DATA_PROTOCOL_ERR_EVENT,                        /*!< It will callback this event when an error during the protocol format verification on the data from the APP, it needs to packet protocol data that include the protocol format error status by this function 
                                                                 that is named aiio_PacketAPResponseDivideData(), and then send to this protocol data for APP by call other function ,and all kind of error code that is defined in this file that is called aiio_protocol_code.h*/
    AIIO_AP_DATA_PROTOCOL_VER_ERR_EVENT,                    /*!< It will callback this event when an error during protocol version verification on the data from the APP, it needs to packet protocol data that include protocol version error status by this function 
                                                                 that is named aiio_PacketAPResponseDivideData(), and then send to this protocol data for APP by call other function ,and all kind of error code that is defined in this file that is called aiio_protocol_code.h*/
    AIIO_AP_DATA_HARD_ERR_EVENT,                           /*!< It will callback this event when device has a hardware type error during parse protocol on the data from the APP, it needs to packet protocol data that include hardware type error status by this function 
                                                                 that is named aiio_PacketAPResponseDivideData(), and then send to this protocol data for APP by call other function ,and all kind of error code that is defined in this file that is called aiio_protocol_code.h*/
    AIIO_AP_DATA_ENTRYPT_TYPE_UNKNOW_EVENT,                /*!< It will callback this event when an error during encryption type verification on the data from the APP, it needs to packet protocol data that include encryption type error status by this function 
                                                                 that is named aiio_PacketAPResponseDivideData(), and then send to this protocol data for APP by call other function ,and all kind of error code that is defined in this file that is called aiio_protocol_code.h*/
    AIIO_AP_DATA_PARSE_FAIL_EVENT,                        /*!< It will callback this event when the json parse fail on data from the APP, it needs to packet protocol data that include json parse fail error status by this function 
                                                                 that is named aiio_PacketAPResponseDivideData(),and then send to this protocol data for APP by call other function , and all kind of error code that is defined in this file that is called aiio_protocol_code.h*/
    AIIO_AP_DATA_PARSE_OK_EVENT                            /*!< It will callback this event when the json data parse successfully on data from APP, it needs to packet protocol data that include json parsing success error status by this function 
                                                                 that is named aiio_PacketAPResponseDivideData(),and then send to this protocol data for APP by call other function , and all kind of error code that is defined in this file that is called aiio_protocol_code.h*/
}aiio_ap_data_parse_event_t;



/**
 * @brief   The protocol event of the AP distribution network
 * @note     Parsed protocol data in aiio_ap_protocol_event_t structure to application by the callback function that is named aiio_ParseApProtocolData(), 
 */
typedef struct 
{
    aiio_ap_data_t                 *ap_data;                  /*!< The information of AP distribution network structure*/
    aiio_ap_data_parse_event_t     event;                      /*!< Parse protocol the event of the AP distribution network */
    aiio_ap_entrypt_type_t         entrypt_type;               /*!< The encryption type of the AP distribution network communication*/
    char                           *entrypt_key;               /*!< The encryption key of the AP distribution network communication*/
}aiio_ap_protocol_event_t;


typedef void (*aiio_ap_data_parse_cb)(aiio_ap_protocol_event_t *event);

/**
 * @brief   Parse package data of the AP distribution network
 * 
 * @param[in]   data                    The source data of the AP distribution network
 * @param[in]   data_len                The length of the source data of the AP distribution network
 * @param[in]   data_parse_cb           The callback function, the callback function is called by this function after the source data is parse.
 * 
 * @return  int  Reture the length of the packaged data or all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_ParseApProtocolData(uint8_t *data, uint16_t data_len, aiio_ap_data_parse_cb data_parse_cb);




/**
 * @brief   Pack the error code protocol packets in response to the APP, this function is mainly response json data to APP.
 * @note    This function only pack the whole package data, and this function needs to package the data into encrypted data by the paramter entrypt_type.
 * 
 * @param[in]   packet_data             The buffer space that receives the packaged responsed data
 * @param[in]   packet_data_len         The length of the buffer space that receives the packaged responsed data
 * @param[in]   entrypt_type            The encrypt type of the package responsed data
 * @param[in]   entrypt_key             encryption key
 * @param[in]   deviceid                device id
 * @param[in]   error_code              The error code that needs to response to app. all kind of error code that is defined in this file that is called aiio_protocol_code.h
 * 
 * @return  int  Reture the length of the packaged data or all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_PacketApResponseData(char *packet_data, uint32_t packet_data_len,  aiio_ap_entrypt_type_t entrypt_type, char *entrypt_key, char *deviceid, int8_t error_code);



#endif
