/**
 * @brief      mdns application interface.
 *
 * @file       aiio_mdns.h
 * @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       mdns application interface.
 * @par        Change Logs:
 * <table>
 * <tr><th>Date            <th>Version     <th>Author      <th>Notes
 * <tr><td>2023/02/16      <td>1.0.0       <td>yanch       <td>Define the first version of the MDNS interface
 * </table>
 *
 */

#ifndef __AIIO_MDNS_H__
#define __AIIO_MDNS_H__

#include "aiio_errcode.h"

#define TXT_MAX_NUM (11)
#define TXT_KEY_MAX_LEN (10)
#define TXT_VALUE_MAX_LEN (250)

/**
 * @brief mdns configuration structure
 */
typedef struct
{
    char *host_name;    /**< the host name */
    char *service_type; /**< Service type */
    uint16_t port;      /**< The port number */
} aiio_mdns_config_t;

/**
 * @brief key-value pair structure
 */
typedef struct
{
    char key[TXT_KEY_MAX_LEN];     /**< key name */
    char value[TXT_VALUE_MAX_LEN]; /**< key value */
} aiio_mdns_txt_item_t;

/**
 * @brief   mdns initialization function
 *
 * @param[in]   config  mdns configuration parameters
 *
 * @return  aiio_ret_t
 * @retval  AIIO_OK     Deinitialization succeeded
 * @retval  AIIO_ERROR  Deinitialization failed
 *
 * @note    Initialize mdns and open mdns service.
 * @see
 */
aiio_ret_t aiio_mdns_init(aiio_mdns_config_t config);

/**
 * @brief   mdns deinitialization function
 *
 * @return  aiio_ret_t
 * @retval  AIIO_OK     Deinitialization succeeded
 * @retval  AIIO_ERROR  Deinitialization failed
 *
 * @note    Deinitialize mdns and release related resources
 * @see
 */
aiio_ret_t aiio_mdns_deinit(void);

/**
 * @brief   Update txt record function
 *
 * @param[in]   service_type    Service type
 * @param[in]   protocol        Protocol type, for example: _tcp, _udp, etc.
 * @param[in]   txt             Txt key-value pair array
 * @param[in]   num_items       The length of the key-value pair array
 *
 * @return  aiio_ret_t
 * @retval  AIIO_OK     Processed successfully
 * @retval  AIIO_ERROR  Processing failed
 *
 * @note
 * @see
 */
aiio_ret_t aiio_mdns_update_txt(char *service_type, uint16_t protocol, aiio_mdns_txt_item_t txt[], uint8_t num_items);

#endif // !__AIIO_MDNS_H__
