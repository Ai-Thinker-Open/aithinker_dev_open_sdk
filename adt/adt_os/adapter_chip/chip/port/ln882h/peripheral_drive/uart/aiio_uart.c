#include "aiio_uart.h"
#include "aiio_error.h"
#include "stdarg.h"

#include "serial.h"
#include "aiio_stdio.h"
#include "aiio_autoconf.h"

typedef enum
{
    GPIO_A = 0,
    GPIO_B = 1,
}gpio_port_t;

typedef struct
{
    uint32_t baudrate;
    gpio_port_t uart_tx_port;
    gpio_pin_t  uart_tx_pin;
    gpio_port_t uart_rx_port;
    gpio_pin_t  uart_rx_pin;
}uart_pin_cfg_t;

static int uart_log_num = 0;

Serial_t aiio_LogSerial;

int32_t aiio_uart_log_init(aiio_uart_config_t uart)
{
    serial_init(&aiio_LogSerial, uart.uart_num, uart.baud_rate, NULL);
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

    if (0 < aiio_vsprintf(string, fmt, argp))
    {
        while ('\0' != (ch = *(str++)))
        {
            serial_putchar(&aiio_LogSerial, ch);
        }
    }

    va_end(argp);
    return AIIO_OK;
}
typedef struct
{
    uint8_t inited : 1;
    uint8_t rx_irq_en : 1;
    uint8_t tx_irq_en : 1;
    uint8_t reserved : 5;
    uart_pin_cfg_t uart;
    aiio_uart_int_callback irq_rx_bk; /*!< UART recv data callback function*/
} aiio_uart_dev_t;

static aiio_uart_dev_t uart_instance[] =
    {
        [AIIO_UART0] = {
            .inited = 0,
            .irq_rx_bk = NULL,
            .uart = {
                .uart_rx_port = UART1_BASE,
                .uart_tx_port = UART1_BASE,
                .uart_tx_pin = GPIO_PIN_8,
                .uart_rx_pin = GPIO_PIN_9,
                .baudrate = 115200, // 设置波特率
            }},
        [AIIO_UART1] = {
            .inited = 0,
            .irq_rx_bk = NULL,
            .uart = {
                .uart_rx_port = UART0_BASE,
                .uart_tx_port = UART0_BASE,
                .uart_tx_pin = GPIO_PIN_2,
                .uart_rx_pin = GPIO_PIN_3,
                .baudrate = 115200, // 设置波特率
            },
        },
        [AIIO_UART2] = {
            .inited = 0,
            .irq_rx_bk = NULL,
            .uart = {
                .uart_rx_port = UART0_BASE,
                .uart_tx_port = UART0_BASE,
                .uart_tx_pin = GPIO_PIN_11,
                .uart_rx_pin = GPIO_PIN_12,
                .baudrate = 115200, // 设置波特率
            },
        },

};
#if CONFIG_USER_UART_CUSTOM
/**
 * hal uart RX interrupt callback
 */
void UART0_IRQHandler(void)
{
    if (hal_uart_flag_get(UART0_BASE, USART_FLAG_RXNE))
    {
        if (uart_instance[AIIO_UART0].irq_rx_bk)
        {
            uint16_t c = hal_uart_recv_data(UART0_BASE);
            (*uart_instance[AIIO_UART1].irq_rx_bk)((uint8_t*)&c, 1);
        }
        else
        {
            hal_uart_recv_data(UART0_BASE);
        }
    }
}
void UART1_IRQHandler(void)
{
    if (hal_uart_flag_get(UART1_BASE, USART_FLAG_RXNE))
    {
        if (uart_instance[AIIO_UART1].irq_rx_bk)
        {
            uint16_t c = hal_uart_recv_data(UART1_BASE);
            (*uart_instance[AIIO_UART1].irq_rx_bk)((uint8_t*)&c, 1);
        }
        else
        {
            hal_uart_recv_data(UART1_BASE);
        }
    }
}
void UART2_IRQHandler(void)
{
    if (hal_uart_flag_get(UART2_BASE, USART_FLAG_RXNE))
    {
        if (uart_instance[AIIO_UART2].irq_rx_bk)
        {
            uint16_t c = hal_uart_recv_data(UART2_BASE);
            (*uart_instance[AIIO_UART2].irq_rx_bk)((uint8_t*)&c, 1);
        }
        else
        {
            hal_uart_recv_data(UART2_BASE);
        }
    }
}
#endif

