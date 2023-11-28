/**
 * @file     ln_drv_spi_flash.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __LN_DRV_SPI_FLASH_H
#define __LN_DRV_SPI_FLASH_H


#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#define TEM_HUM_ADDR        0x88


#include "ln_drv_spi.h"


void spi_flash_read_id(uint8_t *id);
void spi_flash_read_status(uint8_t *status);
void spi_flash_write_enable(void);
void spi_flash_erase_flash(uint32_t add);
void spi_flash_write_flash(uint32_t addr ,uint8_t *data, uint32_t length);
void spi_flash_read_flash(uint32_t addr ,uint8_t *data, uint32_t length);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_SPI_FLASH_H


