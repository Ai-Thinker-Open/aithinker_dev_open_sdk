/**
 ****************************************************************************************
 *
 * @file lanp_common.h
 *
 * @brief Header File - Location and Navigation Profile common types.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


#ifndef _LANP_COMMON_H_
#define _LANP_COMMON_H_

/**
 ****************************************************************************************
 * @addtogroup LANP Location and Navigation Profile
 * @ingroup PROFILE
 * @brief Location and Navigation Profile
 *
 * The Location and Navigation Profile is used to enable a collector device in order to obtain
 * data from a Location and Navigation Sensor (LAN Sensor) that exposes the Location and Navigation Service.
 *
 * This file contains all definitions that are common for the server and the client parts
 * of the profile.
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "co_math.h"
#include "prf_types.h"
#include <stdint.h>

/*
 * DEFINES
 ****************************************************************************************
 */
/// Inappropriate Connection Parameters Error Code
#define LAN_ERROR_INAPP_CONNECT_PARAMS  (0x80)
/// Procedure Already in Progress Error Code
#define LAN_ERROR_PROC_IN_PROGRESS      (0xFE)

#define LANP_LAN_LSPEED_MAX_PAIR_SIZE           (8)

/// LAN Location and Speed Value Max Length
#define LANP_LAN_LOC_SPEED_MAX_LEN              (28)
/// LAN Location and Speed Value Min Length
#define LANP_LAN_LOC_SPEED_MIN_LEN              (2)

/// LAN Position Quality Value Max Length
#define LANP_LAN_POSQ_MAX_LEN                   (16)
/// LAN Position Quality Value Min Length
#define LANP_LAN_POSQ_MIN_LEN                   (2)

/// LAN Control Point Value Request Max Length
#define LANP_LAN_LN_CNTL_PT_REQ_MAX_LEN         (5)
/// LAN Control Point Value Request Min Length
#define LANP_LAN_LN_CNTL_PT_REQ_MIN_LEN         (1)

/// LAN Control Point data Max Length (from 2 to max route name)
#define LANP_LAN_LN_CNTL_DATA_MAX_LEN           (128)
/// LAN Control Point Value Response Max Length
#define LANP_LAN_LN_CNTL_PT_RSP_MAX_LEN         (3 + LANP_LAN_LN_CNTL_DATA_MAX_LEN)
/// LAN Control Point Value Response Min Length
#define LANP_LAN_LN_CNTL_PT_RSP_MIN_LEN         (3)

/// LAN Navigation Value Max Length
#define LANP_LAN_NAVIGATION_MAX_LEN             (21)
/// LAN Navigation Value Min Length
#define LANP_LAN_NAVIGATION_MIN_LEN             (6)

/// All Supported
#define LANP_FEAT_ALL_SUPP                      (0x001FFFFF)

/// All Present
#define LANP_LSPEED_ALL_PRESENT                 (0x1FFF)

/// All Present
#define LANP_POSQ_ALL_PRESENT                   (0x01FF)

/// ALl present
#define LANP_NAVI_ALL_PRESENT                   (0x01FF)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// LANP Service Characteristics
enum lanp_lans_char
{
    /// LN Feature
    LANP_LANS_LN_FEAT_CHAR,
    /// Location and speed
    LANP_LANS_LOC_SPEED_CHAR,
    /// Position quality
    LANP_LANS_POS_Q_CHAR,
    /// LN Control Point
    LANP_LANS_LN_CTNL_PT_CHAR,
    /// Navigation
    LANP_LANS_NAVIG_CHAR,

    LANP_LANS_CHAR_MAX
};

/// LANP Feature Flags
enum lanp_feat_flags
{
    /// Instantaneous Speed Supported
    LANP_FEAT_INSTANTANEOUS_SPEED_SUP_POS           = 0,
    LANP_FEAT_INSTANTANEOUS_SPEED_SUP_BIT           = CO_BIT(LANP_FEAT_INSTANTANEOUS_SPEED_SUP_POS),

    /// Total Distance Supported
    LANP_FEAT_TOTAL_DISTANCE_SUP_POS                = 1,
    LANP_FEAT_TOTAL_DISTANCE_SUP_BIT                = CO_BIT(LANP_FEAT_TOTAL_DISTANCE_SUP_POS),

