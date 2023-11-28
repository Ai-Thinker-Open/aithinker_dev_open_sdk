#include "ln882h.h"
#include "proj_config.h"
#include "utils/debug/log.h"
#include "serial.h"

#include "stddef.h"

#define LOG_PORT_BAUDRATE  CFG_UART_BAUDRATE_LOG

Serial_t m_LogSerial;

int m_LogSerial_inited_flag = 0;

int log_stdio_write(char *buf, size_t size)
{
    int ret = 0;
	if (m_LogSerial_inited_flag == 0) return ret;

    ret = serial_write(&m_LogSerial, (const void *)buf, size);
    return ret;
}

#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')

/**
* dump_hex
* 
* @brief dump data in hex format
* 
* @param buf: User buffer
* @param size: Dump data size
* @param number: The number of outputs per line
* 
* @return void
*/
static void dump_hex(uint8_t level, const uint8_t *buf, uint32_t size, uint32_t number)
{
    uint32_t i, j;

    for (i = 0; i < size; i += number)
    {
        LOG(level, "%08X: ", i);

        for (j = 0; j < number; j++)
        {
            if (j % 8 == 0)
            {
                LOG(level, " ");
            }

            if (i + j < size) {
                LOG(level, "%02X ", buf[i + j]);
            } else {
                LOG(level, "   ");
            }
        }
        LOG(level, " ");

        for (j = 0; j < number; j++)
        {
            if (i + j < size)
            {
                LOG(level, "%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
            }
        }
        LOG(level, "\r\n");
    }
}

void hexdump(uint8_t level, const char *info, void *buff, uint32_t count)
{
    LOG(level, "%s:\r\n", info);
    dump_hex(level, (const uint8_t *)buff, count, 16);
}


void log_init(void)
{
    serial_init(&m_LogSerial, SER_PORT_UART0, LOG_PORT_BAUDRATE, NULL);

    m_LogSerial_inited_flag = 1;
}

void log_deinit(void)
{
    serial_deinit(&m_LogSerial);

    m_LogSerial_inited_flag = 0;
}

