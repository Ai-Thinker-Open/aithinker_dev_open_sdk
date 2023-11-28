/**
 * @brief   Declare all kind of platform ota data proccess interface
 * 
 * @file    aiio_platform_ota.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-27          <td>1.0.0            <td>zhuolm             <td> The all kind of platform ota data proccess interface
 */
#ifndef __AIIO_PLATFORM_OTA_H_
#define __AIIO_PLATFORM_OTA_H_
#include "aiio_log.h"
#include "aiio_type.h"
#include "aiio_error.h"



/**
 * @brief   OTA status code
 */
#define AIIO_OTA_INSTALL_SUCCESS        (0xaa55)


/**
 * @brief   Open the flash and related verification on the platform
 * 
 * @return  int  Reture all kind of error code that is defined in aiio_error.h
 */
int aiio_platform_ota_start(void);


/**
 * @brief   Close the flash and related verification on the platform, and release the relevant resources
 * 
 * @return  int  Reture all kind of error code that is defined in aiio_error.h
 */
int aiio_platform_ota_stop(void);


/**
 * @brief   Check the OTA data and write the OTA data to the flash
 * @note    This function must return AIIO_OTA_INSTALL_SUCCESS when  the firmware is fully written to the flash
 * 
 * @param[in]   data                    The ota data
 * @param[in]   data_len                The length of writting the data to the flash
 * @param[in]   data_total_len          The total length of the ota data
 * 
 * @return  int  Reture all kind of error code that is defined in aiio_error.h or aiio_protocol_code.h
 */
int aiio_platform_ota_install(uint8_t *data, uint32_t data_len, uint32_t data_total_len);


CHIP_API int32_t aiio_init_ota_partition(void);

CHIP_API int32_t aiio_partition_write_ota_farmware(int32_t dst_offset, const void *src, int32_t size);

CHIP_API int32_t aiio_partition_erase(int32_t start_addr, int32_t size);

CHIP_API void aiio_set_boot_partition(void);



#endif