int32_t aiio_uart_init(aiio_uart_config_t uart)
{
    uint8_t uart_num = uart.uart_num;
    if (uart_num >= AIIO_UART_MAX)
        return AIIO_ERROR;

    uart_pin_cfg_t *uart_dev = &uart_instance[uart_num].uart;
    uint32_t uart_base = 0;
    uint32_t gpio_base = 0;
    
    switch(uart_num)
    {
        case AIIO_UART0: uart_base = UART0_BASE; break;
        case AIIO_UART1: uart_base = UART1_BASE; break;
        case AIIO_UART2: uart_base = UART2_BASE; break;
    }
    
    if (uart_instance[uart_num].inited)
    {
        hal_uart_deinit(uart_dev);

        if (uart.irq_rx_bk)
        {
            hal_uart_it_disable(uart_base, USART_IT_RXNE);
        }
    }
    
    uart_dev->baudrate = uart.baud_rate;

    if (uart.uart_tx_pin)
    {
        uart_dev->uart_tx_pin =uart.uart_tx_pin;
    }
    if (uart.uart_rx_pin)
    {
        uart_dev->uart_rx_pin = uart.uart_rx_pin;
    }

    if(uart_dev->uart_tx_port == GPIO_A)
        gpio_base = GPIOA_BASE;
    else if(uart_dev->uart_tx_port == GPIO_B)
        gpio_base = GPIOB_BASE;

    if(uart_num == AIIO_UART0)
    {
        hal_gpio_pin_afio_select(gpio_base,uart_dev->uart_tx_pin,(afio_function_t)(UART0_TX));
        hal_gpio_pin_afio_en(gpio_base,uart_dev->uart_tx_pin,HAL_ENABLE);
    }
    else if(uart_num == AIIO_UART1)
    {
        hal_gpio_pin_afio_select(gpio_base,uart_dev->uart_tx_pin,(afio_function_t)(UART1_TX));
        hal_gpio_pin_afio_en(gpio_base,uart_dev->uart_tx_pin,HAL_ENABLE);
    }
    else if(uart_num == AIIO_UART2)
    {
        hal_gpio_pin_afio_select(gpio_base,uart_dev->uart_tx_pin,(afio_function_t)(UART2_TX));
        hal_gpio_pin_afio_en(gpio_base,uart_dev->uart_tx_pin,HAL_ENABLE);
    }

    if(uart_dev->uart_rx_port == GPIO_A)
        gpio_base = GPIOA_BASE;
    else if(uart_dev->uart_rx_port == GPIO_B)
        gpio_base = GPIOB_BASE;

    if(uart_num == AIIO_UART0)
    {
        hal_gpio_pin_afio_select(gpio_base,uart_dev->uart_rx_pin,(afio_function_t)(UART0_RX));
        hal_gpio_pin_afio_en(gpio_base,uart_dev->uart_rx_pin,HAL_ENABLE);
    }
    else if(uart_num == AIIO_UART1)
    {
        hal_gpio_pin_afio_select(gpio_base,uart_dev->uart_rx_pin,(afio_function_t)(UART1_RX));
        hal_gpio_pin_afio_en(gpio_base,uart_dev->uart_rx_pin,HAL_ENABLE);
    }
    else if(uart_num == AIIO_UART2)
    {
        hal_gpio_pin_afio_select(gpio_base,uart_dev->uart_rx_pin,(afio_function_t)(UART2_RX));
        hal_gpio_pin_afio_en(gpio_base,uart_dev->uart_rx_pin,HAL_ENABLE);
    }

    uart_init_t_def uart_init_struct;
    memset(&uart_init_struct,0,sizeof(uart_init_struct));
    
    uart_init_struct.baudrate = uart_dev->baudrate;              //设置波特率
    uart_init_struct.parity = UART_PARITY_NONE;         //设置校验位
    uart_init_struct.stop_bits = UART_STOP_BITS_1;      //设置停止位
    uart_init_struct.word_len = UART_WORD_LEN_8;        //设置数据长度
    
    hal_uart_init(uart_base,&uart_init_struct);         //初始化UART寄存器
    hal_uart_rx_mode_en(uart_base,HAL_ENABLE);          //使能发送模式
    hal_uart_tx_mode_en(uart_base,HAL_ENABLE);          //使能接收模式
    hal_uart_en(uart_base,HAL_ENABLE);                  //使能UART模块

    if (uart.irq_rx_bk)
    {

        /* 3. 初始化UART中断 */
        NVIC_SetPriority((IRQn_Type)(UART0_IRQn + uart_num), 4);
        NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + uart_num));
        hal_uart_it_en(uart_base, USART_IT_RXNE); // 使能接收中断

        uart_instance[uart_num].irq_rx_bk = uart.irq_rx_bk;

        uart_instance[uart_num].rx_irq_en = 1;
    }

    uart_instance[uart_num].inited = 1;

    return AIIO_OK;
}

