#include "aiio_adapter_include.h"
#include "axk_uart_init.h"
#include "axk_ringbuff.h"

#define USER_UART_BAUD  CONFIG_AXK_AT_UART_BAUD

static void axk_uart_rx_cb(uint8_t *buf, uint16_t size);
static void axk_uart_recv_task(void *param);
static int32_t axk_uart_init(void);
static void axk_at_send_data(uint8_t *data, uint16_t len);

axk_uart_init_t g_uart =
{
    .init = axk_uart_init,
    .send_data = axk_at_send_data,
};

static aiio_uart_config_t uart =
{
    .uart_num = USER_UART,
    .baud_rate = USER_UART_BAUD,
    .uart_tx_pin = USER_UART_TX,
    .uart_rx_pin = USER_UART_RX,
    .irq_rx_bk = axk_uart_rx_cb,
};

static void axk_uart_rx_cb(uint8_t *buf, uint16_t size)
{
    g_ringbuff_ctrl.push_data(buf, size);
}

static int32_t axk_uart_init(void)
{
    return aiio_uart_init(uart);
}

static void axk_at_send_data(uint8_t *data, uint16_t len)
{
    //TODOmutex lock
    aiio_uart_send_data(USER_UART, data, len);
}