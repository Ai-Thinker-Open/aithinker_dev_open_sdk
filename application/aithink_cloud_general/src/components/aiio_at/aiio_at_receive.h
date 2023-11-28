/**
 * @brief   Declare interface of receiving at command
 * 
 * @file    aiio_at_receive.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-07          <td>1.0.0            <td>zhuolm             <td>
 */
#ifndef __AIIO_AT_RECEIVE_H_
#define __AIIO_AT_RECEIVE_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>



typedef void (*Cmd_ProcessCb)(char *cmd, uint16_t cmd_len);

typedef struct
{
    char *cmd;
    Cmd_ProcessCb func;
} ATCmdInfo;




#define AT_RESPONSE(x)              aiio_at_cmd_response(x)




/**
 * @brief   This function is mainly processing AT command
 * 
 * @param[in]   buff     Receiving AT command
 * 
 */
void atCmdExecute(char *buff);

/**
 * @brief   Response AT command
 * 
 * @param[in]   cmd    AT command
 * 
 */
void aiio_at_cmd_response(char *cmd);



#endif
