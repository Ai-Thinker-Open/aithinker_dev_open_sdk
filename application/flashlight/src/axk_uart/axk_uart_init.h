#ifndef _AXK_UART_INIT_H_
#define _AXK_UART_INIT_H_

#include <stdint.h>

#define USER_UART       AIIO_UART0
#define USER_UART_TX    __AIIO_UART0_TX
#define USER_UART_RX    __AIIO_UART0_RX

typedef struct axk_uart_init
{
    int32_t (*init)(void);
    void (*send_data)(uint8_t *data, uint16_t len);
} axk_uart_init_t;


extern axk_uart_init_t g_uart;

#endif //_AXK_UART_INIT_H_
