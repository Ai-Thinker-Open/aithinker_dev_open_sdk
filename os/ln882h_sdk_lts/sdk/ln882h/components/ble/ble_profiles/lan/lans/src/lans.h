/**
 ****************************************************************************************
 *
 * @file lans.h
 *
 * @brief Header file - Location and Navigation Profile Sensor.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _LANS_H_
#define _LANS_H_

/**
 ****************************************************************************************
 * @addtogroup LANS Location and Navigation Profile Sensor
 * @ingroup LAN
 * @brief Location and Navigation Profile Sensor
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_LN_SENSOR)
#include "lan_common.h"
#include "lans_task.h"
#include "prf_types.h"
#include "prf.h"
#include "attm.h"
#include "co_math.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/// Maximum number of Location and Navigation Profile Sensor role task instances
#define LANS_IDX_MAX        (1)

/// Disabled fields of Mask Location and Speed Characteristic Content
/// Save Bit 0 - 6 to mask Location and Speed Characteristic Flags
#define LANS_LSPEED_DISABLED_MASK       (0x7F)

/// Navigation Control parameter value mask (value 0 - 5 corresponds bit 0 - 3 in this parameter)
#define LANS_NAV_CTRL_PT_VAL_MASK       (0x07)

/*
 * MACROS
 ****************************************************************************************
 */

/// Check if a specific characteristic has been added in the database (according to mask)
#define LANS_IS_FEATURE_SUPPORTED(prfl_cfg, mask)  ((prfl_cfg & mask) == mask)

/// MACRO TO CALCULATE HANDLE    shdl + idx - POSQ - CNTL PT
/// POSQ is 2 if the Position quality is supported otherwise 0
/// CNTL PT is 3 if the CP characteristic is supported otherwise 0
#define LANS_HANDLE(idx) \
    (p_lans_env->shdl + (idx) - \
        ((!(LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_POS_Q_MASK)) && \
                ((idx) > LNS_IDX_POS_Q_CHAR))? (2) : (0)) - \
        ((!(LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_LN_CTNL_PT_MASK)) && \
                ((idx) > LNS_IDX_LN_CTNL_PT_CHAR))? (3) : (0)))

/// Get database attribute index
#define LANS_IDX(hdl) \
    ((hdl - p_lans_env->shdl) + \
        ((!(LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_POS_Q_MASK)) && \
                ((hdl - p_lans_env->shdl) > LNS_IDX_POS_Q_CHAR)) ? (2) : (0)) + \
        ((!(LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_LN_CTNL_PT_MASK)) && \
                ((hdl - p_lans_env->shdl) > LNS_IDX_LN_CTNL_PT_CHAR)) ? (3) : (0)))

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the LANS task
enum lans_state
{
    /// Idle state
    LANS_IDLE,
    /// Busy state
    LANS_BUSY,

    /// Number of defined states.
    LANS_STATE_MAX
};

/// Location and Navigation Service - Attribute List
enum lans_cps_att_list
{
    /// Location and Navigation Service
    LNS_IDX_SVC,
    /// LN Feature
    LNS_IDX_LN_FEAT_CHAR,
    LNS_IDX_LN_FEAT_VAL,
    /// Location and Speed
    LNS_IDX_LOC_SPEED_CHAR,
    LNS_IDX_LOC_SPEED_VAL,
    LNS_IDX_LOC_SPEED_NTF_CFG,
    /// Position Quality
    LNS_IDX_POS_Q_CHAR,
    LNS_IDX_POS_Q_VAL,
    /// LN Control Point
    LNS_IDX_LN_CTNL_PT_CHAR,
    LNS_IDX_LN_CTNL_PT_VAL,
    LNS_IDX_LN_CTNL_PT_IND_CFG,
    /// Navigation
    LNS_IDX_NAVIGATION_CHAR,
    LNS_IDX_NAVIGATION_VAL,
    LNS_IDX_NAVIGATION_NTF_CFG,

    /// Number of attributes
    LNS_IDX_NB,
};

