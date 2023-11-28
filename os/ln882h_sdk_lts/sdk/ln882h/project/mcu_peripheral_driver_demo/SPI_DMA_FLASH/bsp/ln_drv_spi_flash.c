/**
 * @file     ln_drv_spi_flash.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_spi_flash.h"

/**
 * @brief SPI flash 读取flash id.
 * 
 * @param id 读取到的ID
 */
void spi_flash_read_id(uint8_t *id)
{
    uint8_t CMD_READ_ID[4] = {0x9F,0xFF,0xFF,0xFF};
    uint8_t buffer[4];
    spi_dma_master_write_and_read_data(CMD_READ_ID,buffer,4);
    memcpy(id,buffer+1,3);
}

/**
 * @brief 读取SPI flash 的状态
 * 
 * @param status 读取到的状态
 */
void spi_flash_read_status(uint8_t *status)
{
    uint8_t CMD_READ_STATUS_1[2] = {0x05,0xFF};
    uint8_t CMD_READ_STATUS_2[2] = {0x35,0xFF};
    uint8_t buffer[2] = {0,0};
    spi_dma_master_write_and_read_data(CMD_READ_STATUS_1,buffer,2);
    memcpy(status,buffer+1,1);
    spi_dma_master_write_and_read_data(CMD_READ_STATUS_2,buffer,2);
    memcpy(status+1,buffer+1,1);
}

/**
 * @brief 写使能
 * 
 */
void spi_flash_write_enable()
{
    uint8_t CMD_WRITE_ENABLE[1] = {0x06};
    uint8_t buffer[1];
    spi_dma_master_write_and_read_data(CMD_WRITE_ENABLE,buffer,1);
}

/**
 * @brief 擦除对应地址4k的Flash
 * 
 * @param add 要擦除的地址
 */
void spi_flash_erase_flash(uint32_t add)
{
    spi_flash_write_enable();
    uint8_t CMD_ERASE_FLASH[4] = {0x20,0x00,0x00,0x00};
    uint8_t buffer[4];
    CMD_ERASE_FLASH[1] = (add >> 16) & 0xFF;
    CMD_ERASE_FLASH[2] = (add >> 8)  & 0xFF;
    CMD_ERASE_FLASH[3] = (add >> 0)  & 0xFF;
    spi_dma_master_write_and_read_data(CMD_ERASE_FLASH,buffer,4);
}

/**
 * @brief SPI FLASH 写入数据
 * 
 * @param addr   flash地址
 * @param data   要写入的数据
 * @param length 要写入数据的长度
 */
void spi_flash_write_flash(uint32_t addr ,uint8_t *data, uint32_t length)
{   
    uint8_t buffer[4] = {0x02,0x00,0x00,0x00};
    spi_flash_write_enable();
    buffer[1] = (addr >> 16) & 0xFF;
    buffer[2] = (addr >> 8)  & 0xFF;
    buffer[3] = (addr >> 0)  & 0xFF;
    spi_dma_master_write_data_with_addr(buffer,4,data,length);
}

/**
 * @brief SPI FLASH 读取数据
 * 
 * @param addr   flash地址
 * @param data   读取数据存储的指针
 * @param length 要读取的数据长度
 */
void spi_flash_read_flash(uint32_t addr ,uint8_t *data, uint32_t length)
{
    uint8_t buffer[4] = {0x03,0x00,0x00,0x00};
    buffer[1] = (addr >> 16) & 0xFF;
    buffer[2] = (addr >> 8)  & 0xFF;
    buffer[3] = (addr >> 0)  & 0xFF;
    
    spi_dma_master_read_data_with_addr(buffer,4,data,length);
}
