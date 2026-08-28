#include "aiio_uart.h"
#include "aiio_error.h"
#include "stdarg.h"

#include "bflb_mtimer.h"
#include "bflb_uart.h"
#include "bflb_gpio.h"
#include "board.h"

typedef struct
{
    uint8_t inited;
    uint8_t rx_irq_en;
    uint8_t tx_irq_en;
    uint8_t reserved;
    uint8_t func_uart_tx;
    uint8_t func_uart_rx;
    struct bflb_uart_config_s uart;
    struct bflb_device_s *uartx;
    uint8_t dev_name[6];
    aiio_uart_int_callback irq_rx_bk; /*!< UART recv data callback function*/
} aiio_uart_dev_t;

static uint8_t uart_recv_buf[16];

static aiio_uart_dev_t uart_instance[AIIO_UART_MAX] =
    {
        [AIIO_UART0] = {
            .inited = 0,
            .irq_rx_bk = NULL,
            .dev_name = "uart0",
            .func_uart_tx = GPIO_UART_FUNC_UART0_TX,
            .func_uart_rx = GPIO_UART_FUNC_UART0_RX,
            .uart = {
                .baudrate = 115200,
                .data_bits = UART_DATA_BITS_8,
                .stop_bits = UART_STOP_BITS_1,
                .parity = UART_PARITY_NONE,
                .flow_ctrl = 0,
                .tx_fifo_threshold = 7,
                .rx_fifo_threshold = 7,
            },
        },
        [AIIO_UART1] = {
            .inited = 0,
            .irq_rx_bk = NULL,
            .dev_name = "uart1",
            .func_uart_tx = GPIO_UART_FUNC_UART1_TX,
            .func_uart_rx = GPIO_UART_FUNC_UART1_RX,
            .uart = {
                .baudrate = 115200,
                .data_bits = UART_DATA_BITS_8,
                .stop_bits = UART_STOP_BITS_1,
                .parity = UART_PARITY_NONE,
                .flow_ctrl = 0,
                .tx_fifo_threshold = 7,
                .rx_fifo_threshold = 7,
            },
        },

};

void aiio_uart_isr(int irq, void *arg)
{
    struct bflb_device_s *uartx = NULL;
    uint32_t intstatus;
    uint8_t recv_len;
    for (uint8_t i = 0; i < (sizeof(uart_instance) / sizeof(uart_instance[0])); i++)
    {
        if (uart_instance[i].inited && uart_instance[i].rx_irq_en)
        {
            uartx = uart_instance[i].uartx;
            intstatus = bflb_uart_get_intstatus(uartx);

            if (intstatus & UART_INTSTS_RX_FIFO)
            {
                while (bflb_uart_rxavailable(uartx))
                {
                    memset(uart_recv_buf, 0x00,sizeof(uart_recv_buf));
                    recv_len = bflb_uart_get(uartx, uart_recv_buf, sizeof(uart_recv_buf));
                    (*uart_instance[i].irq_rx_bk)(uart_recv_buf, recv_len);
                }
            }

            if (intstatus & UART_INTSTS_RTO)
            {
                while (bflb_uart_rxavailable(uartx))
                {
                    memset(uart_recv_buf, 0x00,sizeof(uart_recv_buf));
                    recv_len = bflb_uart_get(uartx, uart_recv_buf, sizeof(uart_recv_buf));
                    (*uart_instance[i].irq_rx_bk)(uart_recv_buf, recv_len);
                }
                bflb_uart_int_clear(uartx, UART_INTCLR_RTO);
            }
            if (intstatus & UART_INTSTS_TX_FIFO)
            {
                bflb_uart_txint_mask(uartx, true);
            }
        }
    }
}