/// Profile Configuration
enum lans_prf_cfg_flag_bf
{
    /// Control Point supported flag
    LANS_CTNL_PT_CHAR_SUPP_FLAG_POS         = 0,
    LANS_CTNL_PT_CHAR_SUPP_FLAG_BIT         = CO_BIT(LANS_CTNL_PT_CHAR_SUPP_FLAG_POS),

    /// Broadcast supported flag
    LANS_NAVIGATION_SUPP_FLAG_POS           = 1,
    LANS_NAVIGATION_SUPP_FLAG_BIT           = CO_BIT(LANS_NAVIGATION_SUPP_FLAG_POS),
};

/// Profile Configuration NTF/IND Flags bit field
enum lans_prf_cfg_ntfind_flag_bf
{
    /// Location and Speed - Client Char. Cfg
    LANS_PRF_CFG_FLAG_LOC_SPEED_NTF_POS         = 0,
    LANS_PRF_CFG_FLAG_LOC_SPEED_NTF_BIT         = CO_BIT(LANS_PRF_CFG_FLAG_LOC_SPEED_NTF_POS),

    /// Control Point - Client Characteristic configuration
    LANS_PRF_CFG_FLAG_LN_CTNL_PT_IND_POS        = 1,
    LANS_PRF_CFG_FLAG_LN_CTNL_PT_IND_BIT        = CO_BIT(LANS_PRF_CFG_FLAG_LN_CTNL_PT_IND_POS),


    /// Navigation - Client Characteristic configuration
    LANS_PRF_CFG_FLAG_NAVIGATION_NTF_POS        = 2,
    LANS_PRF_CFG_FLAG_NAVIGATION_NTF_BIT        = CO_BIT(LANS_PRF_CFG_FLAG_NAVIGATION_NTF_POS),

    /// Bonded Data configured
    LANS_PRF_CFG_PERFORMED_OK_POS               = 3,
    LANS_PRF_CFG_PERFORMED_OK_BIT               = CO_BIT(LANS_PRF_CFG_PERFORMED_OK_POS),
};

/// LANS Configuration Flag Masks
enum lans_config_flag_mask
{
    /// Mandatory Attributes (LN Feature + Location and Speed)
    LANS_MANDATORY_LSB            = 0,
    LANS_MANDATORY_MASK           = (0x003F),

    /// Position quality Attribute
    LANS_POS_Q_LSB                = 6,
    LANS_POS_Q_MASK               = (0x00C0),

    /// Control Point Attributes
    LANS_LN_CTNL_PT_LSB           = 8,
    LANS_LN_CTNL_PT_MASK          = (0x0700),

    /// Navigation Attributes
    LANS_NAVI_LSB                 = 11,
    LANS_NAVI_MASK                = (0x3800),
};

/// Navigation control enable bit field
enum lans_navi_ctrl_en_bf
{
    /// Enable bit (RFU bit 7)
    LANS_NAV_CTRL_PT_EN_POS       = 7,
    LANS_NAV_CTRL_PT_EN_BIT       = CO_BIT(LANS_NAV_CTRL_PT_EN_POS),
};

/*
 * STRUCTURES
 ****************************************************************************************
 */
/// ongoing operation information
struct lans_op
{
    /// On-going operation command
    struct ke_msg *p_cmd;
    /// notification pending
    struct gattc_send_evt_cmd *p_ntf_pending;

    /// Cursor on connection
    uint8_t cursor;
};

/// Location and Navigation environment variable per connection
struct lans_cnx_env
{
    /// Location and speed content mask (bit 0 - 6)
    /// 0 : leave as default, 1 : turn off
    uint16_t mask_lspeed_content;
    /// Profile Notify/Indication Flags
    uint8_t prfl_ntf_ind_cfg;
    /// LN Navigation Control parameter (value 0 - 5)
    uint8_t nav_ctrl;
};


/// Location and Navigation Profile Sensor environment variable
struct lans_env_tag
{
    /// profile environment
    prf_env_t prf_env;

    /// Environment variable pointer for each connections
    struct lans_cnx_env env[BLE_CONNECTION_MAX];
    /// Position quality
    struct lanp_posq *p_posq;
    /// On-going operation
    struct lans_op *p_op_data;

