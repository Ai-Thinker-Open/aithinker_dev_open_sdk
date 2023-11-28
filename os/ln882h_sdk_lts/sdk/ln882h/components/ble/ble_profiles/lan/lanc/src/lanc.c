/**
 ****************************************************************************************
 *
 * @file lanc.c
 *
 * @brief Location and Navigation Profile Collector implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup LANC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_LN_COLLECTOR)
#include "lan_common.h"

#include "lanc.h"
#include "lanc_task.h"

#include "ke_timer.h"
#include "ke_mem.h"
#include "co_utils.h"

/*
 * GLOBAL VARIABLES DECLARATION
 ****************************************************************************************
 */


/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the LANC module.
 * This function performs all the initializations of the Profile module.
 *  - Creation of database (if it's a service)
 *  - Allocation of profile required memory
 *  - Initialization of task descriptor to register application
 *      - Task State array
 *      - Number of tasks
 *      - Default task handler
 *
 * @param[out]    p_env        Collector or Service allocated environment data.
 * @param[in|out] p_start_hdl  Service start handle (0 - dynamically allocated), only applies for services.
 * @param[in]     app_task     Application task number.
 * @param[in]     sec_lvl      Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 * @param[in]     p_params     Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 ****************************************************************************************
 */

static uint8_t lanc_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl,  void *p_params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct lanc_env_tag *p_lanc_env =
            (struct lanc_env_tag *) ke_malloc(sizeof(struct lanc_env_tag), KE_MEM_ATT_DB);

    // allocate LANC required environment variable
    p_env->env = (prf_env_t *) p_lanc_env;

    p_lanc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    p_lanc_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    p_env->id = TASK_ID_LANC;
    lanc_task_init(&(p_env->desc));

    for (idx = 0; idx < LANC_IDX_MAX ; idx++)
    {
        p_lanc_env->p_env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(p_env->task, idx), LANC_FREE);
    }

    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief Clean-up connection dedicated environment parameters
 * This function performs cleanup of ongoing operations
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void lanc_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct lanc_env_tag *p_lanc_env = (struct lanc_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if (p_lanc_env->p_env[conidx] != NULL)
    {
        if (p_lanc_env->p_env[conidx]->p_operation != NULL)
        {
            ke_free(ke_param2msg(p_lanc_env->p_env[conidx]->p_operation));
        }
        ke_timer_clear(LANC_TIMEOUT_TIMER_IND, prf_src_task_get(&p_lanc_env->prf_env, conidx));

        ke_free(p_lanc_env->p_env[conidx]);
        p_lanc_env->p_env[conidx] = NULL;
    }

    // Put LAN Client in Free state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), LANC_FREE);
}

