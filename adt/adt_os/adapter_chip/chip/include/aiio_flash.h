/**
 * @brief      flash application interface.
 *
 * @file       aiio_flash.h
 * @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       flash application interface.
 * @par        Change Logs:
 * <table>
 * <tr><th>Date            <th>Version     <th>Author      <th>Notes
 * <tr><td>2023/02/19      <td>1.0.0       <td>yanch       <td>Define flash API
 * <tr><td>2023/04/18      <td>1.0.1       <td>linzy       <td>Add a comment
 * <tr><td>2023/05/05      <td>1.0.2       <td>linzy       <td>Modify the comment
 * <tr><td>2023/06/06      <td>1.0.2       <td>linzy       <td>update flash interface
 * </table>
 *
 */

#ifndef __AIIO_FLASH_H__
#define __AIIO_FLASH_H__

#include "stdint.h"
#include "aiio_type.h"

/**
 * @brief flash partition device
 */
typedef struct flash_dev {
    void *flash_dev;    /**< @brief flash device */
}flash_dev_t;

/**
 * @brief  flash partition manage struct
 */
typedef struct {
    const char  *partition_description; /**< @brief name */
    uint32_t     partition_start_addr;  /**< @brief start addr */
    uint32_t     partition_length;      /**< @brief length */
    uint32_t     partition_options;     /**< @brief options */
}logic_partition_t;

/** @brief flash function initializes
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_init(void);

/** @brief flash function fail to initializes
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_deinit(void);

/** @brief write flash function
 *  @param[in]      write_addr          write flash address
 *  @param[in]      write_src_data      write flash source date
 *  @param[in]      write_length        write flash length
 *  @return                             Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK             Init successful.
 *  @retval         AIIO_ERROR          Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_write(uint32_t write_addr, uint8_t *write_src_data, uint32_t write_length);

/** @brief Write data to an area on a flash logical partition with erase first
 *  @param[in/out]  addr                Point to the start address that the data is written to, and
 *                                      point to the last unwritten address after this function is
 *                                      returned, so you can call this function serval times without
 *                                      update this start address.
 *  @param[in]      src_data            point to the data buffer that will be written to flash
 *  @param[in]      length              The length of the buffer
 *  @return                             Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK             Init successful.
 *  @retval         AIIO_ERROR          Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_write_erase(uint32_t addr, uint8_t *src_data, uint32_t length);

/** @brief Read data from an area on a Flash to data buffer
 *  @param[in/out]  read_addr          Point to the start address that the data is read, and
 *                                     point to the last unread address after this function is
 *                                     returned, so you can call this function serval times without
 *                                     update this start address.
 *  @param[in]      read_dst_data      Point to the data buffer that stores the data read from flash
 *  @param[in]      read_length        The length of the buffer
 *  @return                            Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK            Init successful.
 *  @retval         AIIO_ERROR         Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_read(uint32_t read_addr, uint8_t *read_dst_data, uint32_t read_length);

/** @brief Erase an area on a Flash logical partition
 *  @param[in]     erase_addr          Start address of the erased flash area
 *  @param[in]     erase_length        Size of the erased flash area
 *  @return                            Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval        AIIO_OK             Init successful.
 *  @retval        AIIO_ERROR          Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_erase(uint32_t erase_addr, uint32_t erase_length);

/** @brief Open a flash partition device
 *  @param[in]     name                flash partition name
 *  @param[in]     flags               flash flags
 *  @return                            Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval        AIIO_OK             Init successful.
 *  @retval        AIIO_ERROR          Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_open(const char *name, unsigned int flags);

/** @brief Get the information of the specified flash area
 *  @param[in]     p_dev               The target flash logical partition device
 *  @param[in]     partition           The buffer to store partition info
 *  @return                            Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval        AIIO_OK             Init successful.
 *  @retval        AIIO_ERROR          Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_info_get(flash_dev_t *p_dev, logic_partition_t *partition);

/** @brief Close a flash partition device
 *  @param[in]     p_dev              flash partition device
 *  @return                           Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval        AIIO_OK            Init successful.
 *  @retval        AIIO_ERROR         Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_close(flash_dev_t *p_dev);

/** @brief Get Flash ID
 *  @return                           Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval        AIIO_OK            Init successful.
 *  @retval        AIIO_ERROR         Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_id_read(void);

/** @brief Get Flash  Device ID
 *  @return                           Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval        AIIO_OK            Init successful.
 *  @retval        AIIO_ERROR         Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_device_id_read(void);

/** @brief Get Flash  Unique ID 
 *  @return                           Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval        AIIO_OK            Init successful.
 *  @retval        AIIO_ERROR         Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_unique_id_read(void);

/** @brief Get Flash  Manufacturer ID
 *  @return                           Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval        AIIO_OK            Init successful.
 *  @retval        AIIO_ERROR         Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_manufacturer_id_read(void);

/** @brief Read Status Register
 *  @return                           Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval        AIIO_OK            Init successful.
 *  @retval        AIIO_ERROR         Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_status_read(void);

/** @brief Read Status Register1
 *  @return                           Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval        AIIO_OK            Init successful.
 *  @retval        AIIO_ERROR         Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_status_register1_read(void);

/** @brief Read Status Register2
 *  @return                           Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval        AIIO_OK            Init successful.
 *  @retval        AIIO_ERROR         Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_flash_status_register2_read(void);

#endif
