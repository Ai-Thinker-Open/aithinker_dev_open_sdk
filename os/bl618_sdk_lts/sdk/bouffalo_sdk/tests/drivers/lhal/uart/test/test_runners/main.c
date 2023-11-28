#include "bflb_uart.h"
#include "bflb_clock.h"
#include "board.h"
#include "shell.h"
#include "unity_fixture.h"

static struct bflb_device_s *uart0;

static void run_all_test(void)
{
    RUN_TEST_GROUP(uart_fifo_interrupt_rx);
}

static int test(int argc, const char *argv[])
{
    return UnityMain(argc, argv, run_all_test);
}

int main(void)
{
    board_init();
    uart0 = bflb_device_get_by_name("uart0");
    shell_init();

    // auto start test
    // test(1, (void *)"test");

    while (1) {
        while (bflb_uart_rxavailable(uart0)) {
            shell_handler(bflb_uart_getchar(uart0));
        }
    }

    return 0;
}

SHELL_CMD_EXPORT(test, run test);