/**
 ****************************************************************************************
 * @brief Destruction of the LANC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void lanc_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct lanc_env_tag *p_lanc_env = (struct lanc_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for (idx = 0; idx < LANC_IDX_MAX ; idx++)
    {
        lanc_cleanup(p_env, idx, 0);
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_lanc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void lanc_create(struct prf_task_env *p_env, uint8_t conidx)
{
    // Put LAN Client in Idle state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), LANC_IDLE);
}


/// LANC Task interface required by profile manager
const struct prf_task_cbs lanc_itf =
{
    lanc_init,
    lanc_destroy,
    lanc_create,
    lanc_cleanup,
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *lanc_prf_itf_get(void)
{
   return &lanc_itf;
}

void lanc_enable_rsp_send(struct lanc_env_tag *p_lanc_env, uint8_t conidx, uint8_t status)
{
    // Send to APP the details of the discovered attributes on LANS
    struct lanc_enable_rsp *p_rsp = KE_MSG_ALLOC(
            LANC_ENABLE_RSP,
            prf_dst_task_get(&(p_lanc_env->prf_env), conidx),
            prf_src_task_get(&(p_lanc_env->prf_env), conidx),
            lanc_enable_rsp);

    p_rsp->status = status;

    if (status == GAP_ERR_NO_ERROR)
    {
        p_rsp->lns = p_lanc_env->p_env[conidx]->lans;
        // Register LANC task in gatt for indication/notifications
        prf_register_atthdl2gatt(&(p_lanc_env->prf_env), conidx, &(p_lanc_env->p_env[conidx]->lans.svc));
        // Go to connected state
        ke_state_set(prf_src_task_get(&(p_lanc_env->prf_env), conidx), LANC_IDLE);
    }

    ke_msg_send(p_rsp);
}

void lanc_send_no_conn_cmp_evt(uint8_t src_id, uint8_t dest_id, uint8_t operation)
{
    // Send the message
    struct lanc_cmp_evt *p_evt = KE_MSG_ALLOC(LANC_CMP_EVT,
                                             dest_id, src_id,
                                             lanc_cmp_evt);

    p_evt->operation  = operation;
    p_evt->status     = PRF_ERR_REQ_DISALLOWED;

    ke_msg_send(p_evt);
}

void lanc_send_cmp_evt(struct lanc_env_tag *p_lanc_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct lanc_cmp_evt *p_evt;

    // Free the stored operation if needed
    if (p_lanc_env->p_env[conidx]->p_operation != NULL)
    {
        ke_msg_free(ke_param2msg(p_lanc_env->p_env[conidx]->p_operation));
        p_lanc_env->p_env[conidx]->p_operation = NULL;
    }

    // Go back to the CONNECTED state if the state is busy
    if (ke_state_get(prf_src_task_get(&(p_lanc_env->prf_env), conidx)) == LANC_BUSY)
    {
        ke_state_set(prf_src_task_get(&(p_lanc_env->prf_env), conidx), LANC_IDLE);
    }

    // Send the message
    p_evt = KE_MSG_ALLOC(LANC_CMP_EVT,
                         prf_dst_task_get(&(p_lanc_env->prf_env), conidx),
                         prf_src_task_get(&(p_lanc_env->prf_env), conidx),
                         lanc_cmp_evt);

    p_evt->operation  = operation;
    p_evt->status     = status;

    ke_msg_send(p_evt);
}

uint16_t lanc_get_read_handle_req (struct lanc_env_tag *p_lanc_env, uint8_t conidx, struct lanc_read_cmd *p_param)
{
    // Attribute Handle
    uint16_t handle = ATT_INVALID_SEARCH_HANDLE;

    switch (p_param->read_code)
    {
        // Read LN Feature
        case (LANC_RD_LN_FEAT):
        {
            handle = p_lanc_env->p_env[conidx]->lans.chars[LANP_LANS_LN_FEAT_CHAR].val_hdl;
        } break;

        // Read Position Quality
        case (LANC_RD_POS_Q):
        {
            handle = p_lanc_env->p_env[conidx]->lans.chars[LANP_LANS_POS_Q_CHAR].val_hdl;
        } break;

        // Read Location and Speed Characteristic Client Char. Cfg. Descriptor Value
        case (LANC_RD_WR_LOC_SPEED_CL_CFG):
        {
            handle = p_lanc_env->p_env[conidx]->lans.descs[LANC_DESC_LOC_SPEED_CL_CFG].desc_hdl;
        } break;

        // Read Unread Alert Characteristic Client Char. Cfg. Descriptor Value
        case (LANC_RD_WR_LN_CTNL_PT_CFG):
        {
            handle = p_lanc_env->p_env[conidx]->lans.descs[LANC_DESC_LN_CTNL_PT_CL_CFG].desc_hdl;
        } break;

        // Read Navigation Characteristic Server Char. Cfg. Descriptor Value
        case (LANC_RD_WR_NAVIGATION_CFG):
        {
            handle = p_lanc_env->p_env[conidx]->lans.descs[LANC_DESC_NAVIGATION_CL_CFG].desc_hdl;
        } break;

        default:
        {
            handle = ATT_INVALID_SEARCH_HANDLE;
        } break;
    }

    return handle;
}

uint8_t lanc_get_write_desc_handle_req (uint8_t conidx, struct lanc_cfg_ntfind_cmd *p_param,
        struct lanc_env_tag *p_lanc_env, uint16_t *p_handle)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;

    switch(p_param->desc_code)
    {
        // Write Location and speed Characteristic Client Char. Cfg. Descriptor Value
        case (LANC_RD_WR_LOC_SPEED_CL_CFG):
        {
            if (p_param->ntfind_cfg <= PRF_CLI_START_NTF)
            {
                *p_handle = p_lanc_env->p_env[conidx]->lans.descs[LANC_DESC_LOC_SPEED_CL_CFG].desc_hdl;

                // The descriptor is mandatory
                ASSERT_ERR(*p_handle != ATT_INVALID_SEARCH_HANDLE);
            }
            else
            {
                status = PRF_ERR_INVALID_PARAM;
            }
        } break;

        // Write LN Control Point Characteristic Client Char. Cfg. Descriptor Value
        case (LANC_RD_WR_LN_CTNL_PT_CFG):
        {
            if ((p_param->ntfind_cfg == PRF_CLI_STOP_NTFIND) ||
                (p_param->ntfind_cfg == PRF_CLI_START_IND))
            {
                *p_handle = p_lanc_env->p_env[conidx]->lans.descs[LANC_DESC_LN_CTNL_PT_CL_CFG].desc_hdl;

                if (*p_handle == ATT_INVALID_SEARCH_HANDLE)
                {
                    // The descriptor has not been found.
                    status = PRF_ERR_INEXISTENT_HDL;
                }
            }
            else
            {
                status = PRF_ERR_INVALID_PARAM;
            }
        } break;

        // Write Navigation Characteristic Client Char. Cfg. Descriptor Value
        case (LANC_RD_WR_NAVIGATION_CFG):
        {
            if (p_param->ntfind_cfg <= PRF_CLI_START_NTF)
            {
                *p_handle = p_lanc_env->p_env[conidx]->lans.descs[LANC_DESC_NAVIGATION_CL_CFG].desc_hdl;

                if (*p_handle == ATT_INVALID_SEARCH_HANDLE)
                {
                    // The descriptor has not been found.
                    status = PRF_ERR_INEXISTENT_HDL;
                }
            }
            else
            {
                status = PRF_ERR_INVALID_PARAM;
            }
        }
        break;

        default:
        {
            status = PRF_ERR_INVALID_PARAM;
        } break;
    }

    return (status);
}

uint8_t lanc_unpack_loc_speed_ind (uint8_t conidx, struct gattc_event_ind const *p_param,
        struct lanc_env_tag *p_lanc_env)
{
    // Offset
    uint8_t offset = LANP_LAN_LOC_SPEED_MIN_LEN;

    // LN Measurement value has been received
    struct lanc_value_ind *p_ind = KE_MSG_ALLOC(LANC_VALUE_IND,
                                                prf_dst_task_get(&(p_lanc_env->prf_env), conidx),
                                                prf_src_task_get(&(p_lanc_env->prf_env), conidx),
                                                lanc_value_ind);

    // Attribute code
    p_ind->att_code = LANC_NTF_LOC_SPEED;

    if ((p_param->length >= LANP_LAN_LOC_SPEED_MIN_LEN) &&
            (p_param->length <= LANP_LAN_LOC_SPEED_MAX_LEN))
    {
        // Flags
        p_ind->value.loc_speed.flags = co_read16p(&p_param->value[0]);

        if (GETB(p_ind->value.loc_speed.flags, LANP_LSPEED_INST_SPEED_PRESENT))
        {
            // Unpack instantaneous speed
            p_ind->value.loc_speed.inst_speed = co_read16p(&p_param->value[offset]);
            offset += 2;
        }

        if (GETB(p_ind->value.loc_speed.flags, LANP_LSPEED_TOTAL_DISTANCE_PRESENT))
        {
            // Unpack Total distance (24 bits)
            p_ind->value.loc_speed.total_dist = co_read24p(&p_param->value[offset]);
            offset += 3;
        }

        if (GETB(p_ind->value.loc_speed.flags, LANP_LSPEED_LOCATION_PRESENT))
        {
            // Unpack Location
            p_ind->value.loc_speed.latitude = co_read32p(&p_param->value[offset]);
            offset += 4;
            p_ind->value.loc_speed.longitude = co_read32p(&p_param->value[offset]);
            offset += 4;
        }

        if (GETB(p_ind->value.loc_speed.flags, LANP_LSPEED_ELEVATION_PRESENT))
        {
            // Unpack Elevation (24 bits)
            p_ind->value.loc_speed.elevation = co_read24p(&p_param->value[offset]);
            offset += 3;
        }

        if (GETB(p_ind->value.loc_speed.flags, LANP_LSPEED_HEADING_PRESENT))
        {
            // Unpack heading
            p_ind->value.loc_speed.heading = co_read16p(&p_param->value[offset]);
            offset += 2;
        }

        if (GETB(p_ind->value.loc_speed.flags, LANP_LSPEED_ROLLING_TIME_PRESENT))
        {
            // Unpack rolling time
            p_ind->value.loc_speed.rolling_time = p_param->value[offset];
            offset++;
        }

        if (GETB(p_ind->value.loc_speed.flags, LANP_LSPEED_UTC_TIME_PRESENT))
        {
            //Unpack UTC time
            offset += prf_unpack_date_time(
                    (uint8_t *) &(p_param->value[offset]), &(p_ind->value.loc_speed.date_time));
        }
    }

    // Send the message
    ke_msg_send(p_ind);

    return offset;
}

uint8_t lanc_unpack_navigation_ind (uint8_t conidx, struct gattc_event_ind const *p_param,
        struct lanc_env_tag *p_lanc_env)
{
    // Offset
    uint8_t offset = LANP_LAN_NAVIGATION_MIN_LEN;

    // LN Measurement value has been received
    struct lanc_value_ind *p_ind = KE_MSG_ALLOC(LANC_VALUE_IND,
            prf_dst_task_get(&(p_lanc_env->prf_env), conidx),
            prf_src_task_get(&(p_lanc_env->prf_env), conidx),
            lanc_value_ind);

    // Attribute code
    p_ind->att_code = LANC_NTF_NAVIGATION;

    if ((p_param->length >= LANP_LAN_NAVIGATION_MIN_LEN) &&
            (p_param->length <= LANP_LAN_NAVIGATION_MAX_LEN))
    {
        // Flags
        p_ind->value.navigation.flags = co_read16p(&p_param->value[0]);
        // Bearing
        p_ind->value.navigation.bearing = co_read16p(&p_param->value[2]);
        // Heading
        p_ind->value.navigation.heading = co_read16p(&p_param->value[4]);

        if (GETB(p_ind->value.navigation.flags, LANP_NAVI_REMAINING_DIS_PRESENT))
        {
            //Unpack remaining distance (24 bits)
            p_ind->value.navigation.remaining_distance = co_read24p(&p_param->value[offset]);
            offset += 3;
        }

        if (GETB(p_ind->value.navigation.flags, LANP_NAVI_REMAINING_VER_DIS_PRESENT))
        {
            // Unpack remaining vertical distance (24 bits)
            p_ind->value.navigation.remaining_ver_distance = co_read24p(&p_param->value[offset]);
            offset += 3;
        }

        if (GETB(p_ind->value.navigation.flags, LANP_NAVI_ESTIMATED_TIME_OF_ARRIVAL_PRESENT))
        {
            //Unpack time
            offset += prf_unpack_date_time(
                    (uint8_t *) &(p_param->value[offset]), &(p_ind->value.navigation.estimated_arrival_time));
        }
    }

    // Send the message
    ke_msg_send(p_ind);

    return offset;
}


uint8_t lanc_unpack_pos_q_ind (struct gattc_read_ind const *p_param, struct lanc_value_ind *p_ind)
{
    /*----------------------------------------------------
     * Unpack Position Quality ---------------------------
     *----------------------------------------------------*/
    // Offset
    uint8_t offset = LANP_LAN_POSQ_MIN_LEN;

    // Flags
    p_ind->value.pos_q.flags = co_read16p(&p_param->value[0]);

    if (GETB(p_ind->value.pos_q.flags, LANP_POSQ_NUMBER_OF_BEACONS_IN_SOLUTION_PRESENT))
    {
        //Unpack beacons in solution
        p_ind->value.pos_q.n_beacons_solution = p_param->value[offset];
        offset++;
    }

    if (GETB(p_ind->value.pos_q.flags, LANP_POSQ_NUMBER_OF_BEACONS_IN_VIEW_PRESENT))
    {
        //Unpack beacons in view
        p_ind->value.pos_q.n_beacons_view = p_param->value[offset];
        offset++;
    }

    if (GETB(p_ind->value.pos_q.flags, LANP_POSQ_TIME_TO_FIRST_FIX_PRESENT))
    {
        //Unpack time first fix
        p_ind->value.pos_q.time_first_fix = co_read16p(&p_param->value[offset]);
        offset += 2;
    }

    if (GETB(p_ind->value.pos_q.flags, LANP_POSQ_EHPE_PRESENT))
    {
        //Unpack ehpe
        p_ind->value.pos_q.ehpe = co_read32p(&p_param->value[offset]);
        offset += 4;
    }

    if (GETB(p_ind->value.pos_q.flags, LANP_POSQ_EVPE_PRESENT))
    {
        //Unpack evpe
        p_ind->value.pos_q.evpe = co_read32p(&p_param->value[offset]);
        offset += 4;
    }

    if (GETB(p_ind->value.pos_q.flags, LANP_POSQ_HDOP_PRESENT))
    {
        //Unpack hdop
        p_ind->value.pos_q.hdop = p_param->value[offset];
        offset++;
    }

    if (GETB(p_ind->value.pos_q.flags, LANP_POSQ_VDOP_PRESENT))
    {
        //Unpack vdop
        p_ind->value.pos_q.vdop = p_param->value[offset];
        offset++;
    }

    return offset;
}

