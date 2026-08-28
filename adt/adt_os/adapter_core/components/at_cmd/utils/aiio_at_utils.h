/** @brief      AT utils.
 *
 *  @file       aiio_chip_init.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/28      <td>1.0.0       <td>lins        <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_UTILS_H__
#define __AIIO_AT_UTILS_H__

#include "aiio_type.h"
#include "aiio_error.h"
#include "aiio_os_port.h"

#define AIIO_AT_MAC_FORMATSTRING_LOWCASE "%02x%02x%02x%02x%02x%02x"
#define AIIO_AT_MAC_FORMAT(mac) mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]

#define AIIO_AT_CMD_PARSE_NUMBER(i, num)                        \
    do                                                          \
    {                                                           \
        if ((i >= argc) ||                                      \
            (aiio_at_getint_from_at(num, argv[i])))             \
        {                                                       \
            aiio_at_set_errcode(AIIO_AT_ERR_SYS_PARAM_INVALID); \
            return AIIO_AT_RET_FAILURE;                         \
        }                                                       \
    } while (0);

#define AIIO_AT_CMD_PARSE_NUMBER_EX(i, num, min, max)                 \
    do                                                                \
    {                                                                 \
        if ((i >= argc) ||                                            \
            (aiio_at_getint_from_at_ex(num, argv[i], min, max)))      \
        {                                                             \
            aiio_at_set_errcode(AIIO_AT_ERR_SYS_PARAM_INVALID);       \
            return AIIO_AT_RET_FAILURE;                               \
        }                                                             \
    } while (0);

#define AIIO_IP4ADDR_STRLEN_MAX  (16)
#define aiio_ip4_addr_get_u32(src_ipaddr) ((src_ipaddr)->addr)

typedef struct{
    uint32_t addr;
}aiio_at_ip4_addr_t;

/** @brief Check string is a number
 *
 *  @param[in]      str             String.
 *  @return                         Return the operation status. When the return value is AIIO_OK, is a number.
 *  @retval         AIIO_OK         Successful.
 *  @retval         AIIO_ERROR      Error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_at_str_is_num(const char *str);

/** @brief Get integer from a string
 *
 *  @param[out]     dest            Output number.
 *  @param[in]      src             String.
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Successful.
 *  @retval         AIIO_ERROR      Error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_at_getint_from_at(int32_t *dest, const char *src);

/** @brief Get integer from a string
 *
 *  @param[out]     dest            Output number.
 *  @param[in]      src             String.
 *  @param[in]      min             Number minimum.
 *  @param[in]      max             Number maximum.
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Successful.
 *  @retval         AIIO_ERROR      Error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_at_getint_from_at_ex(int32_t *dest, const char *src, int32_t min, int32_t max);

/** @brief Convert hex string to byte array
 *
 *  @param[out]     dest            Output number.
 *  @param[in]      src             String.
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Successful.
 *  @retval         AIIO_ERROR      Error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_at_hexstr_to_bytes(const char *in, uint8_t *out);

/** @brief Convert byte array to hex string
 *
 *  @param[in]      in              Bytes array.
 *  @param[out]     out             String.
 *  @param[in]      size            Bytes array size.
 *  @return                         Void.
 *  @note
 *  @see
 */
CORE_API void aiio_at_bytes_to_hexstr(const uint8_t *in, char *out, uint16_t size);

/** @brief Parse AT string
 *
 *  @param[in]      in              String input.
 *  @param[out]     out             String output.
 *  @param[in]      size            Buffer size.
 *  @return                         Void.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_at_str_parse(const char *in, char *out, uint16_t size);

#endif
