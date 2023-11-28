#include "bflb_gpio.h"
#include "bflb_uart.h"
#include "unity.h"
#include "unity_fixture.h"

extern uint8_t g_rx_pin;
extern struct bflb_uart_config_s g_rx_cfg;

static struct bflb_device_s *g_gpio;
static struct bflb_device_s *g_uart;

static volatile uint8_t g_index;
static volatile uint8_t g_index_loop;
static volatile uint8_t g_index_recv;

static volatile struct {
    uint64_t total;
    uint8_t expect_value;
    uint8_t actual_value;
    uint8_t recv_timeout;
    uint8_t _rsvd;
} g_loop_reason;

static void index_loop_isr(int irq, void *arg)
{
    if (false == g_index_loop) {
        bflb_irq_disable(g_uart->irq_num);
        return;
    }

    uint32_t intstatus = bflb_uart_get_intstatus(g_uart);

    if (intstatus & UART_INTSTS_RX_FIFO) {
        while (bflb_uart_rxavailable(g_uart)) {
            g_index_recv = true;
            uint8_t value = bflb_uart_getchar(g_uart);
            if (value != g_index) {
                g_loop_reason.expect_value = g_index;
                g_loop_reason.actual_value = value;
                g_index_loop = false;
                return;
            }
            g_index += 1;
            g_loop_reason.total += 1;
        }
    }
    if (intstatus & UART_INTSTS_RTO) {
        while (bflb_uart_rxavailable(g_uart)) {
            g_index_recv = true;
            uint8_t value = bflb_uart_getchar(g_uart);
            if (value != g_index) {
                g_loop_reason.expect_value = g_index;
                g_loop_reason.actual_value = value;
                g_index_loop = false;
                return;
            }
            g_index += 1;
            g_loop_reason.total += 1;
        }

        g_loop_reason.recv_timeout = true;
        g_index_loop = false;
        bflb_uart_int_clear(g_uart, UART_INTCLR_RTO);
        return;
    }
}

TEST_GROUP(uart_fifo_interrupt_rx);

TEST_SETUP(uart_fifo_interrupt_rx)
{
    g_gpio = bflb_device_get_by_name("gpio");
    TEST_ASSERT_NOT_NULL(g_gpio);
    g_uart = bflb_device_get_by_name("uart1");
    TEST_ASSERT_NOT_NULL(g_uart);

    bflb_gpio_uart_init(g_gpio, g_rx_pin, GPIO_UART_FUNC_UART1_RX);
    bflb_uart_init(g_uart, &g_rx_cfg);
    bflb_uart_int_clear(g_uart, UART_INTCLR_RTO);
    bflb_uart_feature_control(g_uart, UART_CMD_CLR_RX_FIFO, 0);
    bflb_irq_clear_pending(g_uart->irq_num);
}

TEST_TEAR_DOWN(uart_fifo_interrupt_rx)
{
    g_gpio = bflb_device_get_by_name("gpio");
    TEST_ASSERT_NOT_NULL(g_gpio);
    g_uart = bflb_device_get_by_name("uart1");
    TEST_ASSERT_NOT_NULL(g_uart);

    bflb_gpio_deinit(g_gpio, g_rx_pin);
    bflb_uart_deinit(g_uart);
}

TEST(uart_fifo_interrupt_rx, index_loop)
{
    char message[128];

    g_index = 0;
    g_index_loop = true;
    g_index_recv = false;
    g_loop_reason.recv_timeout = 0;
    g_loop_reason.total = 0;

    bflb_uart_rxint_mask(g_uart, false);
    bflb_irq_attach(g_uart->irq_num, index_loop_isr, NULL);
    bflb_irq_enable(g_uart->irq_num);

    while (g_index_loop) {
        while (false == g_index_recv)
            ;
        bflb_mtimer_delay_ms(10000);
        printf("heartbeat\r\n");
    }

    if (g_loop_reason.recv_timeout == true) {
        snprintf(message, 128, "Unexpected receive timeout, total %lld",
                 g_loop_reason.total);
    } else {
        snprintf(message, 128, "Expected 0x%02x but 0x%02x, total %lld",
                 g_loop_reason.expect_value,
                 g_loop_reason.actual_value,
                 g_loop_reason.total);
    }

    TEST_FAIL_MESSAGE(message);
}