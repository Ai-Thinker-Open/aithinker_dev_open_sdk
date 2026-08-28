/** @brief      SPI application interface.
 *
 *  @file       aiio_spi.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       SPI application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>spi init
 *  </table>
 *
 */
#ifndef __AIIO_SPI_H__
#define __AIIO_SPI_H__
#include "aiio_log.h"
#include "aiio_type.h"
#include "aiio_error.h"
#include "aiio_gpio.h"

/**
 *  @brief SPI interface enumeration
 */
typedef enum
{
    AIIO_SPI_0,    /*!< SPI interface zero */
    AIIO_SPI_1,    /*!< SPI interface one */
    AIIO_SPI_2,    /*!< SPI interface two */
    AIIO_SPI_SOFT, /*!< SPI software interface */
} aiio_spi_enum_t;

/**
 *  @brief SPI pin config interface
 */
typedef struct
{
    uint32_t aiio_gpio_clk_port;  /*!< GPIO port */
    uint32_t aiio_gpio_clk_pin;  /*!< GPIO pin */
    uint32_t aiio_gpio_mosi_port; /*!< GPIO port */
    uint32_t aiio_gpio_mosi_pin; /*!< GPIO pin */
    uint32_t aiio_gpio_miso_port; /*!< GPIO port */
    uint32_t aiio_gpio_miso_pin; /*!< GPIO pin */
    uint32_t aiio_gpio_cs_port;   /*!< GPIO port */
    uint32_t aiio_gpio_cs_pin;   /*!< GPIO pin */
} aiio_spi_pin_cfg_t;

/** @brief SPI function initializes
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      aiio_spi_cfg    SPI pin config
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_spi_master_init(aiio_spi_enum_t aiio_spi_port, aiio_spi_pin_cfg_t *aiio_spi_cfg);

/** @brief SPI function fails to initialeze
 *
 *  @param[in]      aiio_spi_port              SPI interface
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_spi_master_deinit(aiio_spi_enum_t aiio_spi_port);

/** @brief SPI master write data function
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      addr            SPI address
 *  @param[in]      addr_len        SPI address lenght
 *  @param[in]      send_data       SPI send buff
 *  @param[in]      data_len        SPI data lenght
 *  @param[in]      timeout         SPI delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_spi_master_write(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *send_data, uint32_t data_len,uint32_t timeout);

/** @brief SPI master write byte function
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      addr            SPI address
 *  @param[in]      send_data       SPI send data
 *  @param[in]      timeout         SPI delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_spi_master_write_byte(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t send_data,uint32_t timeout);

/** @brief SPI master read data function
 *
 *  @param[in]      aiio_spi_port  SPI interface
 *  @param[in]      addr           SPI address
 *  @param[in]      addr_len       SPI address lenght
 *  @param[in]      rec_data       SPI receve buff
 *  @param[in]      data_len       SPI data lenght
 *  @param[in]      timeout         SPI delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_spi_master_read(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *rec_data, uint32_t data_len,uint32_t timeout);

/** @brief SPI master read byte function
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      addr            SPI address
 *  @param[in]      addr_len        SPI address lenght
 *  @param[in]      rec_data        SPI receve data
 *  @param[in]      timeout         SPI delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_spi_master_read_byte(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *rec_data,uint32_t timeout);

/** @brief SPI master write and read function
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      send_data       SPI send buff
 *  @param[in]      rec_data        SPI receve buff
 *  @param[in]      data_len        SPI data lenght
 *  @param[in]      timeout         SPI delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_spi_master_write_and_read(aiio_spi_enum_t aiio_spi_port, uint8_t *send_data, uint8_t *rec_data, uint32_t data_len,uint32_t timeout);

#endif
