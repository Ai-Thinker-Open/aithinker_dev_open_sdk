/**
 * @brief   Declare interface of ota protocol parse
 * 
 * @file    aiio_ota_protocol.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-20          <td>1.0.0            <td>zhuolm             <td> The interface of ota protocol parse
 */

#ifndef __AIIO_OTA_PROTOCOL_H_
#define __AIIO_OTA_PROTOCOL_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "aiio_protocol_comm.h"




typedef struct 
{
    uint8_t channel;                        /*!< ota upgrade channel, 1: cloud, 2: APP*/
    char    *pointVer;                      /*!< The appointed version of OTA, only the appointed ota version device can upgrade*/
    char    *baseVer;                       /*!< OTA firmware Minimum compatible version, for example : The baseVer is 2.0.1, it can upgrade that deivce version is 2.0.2, but it can't upgrade that device version is 2.0.0*/
    char    *otaVer;                        /*!< The ota firmware version*/
    char    *mcuBaseVer;                    /*!< The third-party OTA firmware Minimum compatible version, for example : The baseVer is 2.0.1, it can upgrade that deivce version is 2.0.2, but it can't upgrade that device version is 2.0.0*/
    char    *otaType;                       /*!< The ota firmware type, such as module_ota_all or module_mcu_all*/
    char    *download_url;                  /*!< The ota firmware download link */
    char    *PubId;                         /*!< The publish ID from cloud*/
    char    *md5;                           /*!< The ota firmware md5 data*/
    uint32_t timeout;                       /*!< The ota firmware upgrade timeout time*/
}aiio_ota_protocol_msg_t;



/**
 * @brief   Parse ota protocol message
 * 
 * @param[in]   ota_param                   The original ota protocol data
 * @param[in]   ota_protocol_msg            The protocol message
 * 
 * @return  int  Reture all kind of error code that is defined in aiio_protocol_code.h or aiio_error.h
 */
int aiio_OtaProtocolParse(char *ota_param, aiio_ota_protocol_msg_t *ota_protocol_msg);



#endif