uint8_t lanc_pack_ln_ctnl_pt_req (struct lanc_ln_ctnl_pt_cfg_cmd *p_param, uint8_t *p_req, uint8_t *p_status)
{
    // Request Length
    uint8_t req_len = LANP_LAN_LN_CNTL_PT_REQ_MIN_LEN;

    // Set the operation code
    p_req[0] = p_param->ln_ctnl_pt.op_code;

    // Fulfill the message according to the operation code
    switch (p_param->ln_ctnl_pt.op_code)
    {
        case (LANP_LN_CTNL_PT_SET_CUMUL_VALUE):
        {
            // Set the cumulative value (24 bits)
            co_write24p(&p_req[req_len], p_param->ln_ctnl_pt.value.cumul_val);
            // Update length
            req_len += 3;
        } break;

        case (LANP_LN_CTNL_PT_MASK_LSPEED_CHAR_CT):
        {
            // Set mask content
            co_write16p(&p_req[req_len], p_param->ln_ctnl_pt.value.mask_content);
            // Update length
            req_len += 2;
        } break;

        case (LANP_LN_CTNL_PT_NAVIGATION_CONTROL):
        {
            // Set control value
            p_req[req_len] = p_param->ln_ctnl_pt.value.control_value;
            // Update length
            req_len++;
        } break;

        case (LANP_LN_CTNL_PT_REQ_NAME_OF_ROUTE):
        case (LANP_LN_CTNL_PT_SELECT_ROUTE):
        {
            // Set route number
            co_write16p(&p_req[req_len], p_param->ln_ctnl_pt.value.route_number);
            // Update length
            req_len += 2;
        } break;

        case (LANP_LN_CTNL_PT_SET_FIX_RATE):
        {
            // Set the fix rate
            p_req[req_len] = p_param->ln_ctnl_pt.value.fix_rate;
            // Update length
            req_len++;
        } break;

        case (LANP_LN_CTNL_PT_SET_ELEVATION):
        {
            // Set elevation (24 bits)
            co_write24p(&p_req[req_len], p_param->ln_ctnl_pt.value.elevation);
            // Update length
            req_len += 3;
        } break;

        case (LANP_LN_CTNL_PT_REQ_NUMBER_OF_ROUTES):
        {
            // Nothing more to do
        } break;

        default:
        {
            *p_status = PRF_ERR_INVALID_PARAM;
        } break;
    }

    return req_len;
}

