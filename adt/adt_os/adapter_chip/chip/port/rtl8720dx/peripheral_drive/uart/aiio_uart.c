#include "aiio_uart.h"
#include "aiio_error.h"
#include "aiio_chip_spec.h"
#include "stdarg.h"

#include "device.h"
#include "serial_api.h"
#include "serial_ex_api.h"
#include "PinNames.h"

#define PIN_CALCULATE(a, b) ((a)<<5|(b))

typedef struct
{
    uint8_t inited : 1;
    uint8_t rx_irq_en : 1;
    uint8_t tx_irq_en : 1;
    uint8_t reserved : 5;
    serial_t Sobj;
    aiio_uart_int_callback irq_rx_bk; /*!< UART recv data callback function*/
} aiio_uart_dev_t;

static aiio_uart_dev_t s_uart_dev[2] = {0};
static serial_t *ai_LogSobj;
static uint8_t uart_in_at_mode_tag=0;   /*!< 0:normal mode 1:at mode*/

static int8_t uart_index_get(PinName tx)
{
    if ((tx == PA_12) || (tx == PB_1) || (tx == PA_26))  //at
    {
        return __AIIO_UART1;
    }
    // else if ((tx == _PA_18) || (tx == _PA_21) || (tx == _PB_9) || (tx == _PB_19))
    // {
    //     return 0;
    // }
    else if (tx == PA_7)   //log
    {
        return __AIIO_UART0;
    }

    return -1;
}

static void _uart_irq(uint32_t id, SerialIrq event)
{
    serial_t *sobj = (void *)id;
    volatile char rc = 0;
    if (event == RxIrq)
    {
        rc = serial_getc(sobj);
        if (s_uart_dev[sobj->uart_idx-2].irq_rx_bk)
        {
            s_uart_dev[sobj->uart_idx-2].irq_rx_bk(&rc, 1);
        }
    }
}

