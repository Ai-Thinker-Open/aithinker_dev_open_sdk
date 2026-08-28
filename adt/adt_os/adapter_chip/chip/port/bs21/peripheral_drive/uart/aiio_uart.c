#include "aiio_uart.h"
#include "stdio.h"
#include "string.h"
#include "aiio_error.h"
#include "aiio_log.h"
#include "pinctrl.h"
#include "soc_osal.h"
#include "common_def.h"
#include "uart.h"
#include "hal_uart.h"

static uint8_t uart_log_num = 0;
static uint8_t g_uart0_rx_buff[512] = {0};
static uint8_t g_uart1_rx_buff[512] = {0};
static uart_buffer_config_t g_uart0_buffer_config = {
    .rx_buffer = g_uart0_rx_buff,
    .rx_buffer_size = 512};
static uart_buffer_config_t g_uart1_buffer_config = {
    .rx_buffer = g_uart1_rx_buff,
    .rx_buffer_size = 512};

static int usr_uart_write_data(int num, char *buff, unsigned int size)
{
    int ret = uapi_uart_write(num, (unsigned char *)buff, size, 0);
    if (ret == -1)
    {
        return ERRCODE_FAIL;
    }
    return ERRCODE_SUCC;
}

static int32_t usr_uart_read_data(int num, uint8_t *buff, uint32_t len, uint32_t timeout)
{
    buff = buff;
    len = len;
    return uapi_uart_read(num, g_uart1_rx_buff, 512, timeout);
}

static void uart0_read_int_handler(const void *buffer, uint16_t length, bool error)
{
    aiio_log_w("ble_uart_read_int_handler server.\r\n");
    unused(error);
    unused(buffer);
    unused(length);
    // if (ble_uart_get_connection_state() != 0)
    // {
    //     msg_data_t msg_data = {0};
    //     void *buffer_cpy = osal_vmalloc(length);
    //     if (memcpy_s(buffer_cpy, length, buffer, length) != EOK)
    //     {
    //         osal_vfree(buffer_cpy);
    //         return;
    //     }
    //     msg_data.value = (uint8_t *)buffer_cpy;
    //     msg_data.value_len = length;
    //     int msg_ret = osal_msg_queue_write_copy(mouse_msg_queue, (void *)&msg_data, msg_rev_size, 0);
    //     if (msg_ret != OSAL_SUCCESS)
    //     {
    //         osal_printk("msg queue write copy fail.");
    //         osal_vfree(msg_data.value);
    //     }
    // }
}

static void uart1_read_int_handler(const void *buffer, uint16_t length, bool error)
{
    aiio_log_w("ble_uart_read_int_handler server.\r\n");
    unused(error);
    unused(buffer);
    unused(length);
    // if (ble_uart_get_connection_state() != 0)
    // {
    //     msg_data_t msg_data = {0};
    //     void *buffer_cpy = osal_vmalloc(length);
    //     if (memcpy_s(buffer_cpy, length, buffer, length) != EOK)
    //     {
    //         osal_vfree(buffer_cpy);
    //         return;
    //     }
    //     msg_data.value = (uint8_t *)buffer_cpy;
    //     msg_data.value_len = length;
    //     int msg_ret = osal_msg_queue_write_copy(mouse_msg_queue, (void *)&msg_data, msg_rev_size, 0);
    //     if (msg_ret != OSAL_SUCCESS)
    //     {
    //         osal_printk("msg queue write copy fail.");
    //         osal_vfree(msg_data.value);
    //     }
    // }
}