uint8_t lanc_unpack_ln_ctln_pt_ind (struct gattc_event_ind const *p_param, ke_task_id_t src, ke_task_id_t dest)
{
    // Offset
    uint8_t offset = LANP_LAN_LN_CNTL_PT_RSP_MIN_LEN;

    // Control Point value has been received
    struct lanc_ln_ctnl_pt_ind *p_ind = KE_MSG_ALLOC_DYN(LANC_LN_CTNL_PT_IND,
                                            dest,
                                            src,
                                            lanc_ln_ctnl_pt_ind,
                                            p_param->length);
    // Response Op code
    p_ind->rsp.op_code = p_param->value[0];
    // Requested operation code
    p_ind->rsp.req_op_code = p_param->value[1];
    // Response value
    p_ind->rsp.resp_value  = p_param->value[2];


    if ((p_ind->rsp.resp_value == LANP_LN_CTNL_PT_RESP_SUCCESS) && (p_param->length >= 3))
    {
        switch (p_ind->rsp.req_op_code)
        {
            case (LANP_LN_CTNL_PT_REQ_NUMBER_OF_ROUTES):
            {
                p_ind->rsp.value.number_of_routes = co_read16p(&p_param->value[offset]);
                offset += 2;

            } break;

            case (LANP_LN_CTNL_PT_REQ_NAME_OF_ROUTE):
            {
                // Get the length of the route
                p_ind->rsp.value.route.length = (p_param->length - 3);

                for (int i = 0; i < p_ind->rsp.value.route.length; i++)
                {
                    p_ind->rsp.value.route.name[i] = p_param->value[i + 3];
                    offset++;
                }
            } break;

            case (LANP_LN_CTNL_PT_SET_CUMUL_VALUE):
            case (LANP_LN_CTNL_PT_MASK_LSPEED_CHAR_CT):
            case (LANP_LN_CTNL_PT_NAVIGATION_CONTROL):
            case (LANP_LN_CTNL_PT_SELECT_ROUTE):
            case (LANP_LN_CTNL_PT_SET_FIX_RATE):
            case (LANP_LN_CTNL_PT_SET_ELEVATION):
            {
                // No parameters
            } break;

            default:
            {

            } break;
        }
    }

    // Send the message
    ke_msg_send(p_ind);

    return offset;
}
#endif //(BLE_LN_COLLECTOR)

/// @} LAN
