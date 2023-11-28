/**
 ****************************************************************************************
 *
 * @file bcss_task.c
 *
 * @brief Body Composition Service Task Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup BCSSTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_BCS_SERVER)
#include "rwble_hl_error.h"
#include "gapc.h"
#include "gattc.h"
#include "gattc_task.h"
#include "attm.h"
#include "bcss.h"
#include "bcss_task.h"
#include "bcs_common.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref BCSS_ENABLE_REQ message.
 *
 * @param[in] msgid Id of the message received
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int bcss_enable_req_handler(ke_msg_id_t const msgid,
                                    struct bcss_enable_req *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Status
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    struct bcss_enable_rsp *p_cmp_evt = NULL;

    if (ke_state_get(dest_id) == BCSS_IDLE)
    {
        // Get the address of the environment
        struct bcss_env_tag *p_bcss_env = PRF_ENV_GET(BCSS, bcss);

        // Save indication config
        p_bcss_env->prfl_ind_cfg[KE_IDX_GET(src_id)] = p_param->ind_cfg;
        status = GAP_ERR_NO_ERROR;
    }

    // send completed information to APP task that contains error status
    p_cmp_evt = KE_MSG_ALLOC(BCSS_ENABLE_RSP, src_id, dest_id, bcss_enable_rsp);
    p_cmp_evt->status = status;

    ke_msg_send(p_cmp_evt);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the read request from peer device
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_read_req_ind_handler(ke_msg_id_t const msgid,
                                      struct gattc_read_req_ind const *p_param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;

    // check that task is in idle state
    if(ke_state_get(dest_id) == BCSS_IDLE)
    {
        // Get the address of the environment
        struct bcss_env_tag *p_bcss_env = PRF_ENV_GET(BCSS, bcss);
        uint8_t att_idx = p_param->handle - p_bcss_env->shdl;
        uint8_t value[BCS_FEAT_VAL_LEN];
        uint8_t value_size = 0;
        uint8_t status = ATT_ERR_NO_ERROR;
        struct gattc_read_cfm* p_cfm = NULL;

        switch(att_idx)
        {
            case BCSS_IDX_FEAT_VAL:
            {
                value_size = BCS_FEAT_VAL_LEN;
                co_write32p(&(value[0]), p_bcss_env->feature);
            } break;

            case BCSS_IDX_MEAS_CCC:
            {
                value_size = BCS_MEAS_CCC_LEN;
                co_write16p(&(value[0]), p_bcss_env->prfl_ind_cfg[KE_IDX_GET(src_id)]);
            } break;

            default:
            {
                if (p_cfm == NULL)
                {
                    status = ATT_ERR_REQUEST_NOT_SUPPORTED;
                }
            } break;
        }

        p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, value_size);
        p_cfm->length = value_size;
        memcpy(p_cfm->value, value, value_size);
        p_cfm->handle = p_param->handle;
        p_cfm->status = status;

        // Send value to peer device.
        ke_msg_send(p_cfm);
    }
    // else process it later
    else
    {
        msg_status = KE_MSG_SAVED;
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the attribute info request message.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gattc_att_info_req_ind *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct bcss_env_tag *p_bcss_env = PRF_ENV_GET(BCSS, bcss);
    uint8_t att_idx = p_param->handle - p_bcss_env->shdl;
    struct gattc_att_info_cfm *p_cfm;

    //Send write response
    p_cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    p_cfm->handle = p_param->handle;

    // check if it's a client configuration char
    if (att_idx == BCSS_IDX_MEAS_CCC)
    {
        // CCC attribute length = 2
        p_cfm->length = BCS_MEAS_CCC_LEN;
        p_cfm->status = GAP_ERR_NO_ERROR;
    }
    else // not expected request
    {
        p_cfm->length = 0;
        p_cfm->status = ATT_ERR_WRITE_NOT_PERMITTED;
    }

    ke_msg_send(p_cfm);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref BCSS_MEAS_INDICATE_CMD message.
 * Send MEASUREMENT INDICATION to the connected peer case of CCC enabled
 *
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int bcss_meas_indicate_cmd_handler(ke_msg_id_t const msgid,
                                          struct bcss_meas_indicate_cmd *p_param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct bcss_env_tag *p_bcss_env = PRF_ENV_GET(BCSS, bcss);

     // check that task is in idle state
    if (ke_state_get(dest_id) == BCSS_IDLE)
    {
        // Connection index
        uint8_t conidx = KE_IDX_GET(dest_id);

        if (p_bcss_env->prfl_ind_cfg[conidx] & PRF_CLI_START_IND)
        {
            // Allocate the GATT notification message
            struct gattc_send_evt_cmd *p_ind;
            uint16_t max_payload = gattc_get_mtu(conidx) - 3;
            uint16_t next_segment_flags = 0;
            uint16_t length = 0;
            // allocate the maxim block for indication
            uint8_t value[BCSS_MEAS_IND_SIZE];

            // Mask off any illegal bits in the flags field
            p_param->flags &= BCS_MEAS_FLAGS_VALID;

            if (p_param->body_fat_percent == BCS_MEAS_UNSUCCESSFUL)
            {
                // Disable all other optional fields other than Timestamp and User ID
                SETB(p_param->flags, BCS_MEAS_FLAGS_BASAL_METAB_PRESENT, 0);
                SETB(p_param->flags, BCS_MEAS_FLAGS_MUSCLE_PERCENT_PRESENT, 0);
                SETB(p_param->flags, BCS_MEAS_FLAGS_MUSCLE_MASS_PRESENT, 0);
                SETB(p_param->flags, BCS_MEAS_FLAGS_FAT_FREE_MASS_PRESENT, 0);
                SETB(p_param->flags, BCS_MEAS_FLAGS_SOFT_LEAN_MASS_PRESENT, 0);
                SETB(p_param->flags, BCS_MEAS_FLAGS_BODY_WATER_MASS_PRESENT, 0);
                SETB(p_param->flags, BCS_MEAS_FLAGS_IMPEDANCE_PRESENT, 0);
                SETB(p_param->flags, BCS_MEAS_FLAGS_WEIGHT_PRESENT, 0);
                SETB(p_param->flags, BCS_MEAS_FLAGS_HEIGHT_PRESENT, 0);
            }

            //**************************************************************
            // Encode the Fields of the  Measurement
            // if the Application provide flags and fields which do not correspond
            // to the features declared by the server, we adjust the flags field to
            // ensure we only Indicate with fields compatible with our features.
            // Thus the flags fields is encoded last as it will be modified by checks on
            // features.
            //
            // If the Fields provided require 2 ATT_HANDLE_VALUE_INDs. The following will
            // always be contained in the first message (if selected in flags field).
            //          1/ Flags
            //          2/ Body Fat (mandatory)
            //          3/ Measurement Units (retrieve from flags)
            //          4/ Time Stamp
            //          5/ User Id
            //          6/ Basal Metabolism
            //          7/ Muscle Percentage
            //          8/ Muscle Mass
            //********************************************************************
            length += sizeof(uint16_t); // Flags is 16bit

            // Mandatory Body Fat Percentage
            co_write16p(&value[length], p_param->body_fat_percent);
            length += sizeof(uint16_t);

            // Measurement Units
            // 0 for SI and 1 for Imperial
            value[length++] = GETB(p_param->flags, BCS_MEAS_FLAGS_UNITS_IMPERIAL);

            // We always include the Time-Stamp and User Id in the first message - if segmenting.
            // Time stamp shall be included in flags field if the Server supports Time Stamp feature
            if (GETB(p_bcss_env->feature, BCS_FEAT_TIME_STAMP_SUPP))
            {
                SETB(p_param->flags, BCS_MEAS_FLAGS_TIMESTAMP_PRESENT, 1);
                length += prf_pack_date_time(&value[length], &p_param->time_stamp);
            }
            else
            {
                // Shall not be included if the Time Stamp feature is not supported
                SETB(p_param->flags, BCS_MEAS_FLAGS_TIMESTAMP_PRESENT, 0);
            }

            // User ID shall be included in flags field if the Server supports Multiple Users feature
            if (GETB(p_bcss_env->feature, BCS_FEAT_MULTIPLE_USERS_SUPP))
            {
                SETB(p_param->flags, BCS_MEAS_FLAGS_USER_ID_PRESENT, 1);
                value[length++] = p_param->user_id;
            }
            else
            {
                // Shall not be included if the Multiple Users feature is not supported
                SETB(p_param->flags, BCS_MEAS_FLAGS_USER_ID_PRESENT, 0);
            }

            // Basal Metabolism if present and enabled in the features.
            if (GETB(p_param->flags, BCS_MEAS_FLAGS_BASAL_METAB_PRESENT))
            {
                if (GETB(p_bcss_env->feature, BCS_FEAT_BASAL_METAB_SUPP))
                {
                    co_write16p(&value[length], p_param->basal_metab);
                    length += sizeof(uint16_t);
                }
                else
                {
                    SETB(p_param->flags, BCS_MEAS_FLAGS_BASAL_METAB_PRESENT, 0);
                }
            }

            // Muscle Percentage if present and enabled in the features.
            if (GETB(p_param->flags, BCS_MEAS_FLAGS_MUSCLE_PERCENT_PRESENT))
            {
                if (GETB(p_bcss_env->feature, BCS_FEAT_MUSCLE_PERCENTAGE_SUPP))
                {
                    co_write16p(&value[length], p_param->muscle_percent);
                    length += sizeof(uint16_t);
                }
                else
                {
                    SETB(p_param->flags, BCS_MEAS_FLAGS_MUSCLE_PERCENT_PRESENT, 0);
                }
            }

            // Muscle Mass if present and enabled in the features.
            if (GETB(p_param->flags, BCS_MEAS_FLAGS_MUSCLE_MASS_PRESENT))
            {
                if (GETB(p_bcss_env->feature, BCS_FEAT_MUSCLE_MASS_SUPP))
                {
                    co_write16p(&value[length], p_param->muscle_mass);
                    length += sizeof(uint16_t);
                }
                else
                {
                    SETB(p_param->flags, BCS_MEAS_FLAGS_MUSCLE_MASS_PRESENT, 0);
                }
            }

            // Fat Free Mass if present and enabled in the features.
            if (GETB(p_param->flags, BCS_MEAS_FLAGS_FAT_FREE_MASS_PRESENT))
            {
                if (GETB(p_bcss_env->feature, BCS_FEAT_FAT_FREE_MASS_SUPP))
                {
                    if ((length + sizeof(uint16_t)) <= max_payload)
                    {
                        co_write16p(&value[length], p_param->fat_free_mass);
                        length += sizeof(uint16_t);
                    }
                    else
                    {
                        SETB(next_segment_flags, BCS_MEAS_FLAGS_FAT_FREE_MASS_PRESENT, 1);
                    }
                }
                else
                {
                    SETB(p_param->flags, BCS_MEAS_FLAGS_FAT_FREE_MASS_PRESENT, 0);
                }
            }

            // Soft Lean Mass if present and enabled in the features.
            if (GETB(p_param->flags, BCS_MEAS_FLAGS_SOFT_LEAN_MASS_PRESENT))
            {
                if (GETB(p_bcss_env->feature, BCS_FEAT_SOFT_LEAN_MASS_SUPP))
                {
                    if ((length + sizeof(uint16_t)) <= max_payload)
                    {
                        co_write16p(&value[length], p_param->soft_lean_mass);
                        length += sizeof(uint16_t);
                    }
                    else
                    {
                        SETB(next_segment_flags, BCS_MEAS_FLAGS_SOFT_LEAN_MASS_PRESENT, 1);
                    }
                }
                else
                {
                    SETB(p_param->flags, BCS_MEAS_FLAGS_SOFT_LEAN_MASS_PRESENT, 0);
                }
            }

            // Body Water Mass if present and enabled in the features.
            if (GETB(p_param->flags, BCS_MEAS_FLAGS_BODY_WATER_MASS_PRESENT))
            {
                if (GETB(p_bcss_env->feature, BCS_FEAT_BODY_WATER_MASS_SUPP))
                {
                    if ((length + sizeof(uint16_t)) <= max_payload)
                    {
                        co_write16p(&value[length], p_param->body_water_mass);
                        length += sizeof(uint16_t);
                    }
                    else
                    {
                        SETB(next_segment_flags, BCS_MEAS_FLAGS_BODY_WATER_MASS_PRESENT, 1);
                    }
                }
                else
                {
                    SETB(p_param->flags, BCS_MEAS_FLAGS_BODY_WATER_MASS_PRESENT, 0);
                }
            }

            // Impedance if present and enabled in the features.
            if (GETB(p_param->flags, BCS_MEAS_FLAGS_IMPEDANCE_PRESENT))
            {
                if (GETB(p_bcss_env->feature, BCS_FEAT_IMPEDANCE_SUPP))
                {
                    if ((length + sizeof(uint16_t)) <= max_payload)
                    {
                        co_write16p(&value[length], p_param->impedance);
                        length += sizeof(uint16_t);
                    }
                    else
                    {
                        SETB(next_segment_flags, BCS_MEAS_FLAGS_IMPEDANCE_PRESENT, 1);
                    }
                }
                else
                {
                    SETB(p_param->flags, BCS_MEAS_FLAGS_IMPEDANCE_PRESENT, 0);
                }
            }

            // Weight if present and enabled in the features.
            if (GETB(p_param->flags, BCS_MEAS_FLAGS_WEIGHT_PRESENT))
            {
                if (GETB(p_bcss_env->feature, BCS_FEAT_WEIGHT_SUPP))
                {
                    if ((length + sizeof(uint16_t) + sizeof(uint8_t)) <= max_payload)
                    {
                        co_write16p(&value[length], p_param->weight);
                        length += sizeof(uint16_t);

                        //  Get Mass Resolution
                        value[length++] = bcss_get_mass_resol(p_bcss_env->feature);
                    }
                    else
                    {
                        SETB(next_segment_flags, BCS_MEAS_FLAGS_WEIGHT_PRESENT, 1);
                    }
                }
                else
                {
                    SETB(p_param->flags, BCS_MEAS_FLAGS_WEIGHT_PRESENT, 0);
                }
            }

            // Height if present and enabled in the features.
            if (GETB(p_param->flags, BCS_MEAS_FLAGS_HEIGHT_PRESENT))
            {
                if (GETB(p_bcss_env->feature, BCS_FEAT_HEIGHT_SUPP))
                {
                    if ((length + sizeof(uint16_t) + sizeof(uint8_t)) <= max_payload)
                    {
                        co_write16p(&value[length], p_param->height);
                        length += sizeof(uint16_t);

                        // Get Height Resolution
                        value[length++] = bcss_get_hght_resol(p_bcss_env->feature);
                    }
                    else
                    {
                        SETB(next_segment_flags, BCS_MEAS_FLAGS_HEIGHT_PRESENT, 1);
                    }
                }
                else
                {
                    SETB(p_param->flags, BCS_MEAS_FLAGS_HEIGHT_PRESENT, 0);
                }
            }

            // Finally store the flags in Byte 0 - the flags may have been changed in pre-ceeding steps.
            if (next_segment_flags)
            {
                // Indicate this is a multipacket measurement.
                // remove flags for fields which will not be transmitted in first segment
                SETB(p_param->flags, BCS_MEAS_FLAGS_MULTIPACKET_MEAS, 1);

                co_write16p(&value[0], (p_param->flags &= ~next_segment_flags));
                SETB(next_segment_flags, BCS_MEAS_FLAGS_MULTIPACKET_MEAS, 1);
                p_param->flags = next_segment_flags;
                // Store the param in order to send the second segment
                p_bcss_env->meas_cmd_msg[conidx] = ke_param2msg(p_param);

                msg_status = KE_MSG_NO_FREE;
            }
            else
            {
                co_write16p(&value[0], p_param->flags);
                p_bcss_env->meas_cmd_msg[conidx] = 0;
            }

            // Allocate the GATT notification message
            p_ind = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                    KE_BUILD_ID(TASK_GATTC, conidx),
                    dest_id,
                    gattc_send_evt_cmd, length);

            // Fill in the p_parameter structure
            p_ind->operation = GATTC_INDICATE;
            p_ind->handle = p_bcss_env->shdl + BCSS_IDX_MEAS_IND;
            // Pack Measurement record
            p_ind->length = length;
            // data
            memcpy (p_ind->value, &value[0], length);

            // Send the event
            ke_msg_send(p_ind);

            // go to busy state
            ke_state_set(dest_id, BCSS_OP_INDICATE);
        }
        else
        {
            // Send a command complete to the App indicate msg could not be sent.
            bcss_send_cmp_evt(p_bcss_env, conidx, BCSS_MEAS_INDICATE_CMD_OP_CODE, PRF_ERR_IND_DISABLED);
        }

    }
    // else process it later
    else
    {
        msg_status = KE_MSG_SAVED;
    }

    return msg_status;
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_WRITE_REQ_IND message.
 *
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_write_req_ind_handler(ke_msg_id_t const msgid,
                                       struct gattc_write_req_ind const *p_param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct bcss_env_tag *p_bcss_env = PRF_ENV_GET(BCSS, bcss);
    // Message status
    int msg_status = KE_MSG_CONSUMED;

    // Check the connection handle
    if (p_bcss_env != NULL)
    {
        // check that task is in idle state
        if (ke_state_get(dest_id) == BCSS_IDLE)
        {
            // Status
            uint8_t status = GAP_ERR_NO_ERROR;
            uint8_t att_idx = p_param->handle - p_bcss_env->shdl;
            struct gattc_write_cfm *p_cfm = NULL;

            // CSC Measurement Characteristic, Client Characteristic Configuration Descriptor
            if (att_idx == BCSS_IDX_MEAS_CCC)
            {
                uint16_t ntf_cfg;
                uint8_t conidx = KE_IDX_GET(src_id);
                struct bcss_meas_ccc_ind *p_ind = NULL;

                // Get the value
                co_write16p(&ntf_cfg, p_param->value[0]);
                p_bcss_env->prfl_ind_cfg[conidx] = ntf_cfg;

                // Inform the HL about the new configuration
                p_ind = KE_MSG_ALLOC(BCSS_MEAS_CCC_IND,
                        prf_dst_task_get(&p_bcss_env->prf_env, conidx),
                        prf_src_task_get(&p_bcss_env->prf_env, conidx),
                        bcss_meas_ccc_ind);

                p_ind->ind_cfg = ntf_cfg;

                ke_msg_send(p_ind);
            }
            else
            {
                status = PRF_ERR_INVALID_PARAM;
            }
            
            // Send the write response to the peer device
            p_cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
            p_cfm->handle = p_param->handle;
            p_cfm->status = status;
            ke_msg_send(p_cfm);
        }
        else
        {
            msg_status = KE_MSG_SAVED;
        }
    }
    // else drop the message

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles @ref GATTC_CMP_EVT message meaning that an indication
 * has been correctly sent to peer device (but not confirmed by peer device).
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] p_param   Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *p_param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    do
    {
        uint8_t conidx = KE_IDX_GET(src_id);
        // Get the address of the environment
        struct bcss_env_tag *p_bcss_env = PRF_ENV_GET(BCSS, bcss);

        // Check if a connection exists
        if ((p_bcss_env == NULL) || (p_param->operation != GATTC_INDICATE) ||
                (ke_state_get(dest_id) != BCSS_OP_INDICATE))
        {
            break;
        }

        if (p_bcss_env->meas_cmd_msg[conidx])
        {
            // If this is the CFM to the first part of a segmented message we must Tx the second part.
            if (p_bcss_env->prfl_ind_cfg[conidx] & PRF_CLI_START_IND)
            {
                // Allocate the GATT notification message
                struct gattc_send_evt_cmd *p_ind;
                struct bcss_meas_indicate_cmd *p_cmd = ke_msg2param(p_bcss_env->meas_cmd_msg[conidx]);
                // allocate the maxim block for indication
                uint8_t value[BCSS_MEAS_IND_SIZE];
                uint8_t length = 0;

                //**************************************************************
                //
                // Following fields will not be present - allways in first segment
                //          3/ Time Stamp
                //          4/ User Id
                //          5/ Basal Metabolism
                //          6/ Muscle Percentage
                //          7/ Muscle Mass
                //********************************************************************
                co_write16p(&value[length], p_cmd->flags);
                // Flags is 16bit
                length += sizeof(uint16_t);

                // Mandatory Body Fat Percentage
                co_write16p(&value[length], p_cmd->body_fat_percent);
                length += sizeof(uint16_t);

                if (GETB(p_cmd->flags, BCS_MEAS_FLAGS_FAT_FREE_MASS_PRESENT))
                {
                    co_write16p(&value[length], p_cmd->fat_free_mass);
                    length += sizeof(uint16_t);
                }

                if (GETB(p_cmd->flags, BCS_MEAS_FLAGS_SOFT_LEAN_MASS_PRESENT))
                {
                    co_write16p(&value[length], p_cmd->soft_lean_mass);
                    length += sizeof(uint16_t);
                }

                if (GETB(p_cmd->flags, BCS_MEAS_FLAGS_BODY_WATER_MASS_PRESENT))
                {
                    co_write16p(&value[length], p_cmd->body_water_mass);
                    length += sizeof(uint16_t);
                }

                if (GETB(p_cmd->flags, BCS_MEAS_FLAGS_IMPEDANCE_PRESENT))
                {
                    co_write16p(&value[length], p_cmd->impedance);
                    length += sizeof(uint16_t);
                }

                if (GETB(p_cmd->flags, BCS_MEAS_FLAGS_WEIGHT_PRESENT))
                {
                    co_write16p(&value[length], p_cmd->weight);
                    length += sizeof(uint16_t);

                    //  Get Mass Resolution
                    value[length++] = bcss_get_mass_resol(p_bcss_env->feature);
                }

                if (GETB(p_cmd->flags, BCS_MEAS_FLAGS_HEIGHT_PRESENT))
                {
                    co_write16p(&value[length], p_cmd->height);
                    length += sizeof(uint16_t);

                    // Get Height Resolution
                    value[length++] = bcss_get_hght_resol(p_bcss_env->feature);
                }

                // Finally store the flags in Byte 0 - the flags may have been changed in pre-ceeding steps.
                co_write16p(&value[0], p_cmd->flags);
                // Allocate the GATT notification message
                p_ind = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                        KE_BUILD_ID(TASK_GATTC, conidx),
                        dest_id,
                        gattc_send_evt_cmd, length);

                // Fill in the p_parameter structure
                p_ind->operation = GATTC_INDICATE;
                p_ind->handle = p_bcss_env->shdl + BCSS_IDX_MEAS_IND;
                p_ind->length = length;
                // data
                memcpy (p_ind->value, &value[0], length);

                // Send the event
                ke_msg_send(p_ind);

            }

            ke_msg_free(p_bcss_env->meas_cmd_msg[conidx]);
            p_bcss_env->meas_cmd_msg[conidx] = 0;
        }
        else
        {
            // Inform the application that a procedure has been completed
            bcss_send_cmp_evt(p_bcss_env,
                  conidx,
                  BCSS_MEAS_INDICATE_CMD_OP_CODE,
                  p_param->status);

            ke_state_set(dest_id, BCSS_IDLE);
        }

    } while (0);

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(bcss)
{
    {BCSS_ENABLE_REQ,          (ke_msg_func_t) bcss_enable_req_handler},
    /// Send a BCS Measurement to the peer device (Indication)
    {BCSS_MEAS_INDICATE_CMD,   (ke_msg_func_t) bcss_meas_indicate_cmd_handler},

    {GATTC_READ_REQ_IND,       (ke_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_ATT_INFO_REQ_IND,   (ke_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,      (ke_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_CMP_EVT,            (ke_msg_func_t) gattc_cmp_evt_handler},
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void bcss_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct bcss_env_tag *p_bcss_env = PRF_ENV_GET(BCSS, bcss);

    p_task_desc->msg_handler_tab = bcss_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(bcss_msg_handler_tab);
    p_task_desc->state           = p_bcss_env->state;
    p_task_desc->idx_max         = BCSS_IDX_MAX;
}

#endif //(BLE_BCS_SERVER)

/// @} BCSTASK
