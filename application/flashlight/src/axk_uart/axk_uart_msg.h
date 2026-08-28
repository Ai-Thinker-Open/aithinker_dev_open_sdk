#ifndef _AXK_UART_MSG_H_
#define _AXK_UART_MSG_H_

#include <stdint.h>

typedef struct axk_uart_msg
{
    int32_t (*init)(void);
} axk_uart_msg_t;

extern axk_uart_msg_t g_uart_msg;

#endif //_AXK_UART_MSG_H_
