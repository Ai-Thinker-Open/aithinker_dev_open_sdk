#ifndef _AXK_HTTP_SERVER_H_
#define _AXK_HTTP_SERVER_H_

#include <stdint.h>
#include "aiio_adapter_include.h"

typedef struct axk_http_server
{
    aiio_httpd_handle_t server_handle;
    int32_t (*init)(void);
    int32_t (*deinit)(void);
    uint8_t (*get_start_flag)(void);
} axk_http_server_t;


extern axk_http_server_t g_axk_http_server;

#endif //_AXK_HTTP_SERVER_H_