    /// Location Supported
    LANP_FEAT_LOCATION_SUP_POS                      = 2,
    LANP_FEAT_LOCATION_SUP_BIT                      = CO_BIT(LANP_FEAT_LOCATION_SUP_POS),

    /// Elevation Supported
    LANP_FEAT_ELEVATION_SUP_POS                     = 3,
    LANP_FEAT_ELEVATION_SUP_BIT                     = CO_BIT(LANP_FEAT_ELEVATION_SUP_POS),

    /// Heading Supported
    LANP_FEAT_HEADING_SUP_POS                       = 4,
    LANP_FEAT_HEADING_SUP_BIT                       = CO_BIT(LANP_FEAT_HEADING_SUP_POS),

    /// Rolling Time Supported
    LANP_FEAT_ROLLING_TIME_SUP_POS                  = 5,
    LANP_FEAT_ROLLING_TIME_SUP_BIT                  = CO_BIT(LANP_FEAT_ROLLING_TIME_SUP_POS),

    /// UTC Time Supported
    LANP_FEAT_UTC_TIME_SUP_POS                      = 6,
    LANP_FEAT_UTC_TIME_SUP_BIT                      = CO_BIT(LANP_FEAT_UTC_TIME_SUP_POS),

    /// Remaining Distance Supported
    LANP_FEAT_REMAINING_DISTANCE_SUP_POS            = 7,
    LANP_FEAT_REMAINING_DISTANCE_SUP_BIT            = CO_BIT(LANP_FEAT_REMAINING_DISTANCE_SUP_POS),

    /// Remaining Vertical Distance Supported
    LANP_FEAT_REMAINING_VERTICAL_DISTANCE_SUP_POS   = 8,
    LANP_FEAT_REMAINING_VERTICAL_DISTANCE_SUP_BIT   = CO_BIT(LANP_FEAT_REMAINING_VERTICAL_DISTANCE_SUP_POS),

    /// Estimated Time of Arrival Supported
    LANP_FEAT_ESTIMATED_TIME_OF_ARRIVAL_SUP_POS     = 9,
    LANP_FEAT_ESTIMATED_TIME_OF_ARRIVAL_SUP_BIT     = CO_BIT(LANP_FEAT_ESTIMATED_TIME_OF_ARRIVAL_SUP_POS),

    /// Number of Beacons in Solution Supported
    LANP_FEAT_NUMBER_OF_BEACONS_IN_SOLUTION_SUP_POS = 10,
    LANP_FEAT_NUMBER_OF_BEACONS_IN_SOLUTION_SUP_BIT = CO_BIT(LANP_FEAT_NUMBER_OF_BEACONS_IN_SOLUTION_SUP_POS),

    /// Number of Beacons in View Supported
    LANP_FEAT_NUMBER_OF_BEACONS_IN_VIEW_SUP_POS     = 11,
    LANP_FEAT_NUMBER_OF_BEACONS_IN_VIEW_SUP_BIT     = CO_BIT(LANP_FEAT_NUMBER_OF_BEACONS_IN_VIEW_SUP_POS),

    /// Time to First Fix Supported
    LANP_FEAT_TIME_TO_FIRST_FIX_SUP_POS             = 12,
    LANP_FEAT_TIME_TO_FIRST_FIX_SUP_BIT             = CO_BIT(LANP_FEAT_TIME_TO_FIRST_FIX_SUP_POS),

    /// Estimated Horizontal Position Error Supported
    LANP_FEAT_ESTIMATED_HOR_POSITION_ERROR_SUP_POS  = 13,
    LANP_FEAT_ESTIMATED_HOR_POSITION_ERROR_SUP_BIT  = CO_BIT(LANP_FEAT_ESTIMATED_HOR_POSITION_ERROR_SUP_POS),

    /// Estimated Vertical Position Error Supported
    LANP_FEAT_ESTIMATED_VER_POSITION_ERROR_SUP_POS  = 14,
    LANP_FEAT_ESTIMATED_VER_POSITION_ERROR_SUP_BIT  = CO_BIT(LANP_FEAT_ESTIMATED_VER_POSITION_ERROR_SUP_POS),

