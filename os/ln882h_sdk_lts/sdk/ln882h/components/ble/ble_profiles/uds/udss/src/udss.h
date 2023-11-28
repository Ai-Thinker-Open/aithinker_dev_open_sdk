/**
 ****************************************************************************************
 *
 * @file udss.h
 *
 * @brief Header file - User Data Service Profile Sensor.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _UDSS_H_
#define _UDSS_H_

/**
 ****************************************************************************************
 * @addtogroup ENVS User Data Service Profile Sensor
 * @ingroup ENVS
 * @brief User Data Service Profile Sensor
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_UDS_SERVER)
#include "uds_common.h"
#include "udss_task.h"

#include "prf_types.h"
#include "prf.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of User Data Service Profile role task instances
#define UDSS_IDX_MAX        (BLE_CONNECTION_MAX)

/// used to calculate bitmask for attribute database
#define BITS_IN_BYTE        (8)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
 /// Possible states of the TIPS task
enum
{
    /// not connected state
    UDSS_STATE_FREE,
    /// idle state
    UDSS_STATE_IDLE,
    /// read ind 
    UDSS_STATE_OP_READ,
    /// write ind 
    UDSS_STATE_OP_WRITE,
    /// indicate 
    UDSS_STATE_OP_INDICATE,
    /// Notify 
    UDSS_STATE_OP_NOTIFY,

    /// Number of defined states.
    UDSS_STATE_MAX
};

/// User Data Service - Attribute List
enum udss_desc_idx
{
    /// User Data Service
    UDSS_IDX_SVC = 0,
    /// DataBase Change Increment
    UDSS_IDX_DB_CHG_INC_CHAR,
    /// value
    UDSS_IDX_DB_CHG_INC_VAL,
    /// CCC descriptor
    UDSS_IDX_DB_CHG_INC_DESC_CCC,
    /// User Index
    UDSS_IDX_USER_INDEX_CHAR,
    /// value
    UDSS_IDX_USER_INDEX_VAL,
    /// User Control Point
    UDSS_IDX_USER_CTRL_PT_CHAR,
    /// value
    UDSS_IDX_USER_CTRL_PT_VAL,
    /// CCC descriptor
    UDSS_IDX_USER_CTRL_PT_DESC_CCC,

    /// Strings: utf8s
    UDS_IDX_FIRST_NAME_CHAR,
    UDS_IDX_FIRST_NAME_VAL,
    UDS_IDX_FIRST_NAME_EXT,
    UDS_IDX_LAST_NAME_CHAR,
    UDS_IDX_LAST_NAME_VAL,
    UDS_IDX_LAST_NAME_EXT,
    UDS_IDX_EMAIL_ADDRESS_CHAR,
    UDS_IDX_EMAIL_ADDRESS_VAL,
    UDS_IDX_EMAIL_ADDRESS_EXT,
    UDS_IDX_LANGUAGE_CHAR,
    UDS_IDX_LANGUAGE_VAL,
    UDS_IDX_LANGUAGE_EXT,
    /// Date
    UDS_IDX_DATE_OF_BIRTH_CHAR,
    UDS_IDX_DATE_OF_BIRTH_VAL,
    UDS_IDX_DATE_OF_THRESHOLD_ASSESSMENT_CHAR,
    UDS_IDX_DATE_OF_THRESHOLD_ASSESSMENT_VAL,
    /// uint16
    UDS_IDX_WEIGHT_CHAR,
    UDS_IDX_WEIGHT_VAL,
    UDS_IDX_HEIGHT_CHAR,
    UDS_IDX_HEIGHT_VAL,
    UDS_IDX_WAIST_CIRCUMFERENCE_CHAR,
    UDS_IDX_WAIST_CIRCUMFERENCE_VAL,
    UDS_IDX_HIP_CIRCUMFERENCE_CHAR,
    UDS_IDX_HIP_CIRCUMFERENCE_VAL,
    /// uint8
    UDS_IDX_AGE_CHAR,
    UDS_IDX_AGE_VAL,
    UDS_IDX_GENDER_CHAR,
    UDS_IDX_GENDER_VAL,
    UDS_IDX_VO2_MAX_CHAR,
    UDS_IDX_VO2_MAX_VAL,
    UDS_IDX_MAX_HEART_RATE_CHAR,
    UDS_IDX_MAX_HEART_RATE_VAL,
    UDS_IDX_RESTING_HEART_RATE_CHAR,
    UDS_IDX_RESTING_HEART_RATE_VAL,
    UDS_IDX_MAXIMUM_RECOMMENDED_HEART_RATE_CHAR,
    UDS_IDX_MAXIMUM_RECOMMENDED_HEART_RATE_VAL,
    UDS_IDX_AEROBIC_THRESHHOLD_CHAR,
    UDS_IDX_AEROBIC_THRESHHOLD_VAL,
    UDS_IDX_ANAEROBIC_THRESHHOLD_CHAR,
    UDS_IDX_ANAEROBIC_THRESHHOLD_VAL,
    UDS_IDX_SPORT_TYPE_FOR_AEROBIC_AND_ANAEROBIC_THRESHOLDS_CHAR,
    UDS_IDX_SPORT_TYPE_FOR_AEROBIC_AND_ANAEROBIC_THRESHOLDS_VAL,
    UDS_IDX_FAT_BURN_HEART_RATE_LOWER_LIMIT_CHAR,
    UDS_IDX_FAT_BURN_HEART_RATE_LOWER_LIMIT_VAL,
    UDS_IDX_FAT_BURN_HEART_RATE_UPPER_LIMIT_CHAR,
    UDS_IDX_FAT_BURN_HEART_RATE_UPPER_LIMIT_VAL,
    UDS_IDX_AEROBIC_HEART_RATE_LOWER_LIMIT_CHAR,
    UDS_IDX_AEROBIC_HEART_RATE_LOWER_LIMIT_VAL,
    UDS_IDX_AEROBIC_HEART_RATE_UPPER_LIMIT_CHAR,
    UDS_IDX_AEROBIC_HEART_RATE_UPPER_LIMIT_VAL,
    UDS_IDX_ANAEROBIC_HEART_RATE_LOWER_LIMIT_CHAR,
    UDS_IDX_ANAEROBIC_HEART_RATE_LOWER_LIMIT_VAL,
    UDS_IDX_ANAEROBIC_HEART_RATE_UPPER_LIMIT_CHAR,
    UDS_IDX_ANAEROBIC_HEART_RATE_UPPER_LIMIT_VAL,
    UDS_IDX_TWO_ZONE_HEART_RATE_LIMIT_CHAR,
    UDS_IDX_TWO_ZONE_HEART_RATE_LIMIT_VAL,
    /// set
    UDS_IDX_FIVE_ZONE_HEART_RATE_LIMITS_CHAR,
    UDS_IDX_FIVE_ZONE_HEART_RATE_LIMITS_VAL,
    UDS_IDX_THREE_ZONE_HEART_RATE_LIMITS_CHAR,
    UDS_IDX_THREE_ZONE_HEART_RATE_LIMITS_VAL,

#ifdef USE_EXTRA_CHARS
    UDS_IDX_USER_DEFINED_1_CHAR,
    UDS_IDX_USER_DEFINED_1_VAL,
    UDS_IDX_USER_DEFINED_1_EXT,
    UDS_IDX_USER_DEFINED_2_CHAR,
    UDS_IDX_USER_DEFINED_2_VAL,
    UDS_IDX_USER_DEFINED_2_EXT,
#endif //def USE_EXTRA_CHARS

    UDSS_IDX_ATT_NB
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// on-going operation
struct udss_operation
{
    /// Handle impacted by operation
    uint16_t handle;
};

/// User Data Service Profile environment variable
struct udss_env_tag
{
    /// profile environment
    prf_env_t prf_env;

    /// User Data Service Service Start Handle
    uint16_t shdl;

    /// On-going operation (requested by peer device)
    struct udss_operation op[BLE_CONNECTION_MAX]; 

    /// State of different task instances
    ke_state_t state[UDSS_IDX_MAX];

    /// Database configuration - Keeps enabled User Data Characteristics
    uint32_t char_mask;
    /// CCC
    uint16_t db_chg_inc_ccc[BLE_CONNECTION_MAX];
    uint16_t user_ctrl_pt_ccc[BLE_CONNECTION_MAX];
};

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief convert Characteristic type to 'handle'
 *
 * @param[in]  char_id       Characteristic type index see @enum uds_char_id.
 * @param[out] p_handle_raw  Handle from the GATT request.
 *
 * @return ATT_ERR_INVALID_HANDLE if this characteristic is not present.
 ****************************************************************************************
 */
