/**
 ****************************************************************************************
 *
 * @file pasp_common.h
 *
 * @brief Header File - Phone Alert Status Profile Common types.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


#ifndef _PASP_COMMON_H_
#define _PASP_COMMON_H_

/**
 ****************************************************************************************
 * @addtogroup PASP Phone Alert Status Profile
 * @ingroup PROFILE
 * @brief Phone Alert Status Profile
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "prf_utils.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Alert Status all bit enabled
#define PASP_ALERT_STATUS_VAL_MAX       (0x07)


/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/// Phone Alert Status Alert Status bit field
enum pasp_alert_status_bf
{
    /// Ringer State
    /// 0 not active and 1 active
    PASP_RINGER_STATE_POS = 0,
    PASP_RINGER_STATE_BIT = CO_BIT(PASP_RINGER_STATE_POS),

    /// Vibrate State
    /// 0 not active and 1 active
    PASP_VIBRATE_STATE_POS = 1,
    PASP_VIBRATE_STATE_BIT = CO_BIT(PASP_VIBRATE_STATE_POS),

    /// Display Alert State
    /// 0 not active and 1 active
    PASP_DISP_ALERT_STATE_POS = 2,
    PASP_DISP_ALERT_STATE_BIT = CO_BIT(PASP_DISP_ALERT_STATE_POS),
};

/// Phone Alert Status Ringer Setting value
enum pasp_ringer_setting_value
{
    /// Ringer Silent
    PASP_RINGER_SILENT = (0),

    /// Ringer Normal
    PASP_RINGER_NORMAL = (1),
};

/// Phone Alert Status Ringer Control Point value
enum pasp_ringer_ctnl_pt_value
{
    /// Set Silent Mode
    PASP_SILENT_MODE = (1),

    /// Mute Once
    PASP_MUTE_ONCE = (2),

    /// Cancel Silent Mode
    PASP_CANCEL_SILENT_MODE = (3),
};

/// @} pasp_common

#endif //(_HTP_COMMON_H_)
