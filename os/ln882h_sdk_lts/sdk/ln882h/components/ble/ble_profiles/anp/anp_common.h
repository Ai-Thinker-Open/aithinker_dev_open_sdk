/**
 ****************************************************************************************
 *
 * @file anp_common.h
 *
 * @brief Header File - Alert Notification Profile common types.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


#ifndef _ANP_COMMON_H_
#define _ANP_COMMON_H_

/**
 ****************************************************************************************
 * @addtogroup ANP Alert Notification Profile
 * @ingroup ANP
 * @brief Alert Notification Profile
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */


#include <stdint.h>
#include "prf_utils.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Command Not Supported - Protocol
#define ANP_CMD_NOT_SUPPORTED               (0xA0)
/// Category Not Supported - Proprietary
#define ANP_CAT_NOT_SUPPORTED               (0xA1)

/// New Alert Characteristic Value - Text String Information Max Length
#define ANS_NEW_ALERT_STRING_INFO_MAX_LEN   (18)
/// New Alert Characteristic Value Max Length
#define ANS_NEW_ALERT_MAX_LEN (ANS_NEW_ALERT_STRING_INFO_MAX_LEN + 2)
/// Bonded data configured
#define ANPS_FLAG_CFG_PERFORMED_OK          (0x10)
/// Alert Category ID 1 Mask
#define ANP_CAT_ID_1_MASK                   (ANP_CAT_ID_HIGH_PRTY_ALERT_BIT | ANP_CAT_ID_INST_MSG_BIT)
/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Category ID Field Keys
enum anp_category_id
{
    /// Simple Alert
    CAT_ID_SPL_ALERT                    = 0,
    /// Email
    CAT_ID_EMAIL,
    /// News Feed
    CAT_ID_NEWS,
    /// Incoming Call
    CAT_ID_CALL,
    /// Missed Call
    CAT_ID_MISSED_CALL,
    /// SMS/MMS
    CAT_ID_SMS_MMS,
    /// Voice Mail
    CAT_ID_VOICE_MAIL,
    /// Schedule
    CAT_ID_SCHEDULE,
    /// High Priority Alert
    CAT_ID_HIGH_PRTY_ALERT,
    /// Instant Message
    CAT_ID_INST_MSG,

    CAT_ID_NB,

    /// All supported category
    CAT_ID_ALL_SUPPORTED_CAT            = 255,
};

/// Command ID Field Keys
enum anp_cmd_id
{
    /// Enable New Incoming Alert Notification
    CMD_ID_EN_NEW_IN_ALERT_NTF          = 0,
    /// Enable Unread Category Status Notification
    CMD_ID_EN_UNREAD_CAT_STATUS_NTF,
    /// Disable New Incoming Alert Notification
    CMD_ID_DIS_NEW_IN_ALERT_NTF,
    /// Disable Unread Category Status Notification
    CMD_ID_DIS_UNREAD_CAT_STATUS_NTF,
    /// Notify New Incoming Alert immediately
    CMD_ID_NTF_NEW_IN_ALERT_IMM,
    /// Notify Unread Category Status immediately
    CMD_ID_NTF_UNREAD_CAT_STATUS_IMM,

    CMD_ID_NB,
};

/// Characteristic Codes
enum
{
    /// Supported New Alert Category Characteristic
    ANS_SUPP_NEW_ALERT_CAT_CHAR,
    /// New Alert Characteristic
    ANS_NEW_ALERT_CHAR,
    /// Supported Unread Alert Category Characteristic
    ANS_SUPP_UNREAD_ALERT_CAT_CHAR,
    /// Unread Alert Status Characteristic
    ANS_UNREAD_ALERT_STATUS_CHAR,
    /// Alert Notification Control Point Characteristic
    ANS_ALERT_NTF_CTNL_PT_CHAR,

    ANS_CHAR_MAX,
};

/// Alert codes
enum anp_alert_codes
{
    /// New Alert
    ANP_NEW_ALERT      = 0,
    /// Unread Alert
    ANP_UNREAD_ALERT,
};

