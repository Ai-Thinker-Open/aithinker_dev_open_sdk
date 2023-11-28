#ifndef _SYS_DRIVER_PORT_H_
#define _SYS_DRIVER_PORT_H_

#include "serial.h"
#include "serial_hw.h"
#include "ln_at_transfer.h"
#include "utils/debug/log.h"


//UART
//******************************************
typedef struct
{
    uint8_t databit;
    uint8_t stopbit;
    uint8_t parity;
    uint8_t flow;
    uint32_t baud;
}sys_uart_status_t;

typedef struct
{
    void (*uart_init)(uint32_t baud);
    void (*uart_deinit)(void);
    int (*uart_set_baud)(uint32_t baud);
}sys_uart_func_t;
//******************************************

//SYS
//******************************************
typedef struct
{
    sys_uart_status_t uart_status;
}at_sys_status_t;

typedef struct
{
    void (*adapter_init_func)(void);
    
    sys_uart_func_t uart_func;
}at_sys_func_t;

typedef struct
{
    at_sys_status_t sys_status;
    
    at_sys_func_t sys_func;
}at_sys_adapter_t;

extern at_sys_adapter_t* at_sys_adapter_get_handle(void);

#endif

