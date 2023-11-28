#include "proj_config.h"
#include "hal/hal_uart.h"
#include "serial.h"
#include "serial_hw.h"
#include "utils/debug/ln_assert.h"
#include "hal/hal_gpio.h"


#define UART0_TX_BUF_SIZE  CFG_UART0_TX_BUF_SIZE
#define UART0_RX_BUF_SIZE  CFG_UART0_RX_BUF_SIZE

/* TX and RX fifo buffer */
uint8_t uart0_txbuf[UART0_TX_BUF_SIZE];
uint8_t uart0_rxbuf[UART0_RX_BUF_SIZE];

/* From the high-level serial driver */
extern Serial_t serial_handles[1];

/* UART device*/
typedef struct
{
    uint32_t     uart_base;
    uart_init_t_def  init_cfg;
} uart_dev_t;
static uart_dev_t g_uart0;

/* serial */
typedef struct
{
    struct SerialHardware  Hardware;
    struct Serial         *serial;
} ln_serial_t;
ln_serial_t uart_serial[1];


static void uart_io_pin_request(struct Serial *serial)
{
    if (serial->port_id == SER_PORT_UART0)
    {
        hal_gpio_pin_afio_select(GPIOB_BASE,GPIO_PIN_8,UART0_RX);
        hal_gpio_pin_afio_select(GPIOB_BASE,GPIO_PIN_9,UART0_TX);
        hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_8,HAL_ENABLE);
        hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_9,HAL_ENABLE);
    } 
    else if (serial->port_id == SER_PORT_UART1)
    {
        hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_2,UART1_TX);
        hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_3,UART1_RX);
        hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_2,HAL_ENABLE);
        hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_3,HAL_ENABLE);
    } 
    else if (serial->port_id == SER_PORT_UART2)
    {
    }
}

static void uart_io_pin_release(struct Serial *serial)
{
    if (serial == NULL) 
    {
        return;
    }

    if (serial->port_id == SER_PORT_UART0) 
    {
        hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_8,HAL_DISABLE);
        hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_9,HAL_DISABLE);
    } 
    else if (serial->port_id == SER_PORT_UART1) 
    {
        hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_2,HAL_DISABLE);
        hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_3,HAL_DISABLE);
    } 
    else if (serial->port_id == SER_PORT_UART2)
    {
    }
}


static void hw_uart0_init(struct SerialHardware *_hw, struct Serial *serial, uint32_t baudrate)
{
    ln_serial_t *hw = NULL;

    LN_ASSERT(_hw && serial);
    hw = (ln_serial_t *)_hw;
    hw->serial = serial;

    g_uart0.uart_base          = UART0_BASE;
    g_uart0.init_cfg.baudrate  = baudrate;//115200 921600 2000000
    g_uart0.init_cfg.word_len  = UART_WORD_LEN_8;
    g_uart0.init_cfg.parity    = UART_PARITY_NONE;
    g_uart0.init_cfg.stop_bits = UART_STOP_BITS_1;

    hal_uart_init(g_uart0.uart_base, &g_uart0.init_cfg);

    hal_uart_rx_mode_en(UART0_BASE, HAL_ENABLE);
    hal_uart_tx_mode_en(UART0_BASE, HAL_ENABLE);
    hal_uart_en(UART0_BASE, HAL_ENABLE);

    hal_uart_it_en(UART0_BASE, USART_IT_RXNE);
    //uart_it_enable(UART0_BASE, USART_IT_TXE);//uart_it_enable(UART0_BASE, USART_IT_TXE);
    NVIC_EnableIRQ(UART0_IRQn);

    //request pin for uart
    uart_io_pin_request(hw->serial);
}

static void hw_uart_cleanup(struct SerialHardware *_hw)
{
    ln_serial_t *hw = NULL;

    LN_ASSERT(_hw);
    hw = (ln_serial_t *)_hw;
    uart_io_pin_release(hw->serial);
    hw->serial = NULL;
}
#if 1
static void hw_uart_tx_start_polling(struct SerialHardware * _hw)
{
    uint8_t ch;
    ln_serial_t *hw = NULL;
    uart_dev_t * pdev;

    LN_ASSERT(_hw);
    hw = (ln_serial_t *)_hw;
    while(!fifo_isempty(&hw->serial->txfifo))
    {
        ch = fifo_pop(&hw->serial->txfifo);
        pdev = (uart_dev_t *)hw->Hardware.hw_device;
        while (hal_uart_flag_get(pdev->uart_base, USART_FLAG_TX_FIFO_FULL) == HAL_SET) {};
        hal_uart_send_data(pdev->uart_base, ch);
    }
}
#else

