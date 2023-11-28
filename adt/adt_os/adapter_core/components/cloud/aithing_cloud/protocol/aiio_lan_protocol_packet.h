/**
 * @brief   Declare the protocol interface of Lan network communication
 * 
 * @file    aiio_lan_protocol_packet.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note    This file is mainly for describing Lan network communication interface, it will declare some packing protocol interface.
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-15          <td>1.0.0            <td>zhuolm             <td>  The protocol interface of Lan network communication
 */
#ifndef __AIIO_LAN_PROTOCOL_PACKET_H_
#define __AIIO_LAN_PROTOCOL_PACKET_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "aiio_protocol_comm.h"


#define     LAN_PRO_HEAD_LEN                    6
#define     LAN_PRO_CRC_LEN                     2
#define     LAN_PRO_ENC_FLAG                    1
#define     LAN_PRO_NO_ENC_FLAG                 0
#define     LAN_PRO_WHOLE_DATA                  1
#define     LAN_PRO_HIGHT_4BIT                  4

#define     LAN_PROTOCOL_DATA_MAX_LEN           1000

#define      LAN_PROTOCOL_VERSION                1


typedef enum
{   
    LAN_PROTOCOL_VERSIN_POSITION = 0,
    LAN_PROTOCOL_FRAME_POSITION,
    LAN_PROTOCOL_TOTAL_LEN_POSITION,
    LAN_PROTOCOL_DIVIDE_FLAG_POSITION = 4,
    LAN_PROTOCOL_ENTRYPT_TYPE_POSITION,
    LAN_PROTOCOL_DATA_POSITION = 6,
    LAN_PROTOCOL_CRC_POSITION = 6
}lan_protocol_structure_t;



typedef void (*aiio_lan_protocol_cb)(char *data, uint16_t data_len);


/**
 * @brief   This function is mainly packaged with unencrpted response data
 * @note    This function is mainly to package the source_data in the protocol format, it will send the packaged protocol data to the APP throuth the response_cb.
 * 
 * @param[in]   source_data             response data for APP
 * @param[in]   source_data_len         the length of response
 * @param[in]   frame                   received frame data from APP
 * @param[in]   response_cb             callback function, This callback function will output the packaged protocol data
 * 
 * @return  int  Reture the length of the packaged data or all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_PacketLanNoEncryptResponseData(char *source_data, uint16_t source_data_len, uint8_t frame, aiio_lan_protocol_cb response_cb);


/**
 * @brief   This function is mainly packaged with encrpted response data
 * @note    This function is mainly to package the source_data in the protocol format, it will send the packaged protocol data to the APP throuth the response_cb.
 * 
 * @param[in]   source_data             response data for APP
 * @param[in]   source_data_len         the length of response
 * @param[in]   frame                   received frame data from APP
 * @param[in]   response_cb             callback function, This callback function will output the packaged protocol data
 * 
 * @return  int  Reture the length of the packaged data or all kind of error code that is defined in this file that is called aiio_protocol_code.h
 */
int aiio_PacketLanEncryptResponseData(char *source_data, uint16_t source_data_len, uint8_t frame, aiio_lan_protocol_cb response_cb);



#endif