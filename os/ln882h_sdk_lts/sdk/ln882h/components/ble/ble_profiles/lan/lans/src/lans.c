/**
 ****************************************************************************************
 *
 * @file lans.c
 *
 * @brief Location and Navigation Profile Sensor implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup LANS
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

#include "gap.h"
#include "gattc_task.h"
#include "gattc.h"
#include "attm.h"
#include "lans.h"
#include "lans_task.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 *  CYCLING POWER SERVICE ATTRIBUTES
 ****************************************************************************************
 */

/// Full LANS Database Description - Used to add attributes into the database
static const struct attm_desc lans_att_db[LNS_IDX_NB] =
{
    // Location and Navigation Service Declaration
    [LNS_IDX_SVC]               = {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

    // LN Feature Characteristic Declaration
    [LNS_IDX_LN_FEAT_CHAR]      = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // LN Feature Characteristic Value
    [LNS_IDX_LN_FEAT_VAL]       = {ATT_CHAR_LN_FEAT, PERM(RD, ENABLE), PERM(RI, ENABLE), sizeof(uint32_t)},

    // Location and Speed Characteristic Declaration
    [LNS_IDX_LOC_SPEED_CHAR]    = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Location and Speed Characteristic Value
    [LNS_IDX_LOC_SPEED_VAL]     = {ATT_CHAR_LOC_SPEED, PERM(NTF, ENABLE), PERM(RI, ENABLE), LANP_LAN_LOC_SPEED_MAX_LEN},
    // Location and Speed Characteristic - Client Characteristic Configuration Descriptor
    [LNS_IDX_LOC_SPEED_NTF_CFG] = {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},

    // Position Quality Characteristic Declaration
    [LNS_IDX_POS_Q_CHAR]        = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Position Quality Characteristic Value
    [LNS_IDX_POS_Q_VAL]         = {ATT_CHAR_POS_QUALITY, PERM(RD, ENABLE), PERM(RI, ENABLE), LANP_LAN_POSQ_MAX_LEN},

    // LN Control Point Characteristic Declaration
    [LNS_IDX_LN_CTNL_PT_CHAR]   = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // LN Control Point Characteristic Value - The response has the maximal length
    [LNS_IDX_LN_CTNL_PT_VAL]    = {ATT_CHAR_LN_CNTL_PT, PERM(IND, ENABLE) | PERM(WRITE_REQ, ENABLE),
                                                             PERM(RI, ENABLE), LANP_LAN_LN_CNTL_PT_RSP_MAX_LEN},
    // LN Control Point Characteristic - Client Characteristic Configuration Descriptor
    [LNS_IDX_LN_CTNL_PT_IND_CFG]= {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},

    // Navigation Characteristic Declaration
    [LNS_IDX_NAVIGATION_CHAR]   = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Navigation Characteristic Value
    [LNS_IDX_NAVIGATION_VAL] = {ATT_CHAR_NAVIGATION, PERM(NTF, ENABLE), PERM(RI, ENABLE), LANP_LAN_NAVIGATION_MAX_LEN},
    // Navigation Characteristic - Client Characteristic Configuration Descriptor
    [LNS_IDX_NAVIGATION_NTF_CFG]= {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the LANS module.
 * This function performs all the initializations of the Profile module.
 *  - Creation of database (if it's a service)
 *  - Allocation of profile required memory
 *  - Initialization of task descriptor to register application
 *      - Task State array
 *      - Number of tasks
 *      - Default task handler
 *
 * @param[out]    p_env         Collector or Service allocated environment data.
 * @param[in|out] p_start_hdl   Service start handle (0 - dynamically allocated), only applies for services.
 * @param[in]     app_task      Application task number.
 * @param[in]     sec_lvl       Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 * @param[in]     p_params      Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 ****************************************************************************************
 */
static uint8_t lans_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl, struct lans_db_cfg *p_params)
{
    //------------------ create the attribute database for the profile -------------------
    // Service Configuration Flag
    uint16_t cfg_flag = LANS_MANDATORY_MASK;
    // Database Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;

    /*
     * Check if Position Quality shall be added.
     */
    if((GETB(p_params->ln_feature, LANP_FEAT_NUMBER_OF_BEACONS_IN_SOLUTION_SUP)) ||
       (GETB(p_params->ln_feature, LANP_FEAT_NUMBER_OF_BEACONS_IN_VIEW_SUP)) ||
       (GETB(p_params->ln_feature, LANP_FEAT_TIME_TO_FIRST_FIX_SUP)) ||
       (GETB(p_params->ln_feature, LANP_FEAT_ESTIMATED_HOR_POSITION_ERROR_SUP)) ||
       (GETB(p_params->ln_feature, LANP_FEAT_ESTIMATED_VER_POSITION_ERROR_SUP)) ||
       (GETB(p_params->ln_feature, LANP_FEAT_HOR_DILUTION_OF_PRECISION_SUP)) ||
       (GETB(p_params->ln_feature, LANP_FEAT_VER_DILUTION_OF_PRECISION_SUP)) ||
       (GETB(p_params->ln_feature, LANP_FEAT_POSITION_STATUS_SUP)))
    {
        //Add configuration to the database
        cfg_flag |= LANS_POS_Q_MASK;
    }

    /*
     * Check if the Navigation characteristic shall be added.
     */
    if ((GETB(p_params->prfl_config, LANS_NAVIGATION_SUPP_FLAG)) ||
        (GETB(p_params->ln_feature, LANP_FEAT_NUMBER_OF_BEACONS_IN_SOLUTION_SUP)) ||
        (GETB(p_params->ln_feature, LANP_FEAT_REMAINING_DISTANCE_SUP)) ||
        (GETB(p_params->ln_feature, LANP_FEAT_REMAINING_VERTICAL_DISTANCE_SUP)) ||
        (GETB(p_params->ln_feature, LANP_FEAT_ESTIMATED_TIME_OF_ARRIVAL_SUP)) ||
        (GETB(p_params->ln_feature, LANP_FEAT_POSITION_STATUS_SUP)))
    {
        cfg_flag |= LANS_NAVI_MASK;
    }
    /*
     * Check if the LN Control Point characteristic shall be added
     */
    if ((LANS_IS_FEATURE_SUPPORTED(cfg_flag, LANS_NAVI_MASK)) ||
        (GETB(p_params->prfl_config, LANS_CTNL_PT_CHAR_SUPP_FLAG)) ||
        (GETB(p_params->ln_feature, LANP_FEAT_TOTAL_DISTANCE_SUP)) ||
        (GETB(p_params->ln_feature, LANP_FEAT_LSPEED_CHAR_CT_MASKING_SUP)) ||
        (GETB(p_params->ln_feature, LANP_FEAT_FIX_RATE_SETTING_SUP)) ||
        ((GETB(p_params->ln_feature, LANP_FEAT_ELEVATION_SUP)) &&
        (GETB(p_params->ln_feature, LANP_FEAT_ELEVATION_SETTING_SUP))))
    {
        cfg_flag |= LANS_LN_CTNL_PT_MASK;
    }

    // Add service in the database
    status = attm_svc_create_db(p_start_hdl, ATT_SVC_LOCATION_AND_NAVIGATION, (uint8_t *)&cfg_flag,
            LNS_IDX_NB, NULL, p_env->task, &lans_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_MI, DISABLE));

    //-------------------- allocate memory required for the profile  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {
        // Allocate LANS required environment variable
        struct lans_env_tag* p_lans_env =
                (struct lans_env_tag* ) ke_malloc(sizeof(struct lans_env_tag), KE_MEM_ATT_DB);

        // Initialize LANS environment
        p_env->env           = (prf_env_t*) p_lans_env;
        p_lans_env->shdl     = *p_start_hdl;
        p_lans_env->prfl_cfg = cfg_flag;
        p_lans_env->features = p_params->ln_feature;
        p_lans_env->operation = LANS_RESERVED_OP_CODE;
        p_lans_env->p_posq = (LANS_IS_FEATURE_SUPPORTED(cfg_flag, LANS_POS_Q_MASK)) ?
                (struct lanp_posq *) ke_malloc(sizeof(struct lanp_posq), KE_MEM_ATT_DB) : NULL;

        p_lans_env->p_op_data = NULL;
        memset(p_lans_env->env, 0, sizeof(p_lans_env->env));

        p_lans_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        // Mono Instantiated task
        p_lans_env->prf_env.prf_task    = p_env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        p_env->id = TASK_ID_LANS;
        lans_task_init(&(p_env->desc));

        // Put CPS in Idle state
        ke_state_set(p_env->task, LANS_IDLE);
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the LANS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void lans_destroy(struct prf_task_env *p_env)
{
    struct lans_env_tag *p_lans_env = (struct lans_env_tag *) p_env->env;

    // free profile environment variables
    p_env->env = NULL;

    if (p_lans_env->p_posq != NULL)
    {
        ke_free(p_lans_env->p_posq);
    }

    if (p_lans_env->p_op_data != NULL)
    {
        ke_free(p_lans_env->p_op_data->p_cmd);

        if(p_lans_env->p_op_data->p_ntf_pending)
        {
            ke_free(p_lans_env->p_op_data->p_ntf_pending);
        }

        ke_free(p_lans_env->p_op_data);
    }

    ke_free(p_lans_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void lans_create(struct prf_task_env *p_env, uint8_t conidx)
{
    struct lans_env_tag *p_lans_env = (struct lans_env_tag *) p_env->env;

    memset(&(p_lans_env->env[conidx]), 0, sizeof(struct lans_cnx_env));
}

/**
 ****************************************************************************************
 * @brief Handles Disconnection
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void lans_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct lans_env_tag *p_lans_env = (struct lans_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    memset(&(p_lans_env->env[conidx]), 0, sizeof(struct lans_cnx_env));
}

/// LANS Task interface required by profile manager
const struct prf_task_cbs lans_itf =
{
    (prf_init_fnct) lans_init,
    lans_destroy,
    lans_create,
    lans_cleanup,
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* lans_prf_itf_get(void)
{
   return &lans_itf;
}

void lans_send_cmp_evt(uint8_t conidx, uint8_t src_id, uint8_t dest_id, uint8_t operation, uint8_t status)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);
    struct lans_cmp_evt *p_evt;

    // Go back to the Connected state if the state is busy
    if (ke_state_get(src_id) == LANS_BUSY)
    {
        ke_state_set(src_id, LANS_IDLE);
    }

    // Set the operation code
    p_lans_env->operation = LANS_RESERVED_OP_CODE;

    // Send the message
    p_evt = KE_MSG_ALLOC(LANS_CMP_EVT,
                         dest_id, src_id,
                         lans_cmp_evt);

    p_evt->conidx     = conidx;
    p_evt->operation  = operation;
    p_evt->status     = status;

    ke_msg_send(p_evt);
}

uint8_t lans_pack_loc_speed_ntf(struct lanp_loc_speed *p_param, uint8_t *p_pckd_loc_speed)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);
    // Packed Measurement length
    uint8_t pckd_loc_speed_len = LANP_LAN_LOC_SPEED_MIN_LEN;

    // Check provided flags
    if (GETB(p_param->flags, LANP_LSPEED_INST_SPEED_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_INSTANTANEOUS_SPEED_SUP))
        {
            //Pack instantaneous speed
            co_write16p(&p_pckd_loc_speed[pckd_loc_speed_len], p_param->inst_speed);
            pckd_loc_speed_len += 2;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_param->flags, LANP_LSPEED_INST_SPEED_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, LANP_LSPEED_TOTAL_DISTANCE_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_TOTAL_DISTANCE_SUP))
        {
            //Pack total distance (24bits)
            co_write24p(&p_pckd_loc_speed[pckd_loc_speed_len], p_param->total_dist);
            pckd_loc_speed_len += 3;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_param->flags, LANP_LSPEED_TOTAL_DISTANCE_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, LANP_LSPEED_LOCATION_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_LOCATION_SUP))
        {
            //Pack Location
            co_write32p(&p_pckd_loc_speed[pckd_loc_speed_len], p_param->latitude);
            pckd_loc_speed_len += 4;
            co_write32p(&p_pckd_loc_speed[pckd_loc_speed_len], p_param->longitude);
            pckd_loc_speed_len += 4;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_param->flags, LANP_LSPEED_LOCATION_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, LANP_LSPEED_ELEVATION_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_ELEVATION_SUP))
        {
            //Pack elevation (24 bits)
            co_write24p(&p_pckd_loc_speed[pckd_loc_speed_len], p_param->elevation);
            pckd_loc_speed_len += 3;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_param->flags, LANP_LSPEED_ELEVATION_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, LANP_LSPEED_HEADING_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_HEADING_SUP))
        {
            //Pack Extreme Force Magnitudes (Maximum Force Magnitude & Minimum Force Magnitude)
            co_write16p(&p_pckd_loc_speed[pckd_loc_speed_len], p_param->heading);
            pckd_loc_speed_len += 2;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_param->flags, LANP_LSPEED_HEADING_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, LANP_LSPEED_ROLLING_TIME_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_ROLLING_TIME_SUP))
        {
            //Pack rolling time
            p_pckd_loc_speed[pckd_loc_speed_len] = p_param->rolling_time;
            pckd_loc_speed_len++;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_param->flags, LANP_LSPEED_ROLLING_TIME_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, LANP_LSPEED_UTC_TIME_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_UTC_TIME_SUP))
        {
            // Pack UTC time
            pckd_loc_speed_len += prf_pack_date_time(&p_pckd_loc_speed[pckd_loc_speed_len], &(p_param->date_time));

        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_param->flags, LANP_LSPEED_UTC_TIME_PRESENT, 0);
        }
    }

    // Flags value
    co_write16p(&p_pckd_loc_speed[0], p_param->flags);

    return pckd_loc_speed_len;
}

uint8_t lans_split_loc_speed_ntf(uint8_t conidx, struct gattc_send_evt_cmd *p_loc_speed_ntf1)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);
    // Extract flags info
    uint16_t flags = co_read16p(&p_loc_speed_ntf1->value[0]);
    // Allocate the GATT notification message
    struct gattc_send_evt_cmd *p_loc_speed_ntf2 = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KE_BUILD_ID(TASK_GATTC, conidx), prf_src_task_get(&(p_lans_env->prf_env), conidx),
            gattc_send_evt_cmd, LANP_LAN_LOC_SPEED_MAX_LEN);
    // Current position
    uint8_t len = 0;

    // Fill in the parameter structure
    p_loc_speed_ntf2->operation = GATTC_NOTIFY;
    p_loc_speed_ntf2->handle = LANS_HANDLE(LNS_IDX_LOC_SPEED_VAL);
    p_loc_speed_ntf2->length = LANP_LAN_LOC_SPEED_MIN_LEN;

    // Copy status flags
    co_write16p(&p_loc_speed_ntf2->value[0], (flags & (LANP_LSPEED_POSITION_STATUS_LSB_BIT |
                                LANP_LSPEED_POSITION_STATUS_MSB_BIT |
                                LANP_LSPEED_SPEED_AND_DISTANCE_FORMAT_BIT |
                                LANP_LSPEED_ELEVATION_SOURCE_LSB_BIT |
                                LANP_LSPEED_ELEVATION_SOURCE_MSB_BIT |
                                LANP_LSPEED_HEADING_SOURCE_BIT)));

    for (uint16_t feat = LANP_LSPEED_INST_SPEED_PRESENT_BIT;
            feat <= LANP_LSPEED_POSITION_STATUS_LSB_BIT;
            feat <<= 1)
    {
        // First message fits within the MTU
        if (p_loc_speed_ntf1->length <= gattc_get_mtu(conidx) - 3)
        {
            // Stop splitting
            break;
        }

        if ((flags & feat) != feat)
        {
            continue;
        }

        switch (feat)
        {
            case LANP_LSPEED_ROLLING_TIME_PRESENT_BIT:
            {
                // Copy uint8
                p_loc_speed_ntf2->value[p_loc_speed_ntf2->length] =
                        p_loc_speed_ntf1->value[LANP_LAN_LOC_SPEED_MIN_LEN];
                len = 1;
            } break;

            case LANP_LSPEED_INST_SPEED_PRESENT_BIT:
            case LANP_LSPEED_HEADING_PRESENT_BIT:
            {
                // Copy uint16
                memcpy(&p_loc_speed_ntf2->value[p_loc_speed_ntf2->length],
                        &p_loc_speed_ntf1->value[LANP_LAN_LOC_SPEED_MIN_LEN],
                        2);
                len = 2;
            } break;

            case LANP_LSPEED_TOTAL_DISTANCE_PRESENT_BIT:
            case LANP_LSPEED_ELEVATION_PRESENT_BIT:
            {
                // Copy uint24
                memcpy(&p_loc_speed_ntf2->value[p_loc_speed_ntf2->length],
                        &p_loc_speed_ntf1->value[LANP_LAN_LOC_SPEED_MIN_LEN],
                        3);
                len = 3;
            } break;

            case LANP_LSPEED_LOCATION_PRESENT_BIT:
            {
                // Copy latitude and longitude
                memcpy(&p_loc_speed_ntf2->value[p_loc_speed_ntf2->length],
                        &p_loc_speed_ntf1->value[LANP_LAN_LOC_SPEED_MIN_LEN],
                        8);
                len = 8;
            } break;

            case LANP_LSPEED_UTC_TIME_PRESENT_BIT:
            {
                // Copy time
                memcpy(&p_loc_speed_ntf2->value[p_loc_speed_ntf2->length],
                        &p_loc_speed_ntf1->value[LANP_LAN_LOC_SPEED_MIN_LEN],
                        7);
                len = 7;
            } break;

            default:
            {
                len = 0;
            } break;
        }

        if (len)
        {
            // Update values
            p_loc_speed_ntf2->length += len;
            // Remove field and flags from the first ntf
            p_loc_speed_ntf1->length -= len;
            memcpy(&p_loc_speed_ntf1->value[LANP_LAN_LOC_SPEED_MIN_LEN],
                    &p_loc_speed_ntf1->value[LANP_LAN_LOC_SPEED_MIN_LEN + len],
                    p_loc_speed_ntf1->length);
            // Update flags
            p_loc_speed_ntf1->value[0] &= ~feat;
            p_loc_speed_ntf2->value[0] |= feat;
        }
    }

    // store the pending notification to send
    p_lans_env->p_op_data->p_ntf_pending = p_loc_speed_ntf2;

    return p_loc_speed_ntf2->length;
}

uint8_t lans_update_characteristic_config(uint8_t conidx, uint8_t prfl_config,
        struct gattc_write_req_ind const *p_param)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Get the value
    uint16_t ntf_cfg = co_read16p(&p_param->value[0]);

    // Check if the value is correct
    if (GETB(prfl_config, LANS_PRF_CFG_FLAG_LN_CTNL_PT_IND))
    {
        if (ntf_cfg == PRF_CLI_STOP_NTFIND)
        {
            // Save the new configuration in the environment
            SETB(p_lans_env->env[conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_LN_CTNL_PT_IND, 0);
        }
        else if (ntf_cfg == PRF_CLI_START_IND)
        {
            SETB(p_lans_env->env[conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_LN_CTNL_PT_IND, 1);
        }
        else
        {
            status = PRF_ERR_INVALID_PARAM;
        }
    }
    else if (GETB(prfl_config, LANS_PRF_CFG_FLAG_LOC_SPEED_NTF))
    {
        if (ntf_cfg == PRF_CLI_STOP_NTFIND)
        {
            // Save the new configuration in the environment
            SETB(p_lans_env->env[conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_LOC_SPEED_NTF, 0);
        }
        else if (ntf_cfg == PRF_CLI_START_NTF)
        {
            SETB(p_lans_env->env[conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_LOC_SPEED_NTF, 1);
        }
        else
        {
            status = PRF_ERR_INVALID_PARAM;
        }
    }
    else if (GETB(prfl_config, LANS_PRF_CFG_FLAG_NAVIGATION_NTF))
    {
        if (ntf_cfg == PRF_CLI_STOP_NTFIND)
        {
            // Save the new configuration in the environment
            SETB(p_lans_env->env[conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_NAVIGATION_NTF, 0);
        }
        else if (ntf_cfg == PRF_CLI_START_NTF)
        {
            SETB(p_lans_env->env[conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_NAVIGATION_NTF, 1);
        }
        else
        {
            status = PRF_ERR_INVALID_PARAM;
        }
    }

    if (status == GAP_ERR_NO_ERROR)
    {
        // Allocate message to inform application
        struct lans_cfg_ntfind_ind *p_ind = KE_MSG_ALLOC(LANS_CFG_NTFIND_IND,
                                                         prf_dst_task_get(&(p_lans_env->prf_env), conidx),
                                                         prf_src_task_get(&(p_lans_env->prf_env), conidx),
                                                         lans_cfg_ntfind_ind);

        // Inform APP of configuration change
        p_ind->char_code = prfl_config;
        p_ind->ntf_cfg   = ntf_cfg;
        p_ind->conidx = conidx;
        ke_msg_send(p_ind);

        // Enable Bonded Data
        SETB(p_lans_env->env[conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_PERFORMED_OK, 1);
    }

    return (status);
}

uint8_t lans_pack_navigation_ntf(struct lanp_navigation *p_param, uint8_t *p_pckd_navigation)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);
    // Packed Measurement length
    uint8_t pckd_navigation_len = LANP_LAN_NAVIGATION_MIN_LEN;

    // Check provided flags
    if (GETB(p_param->flags, LANP_NAVI_REMAINING_DIS_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_REMAINING_DISTANCE_SUP))
        {
            //Pack distance (24bits)
            co_write24p(&p_pckd_navigation[pckd_navigation_len], p_param->remaining_distance);
            pckd_navigation_len += 3;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_param->flags, LANP_NAVI_REMAINING_DIS_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, LANP_NAVI_REMAINING_VER_DIS_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_REMAINING_VERTICAL_DISTANCE_SUP))
        {
            //Pack vertical distance (24bits)
            co_write24p(&p_pckd_navigation[pckd_navigation_len], p_param->remaining_ver_distance);
            pckd_navigation_len += 3;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_param->flags, LANP_NAVI_REMAINING_VER_DIS_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, LANP_NAVI_ESTIMATED_TIME_OF_ARRIVAL_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_ESTIMATED_TIME_OF_ARRIVAL_SUP))
        {
            //Pack time
            pckd_navigation_len += prf_pack_date_time(
                    &p_pckd_navigation[pckd_navigation_len], &(p_param->estimated_arrival_time));

        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_param->flags, LANP_NAVI_ESTIMATED_TIME_OF_ARRIVAL_PRESENT, 0);
        }
    }

    // Flags value
    co_write16p(&p_pckd_navigation[0], p_param->flags);
    // Bearing value
    co_write16p(&p_pckd_navigation[2], p_param->bearing);
    // heading value
    co_write16p(&p_pckd_navigation[4], p_param->heading);

    return pckd_navigation_len;
}

void lans_upd_posq(struct lanp_posq param)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);

    // Check provided flags
    if (GETB(param.flags, LANP_POSQ_NUMBER_OF_BEACONS_IN_SOLUTION_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_NUMBER_OF_BEACONS_IN_SOLUTION_SUP))
        {
            //Pack beacons in solution
            p_lans_env->p_posq->n_beacons_solution = param.n_beacons_solution;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(param.flags, LANP_POSQ_NUMBER_OF_BEACONS_IN_SOLUTION_PRESENT, 0);
        }
    }

    if (GETB(param.flags, LANP_POSQ_NUMBER_OF_BEACONS_IN_VIEW_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_NUMBER_OF_BEACONS_IN_VIEW_SUP))
        {
            //Pack beacons in view
            p_lans_env->p_posq->n_beacons_view = param.n_beacons_view;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(param.flags, LANP_POSQ_NUMBER_OF_BEACONS_IN_VIEW_PRESENT, 0);
        }
    }

    if (GETB(param.flags, LANP_POSQ_TIME_TO_FIRST_FIX_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_TIME_TO_FIRST_FIX_SUP))
        {
            //Pack time to fix
            p_lans_env->p_posq->time_first_fix = param.time_first_fix;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(param.flags, LANP_POSQ_TIME_TO_FIRST_FIX_PRESENT, 0);
        }
    }

    if (GETB(param.flags, LANP_POSQ_EHPE_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_ESTIMATED_HOR_POSITION_ERROR_SUP))
        {
            //Pack ehpe
            p_lans_env->p_posq->ehpe = param.ehpe;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(param.flags, LANP_POSQ_EHPE_PRESENT, 0);
        }
    }

    if (GETB(param.flags, LANP_POSQ_EVPE_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_ESTIMATED_VER_POSITION_ERROR_SUP))
        {
            //Pack evpe
            p_lans_env->p_posq->evpe = param.evpe;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(param.flags, LANP_POSQ_EVPE_PRESENT, 0);
        }
    }

    if (GETB(param.flags, LANP_POSQ_HDOP_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_HOR_DILUTION_OF_PRECISION_SUP))
        {
            //Pack hdop
            p_lans_env->p_posq->hdop = param.hdop;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(param.flags, LANP_POSQ_HDOP_PRESENT, 0);
        }
    }

    if (GETB(param.flags, LANP_POSQ_VDOP_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_VER_DILUTION_OF_PRECISION_SUP))
        {
            //Pack vdop
            p_lans_env->p_posq->vdop = param.vdop;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(param.flags, LANP_POSQ_VDOP_PRESENT, 0);
        }
    }

    // Flags value
    p_lans_env->p_posq->flags = param.flags;
}

uint8_t lans_pack_posq(uint8_t *p_pckd_posq)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);

    // Packed length
    uint8_t pckd_posq_len = LANP_LAN_POSQ_MIN_LEN;

    // Check provided flags
    if (GETB(p_lans_env->p_posq->flags, LANP_POSQ_NUMBER_OF_BEACONS_IN_SOLUTION_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_NUMBER_OF_BEACONS_IN_SOLUTION_SUP))
        {
            //Pack beacons in solution
            p_pckd_posq[pckd_posq_len] = p_lans_env->p_posq->n_beacons_solution;
            pckd_posq_len++;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_lans_env->p_posq->flags, LANP_POSQ_NUMBER_OF_BEACONS_IN_SOLUTION_PRESENT, 0);
        }
    }

    if (GETB(p_lans_env->p_posq->flags, LANP_POSQ_NUMBER_OF_BEACONS_IN_VIEW_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_NUMBER_OF_BEACONS_IN_VIEW_SUP))
        {
            //Pack beacons in view
            p_pckd_posq[pckd_posq_len] = p_lans_env->p_posq->n_beacons_view;
            pckd_posq_len++;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_lans_env->p_posq->flags, LANP_POSQ_NUMBER_OF_BEACONS_IN_VIEW_PRESENT, 0);
        }
    }

    if (GETB(p_lans_env->p_posq->flags, LANP_POSQ_TIME_TO_FIRST_FIX_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_TIME_TO_FIRST_FIX_SUP))
        {
            //Pack time to fix
            co_write16p(&p_pckd_posq[pckd_posq_len], p_lans_env->p_posq->time_first_fix);
            pckd_posq_len += 2;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_lans_env->p_posq->flags, LANP_POSQ_TIME_TO_FIRST_FIX_PRESENT, 0);
        }
    }

    if (GETB(p_lans_env->p_posq->flags, LANP_POSQ_EHPE_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_ESTIMATED_HOR_POSITION_ERROR_SUP))
        {
            //Pack ehpe
            co_write32p(&p_pckd_posq[pckd_posq_len], p_lans_env->p_posq->ehpe);
            pckd_posq_len += 4;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_lans_env->p_posq->flags, LANP_POSQ_EHPE_PRESENT, 0);
        }
    }

    if (GETB(p_lans_env->p_posq->flags, LANP_POSQ_EVPE_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_ESTIMATED_VER_POSITION_ERROR_SUP))
        {
            //Pack ehpe
            co_write32p(&p_pckd_posq[pckd_posq_len], p_lans_env->p_posq->evpe);
            pckd_posq_len += 4;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_lans_env->p_posq->flags, LANP_POSQ_EVPE_PRESENT, 0);
        }
    }

    if (GETB(p_lans_env->p_posq->flags, LANP_POSQ_HDOP_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_HOR_DILUTION_OF_PRECISION_SUP))
        {
            //Pack ehpe
            p_pckd_posq[pckd_posq_len] = p_lans_env->p_posq->hdop;
            pckd_posq_len++;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_lans_env->p_posq->flags, LANP_POSQ_HDOP_PRESENT, 0);
        }
    }

    if (GETB(p_lans_env->p_posq->flags, LANP_POSQ_VDOP_PRESENT))
    {
        if (GETB(p_lans_env->features, LANP_FEAT_VER_DILUTION_OF_PRECISION_SUP))
        {
            //Pack ehpe
            p_pckd_posq[pckd_posq_len] = p_lans_env->p_posq->vdop;
            pckd_posq_len++;
        }
        else //Not supported by the profile
        {
            //Force to not supported
            SETB(p_lans_env->p_posq->flags, LANP_POSQ_VDOP_PRESENT, 0);
        }
    }

    // Flags value
    co_write16p(&p_pckd_posq[0], p_lans_env->p_posq->flags);

    return pckd_posq_len;
}

uint8_t lans_pack_ln_ctnl_point_cfm (uint8_t conidx, struct lans_ln_ctnl_pt_cfm *p_param, uint8_t *p_rsp)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);
    // Response Length (At least 3)
    uint8_t rsp_len = LANP_LAN_LN_CNTL_PT_RSP_MIN_LEN;

    // Set the Response Code
    p_rsp[0] = LANP_LN_CTNL_PT_RESPONSE_CODE;
    // Set the Response Value
    p_rsp[2] = (p_param->status > LANP_LN_CTNL_PT_RESP_FAILED) ? LANP_LN_CTNL_PT_RESP_FAILED : p_param->status;

    switch (p_lans_env->operation)
    {
        case (LANS_SET_CUMUL_VALUE_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = LANP_LN_CTNL_PT_SET_CUMUL_VALUE;
        } break;

        case (LANS_MASK_LSPEED_CHAR_CT_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = LANP_LN_CTNL_PT_MASK_LSPEED_CHAR_CT;
            if (p_param->status == LANP_LN_CTNL_PT_RESP_SUCCESS)
            {
                // Ignore bits other than 0 - 6
                p_lans_env->env[conidx].mask_lspeed_content =
                        p_param->value.mask_lspeed_content & LANS_LSPEED_DISABLED_MASK;
            }
        } break;

        case (LANS_NAVIGATION_CONTROL_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = LANP_LN_CTNL_PT_NAVIGATION_CONTROL;
            if (p_param->status == LANP_LN_CTNL_PT_RESP_SUCCESS)
            {
                switch((p_lans_env->env[conidx].nav_ctrl & LANS_NAV_CTRL_PT_VAL_MASK))
                {
                    // Disable notifications
                    case LANP_LN_CTNL_STOP_NAVI:
                    case LANP_LN_CTNL_PAUSE_NAVI:
                    {
                        SETB(p_lans_env->env[conidx].nav_ctrl, LANS_NAV_CTRL_PT_EN, 0);
                    } break;

                    // Enable notifications
                    case LANP_LN_CTNL_START_NAVI:
                    case LANP_LN_CTNL_RESUME_NAVI:
                    case LANP_LN_CTNL_START_NST_WPT:
                    {
                        SETB(p_lans_env->env[conidx].nav_ctrl, LANS_NAV_CTRL_PT_EN, 1);
                    } break;

                    // Do nothing
                    case LANP_LN_CTNL_SKIP_WPT:
                    default:
                    {

                    } break;
                }
            }
        } break;

        case (LANS_REQ_NUMBER_OF_ROUTES_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = LANP_LN_CTNL_PT_REQ_NUMBER_OF_ROUTES;
            if (p_param->status == LANP_LN_CTNL_PT_RESP_SUCCESS)
            {
                co_write16p(&p_rsp[rsp_len], p_param->value.number_of_routes);
                rsp_len += 2;
            }
        } break;

        case (LANS_REQ_NAME_OF_ROUTE_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = LANP_LN_CTNL_PT_REQ_NAME_OF_ROUTE;
            if (p_param->status == LANP_LN_CTNL_PT_RESP_SUCCESS)
            {
                // pack name of route
                for (int i = 0; i < p_param->value.route.length; i++)
                {
                    p_rsp[i + 3] = p_param->value.route.name[i];
                    rsp_len++;
                }
            }
        } break;

        case (LANS_SELECT_ROUTE_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = LANP_LN_CTNL_PT_SELECT_ROUTE;
        } break;

        case (LANS_SET_FIX_RATE_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = LANP_LN_CTNL_PT_SET_FIX_RATE;
        } break;

        case (LANS_SET_ELEVATION_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = LANP_LN_CTNL_PT_SET_ELEVATION;
        } break;

        default:
        {
            p_param->status = LANP_LN_CTNL_PT_RESP_NOT_SUPP;
        } break;
    }

    return rsp_len;
}

uint8_t lans_unpack_ln_ctnl_point_ind (uint8_t conidx, struct gattc_write_req_ind const *p_param)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);

    // Indication Status
    uint8_t ind_status = LANP_LN_CTNL_PT_RESP_NOT_SUPP;

    // Allocate a request indication message for the application
    struct lans_ln_ctnl_pt_req_ind *p_req_ind = KE_MSG_ALLOC(LANS_LN_CTNL_PT_REQ_IND,
                                                            prf_dst_task_get(&(p_lans_env->prf_env), conidx),
                                                            prf_src_task_get(&(p_lans_env->prf_env), conidx),
                                                            lans_ln_ctnl_pt_req_ind);
    // Operation Code
    p_req_ind->op_code = p_param->value[0];
    p_req_ind->conidx = conidx;

    // Operation Code
    switch(p_req_ind->op_code)
    {
        case (LANP_LN_CTNL_PT_SET_CUMUL_VALUE):
        {
            // Check if total distance feature is supported
            if (GETB(p_lans_env->features, LANP_FEAT_TOTAL_DISTANCE_SUP))
            {
                // Provided parameter in not within the defined range
                ind_status = LANP_LN_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 4)
                {
                    // The request can be handled
                    ind_status = LANP_LN_CTNL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_lans_env->operation = LANS_SET_CUMUL_VALUE_OP_CODE;
                    // Cumulative value
                    p_req_ind->value.cumul_val = co_read24p(&p_param->value[1]);
                }
            }
        } break;

        case (LANP_LN_CTNL_PT_MASK_LSPEED_CHAR_CT):
        {
            // Check if the LN Masking feature is supported
            if (GETB(p_lans_env->features, LANP_FEAT_LSPEED_CHAR_CT_MASKING_SUP))
            {
                // Provided parameter in not within the defined range
                ind_status = LANP_LN_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 3)
                {
                    // The request can be handled
                    ind_status = LANP_LN_CTNL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_lans_env->operation = LANS_MASK_LSPEED_CHAR_CT_OP_CODE;
                    // Mask content
                    p_req_ind->value.mask_content = co_read16p(&p_param->value[1]);
                }
            }
        } break;

        case (LANP_LN_CTNL_PT_NAVIGATION_CONTROL):
        {
            // Check if navigation feature is supported
            if ((LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_NAVI_MASK)) &&
                 (GETB(p_lans_env->env[conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_NAVIGATION_NTF)))
            {
                // Provided parameter in not within the defined range
                ind_status = LANP_LN_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 2)
                {
                    if (p_param->value[1] <= 0x05)
                    {
                        // The request can be handled
                        ind_status = LANP_LN_CTNL_PT_RESP_SUCCESS;
                        // Update the environment
                        p_lans_env->operation = LANS_NAVIGATION_CONTROL_OP_CODE;
                        // Control value
                        p_req_ind->value.control_value = p_param->value[1];
                        // Store value in the environment
                        p_lans_env->env[conidx].nav_ctrl = (p_lans_env->env[conidx].nav_ctrl & LANS_NAV_CTRL_PT_EN_BIT)
                                | (p_req_ind->value.control_value & LANS_NAV_CTRL_PT_VAL_MASK);
                    }
                }
            }
        } break;

        case (LANP_LN_CTNL_PT_REQ_NUMBER_OF_ROUTES):
        {
            // Check if navigation feature is supported
            if (LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_NAVI_MASK))
            {
                // The request can be handled
                ind_status = LANP_LN_CTNL_PT_RESP_SUCCESS;
                // Update the environment
                p_lans_env->operation = LANS_REQ_NUMBER_OF_ROUTES_OP_CODE;
            }
        } break;

        case (LANP_LN_CTNL_PT_REQ_NAME_OF_ROUTE):
        {
            // Check if navigation feature is supported
            if (LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_NAVI_MASK))
            {
                // Provided parameter in not within the defined range
                ind_status = LANP_LN_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 3)
                {
                    // The request can be handled
                    ind_status = LANP_LN_CTNL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_lans_env->operation = LANS_REQ_NAME_OF_ROUTE_OP_CODE;
                    // Route number
                    p_req_ind->value.route_number = co_read16p(&p_param->value[1]);
                }
            }
        } break;

        case (LANP_LN_CTNL_PT_SELECT_ROUTE):
        {
            // Check if navigation feature is supported
            if (LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_NAVI_MASK))
            {
                // Provided parameter in not within the defined range
                ind_status = LANP_LN_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 3)
                {
                    // The request can be handled
                    ind_status = LANP_LN_CTNL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_lans_env->operation = LANS_SELECT_ROUTE_OP_CODE;
                    // route number
                    p_req_ind->value.route_number = co_read16p(&p_param->value[1]);
                }
            }
        } break;

        case (LANP_LN_CTNL_PT_SET_FIX_RATE):
        {
            // Check if the LN Masking feature is supported
            if (GETB(p_lans_env->features, LANP_FEAT_FIX_RATE_SETTING_SUP))
            {
                // Provided parameter in not within the defined range
                ind_status = LANP_LN_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 2)
                {
                    // The request can be handled
                    ind_status = LANP_LN_CTNL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_lans_env->operation = LANS_SET_FIX_RATE_OP_CODE;
                    // fix rate
                    p_req_ind->value.fix_rate = p_param->value[1];
                }
            }
        } break;

        case (LANP_LN_CTNL_PT_SET_ELEVATION):
        {
            // Check if the Chain Weight Adjustment feature is supported
            if (GETB(p_lans_env->features, LANP_FEAT_ELEVATION_SETTING_SUP))
            {
                // Provided parameter in not within the defined range
                ind_status = LANP_LN_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 4)
                {
                    // The request can be handled
                    ind_status = LANP_LN_CTNL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_lans_env->operation = LANS_SET_ELEVATION_OP_CODE;
                    // elevation
                    p_req_ind->value.elevation = co_read24p(&p_param->value[1]);
                }
            }
        } break;

        default:
        {
            // Operation Code is invalid, status is already LAN_CTNL_PT_RESP_NOT_SUPP
        } break;
    }

    // If no error raised, inform the application about the request
    if (ind_status == LANP_LN_CTNL_PT_RESP_SUCCESS)
    {
        // Send the request indication to the application
        ke_msg_send(p_req_ind);
        // Go to the Busy status
        ke_state_set(prf_src_task_get(&(p_lans_env->prf_env), conidx), LANS_BUSY);
    }
    else
    {
        // Free the allocated message
        ke_msg_free(ke_param2msg(p_req_ind));
    }

    return ind_status;
}

void lans_exe_operation(uint8_t conidx)
{
    struct lans_env_tag* p_lans_env = PRF_ENV_GET(LANS, lans);

    ASSERT_ERR(p_lans_env->p_op_data != NULL);

    bool finished = true;

    while ((p_lans_env->p_op_data->cursor < BLE_CONNECTION_MAX) && finished)
    {
        uint8_t cursor = p_lans_env->p_op_data->cursor;

        switch (p_lans_env->operation)
        {
            case LANS_NTF_LOC_SPEED_OP_CODE:
            {
                // notification is pending, send it first
                if (p_lans_env->p_op_data->p_ntf_pending != NULL)
                {
                    ke_msg_send(p_lans_env->p_op_data->p_ntf_pending);
                    p_lans_env->p_op_data->p_ntf_pending = NULL;
                    finished = false;
                }
                // Check if sending of notifications has been enabled
                else if (GETB(p_lans_env->env[cursor].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_LOC_SPEED_NTF))
                {
                    struct gattc_send_evt_cmd *p_meas_val;

                    struct lans_ntf_loc_speed_cmd *p_speed_cmd =
                            (struct lans_ntf_loc_speed_cmd *) ke_msg2param(p_lans_env->p_op_data->p_cmd);

                    // Save flags value
                    uint16_t flags = p_speed_cmd->parameters.flags;

                    // Mask unwanted fields if supported
                    if (GETB(p_lans_env->features, LANP_FEAT_LSPEED_CHAR_CT_MASKING_SUP))
                    {
                        p_speed_cmd->parameters.flags &= ~p_lans_env->env[cursor].mask_lspeed_content;
                    }

                    // Allocate the GATT notification message
                    p_meas_val = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                            KE_BUILD_ID(TASK_GATTC, cursor), prf_src_task_get(&(p_lans_env->prf_env),cursor),
                            gattc_send_evt_cmd, LANP_LAN_LOC_SPEED_MAX_LEN);

                    // Fill in the parameter structure
                    p_meas_val->operation = GATTC_NOTIFY;
                    p_meas_val->handle = LANS_HANDLE(LNS_IDX_LOC_SPEED_VAL);
                    // pack measured value in database
                    p_meas_val->length = lans_pack_loc_speed_ntf(&p_speed_cmd->parameters, p_meas_val->value);

                    if (p_meas_val->length > gattc_get_mtu(cursor) - 3)
                    {
                        // Split (if necessary)
                        lans_split_loc_speed_ntf(cursor, p_meas_val);
                    }

                    // Restore flags value
                    p_speed_cmd->parameters.flags = flags;

                    // Send the event
                    ke_msg_send(p_meas_val);

                    finished = false;
                }

                // update cursor only if all notification has been sent
                if (p_lans_env->p_op_data->p_ntf_pending == NULL)
                {
                    p_lans_env->p_op_data->cursor++;
                }
            } break;

            case LANS_NTF_NAVIGATION_OP_CODE:
            {
                struct lans_ntf_navigation_cmd *p_nav_cmd =
                            (struct lans_ntf_navigation_cmd *) ke_msg2param(p_lans_env->p_op_data->p_cmd);

                // Check if sending of notifications has been enabled
                if (GETB(p_lans_env->env[cursor].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_NAVIGATION_NTF)
                        // and if navigation is enabled
                        && GETB(p_lans_env->env[cursor].nav_ctrl, LANS_NAV_CTRL_PT_EN))
                {
                    // Allocate the GATT notification message
                    struct gattc_send_evt_cmd *p_meas_val = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                            KE_BUILD_ID(TASK_GATTC, cursor), prf_src_task_get(&(p_lans_env->prf_env),cursor),
                            gattc_send_evt_cmd, LANP_LAN_NAVIGATION_MAX_LEN);

                    // Fill in the parameter structure
                    p_meas_val->operation = GATTC_NOTIFY;
                    p_meas_val->handle = LANS_HANDLE(LNS_IDX_NAVIGATION_VAL);
                    // pack measured value in database
                    p_meas_val->length = lans_pack_navigation_ntf(&p_nav_cmd->parameters, p_meas_val->value);

                    // Send the event
                    ke_msg_send(p_meas_val);

                    finished = false;
                }

                p_lans_env->p_op_data->cursor++;
            } break;

            default:
            {
                ASSERT_ERR(0);
            } break;
        }
    }

    // check if operation is finished
    if (finished)
    {
        // send response to requester
        lans_send_cmp_evt(conidx, prf_src_task_get(&(p_lans_env->prf_env), conidx),
                   prf_dst_task_get(&(p_lans_env->prf_env), conidx),
                   p_lans_env->operation, GAP_ERR_NO_ERROR);

        // free operation data
        ke_msg_free(p_lans_env->p_op_data->p_cmd);
        ke_free(p_lans_env->p_op_data);
        p_lans_env->p_op_data = NULL;
        // Set the operation code
        p_lans_env->operation = LANS_RESERVED_OP_CODE;
        // go back to idle state
        ke_state_set(prf_src_task_get(&(p_lans_env->prf_env), 0), LANS_IDLE);
    }
}

void lans_send_rsp_ind(uint8_t conidx, uint8_t req_op_code, uint8_t status)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);

    // Allocate the GATT notification message
    struct gattc_send_evt_cmd *p_ctl_pt_rsp = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KE_BUILD_ID(TASK_GATTC, conidx),
            prf_src_task_get(&p_lans_env->prf_env, conidx),
            gattc_send_evt_cmd, LANP_LAN_LN_CNTL_PT_RSP_MIN_LEN);

    // Fill in the parameter structure
    p_ctl_pt_rsp->operation = GATTC_INDICATE;
    p_ctl_pt_rsp->handle = LANS_HANDLE(LNS_IDX_LN_CTNL_PT_VAL);
    // Pack Control Point confirmation
    p_ctl_pt_rsp->length = LANP_LAN_LN_CNTL_PT_RSP_MIN_LEN;
    // Response Code
    p_ctl_pt_rsp->value[0] = LANP_LN_CTNL_PT_RESPONSE_CODE;
    // Request Operation Code
    p_ctl_pt_rsp->value[1] = req_op_code;
    // Response value
    p_ctl_pt_rsp->value[2] = status;

    // Send the event
    ke_msg_send(p_ctl_pt_rsp);
}

#endif //(BLE_LN_SENSOR)

/// @} LANS