    /// Horizontal Dilution of Precision Supported
    LANP_FEAT_HOR_DILUTION_OF_PRECISION_SUP_POS     = 15,
    LANP_FEAT_HOR_DILUTION_OF_PRECISION_SUP_BIT     = CO_BIT(LANP_FEAT_HOR_DILUTION_OF_PRECISION_SUP_POS),

    /// Vertical Dilution of Precision Supported
    LANP_FEAT_VER_DILUTION_OF_PRECISION_SUP_POS     = 16,
    LANP_FEAT_VER_DILUTION_OF_PRECISION_SUP_BIT     = CO_BIT(LANP_FEAT_VER_DILUTION_OF_PRECISION_SUP_POS),

    /// Location and Speed Characteristic Content Masking Supported
    LANP_FEAT_LSPEED_CHAR_CT_MASKING_SUP_POS        = 17,
    LANP_FEAT_LSPEED_CHAR_CT_MASKING_SUP_BIT        = CO_BIT(LANP_FEAT_LSPEED_CHAR_CT_MASKING_SUP_POS),

    /// Fix Rate Setting Supported
    LANP_FEAT_FIX_RATE_SETTING_SUP_POS              = 18,
    LANP_FEAT_FIX_RATE_SETTING_SUP_BIT              = CO_BIT(LANP_FEAT_FIX_RATE_SETTING_SUP_POS),

    /// Elevation Setting Supported
    LANP_FEAT_ELEVATION_SETTING_SUP_POS             = 19,
    LANP_FEAT_ELEVATION_SETTING_SUP_BIT             = CO_BIT(LANP_FEAT_ELEVATION_SETTING_SUP_POS),

    /// Position Status Supported
    LANP_FEAT_POSITION_STATUS_SUP_POS               = 20,
    LANP_FEAT_POSITION_STATUS_SUP_BIT               = CO_BIT(LANP_FEAT_POSITION_STATUS_SUP_POS),

    // Bit 21 - 31 RFU
};

/// LANP Location and speed Flags
enum lanp_lspeed_flags
{
    /// Instantaneous Speed Present
    LANP_LSPEED_INST_SPEED_PRESENT_POS          = 0,
    LANP_LSPEED_INST_SPEED_PRESENT_BIT          = CO_BIT(LANP_LSPEED_INST_SPEED_PRESENT_POS),

    /// Total Distance Present
    LANP_LSPEED_TOTAL_DISTANCE_PRESENT_POS      = 1,
    LANP_LSPEED_TOTAL_DISTANCE_PRESENT_BIT      = CO_BIT(LANP_LSPEED_TOTAL_DISTANCE_PRESENT_POS),

    /// Location Present
    LANP_LSPEED_LOCATION_PRESENT_POS            = 2,
    LANP_LSPEED_LOCATION_PRESENT_BIT            = CO_BIT(LANP_LSPEED_LOCATION_PRESENT_POS),

    /// Elevation Present
    LANP_LSPEED_ELEVATION_PRESENT_POS           = 3,
    LANP_LSPEED_ELEVATION_PRESENT_BIT           = CO_BIT(LANP_LSPEED_ELEVATION_PRESENT_POS),

    /// Heading Present
    LANP_LSPEED_HEADING_PRESENT_POS             = 4,
    LANP_LSPEED_HEADING_PRESENT_BIT             = CO_BIT(LANP_LSPEED_HEADING_PRESENT_POS),

    /// Rolling Time Present
    LANP_LSPEED_ROLLING_TIME_PRESENT_POS        = 5,
    LANP_LSPEED_ROLLING_TIME_PRESENT_BIT        = CO_BIT(LANP_LSPEED_ROLLING_TIME_PRESENT_POS),

    /// UTC Time Present
    LANP_LSPEED_UTC_TIME_PRESENT_POS            = 6,
    LANP_LSPEED_UTC_TIME_PRESENT_BIT            = CO_BIT(LANP_LSPEED_UTC_TIME_PRESENT_POS),

    /// Position Status LSB
    LANP_LSPEED_POSITION_STATUS_LSB_POS         = 7,
    LANP_LSPEED_POSITION_STATUS_LSB_BIT         = CO_BIT(LANP_LSPEED_POSITION_STATUS_LSB_POS),

