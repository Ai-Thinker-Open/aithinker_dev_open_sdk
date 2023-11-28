/**
 * @brief   Declare the protocol interface of Lan network communication
 * 
 * @file    aiio_lan_protocol_parse.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note    This file is mainly for describing Lan network communication interface, it will declare some parsing protocol interface.
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-15          <td>1.0.0            <td>zhuolm             <td>  The protocol interface of Lan network communication
 */
#ifndef __AIIO_LAN_PROTOCOL_PARSE_H_
#define __AIIO_LAN_PROTOCOL_PARSE_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "aiio_protocol_comm.h"



/**
 * @brief The encryption type of the lan network communication
*/
typedef enum 
{
    PROTOCOL_LAN_ENTRYPT_NONE = 0,              /*!< The type that is not encrypted*/
    PROTOCOL_LAN_ENTRYPT_AES_CBC_128,            /*!< The type that is using the AES CBC 128*/    
    PROTOCOL_LAN_ENTRYPT_UNKNOW = 0xff          /*!< The type that is unknow */
}aiio_lan_entrypt_type_t;


/**
 * @brief The status of the protocol data parsing
*/
typedef enum 
{
    AIIO_PROTOCOL_CRC_CHECK_ERR = 0,
    AIIO_PROTOCOL_VER_CHECK_ERR,
    AIIO_PROTOCOL_FRAME_CHECK_ERR,
    AIIO_PROTOCOL_LEN_CHECK_ERR,
    AIIO_PROTOCOL_HARD_ERR,
    AIIO_PROTOCOL_PARSE_FAIL,
    AIIO_PROTOCOL_PARSE_OK
}aiio_lan_protocol_parse_state_t;


/**
 * @brief The structure data of the protocol data parsing
*/
typedef struct 
{
    char                                *data;
    uint8_t                             frame;
    uint16_t                            data_len;
    aiio_lan_entrypt_type_t             data_encrypt_type;
    aiio_lan_protocol_parse_state_t     protocol_parse_state;
}aiio_lan_protocol_parse_event_t;


typedef void (*aiio_lan_protocol_parse_cb)(aiio_lan_protocol_parse_event_t *parse_event);


/**
 * @brief   Parse Lan protocol format data that is from APP
 * 
 * @param[in]   data            The protocol data in Lan protocol format
 * @param[in]   data_len        The length of the protocol data in Lan protocol format
 * @param[in]   parse_data_cb   The callback function, it will transfers Lan data that is parsed
 * 
 * @return  int  Reture all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_ParseLanProtocolData(uint8_t *data, uint16_t data_len, aiio_lan_protocol_parse_cb parse_data_cb);


#endif