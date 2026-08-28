#include "aiio_uart.h"
#include "hosal_uart.h"
#include "aiio_error.h"
#include "stdarg.h"

typedef struct
{
    uint8_t inited : 1;
    uint8_t rx_irq_en : 1;
    uint8_t tx_irq_en : 1;
    uint8_t reserved : 5;
    hosal_uart_dev_t uart;
    aiio_uart_int_callback irq_rx_bk; /*!< UART recv data callback function*/
} aiio_uart_dev_t;

static int uart_log_num = 0;

static aiio_uart_dev_t uart_instance[] =
    {
        [AIIO_UART0] = {
            .inited = 0,
            .irq_rx_bk = NULL,
            .uart = {
                .config = {
                    .uart_id = AIIO_UART0,
                    .tx_pin = 17,
                    .rx_pin = 6,
                    .cts_pin = 255,
                    .rts_pin = 255,
                    .baud_rate = 921600,
                    .data_width = HOSAL_DATA_WIDTH_8BIT,
                    .parity = HOSAL_NO_PARITY,
                    .stop_bits = HOSAL_STOP_BITS_1,
                    .mode = HOSAL_UART_MODE_POLL,
                },
            },
        },
        [AIIO_UART1] = {
            .inited = 0,
            .irq_rx_bk = NULL,
            .uart = {
                .config = {
                    .uart_id = AIIO_UART1,
                    .tx_pin = 4,
                    .rx_pin = 5,
                    .cts_pin = 255,
                    .rts_pin = 255,
                    .baud_rate = 115200,
                    .data_width = HOSAL_DATA_WIDTH_8BIT,
                    .parity = HOSAL_NO_PARITY,
                    .stop_bits = HOSAL_STOP_BITS_1,
                    .mode = HOSAL_UART_MODE_POLL,
                },
            },
        },

};
/**
 * hal uart RX interrupt callback
 */
static int __uart_rx_callback(void *p_arg)
{
    int ret;
    uint8_t data_buf[32];
    hosal_uart_dev_t *p_dev = (hosal_uart_dev_t *)p_arg;

    ret = hosal_uart_receive(p_dev, data_buf, sizeof(data_buf));
    if(ret)
    {
        if (uart_instance[p_dev->config.uart_id].irq_rx_bk)
        {
            (*uart_instance[p_dev->config.uart_id].irq_rx_bk)(data_buf, ret);
        }
    }
    return 0;
}


int32_t aiio_uart_log_init(aiio_uart_config_t uart)
{
    HOSAL_UART_DEV_DECL(uart_dev, uart.uart_num, uart.uart_tx_pin, uart.uart_rx_pin, uart.baud_rate);

    if (hosal_uart_init(&uart_dev) == AIIO_OK)
    {
        uart_log_num = uart.uart_num;
        return AIIO_OK;
    }

    return AIIO_ERROR;
}

int32_t aiio_uart_log_send(const char *fmt, ...)
{
    extern int bl_uart_data_send(uint8_t id, uint8_t data);
    va_list argp;
    va_start(argp, fmt);
    int ch;
    char *str;
    static char string[AIIO_LOG_MAX_LEN] = { 0 };

    memset(string, 0, AIIO_LOG_MAX_LEN);

    str = string;

    if (0 < vsprintf(string, fmt, argp))
    {
        while ('\0' != (ch = *(str++)))
        {
            bl_uart_data_send(uart_log_num, ch);
        }
    }

    va_end(argp);
    return AIIO_OK;
}

int32_t aiio_uart_param_check(aiio_uart_config_t uart)
{
    /*aiio_uart_log_send("\r\n\r\n\r\n[%s()-%d]uart_num=%d AIIO_UART_MAX=%d\r\n"
                    "uart_tx_pin=%d\r\n"
                    "uart_rx_pin=%d\r\n"
                    "baud_rate=%d\r\n"
                    "dataBits=%d\r\n"
                    "stopBits=%d\r\n"
                    "parity=%d\r\n\r\n\r\n",__func__,__LINE__,
                    uart.uart_num,AIIO_UART_MAX,
                    uart.uart_tx_pin,
                    uart.uart_rx_pin,
                    uart.baud_rate,
                    uart.dataBits,
                    uart.stopBits,
                    uart.parity);*/
    if(uart.uart_num >= AIIO_UART_MAX){
        return AIIO_UART_NUMBER_NOT_SUPPORT;
    }
    if(uart.uart_tx_pin >= __AIIO_GPIO_NUM_MAX){
        return AIIO_UART_PIN_NOT_SUPPORT;
    }
    if(uart.uart_rx_pin >= __AIIO_GPIO_NUM_MAX){
        return AIIO_UART_PIN_NOT_SUPPORT;
    }
    switch(uart.dataBits){
        case AIIO_UART_DATA_BITS_5:
        case AIIO_UART_DATA_BITS_6:
        case AIIO_UART_DATA_BITS_7:
        case AIIO_UART_DATA_BITS_8:
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.stopBits){
        case AIIO_UART_STOPBITS_0_5:
        case AIIO_UART_STOP_BITS_1:
        case AIIO_UART_STOP_BITS_2:
        case AIIO_UART_STOP_BITS_1_5:
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.parity){
        case AIIO_UART_PARITY_NONE:
        case AIIO_UART_PARITY_ODD:
        case AIIO_UART_PARITY_EVEN:
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.flowControl){
        case AIIO_UART_FLOW_CONTROL_NONE:
            break;
        case AIIO_UART_FLOW_CONTROL_RTS:
        case AIIO_UART_FLOW_CONTROL_CTS:
        case AIIO_UART_FLOW_CONTROL_RTS_CTS:
            return AIIO_UART_FLOW_CONTROL_NOT_SUPPORT;
        default:
            return AIIO_PARAM_ERROR;
    }
    return AIIO_OK;
}

