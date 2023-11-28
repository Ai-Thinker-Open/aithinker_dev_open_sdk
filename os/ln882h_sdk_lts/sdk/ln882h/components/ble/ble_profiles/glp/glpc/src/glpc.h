/**
 ****************************************************************************************
 *
 * @file glpc.h
 *
 * @brief Header file - Glucose Profile - Collector Role.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef _GLPC_H_
#define _GLPC_H_

/**
 ****************************************************************************************
 * @addtogroup GLPC Glucose Profile Collector
 * @ingroup GLP
 * @brief Glucose Profile Collector
 *
 * The GLPC is responsible for providing Glucose Profile Collector functionalities
 * to upper layer module or application. The device using this profile takes the role
 * of Glucose Profile Collector.
 *
 * Glucose Profile Collector. (GLPC): A GLPC (e.g. PC, phone, etc)
 * is the term used by this profile to describe a device that can interpret Glucose
 * measurement in a way suitable to the user application.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_GL_COLLECTOR)
#include "glp_common.h"
#include "ke_task.h"
#include "prf.h"
#include "prf_types.h"
#include "prf_utils.h"
#include "glpc_task.h"
/*
 * DEFINES
 ****************************************************************************************
 */
/// Maximum number of Glucose Collector task instances
#define GLPC_IDX_MAX    (BLE_CONNECTION_MAX)


/// 30 seconds record access control point timer
#define GLPC_RACP_TIMEOUT                  0x0BB8

/// Possible states of the GLPC task
enum glpc_state
{
    /// Not Connected state
    GLPC_FREE,
    /// IDLE state
    GLPC_IDLE,
    /// Discovering Glucose SVC and Chars
    GLPC_DISCOVERING,
    /// Busy state
    GLPC_BUSY,

    /// Number of defined states.
    GLPC_STATE_MAX
};

/// Internal codes for reading/writing a GLS characteristic with one single request
enum glpc_read_code
{
    /// Read GLP Feature
    GLPC_RD_FEAT = 0,

    /// Read Glucose Measurement Client Char. Configuration Descriptor
    GLPC_RD_DESC_MEAS_CLI_CFG = 1,

    /// Read Glucose Measurement context Client Char. Configuration Descriptor
    GLPC_RD_DESC_MEAS_CTX_CLI_CFG = 2,

    /// Read Record Access control point Client Char. Configuration Descriptor
    GLPC_RD_DESC_RACP_CLI = 3,

};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Glucose Profile Collector environment variable per connection
struct glpc_cnx_env
{
    /// Last requested UUID(to keep track of the two services and char)
    uint16_t last_uuid_req;
    /// counter used to check service uniqueness
    uint8_t nb_svc;
    /// used to store if measurement context
    uint8_t meas_ctx_en;
    ///HTS characteristics
    struct gls_content gls;
    /// Current read Op code @see enum glpc_read_code
    uint8_t read_code;
};

/// Glucose Profile Collector environment variable
struct glpc_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Environment variable pointer for each connections
    struct glpc_cnx_env *p_env[GLPC_IDX_MAX];
    /// State of different task instances
    ke_state_t state[GLPC_IDX_MAX];
};


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve GLP client profile interface
 *
 * @return GLP client profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs *glpc_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send Glucose ATT DB discovery results to GLPC host.
 *
 * @param p_glpc_env    Glucose Profile Client Role task environment
 * @param conidx        Connection index
 * @param status        Response status code
 ****************************************************************************************
 */
void glpc_enable_rsp_send(struct glpc_env_tag *p_glpc_env, uint8_t conidx, uint8_t status);

/**
 ****************************************************************************************
 * @brief Unpack Glucose measurement value
 *
 * @param[in] p_packed_meas Glucose measurement value packed
 * @param[out] p_meas_val Glucose measurement value
 * @param[out] p_seq_num Glucose measurement sequence number
 *
 * @return size of packed value
 ****************************************************************************************
 */
uint8_t glpc_unpack_meas_value(uint8_t *p_packed_meas, struct glp_meas *p_meas_val,
                              uint16_t *p_seq_num);

/**
 ****************************************************************************************
 * @brief Unpack Glucose measurement context value
 *
 * @param[in] p_packed_meas_ctx Glucose measurement context value packed
 * @param[out] p_meas_val Glucose measurement context value
 * @param[out] p_seq_num Glucose measurement sequence number
 *
 * @return size of packed value
 ****************************************************************************************
 */
uint8_t glpc_unpack_meas_ctx_value(uint8_t *p_packed_meas_ctx,
                                 struct glp_meas_ctx *p_meas_ctx_val,
                                 uint16_t *p_seq_num);

/**
 ****************************************************************************************
 * @brief Pack Record Access Control request
 *
 * @param[out] p_packed_val Record Access Control Point value packed
 * @param[in] p_racp_req Record Access Control Request value
 *
 * @return size of packed data
 ****************************************************************************************
 */
uint8_t glpc_pack_racp_req(uint8_t *p_packed_val, const struct glp_racp_req *p_racp_req);

/**
 ****************************************************************************************
 * @brief Unpack Record Access Control response
 *
 * @param[in] p_packed_val Record Access Control Point value packed
 * @param[out] p_racp_rsp Record Access Control Response value
 *
 * @return size of packed data
 ****************************************************************************************
 */
uint8_t glpc_unpack_racp_rsp(uint8_t *p_packed_val, struct glp_racp_rsp *p_racp_rsp);

/**
 ****************************************************************************************
 * @brief Check if collector request is possible or not
 *
 * @param p_glpc_env    Glucose Profile Client Role task environment
 * @param conidx        Connection index
 * @param char_code     Characteristic code @see enum glpc_char_code
 *
 * @return GAP_ERR_NO_ERROR if request can be performed, error code else.
 ****************************************************************************************
 */
uint8_t glpc_validate_request(struct glpc_env_tag *p_glpc_env, uint8_t conidx, uint8_t char_code);

/**
 ****************************************************************************************
 * @brief Gets correct read handle according to the request
 * @param[in] p_glpc_env environment variable
 * @param[in] conidx Connection index
 * @param[in] p_param Pointer to the parameters of the message.
 * @return handle
 ****************************************************************************************
 */
uint16_t glpc_get_read_handle_req(struct glpc_env_tag *p_glpc_env, uint8_t conidx, struct glpc_read_cmd const *p_param);

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * Initialize task handler
 *
 * @param p_task_desc Task descriptor to fill
 ****************************************************************************************
 */
void glpc_task_init(struct ke_task_desc *p_task_desc);

#endif /* (BLE_GL_COLLECTOR) */

/// @} GLPC

#endif /* _GLPC_H_ */
