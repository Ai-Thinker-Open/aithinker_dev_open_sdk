/**
 ****************************************************************************************
 *
 * @file ln_ble_gatt.h
 *
 * @brief GATT API.
 *
 *Copyright (C) 2021. Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

#ifndef _LN_APP_GATT_H_
#define _LN_APP_GATT_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>
#include <stdbool.h>


/*
 * DEFINES
 ****************************************************************************************
 */

#define LN_ATT_UUID_128_LEN                        0x0010

enum
{
    SVC_PREM_TYPE_UUID16_MASK = 0x00,
    SVC_PREM_TYPE_UUID32_MASK = 0x01,
    SVC_PREM_TYPE_UUID128_MASK = 0x02,
};

/// Service Discovery command
/*@TRACE*/
typedef struct
{
    /// GATT Request Type
    /// - GATTC_SDP_DISC_SVC Search specific service
    /// - GATTC_SDP_DISC_SVC_ALL Search for all services
    /// - GATTC_SDP_DISC_CANCEL Cancel Service Discovery Procedure
    uint8_t operation;
    /// Service UUID Length
    uint8_t  uuid_len;
    /// Search start handle
    uint16_t start_hdl;
    /// Search end handle
    uint16_t end_hdl;
    /// Service UUID
    uint8_t  uuid[LN_ATT_UUID_128_LEN];
}  ln_gatt_sdp_cmd_t;

/// Service Discovery Command Structure
/*@TRACE*/
typedef struct
{
    /// GATT request type
    uint8_t  operation;
    /// UUID length
    uint8_t  uuid_len;
    /// start handle range
    uint16_t start_hdl;
    /// start handle range
    uint16_t end_hdl;
    //uuid
    uint8_t* uuid;
} ln_gatt_disc_cmd_t;

/**
 * Attribute Description
 */
/*@TRACE*/
typedef struct
{
    /** Attribute UUID (LSB First) */
    uint8_t uuid[LN_ATT_UUID_128_LEN];

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
}ln_gatt_att_desc_t;

/**
 * Service description
 */
/*@TRACE
 @trc_arr atts $nb_att*/
typedef struct
{
    /// Attribute Start Handle (0 = dynamically allocated)
    uint16_t start_hdl;

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
    uint8_t uuid[LN_ATT_UUID_128_LEN];
    /**
     * point to List of attribute description present in service.
     */
    const ln_gatt_att_desc_t *atts;
}ln_gatt_svc_desc_t;

/// Add service in database request
/*@TRACE*/
typedef struct
{
    /// service description
    ln_gatt_svc_desc_t svc_desc;
} ln_gatt_add_svc_req_t;

/// Simple Read (GATTC_READ or GATTC_READ_LONG)
/*@TRACE
 gattc_read = gattc_read_simple
 gattc_read_long = gattc_read_simple*/
typedef struct
{
    /// attribute handle
    uint16_t handle;
    /// start offset in data payload
    uint16_t offset;
    /// Length of data to read (0 = read all)
    uint16_t length;
} ln_gatt_read_simple_t;

/// Read by UUID: search UUID and read it's characteristic value (GATTC_READ_BY_UUID)
/// Note: it doesn't perform an automatic read long.
/*@TRACE*/
typedef struct
{
    /// Start handle
    uint16_t start_hdl;
    /// End handle
    uint16_t end_hdl;
    /// Size of UUID
    uint8_t uuid_len;
    /// UUID value
    uint8_t* uuid;
} ln_gatt_read_by_uuid_t;

/// Read Multiple short characteristic (GATTC_READ_MULTIPLE)
/*@TRACE*/
typedef struct
{
    /// attribute handle
    uint16_t handle;
    /// Known Handle length (shall be != 0)
    uint16_t len;
}  ln_gatt_read_multiple_t;

/// request union according to read type
/*@TRACE
 @trc_ref gattc_operation
 */
typedef union
{
    /// Simple Read (GATTC_READ or GATTC_READ_LONG)
    //@trc_union parent.operation == GATTC_READ or parent.operation == GATTC_READ_LONG
    ln_gatt_read_simple_t simple;
    /// Read by UUID (GATTC_READ_BY_UUID)
    //@trc_union parent.operation == GATTC_READ_BY_UUID
    ln_gatt_read_by_uuid_t by_uuid;
    /// Read Multiple short characteristic (GATTC_READ_MULTIPLE)
    //@trc_union parent.operation == GATTC_READ_MULTIPLE
    ln_gatt_read_multiple_t multiple[1];
} ln_gatt_read_req_t;

/// Read command (Simple, Long, Multiple, or by UUID)
/*@TRACE*/
typedef struct
{
    /// request type
    uint8_t operation;
    /// number of read (only used for multiple read)
    uint8_t nb;
    /// request union according to read type
    ln_gatt_read_req_t req;
} ln_gatt_read_cmd_t;

/// Write peer attribute value command
/*@TRACE*/
typedef struct
{
    /// Request type
    uint8_t operation;
    /// Perform automatic execution
    /// (if false, an ATT Prepare Write will be used this shall be use for reliable write)
    bool auto_execute;
    /// Attribute handle
    uint16_t handle;
    /// Write offset
    uint16_t offset;
    /// Write length
    uint16_t length;
    //point to value
    uint8_t* value;
} ln_gatt_write_cmd_t;

/// Send an event to peer device
/*@TRACE*/
typedef struct
{
    /// characteristic handle
    uint16_t handle;
    /// length of packet to send
    uint16_t length;
    /// point to value buff
    uint8_t* value;
} ln_gatt_send_evt_cmd_t;