int32_t aiio_uart_init(aiio_uart_config_t uart)
{
    uart_attr_t attr = {
        .baud_rate = uart.baud_rate,
        .data_bits = UART_DATA_BIT_8,
        .stop_bits = UART_STOP_BIT_1,
        .parity = UART_PARITY_NONE};

    uart_pin_config_t pin_config = {
        .tx_pin = uart.uart_tx_pin,
        .rx_pin = uart.uart_rx_pin,
        .cts_pin = PIN_NONE,
        .rts_pin = PIN_NONE};

    aiio_log_w("UART Configuration:");

    aiio_log_w("  uart_log_num      : %u\r\n", uart_log_num);
    aiio_log_w("  uart_num      : %u\r\n", uart.uart_num);
    aiio_log_w("  uart_tx_pin   : %u\r\n", uart.uart_tx_pin);
    aiio_log_w("  uart_rx_pin   : %u\r\n", uart.uart_rx_pin);
    aiio_log_w("  baud_rate     : %lu\r\n", (unsigned long)uart.baud_rate);
    aiio_log_w("  irq_rx_bk     : %p\r\n", (void *)uart.irq_rx_bk);
    aiio_log_w("  dataBits      : %d\r\n", uart.dataBits);
    aiio_log_w("  stopBits      : %d\r\n", uart.stopBits);
    aiio_log_w("  parity        : %d\r\n", uart.parity);
    aiio_log_w("  flowControl   : %d\r\n", uart.flowControl);
    aiio_log_w("  in_at_mode    : %u\r\n", uart.in_at_mode);

    // 判断串口号是否正常，在工作范围之内
    if (uart.uart_num >= AIIO_UART_MAX)
    {
        return AIIO_ERROR;
    }

    // 停止位配置
    switch (uart.stopBits)
    {
    case AIIO_UART_STOPBITS_0_5:
    case AIIO_UART_STOP_BITS_1:
        attr.stop_bits = UART_STOP_BIT_1;
        break;
    case AIIO_UART_STOP_BITS_1_5:
    case AIIO_UART_STOP_BITS_2:
        attr.stop_bits = UART_STOP_BIT_2;
        break;
    default:
        return AIIO_PARAM_ERROR;
    }

    // 数据位配置
    switch (uart.dataBits)
    {
    case AIIO_UART_DATA_BITS_5:
        attr.data_bits = UART_DATA_BIT_5;
        break;
    case AIIO_UART_DATA_BITS_6:
        attr.data_bits = UART_DATA_BIT_6;
        break;
    case AIIO_UART_DATA_BITS_7:
        attr.data_bits = UART_DATA_BIT_7;
        break;
    case AIIO_UART_DATA_BITS_8:
        attr.data_bits = UART_DATA_BIT_8;
        break;
    default:
        return AIIO_PARAM_ERROR;
    }

    // 奇偶校验位配置
    switch (uart.parity)
    {
    case AIIO_UART_PARITY_NONE:
        attr.parity = UART_PARITY_NONE;
        break;
    case AIIO_UART_PARITY_ODD:
        attr.parity = UART_PARITY_ODD;
        break;
    case AIIO_UART_PARITY_EVEN:
        attr.parity = UART_PARITY_EVEN;
        break;
    default:
        return AIIO_PARAM_ERROR;
    }

    // 硬件流控制
    // switch(uart.flowControl){
    //     case AIIO_UART_FLOW_CONTROL_NONE:
    //         break;
    //     case AIIO_UART_FLOW_CONTROL_RTS:
    //     case AIIO_UART_FLOW_CONTROL_CTS:
    //     case AIIO_UART_FLOW_CONTROL_RTS_CTS:
    //         return AIIO_UART_FLOW_CONTROL_NOT_SUPPORT;
    //     default:
    //         return AIIO_PARAM_ERROR;
    // }
    switch (uart.uart_num)
    {
    case 0:
        uapi_uart_deinit(uart.uart_num);
        uapi_pin_set_mode(uart.uart_tx_pin, HAL_PIO_UART_L0_TXD); /* uart0 tx */
        uapi_pin_set_mode(uart.uart_rx_pin, HAL_PIO_UART_L0_RXD); /* uart0 rx */
        uapi_uart_init(uart.uart_num, &pin_config, &attr, NULL, &g_uart0_buffer_config);

        uapi_uart_unregister_rx_callback(uart.uart_num);
        if (uapi_uart_register_rx_callback(uart.uart_num, UART_RX_CONDITION_FULL_OR_SUFFICIENT_DATA_OR_IDLE, 1, uart0_read_int_handler) != ERRCODE_SUCC)
        {
            return AIIO_ERROR;
        }
        break;
    case 1:
        uapi_uart_deinit(uart.uart_num);
        uapi_pin_set_mode(uart.uart_tx_pin, HAL_PIO_UART_H0_TXD); /* uart1 tx */
        uapi_pin_set_mode(uart.uart_rx_pin, HAL_PIO_UART_H0_RXD); /* uart1 rx */
        uapi_uart_init(uart.uart_num, &pin_config, &attr, NULL, &g_uart1_buffer_config);

        uapi_uart_unregister_rx_callback(uart.uart_num);
        if (uapi_uart_register_rx_callback(uart.uart_num, UART_RX_CONDITION_FULL_OR_SUFFICIENT_DATA_OR_IDLE, 1, uart1_read_int_handler) != ERRCODE_SUCC)
        {
            return AIIO_ERROR;
        }
        break;
    default:

        break;
    }

    aiio_log_w("uart init success.");

    return AIIO_OK;
}