static void _log_uart_irq(uint32_t id, SerialIrq event)
{
    serial_t *sobj = (void *)id;
    volatile char rc = 0;

    if (event == RxIrq)
    {
        rc = serial_getc(sobj);
        if (s_uart_dev[sobj->uart_idx-2].irq_rx_bk)
        {
            s_uart_dev[sobj->uart_idx-2].irq_rx_bk(&rc, 1);
        }else if (1==uart_in_at_mode_tag && s_uart_dev[__AIIO_UART1].irq_rx_bk){
            s_uart_dev[__AIIO_UART1].irq_rx_bk(&rc, 1);
        }else{
        }
    }
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
    switch (uart.uart_tx_pin)
    {
        case __AIIO_GPIO_NUM_7:
        case __AIIO_GPIO_NUM_12:
        case __AIIO_GPIO_NUM_1:
        case __AIIO_GPIO_NUM_26:
            break;
        default:
            return AIIO_UART_PIN_NOT_SUPPORT;
    }
    switch (uart.uart_rx_pin)
    {
        case __AIIO_GPIO_NUM_8:
        case __AIIO_GPIO_NUM_13:
        case __AIIO_GPIO_NUM_2:
        case __AIIO_GPIO_NUM_25:
            break;
        default:
            return AIIO_UART_PIN_NOT_SUPPORT;
    }
    switch(uart.dataBits){
        case AIIO_UART_DATA_BITS_5:
        case AIIO_UART_DATA_BITS_6:
            return AIIO_UART_DATA_BITS_NOT_SUPPORT;
        case AIIO_UART_DATA_BITS_7:
        case AIIO_UART_DATA_BITS_8:
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.stopBits){
        case AIIO_UART_STOP_BITS_1:
            break;
        case AIIO_UART_STOP_BITS_2:
            break;
        case AIIO_UART_STOP_BITS_1_5:
            return AIIO_UART_STOP_BITS_NOT_SUPPORT;
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
    PinName tx_pin = 0;
    PinName rx_pin = 0;
    int data_bits=0;
    SerialParity parity=ParityNone;

    uart_in_at_mode_tag=0;
    //aiio_uart_log_send("\r\n\r\n\r\n[%s()-%d]baud_rate=%d dataBits=%d stopBits=%d parity=%d\r\n\r\n\r\n",__func__,__LINE__,uart.baud_rate,uart.dataBits,uart.stopBits,uart.parity);
    if(uart.uart_num >= AIIO_UART_MAX){
        return AIIO_UART_NUMBER_NOT_SUPPORT;
    }
    switch (uart.uart_tx_pin)
    {
        case __AIIO_GPIO_NUM_7:
            tx_pin = PIN_CALCULATE(__AIIO_GPIO_A, __AIIO_GPIO_NUM_7);
            break;
        case __AIIO_GPIO_NUM_12:
            tx_pin = PIN_CALCULATE(__AIIO_GPIO_A, __AIIO_GPIO_NUM_12);
            break;
        case __AIIO_GPIO_NUM_1:
            tx_pin = PIN_CALCULATE(__AIIO_GPIO_B, __AIIO_GPIO_NUM_1);
            break;
        case __AIIO_GPIO_NUM_26:
            tx_pin = PIN_CALCULATE(__AIIO_GPIO_A, __AIIO_GPIO_NUM_26);
            break;
        default:
            return AIIO_UART_PIN_NOT_SUPPORT;
    }
    switch (uart.uart_rx_pin)
    {
        case __AIIO_GPIO_NUM_8:
            rx_pin = PIN_CALCULATE(__AIIO_GPIO_A, __AIIO_GPIO_NUM_8);
            break;
        case __AIIO_GPIO_NUM_13:
            rx_pin = PIN_CALCULATE(__AIIO_GPIO_A, __AIIO_GPIO_NUM_13);
            break;
        case __AIIO_GPIO_NUM_2:
            rx_pin = PIN_CALCULATE(__AIIO_GPIO_B, __AIIO_GPIO_NUM_2);
            break;
        case __AIIO_GPIO_NUM_25:
            rx_pin = PIN_CALCULATE(__AIIO_GPIO_A, __AIIO_GPIO_NUM_25);
            break;
        default:
            return AIIO_UART_PIN_NOT_SUPPORT;
    }
    int8_t uart_index = uart_index_get(tx_pin);
    if (uart_index < __AIIO_UART0 || uart_index > __AIIO_UART1)
    {
        return AIIO_PARAM_ERROR;
    }
    switch(uart.dataBits){
        case AIIO_UART_DATA_BITS_5:
        case AIIO_UART_DATA_BITS_6:
            return AIIO_UART_DATA_BITS_NOT_SUPPORT;
        case AIIO_UART_DATA_BITS_7:
        case AIIO_UART_DATA_BITS_8:
            break;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.stopBits){
        case AIIO_UART_STOP_BITS_1:
            data_bits=1;
            break;
        case AIIO_UART_STOP_BITS_2:
            data_bits=2;
            break;
        case AIIO_UART_STOP_BITS_1_5:
            return AIIO_UART_STOP_BITS_NOT_SUPPORT;
        default:
            return AIIO_PARAM_ERROR;
    }
    switch(uart.parity){
        case AIIO_UART_PARITY_NONE:
            parity=ParityNone;
            break;
        case AIIO_UART_PARITY_ODD:
            parity=ParityOdd;
            break;
        case AIIO_UART_PARITY_EVEN:
            parity=ParityEven;
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
    s_uart_dev[uart_index].inited = 1;
    serial_init(&s_uart_dev[uart_index].Sobj, tx_pin, rx_pin);
    serial_baud(&s_uart_dev[uart_index].Sobj, uart.baud_rate);
    serial_format(&s_uart_dev[uart_index].Sobj,uart.dataBits, parity,data_bits);

    if (uart.irq_rx_bk)
    {
        s_uart_dev[uart_index].rx_irq_en = 1;
        s_uart_dev[uart_index].irq_rx_bk = uart.irq_rx_bk;
        serial_irq_handler(&s_uart_dev[uart_index].Sobj, _uart_irq, (uint32_t)&s_uart_dev[uart_index].Sobj);
        serial_irq_set(&s_uart_dev[uart_index].Sobj, RxIrq, 1);
        serial_irq_set(&s_uart_dev[uart_index].Sobj, TxIrq, 0);
    }

    uart_in_at_mode_tag=uart.in_at_mode;

    return AIIO_OK;
}

int32_t aiio_uart_deinit(aiio_uart_config_t uart)
{
    uint8_t uart_num = uart.uart_num;
    if (uart_num >= AIIO_UART_MAX || !s_uart_dev[uart_num].inited)
        return AIIO_ERROR;

    serial_free(&s_uart_dev[uart_num].Sobj);

    memset(&s_uart_dev[uart_num], 0, sizeof(aiio_uart_dev_t));

    return AIIO_OK;
}

int32_t aiio_uart_send_data(uint8_t uart_num, uint8_t *buf, uint16_t size)
{
    if (uart_num >= AIIO_UART_MAX || !s_uart_dev[uart_num].inited)
        return AIIO_ERROR;

    uint16_t len = 0;

    while (len < size)
    {
        if(1==uart_in_at_mode_tag && __AIIO_UART1==uart_num){
            serial_putc(&s_uart_dev[__AIIO_UART0].Sobj, buf[len]);
        }
        serial_putc(&s_uart_dev[uart_num].Sobj, buf[len++]);
    }

    return AIIO_OK;
}

int32_t aiio_uart_recv_data(uint8_t uart_num, uint8_t *buf, uint16_t size)
{
    if (uart_num >= AIIO_UART_MAX || !s_uart_dev[uart_num].inited)
        return AIIO_ERROR;

    if(s_uart_dev[uart_num].rx_irq_en)
        return AIIO_ERROR;

    buf[0] = serial_getc(&s_uart_dev[uart_num].Sobj);

    return sizeof(uint8_t);
}

int32_t aiio_uart_log_init(aiio_uart_config_t uart)
{
    PinName tx_pin = 0;
    PinName rx_pin = 0;

    switch (uart.uart_tx_pin)
    {
    case __AIIO_GPIO_NUM_7:
        tx_pin = PIN_CALCULATE(__AIIO_GPIO_A, __AIIO_GPIO_NUM_7);
        break;
    
    default:
        return AIIO_ERROR;
    }

    switch (uart.uart_rx_pin)
    {
    case __AIIO_GPIO_NUM_8:
        rx_pin = PIN_CALCULATE(__AIIO_GPIO_A, __AIIO_GPIO_NUM_8);
        break;
    
    default:
        break;
    }

    int8_t uart_index = uart_index_get(tx_pin);
    if (uart_index < __AIIO_UART0 || uart_index > __AIIO_UART1)
    {
        return AIIO_ERROR;
    }

    s_uart_dev[uart_index].inited = 1;
    serial_init(&s_uart_dev[uart_index].Sobj, tx_pin, rx_pin);
    serial_baud(&s_uart_dev[uart_index].Sobj, uart.baud_rate);
    serial_format(&s_uart_dev[uart_index].Sobj, 8, ParityNone, 1);

    //默认开启log rx
    s_uart_dev[uart_index].rx_irq_en = 1;
    if (uart.irq_rx_bk)
    {
        s_uart_dev[uart_index].irq_rx_bk = uart.irq_rx_bk;
    }else{
        s_uart_dev[uart_index].irq_rx_bk = NULL;
    }
    serial_irq_handler(&s_uart_dev[uart_index].Sobj, _log_uart_irq, (uint32_t)&s_uart_dev[uart_index].Sobj);
    serial_irq_set(&s_uart_dev[uart_index].Sobj, RxIrq, 1);
    serial_irq_set(&s_uart_dev[uart_index].Sobj, TxIrq, 0);

    ai_LogSobj = &s_uart_dev[uart_index].Sobj;

    return AIIO_OK;
}

int32_t aiio_uart_log_send(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    int ch;
    char *str;
    char string[AIIO_LOG_MAX_LEN] = { 0 };
    str = string;

    if (0 < vsprintf(string, fmt, argp))
    {
        while ('\0' != (ch = *(str++)))
        {
            serial_putc(ai_LogSobj, ch);
        }
    }

    va_end(argp);
    return AIIO_OK;
}
