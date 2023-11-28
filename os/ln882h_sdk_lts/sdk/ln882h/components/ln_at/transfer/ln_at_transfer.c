/**
 * @file   ln_at_transfer.c
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-28     MurphyZhao   the first version
 */

#include "ln_at_transfer.h"

#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "serial/serial.h"
#include "transparent.h"
#include "ln_at_mqtt.h"
#include "utils/system_parameter.h"
#include "utils/debug/log.h"


#ifdef CFG_UART_BAUDRATE_CONSOLE
  #define CONSOLE_PORT_BAUDRATE       CFG_UART_BAUDRATE_CONSOLE
#else
  #define CONSOLE_PORT_BAUDRATE       (115200)
#endif

static Serial_t       g_at_serial;
static ln_at_transfer_t g_at_transfer;


//sys at uart serial need
at_sys_serial_handle_t g_sys_handle[1] = {0};
at_sys_serial_handle_t *at_sys_serial_get_handle(void);


static void ln_serial_rx_callbcak(void);
static int  ln_transfer_init(void);
static int  ln_transfer_deinit(void);
static int  ln_transfer_write(const char *buf, size_t size);
static int  ln_transfer_putc(char ch);
static char ln_transfer_getc(void);

at_sys_serial_handle_t *at_sys_serial_get_handle(void)
{
    g_sys_handle[0].serial_hdl = &g_at_serial;
    g_sys_handle[0].trans_hdl = &g_at_transfer;
    g_sys_handle[0].rx_cb = ln_serial_rx_callbcak;
    
    return &g_sys_handle[0];
}

int ln_at_write(const char *buf, size_t size)
{
    int ret = 0;
    ret = serial_write(&g_at_serial, (const void *)buf, size);
    return ret;
}

static int ln_transfer_write(const char *buf, size_t size)
{
    int ret = 0;
    ret = serial_write(&g_at_serial, (const void *)buf, size);
    return ret;
}

static int ln_transfer_putc(char ch)
{
    int ret = 0;
    serial_putchar(&g_at_serial, ch);
    return ret;
}

static char ln_transfer_getc(void)
{
    return 0;
}

static void ln_transfer_task(void *arg)
{
    ln_at_transfer_t *transfer = &g_at_transfer;
    Serial_t *port = (Serial_t *)arg;
    int len = 0;
    uint8_t ch;

    if (transfer->status == 0)
    {
        LN_AT_LOG_E("Transfer is not created!\r\n");
        return;
    }

	ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
	ln_mqtt_cfg_t* pMqtt = ln_get_mqtt_handle();
  while(1)
  {
    if (0 == ln_at_sem_wait(transfer->sem, LN_AT_WAIT_FOREVER))
    {
      while (!fifo_isempty(&port->rxfifo))
      {
        len = serial_read(port, &ch, 1);
        if(len > 0)
        {					
					if (p->bSend)
					{
						if(p->_push_back(&ch, 1) == 0){
							p->bSend = 0;
						}
					}
					//reference ESP32-C3 v2.4.0.0
					else if (pMqtt->mqtt_data.iScanf == LN_MQTT_SCANF_PUBRAW )
					{
						//LN_AT_LOG_E("after is_empty: %d\r\n", fifo_isempty(&port->rxfifo));
						pMqtt->scanf(ch, (int)fifo_isempty(&port->rxfifo));
						if (0 == pMqtt->mqtt_data.scanfLen)
						{
							fifo_flush(&port->rxfifo);
						}
					}
					else
					{
						ln_at_preparser(ch);
					}
         }
       }
    }
  }
}

static void ln_serial_rx_callbcak(void)
{
    ln_at_transfer_t *transfer = &g_at_transfer;
    ln_at_sem_release(transfer->sem);
}

static int ln_transfer_init(void)
{
    ln_at_transfer_t *transfer = &g_at_transfer;
    Serial_t *fd = NULL;
    if (transfer->status == 0)
    {
        LN_AT_LOG_E("Transfer is not created!\r\n");
        return -1;
    }

    /*
     *  Init Serial.
     *  Console use SER_PORT_UART0/SER_PORT_UART1.
     */
    fd = &g_at_serial;

    /* SER_PORT_UART0 or SER_PORT_UART1 */
    sys_uart_status_t uart_cfg = {0};
    sysparam_sys_uart_get(&uart_cfg);
    serial_init(fd, SER_PORT_UART1, uart_cfg.baud, ln_serial_rx_callbcak);

    transfer->sem = ln_at_sem_create(0, 1024);
    if (transfer->sem == NULL) {
        return -1;
    }

    if(ln_at_task_create("AT_Transfer",   \
                (void *)ln_transfer_task, \
                fd,                       \
                LN_AT_TRANSFER_TASK_PRI,  \
                LN_AT_TRANSFER_TASK_STACK_SIZE) == NULL)
    {
        return -1;
    }

    serial_write(fd, "Console init ok!\r\n", strlen("Console init ok!\r\n"));

    return 0;
}

static int ln_transfer_deinit(void)
{
    memset(&g_at_transfer, 0x0, sizeof(ln_at_transfer_t));
    return 0;
}

ln_at_transfer_t *ln_transfer_create(void)
{
    ln_at_transfer_t *transfer = &g_at_transfer;
    memset(transfer, 0x0, sizeof(ln_at_transfer_t));

    transfer->init   = ln_transfer_init;
    transfer->deinit = ln_transfer_deinit;
    transfer->getc   = ln_transfer_getc;
    transfer->putc   = ln_transfer_putc;
    transfer->write  = ln_transfer_write;
    transfer->status = 1;
    return transfer;
}