    /// Position Status MSB
    LANP_LSPEED_POSITION_STATUS_MSB_POS         = 8,
    LANP_LSPEED_POSITION_STATUS_MSB_BIT         = CO_BIT(LANP_LSPEED_POSITION_STATUS_MSB_POS),

    /// Speed and Distance format
    LANP_LSPEED_SPEED_AND_DISTANCE_FORMAT_POS   = 9,
    LANP_LSPEED_SPEED_AND_DISTANCE_FORMAT_BIT   = CO_BIT(LANP_LSPEED_SPEED_AND_DISTANCE_FORMAT_POS),

    /// Elevation Source LSB
    LANP_LSPEED_ELEVATION_SOURCE_LSB_POS        = 10,
    LANP_LSPEED_ELEVATION_SOURCE_LSB_BIT        = CO_BIT(LANP_LSPEED_ELEVATION_SOURCE_LSB_POS),

    /// Elevation Source MSB
    LANP_LSPEED_ELEVATION_SOURCE_MSB_POS        = 11,
    LANP_LSPEED_ELEVATION_SOURCE_MSB_BIT        = CO_BIT(LANP_LSPEED_ELEVATION_SOURCE_MSB_POS),

    /// Heading Source
    LANP_LSPEED_HEADING_SOURCE_POS              = 12,
    LANP_LSPEED_HEADING_SOURCE_BIT              = CO_BIT(LANP_LSPEED_HEADING_SOURCE_POS),

    // Bit 13 - 15 RFU
};

/// LANP Position quality Flags
enum lanp_posq_flags
{
    /// Number of Beacons in Solution Present
    LANP_POSQ_NUMBER_OF_BEACONS_IN_SOLUTION_PRESENT_POS = 0,
    LANP_POSQ_NUMBER_OF_BEACONS_IN_SOLUTION_PRESENT_BIT = CO_BIT(LANP_POSQ_NUMBER_OF_BEACONS_IN_SOLUTION_PRESENT_POS),

    /// Number of Beacons in View Present
    LANP_POSQ_NUMBER_OF_BEACONS_IN_VIEW_PRESENT_POS     = 1,
    LANP_POSQ_NUMBER_OF_BEACONS_IN_VIEW_PRESENT_BIT     = CO_BIT(LANP_POSQ_NUMBER_OF_BEACONS_IN_VIEW_PRESENT_POS),

    /// Time to First Fix Present
    LANP_POSQ_TIME_TO_FIRST_FIX_PRESENT_POS             = 2,
    LANP_POSQ_TIME_TO_FIRST_FIX_PRESENT_BIT             = CO_BIT(LANP_POSQ_TIME_TO_FIRST_FIX_PRESENT_POS),

    /// EHPE Present
    LANP_POSQ_EHPE_PRESENT_POS                          = 3,
    LANP_POSQ_EHPE_PRESENT_BIT                          = CO_BIT(LANP_POSQ_EHPE_PRESENT_POS),

    /// EVPE Present
    LANP_POSQ_EVPE_PRESENT_POS                          = 4,
    LANP_POSQ_EVPE_PRESENT_BIT                          = CO_BIT(LANP_POSQ_EVPE_PRESENT_POS),

    /// HDOP Present
    LANP_POSQ_HDOP_PRESENT_POS                          = 5,
    LANP_POSQ_HDOP_PRESENT_BIT                          = CO_BIT(LANP_POSQ_HDOP_PRESENT_POS),

    /// VDOP Present
    LANP_POSQ_VDOP_PRESENT_POS                          = 6,
    LANP_POSQ_VDOP_PRESENT_BIT                          = CO_BIT(LANP_POSQ_VDOP_PRESENT_POS),

    /// Position Status LSB
    LANP_POSQ_POSITION_STATUS_LSB_POS                   = 7,
    LANP_POSQ_POSITION_STATUS_LSB_BIT                   = CO_BIT(LANP_POSQ_POSITION_STATUS_LSB_POS),

