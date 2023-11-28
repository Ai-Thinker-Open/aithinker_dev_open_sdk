#include "unity.h"
#include "unity_fixture.h"
#include "bflb_uart.h"
#include "bflb_gpio.h"
#include "shell.h"

uint8_t g_tx_pin = GPIO_PIN_27;
struct bflb_uart_config_s g_tx_cfg = {
    .baudrate = 2000000,
    .data_bits = UART_DATA_BITS_8,
    .stop_bits = UART_STOP_BITS_1,
    .parity = UART_PARITY_NONE,
    .flow_ctrl = UART_FLOWCTRL_NONE,
    .tx_fifo_threshold = 1,
    .rx_fifo_threshold = 1,
};

uint8_t g_rx_pin = GPIO_PIN_28;
struct bflb_uart_config_s g_rx_cfg = {
    .baudrate = 2000000,
    .data_bits = UART_DATA_BITS_8,
    .stop_bits = UART_STOP_BITS_1,
    .parity = UART_PARITY_NONE,
    .flow_ctrl = UART_FLOWCTRL_NONE,
    .tx_fifo_threshold = 1,
    .rx_fifo_threshold = 1,
};

TEST_GROUP_RUNNER(uart_fifo_interrupt_rx)
{
    RUN_TEST_CASE(uart_fifo_interrupt_rx, index_loop);
}

