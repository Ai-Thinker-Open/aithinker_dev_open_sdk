#include "aiio_chip_init.h"
#include "aiio_chip_spec.h"
#include "aiio_os_port.h"
#include "aiio_autoconf.h"
#include "aiio_error.h"
#include "aiio_adapter_include.h"
#include "syn_user_info.h"

#define MAIN_ENTRY()                __MAIN_ENTRY()
#define AIIO_OS_PRIORITY_DEFAULT    __AIIO_OS_PRIORITY_DEFAULT

#define AIIO_MAIN_STACK             CONFIG_AIIO_MAIN_STACK

#if CONFIG_AIIO_BIND_CHECK_EN
uint8_t g_bind_sta = 0;

void syn_event_cb(uint8_t event, Event_Msg_t *pMsg)
{
    switch (event)
    {
        case SYN_EVT_INIT_INFO:
            break;
        case SYN_EVT_COMPLETE:
            aiio_uart_log_send("SYN_EVT_COMPLETE: %d\r\n", pMsg->state);
            break;
        default:
            break;
    }
}
#endif


MAIN_ENTRY()
{
    extern void aiio_wsrc_call(void);
    aiio_wsrc_call();

    // 芯片初始化
    aiio_chip_init();
    // log初始化
    aiio_log_init();
    #if CONFIG_AIIO_BIND_CHECK_EN
    syn_user_info(syn_event_cb, &g_bind_sta);
    if (0x27 != g_bind_sta) {
        const char *notify = "invaild bind info\r\n";
        aiio_uart_log_send(notify, strlen(notify));
        #if CONFIG_at_cmd
        aiio_uart_config_t uart_cfg = {
            .uart_num = CONFIG_AT_UART_PORT,
            .uart_tx_pin = CONFIG_AT_UART_PIN_TX,
            //.uart_rx_pin = CONFIG_AT_UART_PIN_RX,
            .baud_rate = 115200,
            .dataBits=AIIO_UART_DATA_BITS_8,
            .stopBits=AIIO_UART_STOP_BITS_1,
            .parity=AIIO_UART_PARITY_NONE,
        };
        aiio_uart_init(uart_cfg);
        aiio_uart_send_data(CONFIG_AT_UART_PORT, (uint8_t *)notify, strlen(notify) );
        #endif
        while (1);
    }
    #endif
    // 打印log信息
    aiio_manufacturer_info();
    static aiio_os_thread_handle_t *aiio_main_thread = NULL;
    extern aiio_os_function_return_t aiio_main(void *params);

    if (AIIO_OK != aiio_os_thread_create((aiio_os_thread_handle_t*)&aiio_main_thread, "aiio_main", aiio_main, CONFIG_AIIO_MAIN_STACK, NULL, AIIO_OS_PRIORITY_DEFAULT))
    {
    }
#if CONFIG_AIIO_HEAP_LOG_EN
    static aiio_os_thread_handle_t *aiio_heap_check_thread = NULL;
    if (AIIO_OK != aiio_os_thread_create((aiio_os_thread_handle_t*)&aiio_heap_check_thread, "aiio_heap_check", aiio_heap_check, CONFIG_AIIO_MAIN_STACK / 2, NULL, AIIO_OS_PRIORITY_DEFAULT))
    {
    }
#endif
    aiio_os_thread_start_scheduler();

}


