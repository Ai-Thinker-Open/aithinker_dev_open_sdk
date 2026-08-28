#include "aiio_flash.h"
#include "aiio_type.h"
#include "aiio_error.h"

#include <stdint.h>

#include "flash_api.h"

#define AIIO_FLASH_SECTOR_SIZE  (4096)
int32_t aiio_flash_init(void)
{
    return AIIO_OK;
}

int32_t aiio_flash_deinit(void)
{
    return AIIO_OK;
}

int32_t aiio_flash_write(uint32_t write_addr, uint8_t *write_src_data, uint32_t write_length)
{
    int32_t ret = AIIO_ERROR;
    flash_t flash;

    if (1 != flash_stream_write(&flash, write_addr, write_length, write_src_data))
    {
        goto __err;
    }

    ret = AIIO_OK;

__err:
    return ret;
}

int32_t aiio_flash_read(uint32_t read_addr, uint8_t *read_dst_data, uint32_t read_length)
{
    int32_t ret = AIIO_ERROR;
    flash_t flash;

    if (1 != flash_stream_read(&flash, read_addr, read_length, read_dst_data))
    {
        goto __err;
    }

    ret = AIIO_OK;

__err:
    return ret;
}

int32_t aiio_flash_erase(uint32_t erase_addr, uint32_t erase_length)
{
    flash_t flash;

    int sector_num = (int)erase_length / AIIO_FLASH_SECTOR_SIZE;
    if (erase_length % AIIO_FLASH_SECTOR_SIZE != 0)
    {
        sector_num++;
    }

    for (int i = 0; i < sector_num; i++)
    {
        flash_erase_sector(&flash, erase_addr + (i * AIIO_FLASH_SECTOR_SIZE));
    }

    return AIIO_OK;
}
