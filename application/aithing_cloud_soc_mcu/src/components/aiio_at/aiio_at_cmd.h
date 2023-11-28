/**
 * @brief   Declare AT command process interface
 * 
 * @file    aiio_at_cmd.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-07          <td>1.0.0            <td>zhuolm             <td>
 */
#ifndef __AIIO_AT_CMD_H_
#define __AIIO_AT_CMD_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>



void AT_Test(char *cmd, uint16_t cmd_len);
void At_Key(char *cmd, uint16_t cmd_len);



#endif