static void hw_uart_tx_start_isr(struct SerialHardware * _hw)
{
    ln_serial_t *hw = NULL;
    LN_ASSERT(_hw);
    hw = (ln_serial_t *)_hw;

    if (hw->Hardware.isSending){
        return;
    }

    if(!fifo_isempty(&hw->serial->txfifo))
    {
        hw->Hardware.isSending = LN_TRUE;
        /* Enable TX empty interrupts. */
        uart_it_enable(UART0_BASE, USART_IT_TXE);
    }
}
#endif

static uint8_t hw_uart_tx_is_sending(struct SerialHardware * _hw)
{
    ln_serial_t *hw = NULL;

    LN_ASSERT(_hw);
    hw = (ln_serial_t *)_hw;
    return hw->Hardware.isSending;
}

static uint8_t hw_uart_setBaudrate(struct SerialHardware * _hw, uint32_t baudrate)
{
    ln_serial_t *hw = NULL;
    uart_dev_t * pdev;

    LN_ASSERT(_hw);
    hw = (ln_serial_t *)_hw;

    pdev = (uart_dev_t *)hw->Hardware.hw_device;

    hal_uart_baudrate_set(pdev->uart_base, baudrate);
    return  LN_TRUE;
}

/*
 * High-level interface data structures.
 */
static const struct SerialHardwareVT uart0_vtable =
{
    .init        = hw_uart0_init,
    .cleanup     = hw_uart_cleanup,
    .txStart     = hw_uart_tx_start_polling,//hw_uart_tx_start_polling,//hw_uart_tx_start_isr
    .txSending   = hw_uart_tx_is_sending,
    .setBaudrate = hw_uart_setBaudrate,
};

ln_serial_t uart_serial[1] =
{
    {
        .Hardware =
        {
          .table         = &uart0_vtable,
          .txbuffer      = uart0_txbuf,
          .rxbuffer      = uart0_rxbuf,
          .txbuffer_size = sizeof(uart0_txbuf),
          .rxbuffer_size = sizeof(uart0_rxbuf),
          .hw_device     = (void *)&g_uart0,
          .isSending     = LN_FALSE,
        },
        .serial = NULL,
    },
};

struct SerialHardware *serial_hw_getdesc(serial_port_id_t port_id)
{
    LN_ASSERT(port_id < SER_PORT_NUM);
    return  (struct SerialHardware *)&uart_serial[port_id].Hardware;
}


static inline void uart0_send_data_isr(void)
{
    ln_serial_t *hw = (ln_serial_t *)&uart_serial[SER_PORT_UART0];
    uint8_t tx_char = 0;

    if (fifo_isempty(&hw->serial->txfifo))
    {
        hal_uart_it_disable(UART0_BASE, USART_IT_TXE);
        hw->Hardware.isSending = LN_FALSE;
    }
    else
    {
        tx_char = fifo_pop(&hw->serial->txfifo);
        hal_uart_send_data(UART0_BASE, tx_char);
        while (hal_uart_flag_get(UART0_BASE, USART_FLAG_TX_FIFO_FULL) == HAL_SET) {};
    }
}

static inline void uart0_recv_data_isr(void)
{
    uint8_t ch = 0;

    ln_serial_t *hw = (ln_serial_t *)&uart_serial[SER_PORT_UART0];

    while (fifo_isfull(&hw->serial->rxfifo)){
        serial_purge_rx(hw->serial);
    }

    ch = hal_uart_recv_data(UART0_BASE);

    fifo_push(&hw->serial->rxfifo, ch);
    hw->serial->rx_callback();
}

static inline void serial_uart0_isr_callback(void)
{
    if (hal_uart_flag_get(UART0_BASE, USART_FLAG_RXNE) && hal_uart_it_en_status_get(UART0_BASE,USART_IT_RXNE)) {
        uart0_recv_data_isr();
    }

    if (hal_uart_flag_get(UART0_BASE, USART_FLAG_TXE) && hal_uart_it_en_status_get(UART0_BASE,USART_IT_TXE)) {
        uart0_send_data_isr();
    }
}

///=====================UART0 IQR Handle===============================///
void UART0_IRQHandler(void)
{
    serial_uart0_isr_callback();
}

