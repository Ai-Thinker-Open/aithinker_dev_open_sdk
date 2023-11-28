/**
 * @brief      AES128 fill and cut interface
 *
 * @file       aiio_aes_utils_filling.h
 * @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       AES128 fill and cut interface
 * @par        Change Logs:
 * <table>
 * <tr><th>Date            <th>Version     <th>Author      <th>Notes
 * <tr><td>2023/02/23      <td>1.0.0       <td>yanch       <td>AES128 filling api define
 * </table>
 *
 */
#ifndef __AIIO_AES_UTILS_FILLING_H__
#define __AIIO_AES_UTILS_FILLING_H__
#include "aiio_log.h"
#include "aiio_type.h"
#include "aiio_error.h"

/**
 * @brief aes128 pkcs7 padding method
 *
 * @param[in,out]  data              Data to be filled
 * @param[in]      dataSize          Array capacity for storing data
 * @param[in]      dataLen           Length of data
 *
 * @returns int     Return the fill status. When the return value is AIIO_OK, is successful.
 * @retval  -1      length less than or equal to zero
 * @retval  -3      Failed to fill
 * @retval  >0      padded length
 *
 * @note
 * @see
 */
int pkcs7_padding(char *data, int dataSize, int dataLen);

/**
 * @brief aes128 pkcs7 cutting method
 *
 * @param[in,out]   data      Data to be cut
 * @param[in]       dataLen   Length of data
 *
 * @returns int     Return the cut status. When the return value is AIIO_OK, is successful.
 * @retval  -1      length less than or equal to zero
 * @retval  -3      Padding check failed
 * @retval  >0      cut length
 *
 * @note
 * @see
 */
int pkcs7_cuttingg(char *data, int dataLen);

#endif