uint8_t udss_char_2_handle(uint16_t *p_handle_raw, uint8_t char_id);
 
/**
 ****************************************************************************************
 * @brief Converts Handle into Characteristic type and instance.
 *
 * @param[in]  handle_raw  Handle from the GATT request.
 *
 * @return Characteristic type index see @enum uds_char_id.
 ****************************************************************************************
 */
uint8_t udss_handle_2_char(uint16_t handle_raw);

/**
 ****************************************************************************************
 * @brief Pack Characteristic value depending on Characteristic type
 *
 * @param[in]         char_id - Characteristic Type.
 * @param[in]         p_value  - Structure that keep Characteristic vales.
 * @param[out]        p_data   - Output buffer for packed data.
 * @param[out]        p_plength - Length of the encoded data.
 *
 * @return status of the operation.
 ****************************************************************************************
 */
uint8_t udss_pack_char_value(uint8_t char_id, union char_val *p_val, uint8_t *p_data, uint16_t *p_length);
 
/**
 ****************************************************************************************
 * @brief Retrieve ENVS service profile interface
 *
 * @return ENVS service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* udss_prf_itf_get(void);

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
void udss_task_init(struct ke_task_desc *p_task_desc);

#endif //(BLE_UDS_SERVER)

/// @} ENVS

#endif //(_UDSS_H_)
