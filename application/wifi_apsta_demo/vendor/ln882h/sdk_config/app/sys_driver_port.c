#include "sys_driver_port.h"

static void at_sys_adapter_init(void);

static void at_sys_uart_init(uint32_t baud);
static void at_sys_uart_deinit(void);
static int at_sys_uart_set_baud(uint32_t baud);


at_sys_adapter_t g_sys_adapter ={
    .sys_func.adapter_init_func = at_sys_adapter_init,
};

at_sys_adapter_t* at_sys_adapter_get_handle(void)
{
    return &g_sys_adapter;
}

static void at_sys_adapter_init(void)
{
    at_sys_adapter_t *pada = at_sys_adapter_get_handle();

    pada->sys_func.adapter_init_func = at_sys_adapter_init;
    //uart
    pada->sys_func.uart_func.uart_deinit = NULL;
    pada->sys_func.uart_func.uart_init = at_sys_uart_init;
    pada->sys_func.uart_func.uart_set_baud = at_sys_uart_set_baud;
}

static void at_sys_uart_init(uint32_t baud)
{
    at_sys_serial_handle_t *phdl = at_sys_serial_get_handle();
    serial_init(phdl->serial_hdl, SER_PORT_UART1, baud, phdl->rx_cb);
}

static void at_sys_uart_deinit(void)
{
    at_sys_serial_handle_t *phdl = at_sys_serial_get_handle();
    serial_deinit(phdl->serial_hdl);
}

static int at_sys_uart_set_baud(uint32_t baud)
{
    at_sys_serial_handle_t *phdl = at_sys_serial_get_handle();
    return serial_setbaudrate(phdl->serial_hdl, baud);
}