    /// Feature Configuration Flags
    uint32_t features;

    /// Location and Navigation Service Start Handle
    uint16_t shdl;
    /// Profile Configuration Flags
    /// Every bit corresponds an attribute
    uint16_t prfl_cfg;
    /// Operation
    uint8_t operation;

    /// State of different task instances
    ke_state_t state[LANS_IDX_MAX];
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Retrieve LAN service profile interface
 *
 * @return LAN service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* lans_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send a LANS_CMP_EVT message to the application.
 * @param[in] conidx connection index
 * @param[in] src_id Source task
 * @param[in] dest_id Destination task
 * @param[in] operation Operation completed
 * @param[in] status status of the operation
 ****************************************************************************************
 */
void lans_send_cmp_evt(uint8_t conidx, uint8_t src_id, uint8_t dest_id, uint8_t operation, uint8_t status);

/**
 ****************************************************************************************
 * @brief Packs location and speed notifications
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[out] p_pckd_loc_speed pointer to packed message
 * @return status of the operation
 ****************************************************************************************
 */
uint8_t lans_pack_loc_speed_ntf(struct lanp_loc_speed *p_param, uint8_t *p_pckd_loc_speed);

/**
 ****************************************************************************************
 * @brief Packs navigation notifications
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[out] p_pckd_navigation pointer to packed message
 * @return length
 ****************************************************************************************
 */
uint8_t lans_pack_navigation_ntf(struct lanp_navigation *p_param, uint8_t *p_pckd_navigation);

/**
 ****************************************************************************************
 * @brief Splits notifications in order to be sent with default MTU
 * @param[in] conidx connection index
 * @param[in/out] p_loc_speed_ntf1 first notification
 * @return length
 ****************************************************************************************
 */
uint8_t lans_split_loc_speed_ntf(uint8_t conidx, struct gattc_send_evt_cmd *p_loc_speed_ntf1);

/**
 ****************************************************************************************
 * @brief updates the environment with the descriptor configuration and sends indication
 * @param[in] conidx connection index
 * @param[in] prfl_config requested value
 * @param[in] p_param pointer to the message parameters
 * @return status of the operation
 ****************************************************************************************
 */
uint8_t lans_update_characteristic_config(uint8_t conidx, uint8_t prfl_config,
        struct gattc_write_req_ind const *p_param);

/**
 ****************************************************************************************
 * @brief Packs position quality
 * @param[in] param Pointer to the parameters of the message.
 ****************************************************************************************
 */
void lans_upd_posq(struct lanp_posq param);

/**
 ****************************************************************************************
 * @brief Packs position quality
 * @param[out] p_pckd_posq pointer to packed message
 * @return length
 ****************************************************************************************
 */
uint8_t lans_pack_posq(uint8_t *p_pckd_posq);

/**
 ****************************************************************************************
 * @brief Packs control point
 * @param[in] conidx connection index
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[out] p_rsp pointer to message
 * @return status of the operation
 ****************************************************************************************
 */
uint8_t lans_pack_ln_ctnl_point_cfm (uint8_t conidx, struct lans_ln_ctnl_pt_cfm *p_param, uint8_t *p_rsp);

/**
 ****************************************************************************************
 * @brief Unpack control point and sends indication
 * @param[in] conidx connection index
 * @param[in] p_param pointer to message
 * @return status of the operation
 ****************************************************************************************
 */
uint8_t lans_unpack_ln_ctnl_point_ind (uint8_t conidx, struct gattc_write_req_ind const *p_param);

/**
 ****************************************************************************************
 * @brief  This function fully manage notification of navigation and speed information
 * @param[in] conidx connection index
 ****************************************************************************************
 */
void lans_exe_operation(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Send a control point response to the peer
 * @param[in] conidx connection index
 * @param[in] req_op_code operation code
 * @param[in] status status of the operation
 ****************************************************************************************
 */
void lans_send_rsp_ind(uint8_t conidx, uint8_t req_op_code, uint8_t status);

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
void lans_task_init(struct ke_task_desc *p_task_desc);

#endif //(BLE_LN_SENSOR)

/// @} LANS

#endif //(_LANS_H_)
