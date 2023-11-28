/** @brief      W25QXX application interface.
 *
 *  @file       aiio_w25qxx.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       I2C application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>Define file
 *  </table>
 *
 */
#ifndef __AIIO_W25QXX_H__
#define __AIIO_W25QXX_H__
#include "aiio_spi.h"

/** @brief W25QXX function initializes
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      aiio_spi_cfg    W25QXX pin config
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_w25qxx_init(aiio_spi_enum_t aiio_spi_port, aiio_spi_pin_cfg_t *aiio_spi_cfg);

/** @brief W25QXX read data function
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      word_addr       W25QXX word address
 *  @param[in]      buf             W25QXX buf data
 *  @param[in]      buf_len         W25QXX buf lenght
 *  @param[in]      timeout         delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_w25qxx_read(aiio_spi_enum_t aiio_spi_port, uint16_t word_addr, uint8_t *buf, uint16_t buf_len, uint32_t timeout);

/** @brief W25QXX read byte function
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      word_addr       W25QXX word address
 *  @param[in]      data            W25QXX return data
 *  @param[in]      timeout         delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_w25qxx_read_byte(aiio_spi_enum_t aiio_spi_port, uint16_t word_addr, uint8_t *data, uint32_t timeout);

/** @brief W25QXX write byte function
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      word_addr       W25QXX word address
 *  @param[in]      data            W25QXX data
 *  @param[in]      timeout         delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_w25qxx_write_byte(aiio_spi_enum_t aiio_spi_port, uint16_t word_addr, uint8_t data, uint32_t timeout);

/** @brief W25QXX write data function
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      word_addr       W25QXX word address
 *  @param[in]      buf             W25QXX buf data
 *  @param[in]      buf_len         W25QXX buf lenght
 *  @param[in]      timeout         delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_w25qxx_write(aiio_spi_enum_t aiio_spi_port, uint16_t word_addr, const uint8_t *buf, uint16_t buf_len, uint32_t timeout);

/** @brief W25QXX erase function
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      word_addr       W25QXX word address
 *  @param[in]      timeout         delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_w25qxx_erase(aiio_spi_enum_t aiio_spi_port, uint16_t word_addr, uint32_t timeout);

/** @brief W25QXX enable function
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      timeout         delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_w25qxx_enable(aiio_spi_enum_t aiio_spi_port, uint32_t timeout);

/** @brief W25QXX read status function
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      status          W25QXX status value
 *  @param[in]      timeout         delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_w25qxx_read_status(aiio_spi_enum_t aiio_spi_port, uint8_t *status, uint32_t timeout);

/** @brief W25QXX read id function
 *
 *  @param[in]      aiio_spi_port   SPI interface
 *  @param[in]      id              W25QXX id value
 *  @param[in]      timeout         delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_w25qxx_read_id(aiio_spi_enum_t aiio_spi_port, uint8_t *id, uint32_t timeout);
#endif
