/**
 ****************************************************************************************
 *
 * @file hogpd.h
 *
 * @brief Header file - Hid Over Gatt profile host .
 *
 * Copyright (C) LIGHTNINGSEMI 2020-2026
 *
 *
 ****************************************************************************************
 */

#ifndef HOGPRH_H_
#define HOGPRH_H_

/**
 ****************************************************************************************
 * @addtogroup Hid Over Gatt profile device
 * @ingroup DIS
 * @brief Hid Over Gatt profile device
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwprf_config.h"
#include "rwip_config.h"

#if (BLE_HID_REPORT_HOST)
#include "hogp_common.h"
#include "hogprh_task.h"

#include "prf_ble.h"
#include "prf_types.h"


/*
 * DEFINES
 ****************************************************************************************
 */

#define HOGPRH_IDX_MAX        (BLE_CONNECTION_MAX)

enum
{
    HOGPRH_FREE,
    /// Idle state
    HOGPRH_IDLE,
    
    HOGPRH_DISCOVERING,
    /// Busy state
    HOGPRH_BUSY,
    
    /// Number of defined states.
    HOGPRH_STATE_MAX,
};

/// Environment variable for each Connections
struct hogprh_cnx_env
{
    ///Current Time Service Characteristics
    struct hogprh_device_content device;
    /// Last char. code requested to read.
    uint8_t last_char_code;
    /// counter used to check service uniqueness
    uint8_t nb_svc;
};

/// Device Information Service Client environment variable
struct hogprh_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Environment variable pointer for each connections
    struct hogprh_cnx_env *p_env[HOGPRH_IDX_MAX];
    /// State of different task instances
    ke_state_t state[HOGPRH_IDX_MAX];
};

/**
 ****************************************************************************************
 * @brief Retrieve HID client profile interface
 *
 * @return HID client profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs *hogprh_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Retrieve Attribute handle from service and attribute index
 *
 * @param[in] p_hogprh_env  HID report host environment
 * @param[in] conidx        Connection index
 * @param[in] status        Response status code
 *
 ****************************************************************************************
 */
void hogprh_enable_rsp_send(struct hogprh_env_tag *p_hogprh_env, uint8_t conidx, uint8_t status);

/**
 ****************************************************************************************
 * @brief Retrieve Attribute handle from service and attribute index
 *
 * @param[in] p_hogprh_env  HID report host environment
 * @param[in] conidx        Connection index
 * @param[in] operation     Indicates the operation for which the cmp_evt is being sent
 * @param[in] status        Response status code
 *
 ****************************************************************************************
 */
void hogprh_send_cmp_evt(struct hogprh_env_tag *p_hogprh_env, uint8_t conidx, uint8_t operation, uint8_t status);

/**
 ****************************************************************************************
 * Initialize task handler
 *
 * @param task_desc Task descriptor to fill
 ****************************************************************************************
 */
void hogprh_task_init(struct ke_task_desc *p_task_desc);

#endif//#if (BLE_HID_DEVICE)

#endif ///#ifndef HOGPRH_H_

