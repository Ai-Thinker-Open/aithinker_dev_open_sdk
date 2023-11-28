/**
 * @file     hal_spi.h
 * @author   BSP Team 
 * @brief    This file contains all of spi functions prototypes.
 * @version  0.0.0.1
 * @date     2020-12-09
 * 
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
#ifndef __HAL_SPI_H
#define __HAL_SPI_H

#ifdef __cplusplus
    extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hal/hal_common.h"
#include "reg_spi.h"


#define IS_SPI_ALL_PERIPH(PERIPH) ((PERIPH) == SPI0_BASE || (PERIPH) == SPI1_BASE) 


typedef enum
{
    SPI_DIRECTION_2LINES_FULLDUPLEX = 0,
    SPI_DIRECTION_2LINES_RXONLY     = 1,
    SPI_DIRECTION_1LINE_RX          = 2,
    SPI_DIRECTION_1LINE_TX          = 3,
}spi_direction_t;

#define IS_SPI_DIRECTION_MODE(MODE) (((MODE) == SPI_DIRECTION_2LINES_FULLDUPLEX) || \
                                     ((MODE) == SPI_DIRECTION_2LINES_RXONLY)     || \
                                     ((MODE) == SPI_DIRECTION_1LINE_RX)          || \
                                     ((MODE) == SPI_DIRECTION_1LINE_TX))



typedef enum
{
    SPI_MODE_SLAVE  = 0,
    SPI_MODE_MASTER = 1,
}spi_mode_t;

#define IS_SPI_MODE(MODE) (((MODE) == SPI_MODE_MASTER) || \
                           ((MODE) == SPI_MODE_SLAVE))

typedef enum
{
    SPI_DATASIZE_16B    = 0,
    SPI_DATASIZE_8B     = 1,
}spi_data_size_t;
#define IS_SPI_DATASIZE(DATASIZE) (((DATASIZE) == SPI_DATASIZE_16B) || \
                                   ((DATASIZE) == SPI_DATASIZE_8B))

typedef enum
{
    SPI_CPOL_LOW    = 0,
    SPI_CPOL_HIGH   = 1,
}spi_cpol_t;

#define IS_SPI_CPOL(CPOL) (((CPOL) == SPI_CPOL_LOW) || \
                           ((CPOL) == SPI_CPOL_HIGH))


typedef enum
{
    SPI_CPHA_1EDGE    = 0,
    SPI_CPHA_2EDGE    = 1,
}spi_cpha_t;

#define IS_SPI_CPHA(CPHA) (((CPHA) == SPI_CPHA_1EDGE) || \
                           ((CPHA) == SPI_CPHA_2EDGE))

typedef enum
{
    SPI_NSS_HARD    = 0,
    SPI_NSS_SOFT    = 1,
}spi_nss_model_t;

#define IS_SPI_NSS_MODEL(NSS) (((NSS) == SPI_NSS_SOFT) ||  ((NSS) == SPI_NSS_HARD))

typedef enum
{
    SPI_BAUDRATEPRESCALER_2      = 0x01,
    SPI_BAUDRATEPRESCALER_4      = 0x02,
    SPI_BAUDRATEPRESCALER_8      = 0x03,
    SPI_BAUDRATEPRESCALER_16     = 0x04,
    SPI_BAUDRATEPRESCALER_32     = 0x05,
    SPI_BAUDRATEPRESCALER_64     = 0x06,
    SPI_BAUDRATEPRESCALER_128    = 0x07,
    SPI_BAUDRATEPRESCALER_256    = 0x08,
}spi_baud_rate_precaler_t;

#define IS_SPI_BAUDRATE_PRESCALER(PRESCALER) (((PRESCALER) == SPI_BAUDRATEPRESCALER_2)   || \
                                              ((PRESCALER) == SPI_BAUDRATEPRESCALER_4)   || \
                                              ((PRESCALER) == SPI_BAUDRATEPRESCALER_8)   || \
                                              ((PRESCALER) == SPI_BAUDRATEPRESCALER_16)  || \
                                              ((PRESCALER) == SPI_BAUDRATEPRESCALER_32)  || \
                                              ((PRESCALER) == SPI_BAUDRATEPRESCALER_64)  || \
                                              ((PRESCALER) == SPI_BAUDRATEPRESCALER_128) || \
                                              ((PRESCALER) == SPI_BAUDRATEPRESCALER_256))

typedef enum
{
    SPI_FIRST_BIT_MSB    = 0,
    SPI_FIRST_BIT_LSB    = 1,
}spi_first_bit_t;

#define IS_SPI_FIRST_BIT(BIT) (((BIT) == SPI_FIRST_BIT_MSB) || \
                               ((BIT) == SPI_FIRST_BIT_LSB))

typedef enum
{
    SPI_CRC_TX    = 0,
    SPI_CRC_RX    = 1,
}spi_crc_model_t;
#define IS_SPI_CRC_MODEL(CRC) (((CRC) == SPI_CRC_TX) || ((CRC) == SPI_CRC_RX))

typedef enum
{
    SPI_DMA_TX_EN    = 0,
    SPI_DMA_RX_EN    = 1,
}spi_dma_en_t;
#define IS_SPI_DMA_EN(DMA) (((DMA) == SPI_DMA_TX_EN) || ((DMA) == SPI_DMA_RX_EN))

typedef enum
{
    SPI_STATUS_FLAG_RXNE    = 0x01,
    SPI_STATUS_FLAG_TXE     = 0x02,
                                        //reserved
                                        //reserved
                                        //reserved
    SPI_STATUS_FLAG_CRC_ERR = 0x10,
    SPI_STATUS_FLAG_MODF    = 0x20,     //TODO
    SPI_STATUS_FLAG_OVR     = 0x40,
    SPI_STATUS_FLAG_BSY     = 0x80,
}spi_status_flag_t;
#define SPI_STATUS_FLAG_BIT_NUM         8
#define IS_SPI_STATUS_FLAG(FLAG)        (((((uint32_t)FLAG) & 0xFF) != 0x00U) && ((((uint32_t)FLAG) & ~0xFF) == 0x00U))


typedef enum
{
    SPI_IT_FLAG_RXNE    = 0x01,
    SPI_IT_FLAG_TXE     = 0x02,
                                    //reserved
                                    //reserved
    SPI_IT_FLAG_CRC_ERR = 0x10,     //write 0 clear
    SPI_IT_FLAG_MODF    = 0x20,     //TODO
    SPI_IT_FLAG_OVR     = 0x40,

}spi_it_flag_t;

#define SPI_IT_FLAG_BIT_NUM         7
#define IS_SPI_IT_FLAG(FLAG)        (((((uint32_t)FLAG) & 0x7F) != 0x00U) && ((((uint32_t)FLAG) & ~0x7F) == 0x00U))



#define IS_SPI_CRC_POLYNOMIAL(POLYNOMIAL) (1)


typedef struct
{
    spi_direction_t             spi_direction;                  /*!< Specifies the SPI unidirectional or bidirectional data mode.
                                                                 This parameter can be a value of @ref SPI_data_direction */

    spi_mode_t                  spi_mode;                       /*!< Specifies the SPI operating mode.
                                                                 This parameter can be a value of @ref SPI_mode */

    spi_data_size_t             spi_data_size;                  /*!< Specifies the SPI data size.
                                                                 This parameter can be a value of @ref SPI_data_size */

    spi_cpol_t                  spi_cpol;                       /*!< Specifies the serial clock steady state.
                                                                 This parameter can be a value of @ref SPI_Clock_Polarity */

    spi_cpha_t                  spi_cpha;                       /*!< Specifies the clock active edge for the bit capture.
                                                                 This parameter can be a value of @ref SPI_Clock_Phase */

    spi_nss_model_t             spi_nss_mode;                   /*!< Specifies whether the NSS signal is managed by
                                                                 hardware (NSS pin) or by software using the SSI bit.
                                                                 This parameter can be a value of @ref SPI_Slave_Select_management */

    spi_baud_rate_precaler_t    spi_baud_rate_prescaler;        /*!< Specifies the Baud Rate prescaler value which will be
                                                                 used to configure the transmit and receive SCK clock.
                                                                 This parameter can be a value of @ref SPI_BaudRate_Prescaler.
                                                                @note The communication clock is derived from the master
                                                                 clock. The slave clock does not need to be set. */

    spi_first_bit_t             spi_first_bit;                  /*!< Specifies whether data transfers start from MSB or LSB bit.
                                                                 This parameter can be a value of @ref SPI_MSB_LSB_transmission */

    uint16_t                    spi_crc_polynomial;             /*!< Specifies the polynomial used for the CRC calculation. */

}spi_init_type_def;


                //spi init and config
