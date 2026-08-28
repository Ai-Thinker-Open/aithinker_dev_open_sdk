/**
 * @brief   Declare ota launch interface and ota data transmit callback interface
 * 
 * @file    aiio_ota_transmit.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-08-04          <td>1.0.0            <td>zhuolm             <td> The ota launch interface and ota data transmit callback interface
 */
#ifndef __AIIO_OTA_TRANSMIT_H_
#define __AIIO_OTA_TRANSMIT_H_
#include "aiio_common.h"



#define SOH                     (0x01)  /* start of 128-byte data packet */
#define STX                     (0x02)  /* start of 1024-byte data packet */
#define EOT                     (0x04)  /* end of transmission */
#define ACK                     (0x06)  /* acknowledge */
#define NAK                     (0x15)  /* negative acknowledge */
#define CAN                     (0x18)  /* two of these in succession aborts transfer */
#define CRC16                   (0x43)  /* 'C' == 0x43, request 16-bit CRC */
  /* Normal Xmodem data length */

#define XMODEM_HEAD_LEN           3       /* SOH + BLK + 255 - BLK */
#define XMODEM_CRC_LEN            1
#define XMODEM_ACK_LEN            1

#define MCU_REPLY_INIT          (0X00)
#define MCU_REPLY_TIMEOUT       (0XFF)


#define XMODEM_1k_DATA_LEN            1024  
#define XMODEM_1k_PACKET_LEN          1028

#define XMODEM_DATA_LEN             128  
#define XMODEM_PACKET_LEN           132

 
 





int aiio_ota_transmit_start(uint8_t *ota_info);
int aiio_ota_transmit_stop(void);




#endif

