/** @brief      AT parse interface.
 *
 *  @file       aiio_at_parse.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/03/01      <td>1.0.0       <td>lins        <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_PARSE_H__
#define __AIIO_AT_PARSE_H__

#include "aiio_type.h"

int at_cmd_parse(char *cmd_line, aiio_at_cmd_attr *cmd_parsed);

#endif
