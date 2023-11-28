/**
 * @brief      Ai-Thinker configurable network software local control interface
 *
 * @file       aiio_local_control.h
 * @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       Ai-Thinker configurable network software local control interface, used for NFC swiping and swiping local control
 * @par        Change Logs:
 * <table>
 * <tr><th>Date            <th>Version     <th>Author      <th>Notes
 * <tr><td>2023/02/24      <td>1.0.0       <td>yanch       <td>define local control API
 * </table>
 *
 */
#ifndef __AIIO_LOCAL_CONTROL_H__
#define __AIIO_LOCAL_CONTROL_H__

#include "aiio_log.h"
#include "aiio_type.h"
#include "aiio_error.h"
#include "aiio_errcode.h"
#include "wifi_config_comm_service.h"

/**
 * @brief Analyze the control data sent by the APP
 *
 * @param[in] udp_client    udp client handle
 * @param[in] data          Parsed data
 * @param[in] len           Data length
 *
 * @returns aiio_ret_t      Return data parsing result. When the return value is AIIO_OK, is successful.
 * @retval  AIIO_OK         Parsed successfully
 * @retval  AIIO_ERROR      Parsing failed
 *
 * @note
 * @see
 */
CORE_API aiio_ret_t aiio_local_control_protocol_analysis(udp_socket_client_t *udp_client, uint8_t *data, uint16_t len);

/**
 * @brief Send device control instruction setting result to APP
 *
 * @param[in] udp_client    udp client handle
 * @param[in] result        control results  0 is success, 1 is error
 *
 * @returns aiio_ret_t      Return sending result. When the return value is AIIO_OK, is successful.
 * @retval  AIIO_OK         Sent successfully
 * @retval  AIIO_ERROR      Failed to send
 *
 *  @note
 *  @see
 */
CORE_API aiio_ret_t aiio_local_control_send_response_control(udp_socket_client_t *udp_client, uint8_t result);

/**
 * @brief Send device status to APP
 *
 * @param[in] udp_client    udp client handle
 * @param[in] result        device status
 *
 * @returns aiio_ret_t      Return sending result. When the return value is AIIO_OK, is successful.
 * @retval  AIIO_OK         Sent successfully
 * @retval  AIIO_ERROR      Failed to send
 *
 *  @note
 *  @see
 */
CORE_API aiio_ret_t aiio_local_control_send_device_status(udp_socket_client_t *udp_client, uint8_t status);

/**
 * @brief Send the device unbinding result to the APP
 *
 * @param[in] udp_client    udp client handle
 * @param[in] result        unbind result
 *
 * @returns aiio_ret_t      Return sending result. When the return value is AIIO_OK, is successful.
 * @retval  AIIO_OK         Sent successfully
 * @retval  AIIO_ERROR      Failed to send
 *
 *  @note
 *  @see
 */
CORE_API aiio_ret_t aiio_local_control_send_unbind_results(udp_socket_client_t *udp_client, uint8_t result);

#endif
