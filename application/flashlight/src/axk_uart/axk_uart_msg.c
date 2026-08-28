#include "axk_uart_msg.h"
#include "aiio_adapter_include.h"
#include "axk_ringbuff.h"
#include "axk_uart_init.h"
#include "axk_at_cmd.h"
#include "axk_at.h"
#include "axk_sleep.h"
#include "axk_wifi_init.h"

#include "aiio_os_port.h"
#include "aiio_autoconf.h"



static int32_t axk_uart_msg_task_init(void);

axk_uart_msg_t g_uart_msg =
{
    .init = axk_uart_msg_task_init,
};

static void axk_uart_recv_task(void *param)
{
    (void)(param);

    int32_t buff_size;
    uint8_t cmdstr[AT_CMD_MAX_LEN];
    uint16_t cmdlen = 0;

    // register cmd
    axk_at_esp_cmd_register();

    for (;;)
    {
        //prosess data
        buff_size = g_ringbuff_ctrl.get_size();
        if (buff_size <= 0)
        {
            aiio_os_tick_dealy(aiio_os_ms2tick(5));
            continue;
        }

        if (g_at_ctrl.specific_callback)
        {
            uint8_t ch;
            g_ringbuff_ctrl.pop_data(&ch, 1);
            g_at_ctrl.specific_callback(ch);
            continue;
        }

        g_ringbuff_ctrl.pop_data(&cmdstr[cmdlen], 1);
        cmdlen += 1;

        if (cmdlen > AT_CMD_MAX_LEN)
        {
            cmdlen = 0;

            g_uart.send_data(cmdstr, sizeof(cmdstr));
            g_ringbuff_ctrl.flush();
            memset(cmdstr, 0, sizeof(cmdstr));
            AT_ENTER;
            AT_RESPONSE_ERROR;
            continue;
        }

        if (cmdlen >= 4 && ('\r' == cmdstr[cmdlen - 2]) && ('\n' == cmdstr[cmdlen - 1]))
        {
            cmdstr[cmdlen - 2] = '\0';

        #if CONFIG_AXK_AUTO_SLEEP
            // 已连接网络
            if (g_axk_wifi.got_ip != 0)
            {
                //已连接网络，发配网指令过来，把休眠时间改成2分钟
                if (strstr((const char *)cmdstr, "AT+WEBWIFICFG=1") ||
                    strstr((const char *)cmdstr, "AT+BLUFI=1") ||
                    strstr((const char *)cmdstr, "AT+APPWIFICFG=1"))
                {
                    axk_sleep_timer_change(120000);
                }
                else
                {
                    //已发配网指令，再发其他指令，休眠时间改为30秒
                    axk_sleep_timer_change(30000);
                }
            }
        #endif

            axk_at_cmd_execute((char *)cmdstr);
            cmdlen = 0;
        }
    }
}

static int32_t axk_uart_msg_task_init(void)
{
    return aiio_os_thread_create(NULL, "uart_rx", axk_uart_recv_task, 4096, NULL, 10);
}