void            hal_spi_init(uint32_t spi_x_base,spi_init_type_def* spi_init);
void            hal_spi_deinit(uint32_t spi_x_base);
void            hal_spi_set_nss(uint32_t spi_x_base,spi_nss_model_t software_config);
void            hal_spi_ssoe_en(uint32_t spi_x_base,hal_en_t en);
void            hal_spi_set_data_size(uint32_t spi_x_base,spi_data_size_t data_size);
void            hal_spi_set_bidirectional_line(uint32_t spi_x_base,spi_direction_t spi_direction);

void            hal_spi_en(uint32_t spi_x_base,hal_en_t en);
void            hal_spi_dma_en(uint32_t spi_x_base,spi_dma_en_t spi_dma_en, hal_en_t en);

                //send / receive data 
void            hal_spi_send_data(uint32_t spi_x_base,uint16_t data);
uint16_t        hal_spi_recv_data(uint32_t spi_x_base);
uint8_t         hal_spi_wait_txe(uint32_t spi_x_base,uint32_t  timeout);
uint8_t         hal_spi_wait_rxne(uint32_t spi_x_base,uint32_t  timeout);
uint8_t         hal_spi_wait_bus_idle(uint32_t spi_x_base,uint32_t  timeout);

                //CRC
void            hal_spi_transmit_crc(uint32_t spi_x_base);
void            hal_spi_calculate_crc_en(uint32_t spi_x_base,hal_en_t en);
uint16_t        hal_spi_get_crc(uint32_t spi_x_base,spi_crc_model_t spi_crc_model);
uint16_t        hal_spi_get_crc_polynomial(uint32_t spi_x_base);
void            hal_spi_set_crc_polynomial(uint32_t spi_x_base,uint32_t crc_poly);

                //interrupt
void            hal_spi_it_cfg(uint32_t spi_x_base,spi_it_flag_t spi_it,hal_en_t en);

uint8_t         hal_spi_get_status_flag(uint32_t spi_x_base,spi_status_flag_t spi_status_flag);
void            hal_spi_clr_status_flag(uint32_t spi_x_base,spi_status_flag_t spi_status_flag);

uint8_t         hal_spi_get_it_flag(uint32_t spi_x_base,spi_it_flag_t spi_it_flag);
void            hal_spi_clr_it_flag(uint32_t spi_x_base,spi_it_flag_t spi_it_flag);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_SPI_H */


/**************************************END OF FILE********************************************/


