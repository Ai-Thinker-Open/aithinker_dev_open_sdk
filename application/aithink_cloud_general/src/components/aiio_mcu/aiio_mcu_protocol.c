#include "aiio_mcu_protocol.h"
#include "aiio_common.h"
#include "aiio_uart.h"
#include "aiio_log.h"
#include "aiio_adapter_include.h"
#include "aiio_at_receive.h"
#include "aiio_mcu_system.h"
#include "aiio_softtimer.h"

aiio_device_params DeviceParams = {0};
static uint8_t  uart_ring_buffer[AIIO_UART1_RECEIVE_BUFFER_MAX_LEN] = {0};
static uint8_t  uart_receive_buffer[AIIO_UART1_RECEIVE_BUFFER_MAX_LEN] = {0};
static uint32_t heart_beat_timeout = 1000;
static AIIO_TIMER   mcu_public_timer = -1;
static AIIO_TIMER   heart_beat_timer = -1;
static ring_buff_t uart_ring_buff_hd = {0};

static void aiio_uart1_receive_data(uint8_t *buf, uint16_t size)
{
    ring_buff_push_data(&uart_ring_buff_hd, (char *)buf, size);
    UartRevStream(buf, size);
}


void SetPublicDataTimeout(uint32_t SetTimeout)
{
    aiio_TimerStart(mcu_public_timer, SetTimeout);
}


void  SetHeartBeatTimeout(uint32_t SetTimeout)
{
    heart_beat_timeout = SetTimeout;
    aiio_TimerStart(heart_beat_timer, heart_beat_timeout);
}


static void McuProcessTask(void *arg)
{
    uint16_t cmdlen = 0;
    uint32_t uart_receive_len = 0;

    aiio_TimerStart(heart_beat_timer, heart_beat_timeout);

    while (1)
    {
        UartProcessPro();

        if(aiio_CheckTimerTimeout(heart_beat_timer) == AIIO_TIMER_TIMEOUT)
        {
            aiio_log_i("send heart beat data \r\n");
            UartSendHeatBeat();
        }

        if(aiio_CheckTimerTimeout(mcu_public_timer) == AIIO_TIMER_TIMEOUT)
        {
            aiio_log_d("mcu pubilc data \r\n");
            McuPubliReportAttributes();
        }

        if(HeartBeatTimeout())
        {
            aiio_log_i("send mcu heart beat timeout, then reboot \r\n");
            aiio_os_tick_dealy(aiio_os_ms2tick(20));
            aiio_restart();
        }
        if(ring_buff_get_size(&uart_ring_buff_hd) <= 0)
        {
            aiio_os_tick_dealy(aiio_os_ms2tick(20));
            continue;
        }
        ring_buff_pop_data(&uart_ring_buff_hd, (char *)&uart_receive_buffer[uart_receive_len], 1);
        uart_receive_len += 1;
        if(uart_receive_len >= AIIO_UART1_RECEIVE_BUFFER_MAX_LEN)
        {
            ring_buff_flush(&uart_ring_buff_hd);
            uart_receive_len = 0;
            aiio_memset(uart_receive_buffer, 0, 100);
            continue;
        }
        if((uart_receive_len >= 4) && ('\r' == uart_receive_buffer[uart_receive_len-2]) && ('\n' == uart_receive_buffer[uart_receive_len-1]))
        {
            aiio_log_d("rev:%s \r\n", uart_receive_buffer);
            aiio_log_d("cmdlen:%d \r\n", uart_receive_len);
            uart_receive_buffer[uart_receive_len-2] = '\0';
            atCmdExecute((char *)uart_receive_buffer);
            uart_receive_len = 0;
            aiio_memset(uart_receive_buffer, 0, AIIO_UART1_RECEIVE_BUFFER_MAX_LEN);
        }

    }
}

void aiio_mcu_process_init(void)
{
    aiio_uart_config_t uart_config = {0};

    UartProtocolInit();
    McuCmdDataListInit();
    ring_buff_init(&uart_ring_buff_hd, (char *)uart_ring_buffer, AIIO_UART1_RECEIVE_BUFFER_MAX_LEN);
    IotLoadDeviceParams(&DeviceParams);
    if(DeviceParams.uart_bound == 0){
        memset(&DeviceParams, 0, sizeof(aiio_device_params));
        IotClearDeviceParams();
        DeviceParams.uart_bound = AIIO_UART1_BAUDRATE;
        DeviceParams.work_mode = 1;
        IotSaveDeviceParams(&DeviceParams);
    }
    uart_config.uart_num = AIIO_UART1;
    uart_config.uart_tx_pin = AIIO_UART1_TX_PIN;
    uart_config.uart_rx_pin = AIIO_UART1_RX_PIN;
    uart_config.baud_rate = DeviceParams.uart_bound;
    uart_config.irq_rx_bk = aiio_uart1_receive_data;
    aiio_uart_init(uart_config);

    // // 创建心跳发送定时器
    heart_beat_timer = aiio_TimerAdd(AIIO_REPEAT_TIMER);
    if(heart_beat_timer == -1)
    {
        aiio_log_e("add heart beat timer fail \r\n");
        return ;
    }
    aiio_TimerStop(heart_beat_timer);

    // 创建上报数据发送定时器
    mcu_public_timer = aiio_TimerAdd(AIIO_SINGLE_TIMER);
    if(mcu_public_timer == -1)
    {
        aiio_log_e("add mcu_public_timer fail \r\n");
        return ;
    }
    aiio_TimerStop(mcu_public_timer);

    xTaskCreate(McuProcessTask, (char*)"McuProcessTask", 4*1024/4, NULL, 15, NULL);
}