int32_t aiio_uart_deinit(aiio_uart_config_t uart)
{
    uapi_uart_deinit(uart.uart_num);

    return AIIO_OK;
}

int32_t aiio_uart_send_data(uint8_t uart_num, uint8_t *buf, uint16_t size)
{
    if (uart_num >= AIIO_UART_MAX)
    {
        return AIIO_ERROR;
    }

    return usr_uart_write_data(uart_num, (char *)buf, size);
}

int32_t aiio_uart_recv_data(uint8_t uart_num, uint8_t *buf, uint16_t size)
{
    if (uart_num >= AIIO_UART_MAX)
    {
        return AIIO_ERROR;
    }

    return usr_uart_read_data(uart_num, buf, size, 0);
}

int32_t aiio_uart_log_init(aiio_uart_config_t uart)
{
    uart_attr_t attr = {
        .baud_rate = uart.baud_rate,
        .data_bits = UART_DATA_BIT_8,
        .stop_bits = UART_STOP_BIT_1,
        .parity = UART_PARITY_NONE};

    uart_pin_config_t pin_config = {
        .tx_pin = uart.uart_tx_pin,
        .rx_pin = uart.uart_rx_pin,
        .cts_pin = PIN_NONE,
        .rts_pin = PIN_NONE};

    switch (uart.uart_num)
    {
    case 0:
        uapi_uart_deinit(uart.uart_num);
        uapi_pin_set_mode(uart.uart_tx_pin, HAL_PIO_UART_L0_TXD); /* uart0 tx */
        uapi_pin_set_mode(uart.uart_rx_pin, HAL_PIO_UART_L0_RXD); /* uart0 rx */
        uapi_uart_init(uart.uart_num, &pin_config, &attr, NULL, &g_uart0_buffer_config);
        break;
    case 1:
        uapi_uart_deinit(uart.uart_num);
        uapi_pin_set_mode(uart.uart_tx_pin, HAL_PIO_UART_H0_TXD); /* uart1 tx */
        uapi_pin_set_mode(uart.uart_rx_pin, HAL_PIO_UART_H0_RXD); /* uart1 rx */
        uapi_uart_init(uart.uart_num, &pin_config, &attr, NULL, &g_uart1_buffer_config);
        break;
    default:
        break;
    }

    uart_log_num = uart.uart_num;

    return AIIO_OK;
}

int32_t aiio_uart_log_send(const char *fmt, ...)
{
    if (uart_log_num >= AIIO_UART_MAX)
        return 0;

    char print_buf[512];
    uint32_t len;
    va_list ap;

    va_start(ap, fmt);
    len = vsnprintf(print_buf, sizeof(print_buf), fmt, ap);
    va_end(ap);

    len = (len > sizeof(print_buf)) ? sizeof(print_buf) : len;

    usr_uart_write_data(uart_log_num, (char *)print_buf, len);

    return AIIO_OK;
}
