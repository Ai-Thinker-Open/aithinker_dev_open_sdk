/** @brief      AT http command.
 *
 *  @file       aiio_at_http.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/03/01      <td>1.0.0       <td>wusen       <td>First version
 *  <tr><td>2023/04/07      <td>1.0.1       <td>wusen       <td>add at-http api
 *  </table>
 *
 */
#ifndef __AIIO_AT_HTTP_H__
#define __AIIO_AT_HTTP_H__
#include <stdint.h>
#include "aiio_type.h"

typedef enum{
    AIIO_AT_TRANSPORT_TYPE_HTTP=1,
    AIIO_AT_TRANSPORT_TYPE_HTTPS=2,
}aiio_at_http_transport_type_t;

typedef enum{
    AIIO_AT_HTTP_GET=2,
    AIIO_AT_HTTP_POST=3,
}aiio_at_http_opt_type_t;


typedef enum{
    AIIO_AT_HTTP_PARSE_INIT=0,
    AIIO_AT_HTTP_PARSE_ING=1,
    AIIO_AT_HTTP_FIRST_PARSE_COMNPLETE=2,
    AIIO_AT_HTTP_NOTFIRST_PARSE_COMNPLETE=3,
    AIIO_AT_HTTP_PARSE_ERROR=-1,
    AIIO_AT_HTTP_PARSE_STATUS_ERROR=-2,
    AIIO_AT_HTTP_HEAD_FORMAT_ERROR=-3,
}aiio_at_http_parse_response_t;

CORE_API int32_t aiio_at_http_cmd_regist(void);

#endif
