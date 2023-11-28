/**
 ****************************************************************************************
 *
 * @file gattm_task.h
 *
 * @brief Header file - GATTMTASK.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef GATTM_TASK_H_
#define GATTM_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup GATTMTASK Task
 * @ingroup GATTM
 * @brief Handles ALL GATT block operations not related to a connection.
 *
 * The GATTMTASK is responsible for managing internal attribute database and state of
 * GATT controller which manage GATT block operations related to a connection.
 *
 * Messages may originate from @ref ATTM "ATTM", @ref GAP "GAP" and Application.
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_task.h" // Task definitions
#include "att.h"
#include "gap.h"
#include "ke_msg.h"

/*
 * DEFINES
 ****************************************************************************************
 */


/// GATT Task messages
/*@TRACE*/
enum gattm_msg_id
{
    /* Database Management */
    /// Add service in database request
    GATTM_ADD_SVC_REQ                   = MSG_ID(GATTM, 0x00),
    /// Add service in database response
    GATTM_ADD_SVC_RSP                   = MSG_ID(GATTM, 0x01),

    /* Service management */
    /// Get permission settings of service request
    GATTM_SVC_GET_PERMISSION_REQ        = MSG_ID(GATTM, 0x02),
    /// Get permission settings of service response
    GATTM_SVC_GET_PERMISSION_RSP        = MSG_ID(GATTM, 0x03),
    /// Set permission settings of service request
    GATTM_SVC_SET_PERMISSION_REQ        = MSG_ID(GATTM, 0x04),
    /// Set permission settings of service response
    GATTM_SVC_SET_PERMISSION_RSP        = MSG_ID(GATTM, 0x05),

    /* Attribute Manipulation */
    /// Get permission settings of attribute request
    GATTM_ATT_GET_PERMISSION_REQ        = MSG_ID(GATTM, 0x06),
    /// Get permission settings of attribute response
    GATTM_ATT_GET_PERMISSION_RSP        = MSG_ID(GATTM, 0x07),
    /// Set permission settings of attribute request
    GATTM_ATT_SET_PERMISSION_REQ        = MSG_ID(GATTM, 0x08),
    /// Set permission settings of attribute response
    GATTM_ATT_SET_PERMISSION_RSP        = MSG_ID(GATTM, 0x09),

    /// Get attribute value request
    GATTM_ATT_GET_VALUE_REQ             = MSG_ID(GATTM, 0x0A),
    /// Get attribute value response
    GATTM_ATT_GET_VALUE_RSP             = MSG_ID(GATTM, 0x0B),
    /// Set attribute value request
    GATTM_ATT_SET_VALUE_REQ             = MSG_ID(GATTM, 0x0C),
    /// Set attribute value response
    GATTM_ATT_SET_VALUE_RSP             = MSG_ID(GATTM, 0x0D),
    /// Indication to the task that sends the unknown message
    GATTM_UNKNOWN_MSG_IND               = MSG_ID(GATTM, 0x0E),

    /* Debug messages */
    /// DEBUG ONLY: Destroy Attribute database request
    GATTM_DESTROY_DB_REQ                = MSG_ID(GATTM, 0x0F),
    /// DEBUG ONLY: Destroy Attribute database response
    GATTM_DESTROY_DB_RSP                = MSG_ID(GATTM, 0x10),
    /// DEBUG ONLY: Retrieve list of services request
    GATTM_SVC_GET_LIST_REQ              = MSG_ID(GATTM, 0x11),
    /// DEBUG ONLY: Retrieve list of services response
    GATTM_SVC_GET_LIST_RSP              = MSG_ID(GATTM, 0x12),
    /// DEBUG ONLY: Retrieve information of attribute request
    GATTM_ATT_GET_INFO_REQ              = MSG_ID(GATTM, 0x13),
    /// DEBUG ONLY: Retrieve information of attribute response
    GATTM_ATT_GET_INFO_RSP              = MSG_ID(GATTM, 0x14),
    /// DEBUG ONLY: Set Service visibility request
    GATTM_SVC_VISIBILITY_SET_REQ        = MSG_ID(GATTM, 0x15),
    /// DEBUG ONLY: Set Service visibility response
    GATTM_SVC_VISIBILITY_SET_RSP        = MSG_ID(GATTM, 0x16),

