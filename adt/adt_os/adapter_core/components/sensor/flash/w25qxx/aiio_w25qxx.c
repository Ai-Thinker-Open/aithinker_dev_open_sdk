#include "aiio_w25qxx.h"

int32_t aiio_w25qxx_init(aiio_spi_enum_t aiio_spi_port, aiio_spi_pin_cfg_t *aiio_spi_cfg)
{
    if (aiio_spi_master_init(aiio_spi_port, aiio_spi_cfg) != AIIO_OK)
    {
        aiio_log_w("aiio_w25qxx_init function init is fail!");
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_w25qxx_read(aiio_spi_enum_t aiio_spi_port, uint16_t word_addr, uint8_t *buf, uint16_t buf_len,uint32_t timeout)
{
    uint8_t buffer[4] = {0x03, 0x00, 0x00, 0x00};
    buffer[1] = (word_addr >> 16) & 0xFF;
    buffer[2] = (word_addr >> 8)  & 0xFF;
    buffer[3] = (word_addr >> 0)  & 0xFF;

    if (aiio_spi_master_read(aiio_spi_port, buffer, 4, buf, buf_len,timeout) != AIIO_OK)
    {
        aiio_log_w("aiio_w25qxx_read_status function read id is fail!");
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_w25qxx_read_byte(aiio_spi_enum_t aiio_spi_port, uint16_t word_addr, uint8_t *data,uint32_t timeout)
{
    if (aiio_w25qxx_read(aiio_spi_port, word_addr, data, 1,timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_w25qxx_write_byte(aiio_spi_enum_t aiio_spi_port, uint16_t word_addr, uint8_t data,uint32_t timeout)
{
    if (aiio_w25qxx_write(aiio_spi_port, word_addr, &data, 1,timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_w25qxx_write(aiio_spi_enum_t aiio_spi_port, uint16_t word_addr, const uint8_t *buf, uint16_t buf_len,uint32_t timeout)
{
    uint8_t buffer[4] = {0x02, 0x00, 0x00, 0x00};
    aiio_w25qxx_enable(aiio_spi_port,timeout);
    buffer[1] = (word_addr >> 16) & 0xFF;
    buffer[2] = (word_addr >> 8)  & 0xFF;
    buffer[3] = (word_addr >> 0)  & 0xFF;

    if (aiio_spi_master_write(aiio_spi_port, buffer, 4, buf, buf_len,timeout) != AIIO_OK)
    {
        aiio_log_w("aiio_w25qxx_read_status function read id is fail!");
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_w25qxx_erase(aiio_spi_enum_t aiio_spi_port, uint16_t word_addr,uint32_t timeout)
{
    aiio_w25qxx_enable(aiio_spi_port,timeout);
    uint8_t cmd_erase_flash[4] = {0x20, 0x00, 0x00, 0x00};
    uint8_t buffer[4];
    cmd_erase_flash[1] = (word_addr >> 16) & 0xFF;
    cmd_erase_flash[2] = (word_addr >> 8)  & 0xFF;
    cmd_erase_flash[3] = (word_addr >> 0)  & 0xFF;

    if (aiio_spi_master_write_and_read(aiio_spi_port, cmd_erase_flash, buffer, 1,timeout) != AIIO_OK)
    {
        aiio_log_w("aiio_w25qxx_read_status function read id is fail!");
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_w25qxx_enable(aiio_spi_enum_t aiio_spi_port,uint32_t timeout)
{
    uint8_t cmd_write_enable[1] = {0x06};
    uint8_t buffer[1];

    if (aiio_spi_master_write_and_read(aiio_spi_port, cmd_write_enable, buffer, 1,timeout) != AIIO_OK)
    {
        aiio_log_w("aiio_w25qxx_read_status function read id is fail!");
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_w25qxx_read_status(aiio_spi_enum_t aiio_spi_port, uint8_t *status,uint32_t timeout)
{
    uint8_t cmd_read_status_1[2] = {0x05, 0xFF};
    uint8_t cmd_read_status_2[2] = {0x35, 0xFF};
    uint8_t buffer[2] = {0x00, 0x00};

    if (status == NULL)
    {
        aiio_log_w("aiio_w25qxx_read_status function status is null!");
        return AIIO_ERROR;
    }

    if (aiio_spi_master_write_and_read(aiio_spi_port, cmd_read_status_1, buffer, 2,timeout) != AIIO_OK)
    {
        aiio_log_w("aiio_w25qxx_read_status function read id is fail!");
        return AIIO_ERROR;
    }

    status[0] = buffer[1];

    if (aiio_spi_master_write_and_read(aiio_spi_port, cmd_read_status_2, buffer, 2,timeout) != AIIO_OK)
    {
        aiio_log_w("aiio_w25qxx_read_status function read id is fail!");
        return AIIO_ERROR;
    }

    status[1] = buffer[1];
    return AIIO_OK;
}

int32_t aiio_w25qxx_read_id(aiio_spi_enum_t aiio_spi_port, uint8_t *id,uint32_t timeout)
{
    uint8_t cmd_read_id[4] = {0x9F, 0xFF, 0xFF, 0xFF};

    if (aiio_spi_master_write_and_read(aiio_spi_port, cmd_read_id, id, 4,timeout) != AIIO_OK)
    {
        aiio_log_w("aiio_w25qxx_read_id function read id is fail!");
        return AIIO_ERROR;
    }

    return AIIO_OK;
}
