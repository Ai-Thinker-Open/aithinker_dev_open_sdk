/**
 * @file     bootram_port.c
 * @author   BSP Team
 * @brief    This file provides bootram port function.
 * @version  0.0.0.1
 * @date     2020-12-29
 *
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 *
 */

/* Includes ------------------------------------------------------------------*/

#include "ramcode_port.h"
#include "serial/serial.h"
#include "hal/hal_flash.h"
#include "hal/hal_misc.h"
/*****************************************  variables *********************************************/

static Serial_t bootram_fd;

// buffer for sector alignment.
static uint8_t temp_4k_buffer[SIZE_4KB] = {0};

/*****************************************  functions *********************************************/

int bootram_flash_info(void)
{
    return hal_flash_read_id();
}

int bootram_flash_read(uint32_t offset, uint32_t len, void* buf)
{
    hal_flash_read(offset, len, buf);
    return 0;
}

int bootram_flash_write(uint32_t offset, uint32_t len, const void* buf)
{
    hal_flash_program(offset, len, (uint8_t*)buf);
    return 0;
}

int bootram_flash_erase(uint32_t offset, uint32_t len)
{
    if (offset % SIZE_4KB == 0) {  // 4KB aligned.
        hal_flash_erase(offset, len);
    }
    else {  // NOT 4KB aligned.
        uint32_t base = (offset / SIZE_4KB) * SIZE_4KB;
        uint32_t head = offset - base;
        uint32_t tail = SIZE_4KB - head;

        // read first sector
        memset(temp_4k_buffer, 0, SIZE_4KB);
        hal_flash_read(base, SIZE_4KB, temp_4k_buffer);
        memset(temp_4k_buffer + head, 0xFF, tail);

        // erase
        uint32_t erase_length = len + head;
        hal_flash_erase(base, erase_length);

        // program
        hal_flash_program(base, SIZE_4KB, temp_4k_buffer);
    }

    return 0;
}

int bootram_flash_chiperase(void)
{
    hal_flash_chip_erase();
    return 0;
}

void bootram_user_reboot(void)
{
    hal_misc_reset_all();
}

void bootram_serial_init(void)
{
    serial_init(&bootram_fd, SER_PORT_UART0, CFG_UART_BAUDRATE_CONSOLE, NULL);
}

size_t bootram_serial_write(const void* buf, size_t size)
{
    return serial_write(&bootram_fd, (const void*)buf, size);
}

unsigned char bootram_serial_setbaudrate(uint32_t baudrate)
{
    return serial_setbaudrate(&bootram_fd, baudrate);
}

int bootram_serial_flush(void)
{
    return serial_flush(&bootram_fd);
}

size_t bootram_serial_read(void* buf, size_t size)
{
    return serial_read(&bootram_fd, buf, size);
}