    /// Position Status MSB
    LANP_POSQ_POSITION_STATUS_MSB_POS                   = 8,
    LANP_POSQ_POSITION_STATUS_MSB_BIT                   = CO_BIT(LANP_POSQ_POSITION_STATUS_MSB_POS),

    // Bit 9 - 15 RFU
};

/// LANP Control Point Keys
enum lanp_ln_ctnl_pt_code
{
    /// Reserved value
    LANP_LN_CTNL_PT_RESERVED                = 0,

    /// Set Cumulative Value
    LANP_LN_CTNL_PT_SET_CUMUL_VALUE         = 1,
    /// Mask Location and Speed Characteristic Content
    LANP_LN_CTNL_PT_MASK_LSPEED_CHAR_CT     = 2,
    /// Navigation Control
    LANP_LN_CTNL_PT_NAVIGATION_CONTROL      = 3,
    /// Request Number of Routes
    LANP_LN_CTNL_PT_REQ_NUMBER_OF_ROUTES    = 4,
    /// Request Name of Route
    LANP_LN_CTNL_PT_REQ_NAME_OF_ROUTE       = 5,
    /// Select Route
    LANP_LN_CTNL_PT_SELECT_ROUTE            = 6,
    /// Set Fix Rate
    LANP_LN_CTNL_PT_SET_FIX_RATE            = 7,
    /// Set Elevation
    LANP_LN_CTNL_PT_SET_ELEVATION           = 8,
    /// Response Code
    LANP_LN_CTNL_PT_RESPONSE_CODE           = 32
};

/// LANP Control Point Response Value
enum lanp_ctnl_pt_resp_val
{
    /// Reserved value
    LANP_LN_CTNL_PT_RESP_RESERVED      = 0,

    /// Success
    LANP_LN_CTNL_PT_RESP_SUCCESS       = 1,
    /// Operation Code Not Supported
    LANP_LN_CTNL_PT_RESP_NOT_SUPP      = 2,
    /// Invalid Parameter
    LANP_LN_CTNL_PT_RESP_INV_PARAM     = 3,
    /// Operation Failed
    LANP_LN_CTNL_PT_RESP_FAILED        = 4,
};

/// LANP Navigation Control parameter
enum lanp_navi_ctrl_val
{
    /// Stop Navigation
    LANP_LN_CTNL_STOP_NAVI      = 0x00,
    /// Start Navigation
    LANP_LN_CTNL_START_NAVI     = 0x01,
    /// Pause Navigation
    LANP_LN_CTNL_PAUSE_NAVI     = 0x02,
    /// Resume Navigation
    LANP_LN_CTNL_RESUME_NAVI    = 0x03,
    /// Skip waypoint on route
    LANP_LN_CTNL_SKIP_WPT       = 0x04,
    /// Start Navigation from the nearest waypoint
    LANP_LN_CTNL_START_NST_WPT  = 0x05,

    // 0x06 - 0xFF RFU
};

/// LANP Navigation flags
enum lanp_navi_flags
{
    /// Remaining Distance Present
    LANP_NAVI_REMAINING_DIS_PRESENT_POS             = 0,
    LANP_NAVI_REMAINING_DIS_PRESENT_BIT             = CO_BIT(LANP_NAVI_REMAINING_DIS_PRESENT_POS),

    /// Remaining Vertical Distance Present
    LANP_NAVI_REMAINING_VER_DIS_PRESENT_POS         = 1,
    LANP_NAVI_REMAINING_VER_DIS_PRESENT_BIT         = CO_BIT(LANP_NAVI_REMAINING_VER_DIS_PRESENT_POS),

    /// Estimated Time of Arrival Present
    LANP_NAVI_ESTIMATED_TIME_OF_ARRIVAL_PRESENT_POS = 2,
    LANP_NAVI_ESTIMATED_TIME_OF_ARRIVAL_PRESENT_BIT = CO_BIT(LANP_NAVI_ESTIMATED_TIME_OF_ARRIVAL_PRESENT_POS),

    /// Position Status lsb
    LANP_NAVI_POSITION_STATUS_LSB_POS               = 3,
    LANP_NAVI_POSITION_STATUS_LSB_BIT               = CO_BIT(LANP_NAVI_POSITION_STATUS_LSB_POS),

