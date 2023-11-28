#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"
#include "aiio_uart.h"

#define UART_TEST_PORT CONFIG_USER_UART
#define UART_TEST_TX CONFIG_USER_UART_TX
#define UART_TEST_RX CONFIG_USER_UART_RX

char recv_data[100];
uint32_t recv_cnt = 0;

void aiio_uart_rx_irq_bk(uint8_t *buf, uint16_t size)
{
    // For the serial port receive data callback test, users should build their own receive buf buffers
    if (size > 1)
    {
        memcpy(&recv_data[recv_cnt], buf, size);//Check for overflow
        recv_cnt+=size;
    }
    else
    {
        recv_data[recv_cnt++] = *buf;
    }
    recv_cnt %= 100;
}

void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");
    aiio_uart_config_t uart = {
        .uart_num = UART_TEST_PORT,
        .baud_rate = 115200,
        .uart_tx_pin = UART_TEST_TX,
        .uart_rx_pin = UART_TEST_RX,
        .irq_rx_bk = aiio_uart_rx_irq_bk,
    };
    aiio_uart_init(uart);
    uint32_t i = 0;
    char  tmp_buf[64] = {0};

    while (1)
    {
        sprintf((char *)tmp_buf, "this is a test:%d\r\n", i++);
        // The serial port sends data
        aiio_uart_send_data(UART_TEST_PORT, (uint8_t *)tmp_buf, strlen(tmp_buf));
        if (recv_cnt >= 2)
        {
            aiio_uart_send_data(UART_TEST_PORT, (uint8_t *)recv_data, recv_cnt);
            recv_cnt = 0;
        }
        aiio_log_a("log send data test!");

        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