/// Alert category bit field (@see struct anp_cat_id_bit_mask)
enum anp_alert_category_id_bf
{
    // Alert Category ID Bit Mask 0 Masks
    /// Simple Alert - The title of the alert
    ANP_CAT_ID_SPL_ALERT_SUP_POS            = 0,
    ANP_CAT_ID_SPL_ALERT_SUP_BIT            = CO_BIT(ANP_CAT_ID_SPL_ALERT_SUP_POS),

    /// Email - Sender name
    ANP_CAT_ID_EMAIL_SUP_POS                = 1,
    ANP_CAT_ID_EMAIL_SUP_BIT                = CO_BIT(ANP_CAT_ID_EMAIL_SUP_POS),

    /// News - Title of the news feed
    ANP_CAT_ID_NEWS_SUP_POS                 = 2,
    ANP_CAT_ID_NEWS_SUP_BIT                 = CO_BIT(ANP_CAT_ID_NEWS_SUP_POS),

    /// Call - Caller name or caller ID
    ANP_CAT_ID_CALL_SUP_POS                 = 3,
    ANP_CAT_ID_CALL_SUP_BIT                 = CO_BIT(ANP_CAT_ID_CALL_SUP_POS),

    /// Missed call - Caller name or caller ID
    ANP_CAT_ID_MISSED_CALL_SUP_POS          = 4,
    ANP_CAT_ID_MISSED_CALL_SUP_BIT          = CO_BIT(ANP_CAT_ID_MISSED_CALL_SUP_POS),

    /// SMS - Sender name or caller ID
    ANP_CAT_ID_SMS_MMS_SUP_POS              = 5,
    ANP_CAT_ID_SMS_MMS_SUP_BIT              = CO_BIT(ANP_CAT_ID_SMS_MMS_SUP_POS),

    /// Voice mail - Sender name or caller ID
    ANP_CAT_ID_VOICE_MAIL_SUP_POS           = 6,
    ANP_CAT_ID_VOICE_MAIL_SUP_BIT           = CO_BIT(ANP_CAT_ID_VOICE_MAIL_SUP_POS),

    /// Schedule - Title of the schedule
    ANP_CAT_ID_SCHEDULE_SUP_POS             = 7,
    ANP_CAT_ID_SCHEDULE_SUP_BIT             = CO_BIT(ANP_CAT_ID_SCHEDULE_SUP_POS),

    // Alert Category ID Bit Mask 1 Masks
    /// High Prioritized Alert = Title of the alert
    ANP_CAT_ID_HIGH_PRTY_ALERT_POS          = 0,
    ANP_CAT_ID_HIGH_PRTY_ALERT_BIT          = CO_BIT(ANP_CAT_ID_HIGH_PRTY_ALERT_POS),

    /// Instant Messaging - Sender name
    ANP_CAT_ID_INST_MSG_POS                 = 1,
    ANP_CAT_ID_INST_MSG_BIT                 = CO_BIT(ANP_CAT_ID_INST_MSG_POS),
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Alert Category ID Bit Mask Structure
struct anp_cat_id_bit_mask
{
    /// Category ID Bit Mask 0
    uint8_t cat_id_mask_0;
    /// Category ID Bit Mask 1
    uint8_t cat_id_mask_1;
};

/// New Alert Characteristic Value Structure
struct anp_new_alert
{
    /// Information String Length
    uint8_t info_str_len;

    /// Category ID
    uint8_t cat_id;
    /// Number of alerts
    uint8_t nb_new_alert;
    /// Text String Information
    uint8_t str_info[1];
};

/// Unread Alert Characteristic Value Structure
struct anp_unread_alert
{
    /// Category ID
    uint8_t cat_id;
    /// Number of alert
    uint8_t nb_unread_alert;
};

/// Alert Notification Control Point Characteristic Value Structure
struct anp_ctnl_pt
{
    /// Command ID
    uint8_t cmd_id;
    /// Category ID
    uint8_t cat_id;
};

/// @} anp_common

#endif //(_ANP_COMMON_H_)