int32_t aiio_uart_deinit(aiio_uart_config_t uart)
{
    uint8_t uart_num = uart.uart_num;
    if (uart_num >= AIIO_UART_MAX || !uart_instance[uart_num].inited)
        return AIIO_ERROR;

    uart_instance[uart_num].uartx = bflb_device_get_by_name(uart_instance[uart_num].dev_name);
    bflb_uart_deinit(uart_instance[uart_num].uartx);

    uart_instance[uart_num].inited = 0;
    uart_instance[uart_num].rx_irq_en = 0;
    uart_instance[uart_num].tx_irq_en = 0;

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
    if (uart.uart_num >= AIIO_UART_MAX)
        return AIIO_ERROR;
    uint8_t uart_num = uart.uart_num;
    struct bflb_device_s *gpio;
    struct bflb_device_s *uartx;

    // if(1 == uart_instance[uart_num].inited){
    //     aiio_uart_deinit(uart);
    // }

    gpio = bflb_device_get_by_name("gpio");

    if(NULL == gpio)
        return 1;

    struct bflb_uart_config_s *uart_dev = &uart_instance[uart_num].uart;
    if (uart.uart_tx_pin)
        bflb_gpio_uart_init(gpio, uart.uart_tx_pin, uart_instance[uart_num].func_uart_tx);
    if (uart.uart_rx_pin)
        bflb_gpio_uart_init(gpio, uart.uart_rx_pin, uart_instance[uart_num].func_uart_rx);
    if (uart.baud_rate)
        uart_dev->baudrate = uart.baud_rate;

    uart_instance[uart_num].uartx = bflb_device_get_by_name(uart_instance[uart_num].dev_name);
    if(NULL == uart_instance[uart_num].uartx)
        return 2;

    switch(uart.dataBits){
        case AIIO_UART_DATA_BITS_5:
            uart_dev->data_bits=UART_DATA_BITS_5;
            break;
        case AIIO_UART_DATA_BITS_6:
            uart_dev->data_bits=UART_DATA_BITS_6;
            break;
        case AIIO_UART_DATA_BITS_7:
            uart_dev->data_bits=UART_DATA_BITS_7;
            break;   
        case AIIO_UART_DATA_BITS_8:
            uart_dev->data_bits=UART_DATA_BITS_8;
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.stopBits){
        case AIIO_UART_STOPBITS_0_5:
        case AIIO_UART_STOP_BITS_1:
        case AIIO_UART_STOP_BITS_1_5:
        case AIIO_UART_STOP_BITS_2:
            uart_dev->stop_bits = uart.stopBits;
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.parity){
        case AIIO_UART_PARITY_NONE:
        case AIIO_UART_PARITY_ODD:
        case AIIO_UART_PARITY_EVEN:
            uart_dev->parity = uart.parity;
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

    bflb_uart_init(uart_instance[uart_num].uartx, uart_dev);
    
    bflb_uart_feature_control(uart_instance[uart_num].uartx, UART_CMD_SET_BAUD_RATE, uart_dev->baudrate);
    bflb_uart_feature_control(uart_instance[uart_num].uartx, UART_CMD_SET_DATA_BITS, uart_dev->data_bits);
    bflb_uart_feature_control(uart_instance[uart_num].uartx, UART_CMD_SET_STOP_BITS, uart_dev->stop_bits);
    bflb_uart_feature_control(uart_instance[uart_num].uartx, UART_CMD_SET_PARITY_BITS, uart_dev->parity);

    bflb_uart_txint_mask(uart_instance[uart_num].uartx, true);
    bflb_uart_rxint_mask(uart_instance[uart_num].uartx, false);
    uart_instance[uart_num].inited = 1;
    if (uart.irq_rx_bk)
    {
        uart_instance[uart_num].irq_rx_bk = uart.irq_rx_bk;
        uart_instance[uart_num].rx_irq_en = 1;
        bflb_irq_attach(uart_instance[uart_num].uartx->irq_num, aiio_uart_isr, NULL);
        bflb_irq_enable(uart_instance[uart_num].uartx->irq_num);
    }
    
    return AIIO_OK;
}

int32_t aiio_uart_send_data(uint8_t uart_num, uint8_t *buf, uint16_t size)
{
    if (uart_num >= AIIO_UART_MAX || !uart_instance[uart_num].inited)
        return AIIO_ERROR;

    return bflb_uart_put(uart_instance[uart_num].uartx, buf, size);
}


int32_t aiio_uart_recv_data(uint8_t uart_num, uint8_t *buf, uint16_t size)
{
    if (uart_num >= AIIO_UART_MAX || !uart_instance[uart_num].inited)
        return AIIO_ERROR;

    if(uart_instance[uart_num].rx_irq_en)
        return AIIO_ERROR;

    int ret = bflb_uart_get(uart_instance[uart_num].uartx, buf, size);

    return ret;
}

static int uart_log_num = 0;

int32_t aiio_uart_log_init(aiio_uart_config_t uart)
{
    struct bflb_device_s *uartx;
    uart_log_num    = uart.uart_num;
    if(uart_log_num ==0){
        uartx = bflb_device_get_by_name("uart0");
    }else if(uart_log_num ==1){
        uartx = bflb_device_get_by_name("uart1");
    }
    uart.dataBits   = AIIO_UART_DATA_BITS_8;
    uart.parity     = AIIO_UART_PARITY_NONE;
    uart.stopBits   = AIIO_UART_STOP_BITS_1;
    bflb_uart_set_console(uartx);
    return aiio_uart_init(uart);
}

int32_t aiio_uart_log_send(const char *fmt, ...)
{
    if (uart_log_num >= AIIO_UART_MAX || !uart_instance[uart_log_num].inited)
        return 0;

    char print_buf[512];
    int len;
    va_list ap;

    if (uart_instance[uart_log_num].uartx == NULL)
    {
        return 0;
    }

    va_start(ap, fmt);
    len = vsnprintf(print_buf, sizeof(print_buf), fmt, ap);
    va_end(ap);

    len = (len > sizeof(print_buf)) ? sizeof(print_buf) : len;

    bflb_uart_put(uart_instance[uart_log_num].uartx, (uint8_t *)print_buf, len);

    return AIIO_OK;
}