int32_t aiio_uart_init(aiio_uart_config_t uart)
{
    if(uart.uart_num>=AIIO_UART_MAX)
    return AIIO_ERROR;

    // if(!uart_instance[uart.uart_num].inited){
    //     hosal_uart_finalize(&uart_instance[uart.uart_num].uart);
    // }

    hosal_uart_dev_t *uart_dev =  &uart_instance[uart.uart_num].uart;
    if (uart.uart_tx_pin)
        uart_dev->config.tx_pin = uart.uart_tx_pin;
    if (uart.uart_rx_pin)
        uart_dev->config.rx_pin = uart.uart_rx_pin;
    if (uart.baud_rate)
        uart_dev->config.baud_rate = uart.baud_rate;

    switch(uart.dataBits){
        case AIIO_UART_DATA_BITS_5:
            uart_dev->config.data_width=HOSAL_DATA_WIDTH_5BIT;
            break;
        case AIIO_UART_DATA_BITS_6:
            uart_dev->config.data_width=HOSAL_DATA_WIDTH_6BIT;
            break;
        case AIIO_UART_DATA_BITS_7:
            uart_dev->config.data_width=HOSAL_DATA_WIDTH_7BIT;
            break;
        case AIIO_UART_DATA_BITS_8:
            uart_dev->config.data_width=HOSAL_DATA_WIDTH_8BIT;
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.stopBits){
        case AIIO_UART_STOPBITS_0_5:
        case AIIO_UART_STOP_BITS_1:
        case AIIO_UART_STOP_BITS_2:
        case AIIO_UART_STOP_BITS_1_5:
            uart_dev->config.stop_bits = uart.stopBits;
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.parity){
        case AIIO_UART_PARITY_NONE:
        case AIIO_UART_PARITY_ODD:
        case AIIO_UART_PARITY_EVEN:
            uart_dev->config.parity = uart.parity;
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.flowControl){
        case AIIO_UART_FLOW_CONTROL_NONE:
            break;
        case AIIO_UART_FLOW_CONTROL_RTS:
        case AIIO_UART_FLOW_CONTROL_CTS:
        case AIIO_UART_FLOW_CONTROL_RTS_CTS:
            return AIIO_UART_FLOW_CONTROL_NOT_SUPPORT;
        default:
            return AIIO_PARAM_ERROR;
    }

    if (hosal_uart_init(uart_dev) == AIIO_OK)
    {
        if (uart.irq_rx_bk)
        {
            uart_instance[uart.uart_num].irq_rx_bk = uart.irq_rx_bk;

            /* Configure UART to interrupt mode */
            hosal_uart_ioctl(uart_dev, HOSAL_UART_MODE_SET, (void *)HOSAL_UART_MODE_INT);

            /* Set TX RX interrupt callback */
            hosal_uart_callback_set(uart_dev, HOSAL_UART_RX_CALLBACK,
                                    __uart_rx_callback, uart_dev);

            uart_instance[uart.uart_num].rx_irq_en = 1;
        }
        uart_instance[uart.uart_num].inited = 1;
        return AIIO_OK;
    }
    return AIIO_OK;
}

int32_t aiio_uart_deinit(aiio_uart_config_t uart)
{
    uint8_t uart_num = uart.uart_num;
    if (uart_num >= AIIO_UART_MAX || !uart_instance[uart_num].inited)
        return AIIO_ERROR;

    hosal_uart_dev_t *uart_dev = &uart_instance[uart_num].uart;

    hosal_uart_finalize(&uart_instance[uart_num].uart);

    uart_instance[uart_num].inited = 0;
    uart_instance[uart_num].rx_irq_en = 0;
    uart_instance[uart_num].tx_irq_en = 0;

    return AIIO_OK;
}

int32_t aiio_uart_send_data(uint8_t uart_num, uint8_t *buf, uint16_t size)
{
    if (uart_num >= AIIO_UART_MAX || !uart_instance[uart_num].inited)
        return AIIO_ERROR;

    return hosal_uart_send(&uart_instance[uart_num].uart, buf, size);
}
int32_t aiio_uart_recv_data(uint8_t uart_num, uint8_t *buf, uint16_t size)
{
    if (uart_num >= AIIO_UART_MAX || !uart_instance[uart_num].inited)
        return AIIO_ERROR;

    if(uart_instance[uart_num].rx_irq_en)
        return AIIO_ERROR;

    int ret = hosal_uart_receive(&uart_instance[uart_num].uart, buf, size);

    return ret;
}

