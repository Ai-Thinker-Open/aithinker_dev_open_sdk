/**
 ****************************************************************************************
 *
 * @file bcsc_task.h
 *
 * @brief Header file - Body Composition Service Collector/Client Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _BCSC_TASK_H_
#define _BCSC_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup BCSC  Body Composition Service Collector Task
 * @ingroup BCSC
 * @brief  Body Composition Service Collector
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "prf_types.h"
#include "bcs_common.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs
/*@TRACE*/
enum bcsc_msg_ids
{
    /// Enable the Profile Collector task - at connection
    BCSC_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_BCSC),
    /// Response to Enable the Profile Collector task - at connection
    BCSC_ENABLE_RSP,
    /// Read the Body Composition Feature
    BCSC_RD_FEAT_CMD,
    /// Read the CCC of a measurement sensor characteristic
    BCSC_RD_MEAS_CCC_CMD,
    /// Write the CCC of a measurement sensor characteristic
    BCSC_WR_MEAS_CCC_CMD,
    /// Read the Body Composition Feature
    BCSC_FEAT_IND,
    /// Read the CCC of a measurement sensor characteristic
    BCSC_MEAS_CCC_IND,
    /// Characteristic Measurement Indication from peer
    BCSC_MEAS_IND,
    /// Complete Event Information
    BCSC_CMP_EVT,
};

/// Body Composition Service Characteristics
enum bcsc_chars
{
    /// Body Composition Feature
    BCSC_CHAR_BCS_FEATURE,
    /// Body Composition Measurement
    BCSC_CHAR_BCS_MEAS,

    BCSC_CHAR_BCS_MAX,
};

/// Body Composition Service Characteristic Descriptors
enum bcsc_descs
{
    /// Client config
    BCSC_DESC_BCS_MEAS_CCC,

    BCSC_DESC_BCS_MAX,
};

/*
 * API MESSAGE STRUCTURES
 ****************************************************************************************
 */

/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the Body Composition Service
 */
struct bcsc_bcs_content
{
    /// service info
    struct prf_svc svc;

    /// Characteristic info:
    ///  - Feature
    ///  - Measurement
    struct prf_char_inf chars[BCSC_CHAR_BCS_MAX];

    /// Descriptor handles:
    ///  - Client cfg
    struct prf_char_desc_inf descs[BCSC_DESC_BCS_MAX];
};

/// Parameters of the @ref BCSC_ENABLE_REQ message
struct bcsc_enable_req
{
    /// Connection type
    uint8_t con_type;

    /// Primary or Secondary Service
    uint8_t svc_type;

    /// If Secondary Service - the Start and End Handle for the service are contained
    /// in the bcs.

    /// Existing handle values of BCS
    struct bcsc_bcs_content bcs;

};

/// Parameters of the @ref BCSC_ENABLE_RSP message
struct bcsc_enable_rsp
{
    /// status
    uint8_t status;
    ///  handle values of BCS
    struct bcsc_bcs_content bcs;
};

/// Parameters of the @ref BCSC_RD_FEAT_CMD message
/// Read the Body Composition Feature
struct bcsc_rd_feat_cmd
{
    /// NULL
};

/// Parameters of the @ref BCSC_RD_MEAS_CCC_CMD message
/// Read the CCC of a measurement sensor characteristic
struct bcsc_rd_meas_ccc_cmd
{
    /// NULL
};

/// Parameters of the @ref BCSC_WR_MEAS_CCC_CMD message
/// Write the CCC of a measurement sensor characteristic
struct bcsc_wr_meas_ccc_cmd
{
    /// Client Characteristic Configuration written value
    uint16_t ccc;
};

/// Parameters of the @ref BCSC_FEAT_IND message
/// Read the Body Composition Feature
struct bcsc_feat_ind
{
    /// Body Composition Feature read value
    uint32_t feature;
};

/// Parameters of the @ref BCSC_MEAS_CCC_IND message
/// Read the CCC of a measurement sensor characteristic
struct bcsc_meas_ccc_ind
{
    /// Client Characteristic Configuration read value
    uint16_t ccc;
};

/// Parameters of the @ref BCSC_MEAS_IND message
/// Characteristic Measurement Indication from peer
struct bcsc_meas_ind
{
    /// Presence of optional fields and measurement units
    uint16_t flags;
    /// Body fat percentage value
    uint16_t body_fat_percent;
    /// Measurement Units
    uint8_t meas_u;
    /// Time stamp value
    struct prf_date_time time_stamp;
    /// User ID value
    uint8_t user_id;
    /// Basal Metabolism value
    uint16_t basal_metab;
    /// Muscle Percentage value
    uint16_t muscle_percent;
    /// Muscle Mass value
    uint16_t muscle_mass;
    /// Fat Free Mass value
    uint16_t fat_free_mass;
    /// Soft Lean Mass value
    uint16_t soft_lean_mass;
    /// Body Water Mass value
    uint16_t body_water_mass;
    /// Impedance Value
    uint16_t impedance;
    /// Weight value
    uint16_t weight;
    /// Mass Resolution
    uint8_t mass_resol;
    /// Height value
    uint16_t height;
    /// Height Resolution
    uint8_t hght_resol;
};

/// Parameters of the @ref BCSC_CMP_EVT message
struct bcsc_cmp_evt
{
    /// Operation code
    uint8_t operation;
    /// Status
    uint8_t status;
};

/// @} BCSC TASK

#endif //(_BCSC_TASK_H_)