static int test_cfg(int argc, const char *argv[])
{
    int i;
    uint8_t *pin = NULL;
    struct bflb_uart_config_s *cfg = NULL;

    if (argc == 1) {
        return 0;
    }

    for (i = 1; i < argc;) {
        if (strcmp(argv[i], "tx") == 0) {
            pin = &g_tx_pin;
            cfg = &g_tx_cfg;
            i++;
        } else if (strcmp(argv[i], "rx") == 0) {
            pin = &g_rx_pin;
            cfg = &g_rx_cfg;
            i++;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: test_cfg <tx|rx> [options]\r\n");
            printf("Options:\r\n");
            printf("  -b                          Data bits\r\n");
            printf("                              5, 6, 7, 8\r\n");
            printf("  -c, --ctrl                  Flow control mode\r\n");
            printf("                              none, rts, cts, rts_cts\r\n");
            printf("  -h, --help                  Print this message and exit.\r\n");
            printf("  -p, --parity                Parity mode\r\n");
            printf("                              none, odd, even, mark, space\r\n");
            printf("  -s, --stop                  Stop bits\r\n");
            printf("                              0.5, 1, 1.5, 2\r\n");
            printf("  -B, --baud                  Baudrate\r\n");
            printf("  -P, --pin                   Set Tx|Rx pin.\r\n");
            printf("                              Pay attention to uart signals config\r\n\r\n");
            printf("This program built for uart test\r\n");
            return 0;
        } else if (strcmp(argv[i], "-b") == 0) {
            i++;
            if (i >= argc || cfg == NULL) {
                return -1;
            }
            int data_bits = atoi(argv[i]);

            switch (data_bits) {
                case 5:
                case 6:
                case 7:
                case 8:
                    cfg->data_bits = data_bits - 5;
                    break;
                default:
                    printf("data bit not supported\r\n");
                    return -1;
            }
            i++;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--ctrl") == 0) {
            i++;
            if (i >= argc || cfg == NULL) {
                return -1;
            }

            if (strcmp(argv[i], "none") == 0) {
                cfg->flow_ctrl = UART_FLOWCTRL_NONE;
            } else if (strcmp(argv[i], "rts") == 0) {
                cfg->flow_ctrl = UART_FLOWCTRL_RTS;
            } else if (strcmp(argv[i], "cts") == 0) {
                cfg->flow_ctrl = UART_FLOWCTRL_CTS;
            } else if (strcmp(argv[i], "rts_cts") == 0) {
                cfg->flow_ctrl = UART_FLOWCTRL_RTS_CTS;
            } else {
                printf("flow control not supported\r\n");
                return -1;
            }
            i++;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--parity") == 0) {
            i++;
            if (i >= argc || cfg == NULL) {
                return -1;
            }

            if (strcmp(argv[i], "none") == 0) {
                cfg->parity = UART_PARITY_NONE;
            } else if (strcmp(argv[i], "odd") == 0) {
                cfg->parity = UART_PARITY_ODD;
            } else if (strcmp(argv[i], "even") == 0) {
                cfg->parity = UART_PARITY_EVEN;
            } else if (strcmp(argv[i], "mark") == 0) {
                cfg->parity = UART_PARITY_MARK;
            } else if (strcmp(argv[i], "space") == 0) {
                cfg->parity = UART_PARITY_SPACE;
            } else {
                printf("parity not supported\r\n");
                return -1;
            }
            i++;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--stop") == 0) {
            i++;
            if (i >= argc || cfg == NULL) {
                return -1;
            }

            if (strcmp(argv[i], "0.5") == 0) {
                cfg->stop_bits = UART_STOP_BITS_0_5;
            } else if (strcmp(argv[i], "1") == 0) {
                cfg->stop_bits = UART_STOP_BITS_1;
            } else if (strcmp(argv[i], "1.5") == 0) {
                cfg->stop_bits = UART_STOP_BITS_1_5;
            } else if (strcmp(argv[i], "2") == 0) {
                cfg->stop_bits = UART_STOP_BITS_2;
            } else {
                printf("stop bit not supported\r\n");
                return -1;
            }
            i++;
        } else if (strcmp(argv[i], "-B") == 0 || strcmp(argv[i], "--baud") == 0) {
            i++;
            if (i >= argc || cfg == NULL) {
                return -1;
            }
            int baud = atoi(argv[i]);

            if (baud <= 0) {
                printf("baudrate not supported\r\n");
                return -1;
            } else if (baud > 2000000) {
                printf("warning baudrate too high, may loss\r\n");
            }
            cfg->baudrate = baud;
            i++;
        } else if (strcmp(argv[i], "-P") == 0 || strcmp(argv[i], "--pin") == 0) {
            i++;
            if (i >= argc || pin == NULL) {
                return -1;
            }
            int p = atoi(argv[i]);

            if (p <= 0) {
                printf("pin not supported\r\n");
                return -1;
            }
            *pin = p;
            i++;
        }
    }

    char *stop;

    switch (cfg->stop_bits) {
        case UART_STOP_BITS_0_5:
            stop = "0.5";
            break;
        case UART_STOP_BITS_1:
            stop = "1";
            break;
        case UART_STOP_BITS_1_5:
            stop = "1.5";
            break;
        case UART_STOP_BITS_2:
            stop = "2";
            break;
        default:
            stop = "unknown";
            break;
    }

    char *flow;

    switch (cfg->flow_ctrl) {
        case UART_FLOWCTRL_NONE:
            flow = "none";
            break;
        case UART_FLOWCTRL_RTS:
            flow = "rts";
            break;
        case UART_FLOWCTRL_CTS:
            flow = "cts";
            break;
        case UART_FLOWCTRL_RTS_CTS:
            flow = "rts_cts";
            break;
        default:
            flow = "unknown";
            break;
    }

    char *parity;

    switch (cfg->parity) {
        case UART_PARITY_NONE:
            parity = "none";
            break;
        case UART_PARITY_ODD:
            parity = "odd";
            break;
        case UART_PARITY_EVEN:
            parity = "even";
            break;
        case UART_PARITY_MARK:
            parity = "mark";
            break;
        case UART_PARITY_SPACE:
            parity = "space";
            break;
        default:
            parity = "unknown";
            break;
    }

    printf("             <baud><data><stop><flow><parity>\r\n");
    printf("config %s to <%d><%d><%s><%s><%s> pin <%d>\r\n",
           argv[1],
           cfg->baudrate,
           cfg->data_bits + 5,
           stop, flow, parity,
           *pin);

    return 0;
}

SHELL_CMD_EXPORT(test_cfg, run test_config);