/// Confirm modification of database from upper layer when requested by peer device.
/*@TRACE*/
typedef struct
{
    uint16_t handle;
    /// Status of read request execution by upper layers
    uint8_t status;
    /// Data length read
    uint16_t length;
    /// attribute data value
    uint8_t  value[__ARRAY_EMPTY];
} ln_gatt_read_cfm_t;

/// Confirm modification of database from upper layer when requested by peer device.
/*@TRACE*/
typedef struct
{
    /// Handle of the attribute written
    uint16_t handle;
    /// Status of write command execution by upper layers
    uint8_t status;
} ln_gatt_write_cfm_t;


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Exchange MTU request
 * @note This can only issue by the Client.
 *
 * @param[in] conidx                Connection index
 ****************************************************************************************
 */
void ln_gatt_exc_mtu(int conidx);

/**
 ****************************************************************************************
 * @brief Service Discovery Procedure
 * @note This can only issue by the Client.
 *
 * @param[in] conidx                Connection index
 * @param[in] p_param               Pointer to service discovery paramters,
 * @param[in] p_sdp
 *
 ****************************************************************************************
 */
void ln_gatt_sdp(int conidx, ln_gatt_sdp_cmd_t *p_param);

/**
 ****************************************************************************************
 * @brief Attributes Discovery
 * @note This can only issue by the Client.
 *
 * @param[in] conidx                Connection index
 * @param[in] p_param               Pointer to  discovery paramters,  operation fill in follow
 *  Discover all services
 *   GATTC_DISC_ALL_SVC,
 *   /// Discover services by UUID
 *  GATTC_DISC_BY_UUID_SVC,
 *   /// Discover included services
 *   GATTC_DISC_INCLUDED_SVC,
 *   /// Discover all characteristics
 *    GATTC_DISC_ALL_CHAR,
 *   /// Discover characteristic by UUID
 *   GATTC_DISC_BY_UUID_CHAR,
 *   /// Discover characteristic descriptor
 *   GATTC_DISC_DESC_CHAR,
 ****************************************************************************************
 */
void ln_gatt_discovery(int conidx, ln_gatt_disc_cmd_t *p_param);

/**
 ****************************************************************************************
 * @brief Add a new Service
 * @note This can only issue by the Server.
 *
 * @param[in] p_param               Pointer to service data structure                                    
 ****************************************************************************************
 */
void ln_gatt_add_svc(ln_gatt_add_svc_req_t *p_param);

/**
 ****************************************************************************************
 * @brief Read Attribute
 * @note This can only issue by the Client.
 *
 * @param[in] conidx                Connection index
 * @param[in] p_param               Pointer to   gattc_read_cmd structure,  Operation can fill in one of blow
 *  /// Read attribute
 *   GATTC_READ,
 *   /// Read long attribute
 *   GATTC_READ_LONG,
 *   /// Read attribute by UUID
 *   GATTC_READ_BY_UUID,
 *   /// Read multiple attribute
 *   GATTC_READ_MULTIPLE,
 *
 *
 ****************************************************************************************
 */
void ln_gatt_read(int conidx, ln_gatt_read_cmd_t *p_param);

/**
 ****************************************************************************************
 * @brief Write Attribute
 * @note This can only issue by the Client.
 *
 * @param[in] conidx                Connection index
 *
 * @param[in] p_param               Pointer to attribute write paramters, Operation can fill in one of blow
 * /// Write attribute
    GATTC_WRITE,
    /// Write no response
    GATTC_WRITE_NO_RESPONSE,
    /// Write signed
    GATTC_WRITE_SIGNED,
    /// Execute write
    GATTC_EXEC_WRITE,
  ****************************************************************************************
 */
void  ln_gatt_write(int conidx, ln_gatt_write_cmd_t *p_param);

/**
 ****************************************************************************************
 * @brief Execute Write command for queue writes
 * @note This can only issue by the Client.
 *
 * @param[in] conidx                Connection index
 * @param[in] execute               True: execute, false: cancel
 *
 *
 ****************************************************************************************
 */
void ln_gatt_excute_write(int conidx, bool execute);

/**
 ****************************************************************************************
 * @brief Send indication
 * @note This can only issue by the Server.
 *
 * @param[in] conidx                Connection index
 * @param[in] p_param               Pointer to attribute  paramters struct.
 *
 ****************************************************************************************
 */
void ln_gatt_send_ind(int conidx, ln_gatt_send_evt_cmd_t *p_param);

/**
 ****************************************************************************************
 * @brief Send notification
 * @note This can only issue by the Server.
 *
 * @param[in] conidx                Connection index

 * @param[in] p_param               Pointer to attribute  paramters struct 
 ****************************************************************************************
 */
void ln_gatt_send_ntf(int conidx, ln_gatt_send_evt_cmd_t *p_param);

/**
 ****************************************************************************************
 * @brief Confirm attribute read reqeust
 * @note This can only issue by the Server.
 *
 * @param[in] conidx                Connection index
 *
 * @param[in] p_param               Pointer to   paramters struct
 *
 ****************************************************************************************
 */
void ln_gatt_read_req_cfm(uint8_t conidx, ln_gatt_read_cfm_t *p_param);

/**
 ****************************************************************************************
 * @brief Confirm attribute write reqeust
 * @note This can only issue by the Server.
 *
 * @param[in] conidx                Connection index
 *
 * @param[in] p_param               Pointer to   paramters struct
 *
 ****************************************************************************************
 */
void ln_gatt_write_req_cfm(uint8_t conidx, ln_gatt_write_cfm_t *p_param);

int ln_gatt_app_init(void);

#endif // _LN_APP_GATT_H_