int32_t aiio_uart_deinit(aiio_uart_config_t uart)
{
    uint8_t uart_num = uart.uart_num;
    if (uart_num >= AIIO_UART_MAX || !uart_instance[uart_num].inited)
        return AIIO_ERROR;
    uint8_t uart_irq_addr;
    uint32_t uart_base_addr;
    uart_pin_cfg_t *uart_dev = &uart_instance[uart_num].uart;
    if (AIIO_UART0 == uart_num)
    {
        uart_base_addr = UART0_BASE;
        uart_irq_addr = UART0_IRQn;
    }
    else if (AIIO_UART1 == uart_num)
    {
        uart_base_addr = UART1_BASE;
        uart_irq_addr = UART1_IRQn;
    }
    else
    {
        uart_base_addr = UART2_BASE;
        uart_irq_addr = UART2_IRQn;
    }

    hal_uart_deinit(&uart_instance[uart_num].uart);

    if (uart.irq_rx_bk)
    {
        // hal_uart_it_flag_clear(uart_base_addr, USART_IT_FLAG_CLEAR_CTS);
        hal_uart_it_disable(uart_base_addr, USART_IT_RXNE);
    }

    uart_instance[uart_num].inited = 0;
    uart_instance[uart_num].rx_irq_en = 0;
    uart_instance[uart_num].tx_irq_en = 0;

    return AIIO_OK;
}

int32_t aiio_uart_send_data(uint8_t uart_num, uint8_t *buf, uint16_t size)
{
    if (uart_num >= AIIO_UART_MAX || !uart_instance[uart_num].inited)
        return AIIO_ERROR;
    uint32_t uart_base = 0;
    switch(uart_num)
    {
        case AIIO_UART0: uart_base = UART0_BASE; break;
        case AIIO_UART1: uart_base = UART1_BASE; break;
        case AIIO_UART2: uart_base = UART2_BASE; break;
    }

    for (int i = 0; i < size; i++)
    {
        while (hal_uart_flag_get(uart_base, USART_FLAG_TXE) != HAL_SET);
        hal_uart_send_data(uart_base, buf[i]);
    }
    return AIIO_OK;
}
int32_t aiio_uart_recv_data(uint8_t uart_num, uint8_t *buf, uint16_t size)
{
    if (uart_num >= AIIO_UART_MAX || !uart_instance[uart_num].inited)
        return AIIO_ERROR;
    uint32_t uart_base = 0;
    switch(uart_num)
    {
        case AIIO_UART0: uart_base = UART0_BASE; break;
        case AIIO_UART1: uart_base = UART1_BASE; break;
        case AIIO_UART2: uart_base = UART2_BASE; break;
    }
    *buf = hal_uart_recv_data(uart_base);

    return 1;
}

