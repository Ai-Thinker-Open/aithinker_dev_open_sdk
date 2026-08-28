#ifndef _AXK_HTTP_SERVER_DATA_PARSE_H_
#define _AXK_HTTP_SERVER_DATA_PARSE_H_

#include <stdint.h>
#include <stdio.h>

typedef struct axk_http_server_data_parse
{
    int32_t (*http_server_mqtt_data_parse)(const char *data);
    int32_t (*http_server_wifi_data_parse)(const char *data);
} axk_http_server_data_parse_t;

extern axk_http_server_data_parse_t g_axk_http_server_data_parse;

#endif //_AXK_HTTP_SERVER_DATA_PARSE_H_
