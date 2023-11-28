/** @brief
 *  
 *  @file        main.h
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note 
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/07/31          <td>V1.0.0          <td>hewm            <td>
 *  <table>
 *  
 */

#pragma once

typedef struct 
{
    uint8_t wifi_sta:1;
    uint8_t reserved:7;
}user_info_t;

extern user_info_t user_info;