    /* GATT Caching */
    /// Compute hash value of the attribute database request
    GATTM_ATT_DB_HASH_COMP_REQ          = MSG_ID(GATTM, 0x20),
    /// Compute hash value of the attribute database response
    GATTM_ATT_DB_HASH_COMP_RSP          = MSG_ID(GATTM, 0x21),
};


/**
 * Attribute Description
 */
/*@TRACE*/
struct gattm_att_desc
{
    /** Attribute UUID (LSB First) */
    uint8_t uuid[ATT_UUID_128_LEN];

    /**
     *  Attribute Permission (@see attm_perm_mask)
     */
    uint16_t perm;


    /**
     * Maximum length of the attribute
     *
     * Note:
     * For Included Services and Characteristic Declarations, this field contains targeted
     * handle.
     *
     * For Characteristic Extended Properties, this field contains 2 byte value
     *
     * Not used Client Characteristic Configuration and Server Characteristic Configuration,
     * this field is not used.
     */
    uint16_t max_len;

    /**
     * Attribute Extended permissions
     *
     * Extended Value permission bit field
     *
     *   15   14   13   12   11   10    9    8    7    6    5    4    3    2    1    0
     * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * | RI |UUID_LEN |EKS |                       Reserved                            |
     * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
    *
     * Bit [0-11] : Reserved
     * Bit [12]   : Encryption key Size must be 16 bytes
     * Bit [13-14]: UUID Length             (0 = 16 bits, 1 = 32 bits, 2 = 128 bits, 3 = RFU)
     * Bit [15]   : Trigger Read Indication (0 = Value present in Database, 1 = Value not present in Database)
     */
    uint16_t ext_perm;
};

/**
 * Service description
 */
/*@TRACE
 @trc_arr atts $nb_att*/
struct gattm_svc_desc
{
    /// Attribute Start Handle (0 = dynamically allocated)
    uint16_t start_hdl;
    /// Task identifier that manages service
    uint16_t task_id;

    /**
     *    7    6    5    4    3    2    1    0
     * +----+----+----+----+----+----+----+----+
     * |SEC |UUID_LEN |DIS |  AUTH   |EKS | MI |
     * +----+----+----+----+----+----+----+----+
     *
     * Bit [0]  : Task that manage service is multi-instantiated (Connection index is conveyed)
     * Bit [1]  : Encryption key Size must be 16 bytes
     * Bit [2-3]: Service Permission      (0 = NO_AUTH, 1 = UNAUTH, 2 = AUTH, 3 = Secure Connect)
     * Bit [4]  : Disable the service
     * Bit [5-6]: UUID Length             (0 = 16 bits, 1 = 32 bits, 2 = 128 bits, 3 = RFU)
     * Bit [7]  : Secondary Service       (0 = Primary Service, 1 = Secondary Service)
     */
    uint8_t perm;

    /// Number of attributes
    uint8_t nb_att;

    /** Service  UUID */
    uint8_t uuid[ATT_UUID_128_LEN];

    /**
     * List of attribute description present in service.
     */
    struct gattm_att_desc atts[__ARRAY_EMPTY];
};


/// Add service in database request
/*@TRACE*/
struct gattm_add_svc_req
{
    /// service description
    struct gattm_svc_desc svc_desc;
};

/// Add service in database response
/*@TRACE*/
struct gattm_add_svc_rsp
{
    /// Start handle of allocated service in attribute database
    uint16_t start_hdl;
    /// Return status of service allocation in attribute database.
    uint8_t status;
};

/* Service management */
/// Get permission settings of service request
/*@TRACE*/
struct gattm_svc_get_permission_req
{
    /// Service start attribute handle
    uint16_t start_hdl;
};

/// Get permission settings of service response
/*@TRACE*/
struct gattm_svc_get_permission_rsp
{
    /// Service start attribute handle
    uint16_t start_hdl;
    /// service permission
    uint8_t perm;
    /// Return status
    uint8_t status;
};

/// Set permission settings of service request
/*@TRACE*/
struct gattm_svc_set_permission_req
{
    /// Service start attribute handle
    uint16_t start_hdl;
    /// service permission
    uint8_t perm;
};

/// Set permission settings of service response
/*@TRACE*/
struct gattm_svc_set_permission_rsp
{
    /// Service start attribute handle
    uint16_t start_hdl;
    /// Return status
    uint8_t status;
};


/* Attribute management */
/// Get permission settings of attribute request
/*@TRACE*/
struct gattm_att_get_permission_req
{
    /// Handle of the attribute
    uint16_t handle;
};

