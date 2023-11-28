/**
 * @brief      flash application interface.
 *
 * @file       aiio_flash.c
 * @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       flash application interface.
 * @par        Change Logs:
 * <table>
 * <tr><th>Date            <th>Version     <th>Author      <th>Notes
 * <tr><td>2023/06/27      <td>1.0.0       <td>zhuolm      <td>Define flash API
 * </table>
 *
 */

#include "aiio_type.h"
#include "bflb_flash.h"
#include "aiio_error.h"
#include "aiio_log.h"


int aiio_flash_init(void)
{
    return bflb_flash_init();
}

int aiio_flash_deinit(void)
{
    return AIIO_OK;
}

int aiio_flash_write(uint32_t write_addr, const char *write_data, uint16_t write_length)
{
    return bflb_flash_write(write_addr, (uint8_t *)write_data, write_length);
}

int aiio_flash_read(uint32_t read_addr, uint8_t *data, uint16_t *read_length)
{
    aiio_log_d("read_addr = 0x%08x \r\n", read_addr);
    return bflb_flash_read(read_addr, data, *read_length);
}

int aiio_flash_erase(uint32_t erase_addr, uint16_t erase_length)
{
    return bflb_flash_erase(erase_addr, erase_length);
}