    /// Position Status msb
    LANP_NAVI_POSITION_STATUS_MSB_POS               = 4,
    LANP_NAVI_POSITION_STATUS_MSB_BIT               = CO_BIT(LANP_NAVI_POSITION_STATUS_MSB_POS),

    /// Heading Source
    LANP_NAVI_HEADING_SOURCE_POS                    = 5,
    LANP_NAVI_HEADING_SOURCE_BIT                    = CO_BIT(LANP_NAVI_HEADING_SOURCE_POS),

    /// Navigation Indicator Type
    LANP_NAVI_NAVIGATION_INDICATOR_TYPE_POS         = 6,
    LANP_NAVI_NAVIGATION_INDICATOR_TYPE_BIT         = CO_BIT(LANP_NAVI_NAVIGATION_INDICATOR_TYPE_POS),

    /// Waypoint Reached
    LANP_NAVI_WAYPOINT_REACHED_POS                  = 7,
    LANP_NAVI_WAYPOINT_REACHED_BIT                  = CO_BIT(LANP_NAVI_WAYPOINT_REACHED_POS),

    /// Destination Reached
    LANP_NAVI_DESTINATION_REACHED_POS               = 8,
    LANP_NAVI_DESTINATION_REACHED_BIT               = CO_BIT(LANP_NAVI_DESTINATION_REACHED_POS),

    // Bit 9 - 15 RFU
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Location and Speed
struct lanp_loc_speed
{
    /// Flags
    uint16_t flags;
    /// Instantaneous Speed
    uint16_t inst_speed;
    /// Total distance
    uint32_t total_dist;
    /// Location - Latitude
    int32_t latitude;
    /// Location - Longitude
    int32_t longitude;
    /// Elevation
    int32_t elevation;
    /// Heading
    uint16_t heading;
    /// Rolling time
    uint8_t rolling_time;
    /// UTC Time
    struct prf_date_time date_time;
};

/// LAN Position quality
struct lanp_posq
{
    /// Flags
    uint16_t flags;
    /// Time to First Fix
    uint16_t time_first_fix;
    /// EHPE
    uint32_t ehpe;
    /// EVPE
    uint32_t evpe;
    /// Number of Beacons in Solution
    uint8_t n_beacons_solution;
    /// Number of Beacons in view
    uint8_t n_beacons_view;
    /// HDOP
    uint8_t hdop;
    /// VDOP
    uint8_t vdop;
};

/// LN Control Point Request
struct lan_ln_ctnl_pt_req
{
    /// Operation code @see enum lanp_ln_ctnl_pt_code
    uint8_t op_code;

    /// Value
    union lanp_ln_ctnl_pt_req_val
    {
        /// Cumulative Value (24 bits)
        uint32_t cumul_val;
        /// Mask Content
        uint16_t mask_content;
        /// Navigation Control @see enum lanp_navi_ctrl_val
        uint8_t control_value;
        /// Route number
        uint16_t route_number;
        /// Fix rate
        uint8_t fix_rate;
        /// Elevation
        int32_t elevation;
    } value;
};

/// LAN Control Point Response
struct lanp_ln_ctnl_pt_rsp
{
    /// Operation code - Response Code (0x20)
    uint8_t op_code;
    /// Operation code
    uint8_t req_op_code;
    /// Response Value
    uint8_t resp_value;

    /// Value
    union lanp_ctnl_pt_rsp_val
    {
        /// Number of routes
        uint16_t number_of_routes;

        struct lanp_route_name
        {
            uint8_t length;
            /// Name of Route UTF-8
            uint8_t name[__ARRAY_EMPTY];
        } route;
    } value;
};

/// LAN Navigation
struct lanp_navigation
{
    /// Flags
    uint16_t flags;
    /// Bearing
    uint16_t bearing;
    /// Heading
    uint16_t heading;
    /// Remaining Distance (24 bits)
    uint32_t remaining_distance;
    /// Remaining Vertical Distance (24 bits)
    uint32_t remaining_ver_distance;
    /// Estimated Time of Arrival
    struct prf_date_time estimated_arrival_time;
};


/// @} lanp_common

#endif //(_LANP_COMMON_H_)