/// Get permission settings of attribute response
/*@TRACE*/
struct gattm_att_get_permission_rsp
{
    /// Handle of the attribute
    uint16_t handle;
    /// Attribute permission
    uint16_t perm;
    /// Extended Attribute permission
    uint16_t ext_perm;
    /// Return status
    uint8_t status;
};

/// Set permission settings of attribute request
/*@TRACE*/
struct gattm_att_set_permission_req
{
    /// Handle of the attribute
    uint16_t handle;
    /// Attribute permission
    uint16_t perm;
    /// Extended Attribute permission
    uint16_t ext_perm;
};

/// Set permission settings of attribute response
/*@TRACE*/
struct gattm_att_set_permission_rsp
{
    /// Handle of the attribute
    uint16_t handle;
    /// Return status
    uint8_t status;
};


/// Get attribute value request
/*@TRACE*/
struct gattm_att_get_value_req
{
    /// Handle of the attribute
    uint16_t handle;
};

/// Get attribute value response
/*@TRACE*/
struct gattm_att_get_value_rsp
{
    /// Handle of the attribute
    uint16_t handle;
    /// Attribute value length
    uint16_t length;
    /// Return status
    uint8_t status;
    /// Attribute value
    uint8_t value[__ARRAY_EMPTY];
};

/// Set attribute value request
/*@TRACE*/
struct gattm_att_set_value_req
{
    /// Handle of the attribute
    uint16_t handle;
    /// Attribute value length
    uint16_t length;
    /// Attribute value
    uint8_t value[__ARRAY_EMPTY];
};

/// Set attribute value response
/*@TRACE*/
struct gattm_att_set_value_rsp
{
    /// Handle of the attribute
    uint16_t handle;
    /// Return status
    uint8_t status;
};

/// Indicate that an unknown message has been received
/*@TRACE*/
struct gattm_unknown_msg_ind
{
    /// Unknown message id
    ke_msg_id_t unknown_msg_id;
};

/// DEBUG ONLY: Destroy Attribute database request
/*@TRACE*/
struct gattm_destroy_db_req
{
    /// New Gap Start Handle
    uint16_t gap_hdl;
    /// New Gatt Start Handle
    uint16_t gatt_hdl;
};

/// DEBUG ONLY: Destroy Attribute database Response
/*@TRACE*/
struct gattm_destroy_db_rsp
{
    /// Return status
    uint8_t status;
};


/// Service information
/*@TRACE*/
struct gattm_svc_info
{
    /// Service start handle
    uint16_t start_hdl;
    /// Service end handle
    uint16_t end_hdl;
    /// Service task_id
    uint16_t task_id;
    /// Service permission
    uint8_t perm;
};

/// DEBUG ONLY: Retrieve list of services response
/*@TRACE*/
struct gattm_svc_get_list_rsp
{
    /// Return status
    uint8_t status;
    /// Number of services
    uint8_t nb_svc;
    /// Array of information about services
    struct gattm_svc_info svc[__ARRAY_EMPTY];
};

/// DEBUG ONLY: Retrieve information of attribute request
/*@TRACE*/
struct gattm_att_get_info_req
{
    /// Attribute Handle
    uint16_t handle;
};

/// DEBUG ONLY: Retrieve information of attribute response
/*@TRACE*/
struct  gattm_att_get_info_rsp
{
    /// Return status
    uint8_t status;
    /// UUID Length
    uint8_t uuid_len;
    /// Attribute Handle
    uint16_t handle;
    /// Attribute Permissions
    uint16_t perm;
    /// Extended Attribute permission
    uint16_t ext_perm;
    /// UUID value
    uint8_t uuid[ATT_UUID_128_LEN];
};

/// DEBUG ONLY: Set Service visibility request
/*@TRACE*/
struct gattm_svc_visibility_set_req
{
    /// Service handle
    uint16_t handle;
    /// True to set service visible, false to hide it
    bool     visible;
};

/// DEBUG ONLY: Set Service visibility response
/*@TRACE*/
struct gattm_svc_visibility_set_rsp
{
    /// Return status
    uint8_t  status;
    /// Service handle
    uint16_t handle;
};

/// Compute hash value of the attribute database response
/*@TRACE*/
struct gattm_att_db_hash_comp_rsp
{
    /// Return status
    uint8_t status;
    /// Computed hash value (valid if status is GAP_ERR_NO_ERROR)
    uint8_t hash[GAP_KEY_LEN];
};

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/// @} GATTMTASK
#endif // GATTM_TASK_H_
