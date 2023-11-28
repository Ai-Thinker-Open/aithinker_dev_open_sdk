/**
 ****************************************************************************************
 *
 * @file cpps.c
 *
 * @brief Cycling Power Profile Sensor implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup CPPS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */


#include "rwip_config.h"
#if (BLE_CP_SENSOR)
#include "cpp_common.h"
#include "gap.h"
#include "gattc_task.h"
#include "gattc.h"
#include "cpps.h"
#include "cpps_task.h"
#include "prf_utils.h"
#include "co_math.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Full CPPS Database Description - Used to add attributes into the database
static const struct attm_desc cpps_att_db[CPS_IDX_NB] =
{
    // Cycling Power Service Declaration
    [CPS_IDX_SVC]                =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

    // CP Measurement Characteristic Declaration
    [CPS_IDX_CP_MEAS_CHAR]       =  {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // CP Measurement Characteristic Value
    [CPS_IDX_CP_MEAS_VAL]        =   {ATT_CHAR_CP_MEAS, PERM(NTF, ENABLE), PERM(RI, ENABLE), CPP_CP_MEAS_NTF_MAX_LEN},
    // CP Measurement Characteristic - Client Characteristic Configuration Descriptor
    [CPS_IDX_CP_MEAS_NTF_CFG]    =   {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},
    // CP Measurement Characteristic - Server Characteristic Configuration Descriptor
    [CPS_IDX_CP_MEAS_BCST_CFG]   =   {ATT_DESC_SERVER_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},

    // CP Feature Characteristic Declaration
    [CPS_IDX_CP_FEAT_CHAR]       =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // CP Feature Characteristic Value
    [CPS_IDX_CP_FEAT_VAL]        =   {ATT_CHAR_CP_FEAT, PERM(RD, ENABLE), PERM(RI, ENABLE), sizeof(uint32_t)},

    // Sensor Location Characteristic Declaration
    [CPS_IDX_SENSOR_LOC_CHAR]    =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Sensor Location Characteristic Value
    [CPS_IDX_SENSOR_LOC_VAL]     =   {ATT_CHAR_SENSOR_LOC, PERM(RD, ENABLE), PERM(RI, ENABLE), sizeof(uint8_t)},

    // CP Vector Characteristic Declaration
    [CPS_IDX_VECTOR_CHAR]        =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // CP Vector Characteristic Value
    [CPS_IDX_VECTOR_VAL]         =   {ATT_CHAR_CP_VECTOR, PERM(NTF, ENABLE), PERM(RI, ENABLE), CPP_CP_VECTOR_MAX_LEN},
    // CP Vector Characteristic - Client Characteristic Configuration Descriptor
    [CPS_IDX_VECTOR_NTF_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},

    // CP Control Point Characteristic Declaration
    [CPS_IDX_CTNL_PT_CHAR]       =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // CP Control Point Characteristic Value - The response has the maximal length
    [CPS_IDX_CTNL_PT_VAL]        =   {ATT_CHAR_CP_CNTL_PT, PERM(IND, ENABLE) | PERM(WRITE_REQ, ENABLE),
                                                               PERM(RI, ENABLE), CPP_CP_CNTL_PT_RSP_MAX_LEN},
    // CP Control Point Characteristic - Client Characteristic Configuration Descriptor
    [CPS_IDX_CTNL_PT_IND_CFG]    =   {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the CPPS module.
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
static uint8_t cpps_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl, struct cpps_db_cfg *p_params)
{
    //------------------ create the attribute database for the profile -------------------
    // Service content flag
    uint32_t cfg_flag= CPPS_MANDATORY_MASK;
    // DB Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;

    /*
     * Check if Broadcaster role shall be added.
     */
    if (CPPS_IS_FEATURE_SUPPORTED(p_params->prfl_config, CPPS_BROADCASTER_SUPP_FLAG))
    {
        //Add configuration to the database
        cfg_flag |= CPPS_MEAS_BCST_MASK;
    }
    /*
     * Check if the CP Vector characteristic shall be added.
     * Mandatory if at least one Vector procedure is supported, otherwise excluded.
     */
    if ((GETB(p_params->cp_feature, CPP_FEAT_CRANK_REV_DATA_SUP)) ||
        (GETB(p_params->cp_feature, CPP_FEAT_EXTREME_ANGLES_SUP)) ||
        (GETB(p_params->cp_feature, CPP_FEAT_INSTANT_MEAS_DIRECTION_SUP)))
    {
        cfg_flag |= CPPS_VECTOR_MASK;
    }
    /*
     * Check if the Control Point characteristic shall be added
     * Mandatory if server supports:
     *     - Wheel Revolution Data
     *     - Multiple Sensor Locations
     *     - Configurable Settings (CPP_CTNL_PT_SET codes)
     *     - Offset Compensation
     *     - Server allows to be requested for parameters (CPP_CTNL_PT_REQ codes)
     */
    if ((CPPS_IS_FEATURE_SUPPORTED(p_params->prfl_config, CPPS_CTNL_PT_CHAR_SUPP_FLAG)) ||
        (GETB(p_params->cp_feature, CPP_FEAT_WHEEL_REV_DATA_SUP)) ||
        (GETB(p_params->cp_feature, CPP_FEAT_MULT_SENSOR_LOC_SUP)) ||
        (GETB(p_params->cp_feature, CPP_FEAT_CRANK_LENGTH_ADJ_SUP)) ||
        (GETB(p_params->cp_feature, CPP_FEAT_CHAIN_LENGTH_ADJ_SUP)) ||
        (GETB(p_params->cp_feature, CPP_FEAT_CHAIN_WEIGHT_ADJ_SUP)) ||
        (GETB(p_params->cp_feature, CPP_FEAT_SPAN_LENGTH_ADJ_SUP)) ||
        (GETB(p_params->cp_feature, CPP_FEAT_OFFSET_COMP_SUP)))
    {
        cfg_flag |= CPPS_CTNL_PT_MASK;
    }

    // Add service in the database
    status = attm_svc_create_db(p_start_hdl, ATT_SVC_CYCLING_POWER, (uint8_t *)&cfg_flag,
            CPS_IDX_NB, NULL, p_env->task, &cpps_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_MI, DISABLE));

    //-------------------- allocate memory required for the profile  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {
        // Allocate CPPS required environment variable
        struct cpps_env_tag *p_cpps_env =
                (struct cpps_env_tag *) ke_malloc(sizeof(struct cpps_env_tag), KE_MEM_ATT_DB);

        // Initialize CPPS environment
        p_env->env = (prf_env_t *) p_cpps_env;
        p_cpps_env->shdl = *p_start_hdl;
        p_cpps_env->prfl_cfg = cfg_flag;
        p_cpps_env->features = p_params->cp_feature;
        p_cpps_env->sensor_loc = p_params->sensor_loc;
        p_cpps_env->cumul_wheel_rev = p_params->wheel_rev;
        p_cpps_env->operation = CPPS_RESERVED_OP_CODE;

        p_cpps_env->p_op_data = NULL;
        memset(p_cpps_env->env, 0, sizeof(p_cpps_env->env));

        p_cpps_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        // Mono Instantiated task
        p_cpps_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        p_env->id = TASK_ID_CPPS;
        cpps_task_init(&(p_env->desc));

        /*
         * Check if the Broadcaster role shall be added.
         */
        if (CPPS_IS_FEATURE_SUPPORTED(p_cpps_env->prfl_cfg, CPPS_MEAS_BCST_MASK))
        {
            // Optional Permissions
            uint16_t perm = PERM(NTF, ENABLE) | PERM(BROADCAST,ENABLE);
            //Add configuration to the database
            attm_att_set_permission(CPPS_HANDLE(CPS_IDX_CP_MEAS_VAL), perm, 0);
        }

        // Put CPS in Idle state
        ke_state_set(p_env->task, CPPS_IDLE);
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the CPPS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void cpps_destroy(struct prf_task_env *p_env)
{
    struct cpps_env_tag *p_cpps_env = (struct cpps_env_tag *) p_env->env;

    if (p_cpps_env->p_op_data != NULL)
    {
        ke_free(p_cpps_env->p_op_data->p_cmd);
        if(p_cpps_env->p_op_data->p_ntf_pending)
        {
            ke_free(p_cpps_env->p_op_data->p_ntf_pending);
        }
        ke_free(p_cpps_env->p_op_data);
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_cpps_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void cpps_create(struct prf_task_env *p_env, uint8_t conidx)
{
    struct cpps_env_tag *p_cpps_env = (struct cpps_env_tag *) p_env->env;

    memset(&(p_cpps_env->env[conidx]), 0, sizeof(struct cpps_cnx_env));
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
static void cpps_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct cpps_env_tag *p_cpps_env = (struct cpps_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    memset(&(p_cpps_env->env[conidx]), 0, sizeof(struct cpps_cnx_env));
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// CPPS Task interface required by profile manager
const struct prf_task_cbs cpps_itf =
{
    (prf_init_fnct) cpps_init,
    cpps_destroy,
    cpps_create,
    cpps_cleanup,
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *cpps_prf_itf_get(void)
{
   return &cpps_itf;
}

uint8_t cpps_pack_meas_ntf(struct cpp_cp_meas *p_param, uint8_t *p_pckd_meas)
{
    // Packed Measurement length
    uint8_t pckd_meas_len = CPP_CP_MEAS_NTF_MIN_LEN;
    // Get the address of the environment
    struct cpps_env_tag *p_cpps_env = PRF_ENV_GET(CPPS, cpps);

    // Check provided flags
    if (GETB(p_param->flags, CPP_MEAS_PEDAL_POWER_BALANCE_PRESENT))
    {
        if (GETB(p_cpps_env->features, CPP_FEAT_PEDAL_POWER_BALANCE_SUP))
        {
            // Pack Pedal Power Balance info
            p_pckd_meas[pckd_meas_len] = p_param->pedal_power_balance;
            pckd_meas_len++;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(p_param->flags, CPP_MEAS_PEDAL_POWER_BALANCE_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, CPP_MEAS_ACCUM_TORQUE_PRESENT))
    {
        if (GETB(p_cpps_env->features, CPP_FEAT_ACCUM_TORQUE_SUP))
        {
            // Pack Accumulated Torque info
            co_write16p(&p_pckd_meas[pckd_meas_len], p_param->accum_torque);
            pckd_meas_len += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(p_param->flags, CPP_MEAS_ACCUM_TORQUE_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, CPP_MEAS_WHEEL_REV_DATA_PRESENT))
    {
        if (GETB(p_cpps_env->features, CPP_FEAT_WHEEL_REV_DATA_SUP))
        {
            // Pack Wheel Revolution Data (Cumulative Wheel & Last Wheel Event Time)
            co_write32p(&p_pckd_meas[pckd_meas_len], p_cpps_env->cumul_wheel_rev);
            pckd_meas_len += 4;
            co_write16p(&p_pckd_meas[pckd_meas_len], p_param->last_wheel_evt_time);
            pckd_meas_len += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(p_param->flags, CPP_MEAS_WHEEL_REV_DATA_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, CPP_MEAS_CRANK_REV_DATA_PRESENT))
    {
        if (GETB(p_cpps_env->features, CPP_FEAT_CRANK_REV_DATA_SUP))
        {
            // Pack Crank Revolution Data (Cumulative Crank & Last Crank Event Time)
            co_write16p(&p_pckd_meas[pckd_meas_len], p_param->cumul_crank_rev);
            pckd_meas_len += 2;
            co_write16p(&p_pckd_meas[pckd_meas_len], p_param->last_crank_evt_time);
            pckd_meas_len += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(p_param->flags, CPP_MEAS_CRANK_REV_DATA_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, CPP_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT))
    {
        if (GETB(p_cpps_env->features, CPP_FEAT_EXTREME_MAGNITUDES_SUP)
                && (!GETB(p_cpps_env->features, CPP_FEAT_SENSOR_MEAS_CONTEXT)))
        {
            // Pack Extreme Force Magnitudes (Maximum Force Magnitude & Minimum Force Magnitude)
            co_write16p(&p_pckd_meas[pckd_meas_len], p_param->max_force_magnitude);
            pckd_meas_len += 2;
            co_write16p(&p_pckd_meas[pckd_meas_len], p_param->min_force_magnitude);
            pckd_meas_len += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(p_param->flags, CPP_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, CPP_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT))
    {
        if (GETB(p_cpps_env->features, CPP_FEAT_EXTREME_MAGNITUDES_SUP)
                && GETB(p_cpps_env->features, CPP_FEAT_SENSOR_MEAS_CONTEXT))
        {
            // Pack Extreme Force Magnitudes (Maximum Force Magnitude & Minimum Force Magnitude)
            co_write16p(&p_pckd_meas[pckd_meas_len], p_param->max_torque_magnitude);
            pckd_meas_len += 2;
            co_write16p(&p_pckd_meas[pckd_meas_len], p_param->min_torque_magnitude);
            pckd_meas_len += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(p_param->flags, CPP_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, CPP_MEAS_EXTREME_ANGLES_PRESENT))
    {
        if (GETB(p_cpps_env->features, CPP_FEAT_EXTREME_ANGLES_SUP))
        {
            uint32_t angle;

            // Pack Extreme Angles (Maximum Angle & Minimum Angle)
            // Force to 12 bits
            p_param->max_angle &= 0x0FFF;
            p_param->min_angle &= 0x0FFF;
            angle = (uint32_t) (p_param->max_angle | (p_param->min_angle << 12));
            co_write24p(&p_pckd_meas[pckd_meas_len], angle);
            pckd_meas_len += 3;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(p_param->flags, CPP_MEAS_EXTREME_ANGLES_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, CPP_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT))
    {
        if (GETB(p_cpps_env->features, CPP_FEAT_TOPBOT_DEAD_SPOT_ANGLES_SUP))
        {
            // Pack Top Dead Spot Angle
            co_write16p(&p_pckd_meas[pckd_meas_len], p_param->top_dead_spot_angle);
            pckd_meas_len += 2;
        }
        else
        {
            // Force to not supported
            SETB(p_param->flags, CPP_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, CPP_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT))
    {
        if (GETB(p_cpps_env->features, CPP_FEAT_TOPBOT_DEAD_SPOT_ANGLES_SUP))
        {
            // Pack Bottom Dead Spot Angle
            co_write16p(&p_pckd_meas[pckd_meas_len], p_param->bot_dead_spot_angle);
            pckd_meas_len += 2;
        }
        else
        {
            // Force to not supported
            SETB(p_param->flags, CPP_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, CPP_MEAS_ACCUM_ENERGY_PRESENT))
    {
        if (GETB(p_cpps_env->features, CPP_FEAT_ACCUM_ENERGY_SUP))
        {
            // Pack Accumulated Energy
            co_write16p(&p_pckd_meas[pckd_meas_len], p_param->accum_energy);
            pckd_meas_len += 2;
        }
        else
        {
            // Force to not supported
            SETB(p_param->flags, CPP_MEAS_ACCUM_ENERGY_PRESENT, 0);
        }
    }

    // Flags value
    co_write16p(&p_pckd_meas[0], p_param->flags);
    // Instant Power (Mandatory)
    co_write16p(&p_pckd_meas[2], p_param->inst_power);

    return pckd_meas_len;
}

uint8_t cpps_split_meas_ntf(uint8_t conidx, struct gattc_send_evt_cmd *p_meas_ntf1)
{
    // Get the address of the environment
    struct cpps_env_tag *p_cpps_env = PRF_ENV_GET(CPPS, cpps);
    // Extract flags info
    uint16_t flags = co_read16p(&p_meas_ntf1->value[0]);
    // Allocate the GATT notification message
    struct gattc_send_evt_cmd *p_meas_ntf2 = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KE_BUILD_ID(TASK_GATTC, conidx), prf_src_task_get(&p_cpps_env->prf_env, conidx),
            gattc_send_evt_cmd, CPP_CP_MEAS_NTF_MAX_LEN);
    // Current position
    uint8_t len = 0;

    // Fill in the parameter structure
    p_meas_ntf2->operation = GATTC_NOTIFY;
    p_meas_ntf2->handle = CPPS_HANDLE(CPS_IDX_CP_MEAS_VAL);
    p_meas_ntf2->length = CPP_CP_MEAS_NTF_MIN_LEN;

    // Copy status flags
    co_write16p(&p_meas_ntf2->value[0], (flags & (CPP_MEAS_PEDAL_POWER_BALANCE_REFERENCE_BIT |
                                                CPP_MEAS_ACCUM_TORQUE_SOURCE_BIT |
                                                CPP_MEAS_OFFSET_COMPENSATION_INDICATOR_BIT)));
    // Copy Instantaneous power
    memcpy(&p_meas_ntf2->value[2], &p_meas_ntf1->value[2], 2);

    for (uint16_t feat = CPP_MEAS_PEDAL_POWER_BALANCE_PRESENT_BIT;
            feat <= CPP_MEAS_OFFSET_COMPENSATION_INDICATOR_BIT;
            feat <<= 1)
    {
        // First message fits within the MTU
        if (p_meas_ntf1->length <= gattc_get_mtu(conidx) - 3)
        {
            // Stop splitting
            break;
        }

        // Check if specific bit presented in Measurement flags
        if ((flags & feat) == feat)
        {
            switch (feat)
            {
                case CPP_MEAS_PEDAL_POWER_BALANCE_PRESENT_BIT:
                {
                    // Copy uint8
                    p_meas_ntf2->value[p_meas_ntf2->length] = p_meas_ntf1->value[CPP_CP_MEAS_NTF_MIN_LEN];
                    len = 1;
                } break;

                case CPP_MEAS_ACCUM_TORQUE_PRESENT_BIT:
                case CPP_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT_BIT:
                case CPP_MEAS_ACCUM_ENERGY_PRESENT_BIT:
                case CPP_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT_BIT:
                {
                    // Copy uint16
                    memcpy(&p_meas_ntf2->value[p_meas_ntf2->length], &p_meas_ntf1->value[CPP_CP_MEAS_NTF_MIN_LEN], 2);
                    len = 2;
                } break;

                case CPP_MEAS_EXTREME_ANGLES_PRESENT_BIT:
                {
                    // Copy uint24
                    memcpy(&p_meas_ntf2->value[p_meas_ntf2->length], &p_meas_ntf1->value[CPP_CP_MEAS_NTF_MIN_LEN], 3);
                    len = 3;
                } break;

                case CPP_MEAS_CRANK_REV_DATA_PRESENT_BIT:
                case CPP_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT_BIT:
                case CPP_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT_BIT:
                {
                    // Copy uint16 + uint16
                    memcpy(&p_meas_ntf2->value[p_meas_ntf2->length], &p_meas_ntf1->value[CPP_CP_MEAS_NTF_MIN_LEN], 4);
                    len = 4;
                } break;

                case CPP_MEAS_WHEEL_REV_DATA_PRESENT_BIT:
                {
                    // Copy uint32 + uint16
                    memcpy(&p_meas_ntf2->value[p_meas_ntf2->length], &p_meas_ntf1->value[CPP_CP_MEAS_NTF_MIN_LEN], 6);
                    len = 6;
                } break;

                default:
                {
                    len = 0;
                } break;
            }

            if (len)
            {
                // Update values
                p_meas_ntf2->length += len;
                // Remove field and flags from the first ntf
                p_meas_ntf1->length -= len;
                memcpy(&p_meas_ntf1->value[CPP_CP_MEAS_NTF_MIN_LEN],
                       &p_meas_ntf1->value[CPP_CP_MEAS_NTF_MIN_LEN + len],
                       p_meas_ntf1->length);
                // Update flags
                p_meas_ntf1->value[0] &= ~feat;
                p_meas_ntf2->value[0] |= feat;
            }
        }
    }

    // store the pending notification to send
    p_cpps_env->p_op_data->p_ntf_pending = p_meas_ntf2;

    return p_meas_ntf2->length;
}

uint8_t cpps_update_characteristic_config(uint8_t conidx, uint8_t prfl_config,
        struct gattc_write_req_ind const *p_param)
{
    // Get the address of the environment
    struct cpps_env_tag *p_cpps_env = PRF_ENV_GET(CPPS, cpps);

    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Get the value
    uint16_t ntf_cfg = co_read16p(&p_param->value[0]);

    switch (prfl_config)
    {
        case CPP_PRF_CFG_FLAG_CTNL_PT_IND_BIT:
        {
            // Check CCC configuration
            if (ntf_cfg == PRF_CLI_STOP_NTFIND)
            {
                // Save the new configuration in the environment
                SETB(p_cpps_env->env[conidx].prfl_ntf_ind_cfg, CPP_PRF_CFG_FLAG_CTNL_PT_IND, 0);
            }
            else if (ntf_cfg == PRF_CLI_START_IND)
            {
                // Save the new configuration in the environment
                SETB(p_cpps_env->env[conidx].prfl_ntf_ind_cfg, CPP_PRF_CFG_FLAG_CTNL_PT_IND, 1);
            }
            else
            {
                status = PRF_ERR_INVALID_PARAM;
            }

        } break;

        case CPP_PRF_CFG_FLAG_VECTOR_NTF_BIT:
        {
            // Do not save until confirmation
        } break;

        case CPP_PRF_CFG_FLAG_SP_MEAS_NTF_BIT:
        {
            // Check CCC configuration
            if ((ntf_cfg == PRF_SRV_STOP_BCST) || (ntf_cfg == PRF_SRV_START_BCST))
            {
                // Save the new configuration in the environment
                p_cpps_env->broadcast_enabled = (ntf_cfg == PRF_SRV_STOP_BCST) ?
                        false : true;

                // Update value for every connection (useful for bond data)
                for (uint8_t i = 0; i < BLE_CONNECTION_MAX; i++)
                {
                    if (ntf_cfg == PRF_SRV_STOP_BCST)
                    {
                        SETB(p_cpps_env->env[conidx].prfl_ntf_ind_cfg, CPP_PRF_CFG_FLAG_SP_MEAS_NTF, 0);
                    }
                    else
                    {
                        SETB(p_cpps_env->env[conidx].prfl_ntf_ind_cfg, CPP_PRF_CFG_FLAG_SP_MEAS_NTF, 1);
                    }
                }
            }
            else
            {
                status = PRF_ERR_INVALID_PARAM;
            }
        } break;

        case CPP_PRF_CFG_FLAG_CP_MEAS_NTF_BIT:
        {
            if (ntf_cfg == PRF_CLI_STOP_NTFIND)
            {
                SETB(p_cpps_env->env[conidx].prfl_ntf_ind_cfg, CPP_PRF_CFG_FLAG_CP_MEAS_NTF, 0);
            }
            else if (ntf_cfg == PRF_CLI_START_NTF)
            {
                SETB(p_cpps_env->env[conidx].prfl_ntf_ind_cfg, CPP_PRF_CFG_FLAG_CP_MEAS_NTF, 1);
            }
            else
            {
                status = PRF_APP_ERROR;
            }
        } break;

        default:
        {
            status = ATT_ERR_INVALID_HANDLE;
        } break;
    }

    if (status == GAP_ERR_NO_ERROR)
    {
        if (prfl_config == CPP_PRF_CFG_FLAG_VECTOR_NTF_BIT)
        {
            // Allocate message to inform application
            struct cpps_vector_cfg_req_ind *p_ind = KE_MSG_ALLOC(CPPS_VECTOR_CFG_REQ_IND,
                                                    prf_dst_task_get(&p_cpps_env->prf_env, conidx),
                                                    prf_src_task_get(&p_cpps_env->prf_env, conidx),
                                                    cpps_vector_cfg_req_ind);

            // Inform APP of configuration change
            p_ind->char_code = prfl_config;
            p_ind->ntf_cfg   = ntf_cfg;
            p_ind->conidx = conidx;
            ke_msg_send(p_ind);
        }
        else
        {
            // Allocate message to inform application
            struct cpps_cfg_ntfind_ind *p_ind = KE_MSG_ALLOC(CPPS_CFG_NTFIND_IND,
                                                prf_dst_task_get(&p_cpps_env->prf_env, conidx),
                                                prf_src_task_get(&p_cpps_env->prf_env, conidx),
                                                cpps_cfg_ntfind_ind);

            // Inform APP of configuration change
            p_ind->char_code = prfl_config;
            p_ind->ntf_cfg   = ntf_cfg;
            p_ind->conidx = conidx;
            ke_msg_send(p_ind);
        }

        // Enable Bonded Data
        SETB(p_cpps_env->env[conidx].prfl_ntf_ind_cfg, CPP_PRF_CFG_PERFORMED_OK, 1);
    }
    return (status);
}

uint8_t cpps_pack_vector_ntf(struct cpp_cp_vector *p_param, uint8_t *p_pckd_vector)
{
    // Get the address of the environment
    struct cpps_env_tag *p_cpps_env = PRF_ENV_GET(CPPS, cpps);
    // Packed Measurement length
    uint8_t pckd_vector_len = CPP_CP_VECTOR_MIN_LEN;

    // Check provided flags
    if (GETB(p_param->flags, CPP_VECTOR_CRANK_REV_DATA_PRESENT))
    {
        if (GETB(p_cpps_env->features, CPP_FEAT_CRANK_REV_DATA_SUP))
        {
            // Pack Crank Revolution Data (Cumulative Crank & Last Crank Event Time)
            co_write16p(&p_pckd_vector[pckd_vector_len], p_param->cumul_crank_rev);
            pckd_vector_len += 2;
            co_write16p(&p_pckd_vector[pckd_vector_len], p_param->last_crank_evt_time);
            pckd_vector_len += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(p_param->flags, CPP_VECTOR_CRANK_REV_DATA_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, CPP_VECTOR_FIRST_CRANK_MEAS_ANGLE_PRESENT))
    {
        if (GETB(p_cpps_env->features, CPP_FEAT_EXTREME_ANGLES_SUP))
        {
            // Pack First Crank Measurement Angle
            co_write16p(&p_pckd_vector[pckd_vector_len], p_param->first_crank_meas_angle);
            pckd_vector_len += 2;
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(p_param->flags, CPP_VECTOR_FIRST_CRANK_MEAS_ANGLE_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, CPP_VECTOR_INST_FORCE_MAGNITUDE_ARRAY_PRESENT))
    {
        if (!GETB(p_cpps_env->features, CPP_FEAT_SENSOR_MEAS_CONTEXT))
        {
            // Pack Instantaneous Force Magnitude Array
            if ((p_param->nb > CPP_CP_VECTOR_MIN_LEN) &&
                    (p_param->nb <= CPP_CP_VECTOR_MAX_LEN - pckd_vector_len))
            {
                for(int j = 0; j < p_param->nb; j++)
                {
                    co_write16p(&p_pckd_vector[pckd_vector_len], p_param->force_torque_magnitude[j]);
                    pckd_vector_len += 2;
                }
            }
            else
            {
                return 0;
            }
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(p_param->flags, CPP_VECTOR_INST_FORCE_MAGNITUDE_ARRAY_PRESENT, 0);
        }
    }

    if (GETB(p_param->flags, CPP_VECTOR_INST_TORQUE_MAGNITUDE_ARRAY_PRESENT))
    {
        if (GETB(p_cpps_env->features, CPP_FEAT_SENSOR_MEAS_CONTEXT))
        {
            // Pack Instantaneous Torque Magnitude Array
            if ((p_param->nb > CPP_CP_VECTOR_MIN_LEN) &&
                    (p_param->nb <= CPP_CP_VECTOR_MAX_LEN - pckd_vector_len))
            {
                for(int j = 0; j < p_param->nb; j++)
                {
                    co_write16p(&p_pckd_vector[pckd_vector_len], p_param->force_torque_magnitude[j]);
                    pckd_vector_len += 2;
                }
            }
            else
            {
                return 0;
            }
        }
        else // Not supported by the profile
        {
            // Force to not supported
            SETB(p_param->flags, CPP_VECTOR_INST_TORQUE_MAGNITUDE_ARRAY_PRESENT, 0);
        }
    }

    // Flags value
    p_pckd_vector[0] = p_param->flags;

    return pckd_vector_len;
}

void cpps_send_rsp_ind(uint8_t conidx, uint8_t req_op_code, uint8_t status)
{
    // Get the address of the environment
    struct cpps_env_tag *p_cpps_env = PRF_ENV_GET(CPPS, cpps);

    // Allocate the GATT notification message
    struct gattc_send_evt_cmd *p_ctl_pt_rsp = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KE_BUILD_ID(TASK_GATTC, conidx),
            prf_src_task_get(&p_cpps_env->prf_env, conidx),
            gattc_send_evt_cmd, CPP_CP_CNTL_PT_RSP_MIN_LEN);

    // Fill in the parameter structure
    p_ctl_pt_rsp->operation = GATTC_INDICATE;
    p_ctl_pt_rsp->handle = CPPS_HANDLE(CPS_IDX_CTNL_PT_VAL);
    // Pack Control Point confirmation
    p_ctl_pt_rsp->length = CPP_CP_CNTL_PT_RSP_MIN_LEN;
    // Response Code
    p_ctl_pt_rsp->value[0] = CPP_CTNL_PT_RSP_CODE;
    // Request Operation Code
    p_ctl_pt_rsp->value[1] = req_op_code;
    // Response value
    p_ctl_pt_rsp->value[2] = status;

    // Send the event
    ke_msg_send(p_ctl_pt_rsp);
}

uint8_t cpps_unpack_ctnl_point_ind (uint8_t conidx, struct gattc_write_req_ind const *p_param)
{
    // Get the address of the environment
    struct cpps_env_tag *p_cpps_env = PRF_ENV_GET(CPPS, cpps);

    // Indication Status
    uint8_t ind_status = CPP_CTNL_PT_RESP_NOT_SUPP;

    // Allocate a request indication message for the application
    struct cpps_ctnl_pt_req_ind *p_req_ind = KE_MSG_ALLOC(CPPS_CTNL_PT_REQ_IND,
            prf_dst_task_get(&p_cpps_env->prf_env, conidx),
            prf_src_task_get(&p_cpps_env->prf_env, conidx), cpps_ctnl_pt_req_ind);

    // Operation Code
    p_req_ind->op_code = p_param->value[0];
    p_req_ind->conidx = conidx;

    // Operation Code
    switch (p_req_ind->op_code)
    {
        case (CPP_CTNL_PT_SET_CUMUL_VAL):
        {
            // Check if the Wheel Revolution Data feature is supported
            if (GETB(p_cpps_env->features, CPP_FEAT_WHEEL_REV_DATA_SUP))
            {
                // Provided parameter in not within the defined range
                ind_status = CPP_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 5)
                {
                    // The request can be handled
                    ind_status = CPP_CTNL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_cpps_env->operation = CPPS_CTNL_PT_SET_CUMUL_VAL_OP_CODE;
                    // Cumulative value
                    p_req_ind->value.cumul_val = co_read32p(&p_param->value[1]);
                }
            }
        } break;

        case (CPP_CTNL_PT_UPD_SENSOR_LOC):
        {
            // Check if the Multiple Sensor Location feature is supported
            if (GETB(p_cpps_env->features, CPP_FEAT_MULT_SENSOR_LOC_SUP))
            {
                // Provided parameter in not within the defined range
                ind_status = CPP_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 2)
                {
                    // Check the sensor location value
                    if (p_param->value[1] < CPP_LOC_MAX)
                    {
                        // The request can be handled
                        ind_status = CPP_CTNL_PT_RESP_SUCCESS;
                        // Update the environment
                        p_cpps_env->operation = CPPS_CTNL_PT_UPD_SENSOR_LOC_OP_CODE;
                        // Sensor Location
                        p_req_ind->value.sensor_loc = p_param->value[1];
                    }
                }
            }
        } break;

        case (CPP_CTNL_PT_REQ_SUPP_SENSOR_LOC):
        {
            // Check if the Multiple Sensor Location feature is supported
            if (GETB(p_cpps_env->features, CPP_FEAT_MULT_SENSOR_LOC_SUP))
            {
                // The request can be handled
                ind_status = CPP_CTNL_PT_RESP_SUCCESS;
                // Update the environment
                p_cpps_env->operation = CPPS_CTNL_PT_REQ_SUPP_SENSOR_LOC_OP_CODE;
            }
        } break;

        case (CPP_CTNL_PT_SET_CRANK_LENGTH):
        {
            // Check if the Crank Length Adjustment feature is supported
            if (GETB(p_cpps_env->features, CPP_FEAT_CRANK_LENGTH_ADJ_SUP))
            {
                // Provided parameter in not within the defined range
                ind_status = CPP_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 3)
                {
                    // The request can be handled
                    ind_status = CPP_CTNL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_cpps_env->operation = CPPS_CTNL_PT_SET_CRANK_LENGTH_OP_CODE;
                    // Crank Length
                    p_req_ind->value.crank_length = co_read16p(&p_param->value[1]);
                }
            }
        } break;

        case (CPP_CTNL_PT_REQ_CRANK_LENGTH):
        {
            // Optional even if feature not supported
            ind_status = CPP_CTNL_PT_RESP_SUCCESS;
            // Update the environment
            p_cpps_env->operation = CPPS_CTNL_PT_REQ_CRANK_LENGTH_OP_CODE;
        } break;

        case (CPP_CTNL_PT_SET_CHAIN_LENGTH):
        {
            // Check if the Chain Length Adjustment feature is supported
            if (GETB(p_cpps_env->features, CPP_FEAT_CHAIN_LENGTH_ADJ_SUP))
            {
                // Provided parameter in not within the defined range
                ind_status = CPP_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 3)
                {
                    // The request can be handled
                    ind_status = CPP_CTNL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_cpps_env->operation = CPPS_CTNL_PT_SET_CHAIN_LENGTH_OP_CODE;
                    // Chain Length
                    p_req_ind->value.crank_length = co_read16p(&p_param->value[1]);
                }
            }
        } break;

        case (CPP_CTNL_PT_REQ_CHAIN_LENGTH):
        {
            // Optional even if feature not supported
            ind_status = CPP_CTNL_PT_RESP_SUCCESS;
            // Update the environment
            p_cpps_env->operation = CPPS_CTNL_PT_REQ_CHAIN_LENGTH_OP_CODE;
        } break;

        case (CPP_CTNL_PT_SET_CHAIN_WEIGHT):
        {
            // Check if the Chain Weight Adjustment feature is supported
            if (GETB(p_cpps_env->features, CPP_FEAT_CHAIN_WEIGHT_ADJ_SUP))
            {
                // Provided parameter in not within the defined range
                ind_status = CPP_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 3)
                {
                    // The request can be handled
                    ind_status = CPP_CTNL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_cpps_env->operation = CPPS_CTNL_PT_SET_CHAIN_WEIGHT_OP_CODE;
                    // Chain Weight
                    p_req_ind->value.chain_weight = co_read16p(&p_param->value[1]);
                }
            }
        } break;

        case (CPP_CTNL_PT_REQ_CHAIN_WEIGHT):
        {
            // Optional even if feature not supported
            ind_status = CPP_CTNL_PT_RESP_SUCCESS;
            // Update the environment
            p_cpps_env->operation = CPPS_CTNL_PT_REQ_CHAIN_WEIGHT_OP_CODE;
        } break;

        case (CPP_CTNL_PT_SET_SPAN_LENGTH):
        {
            // Check if the Span Length Adjustment feature is supported
            if (GETB(p_cpps_env->features, CPP_FEAT_SPAN_LENGTH_ADJ_SUP))
            {
                // Provided parameter in not within the defined range
                ind_status = CPP_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 3)
                {
                    // The request can be handled
                    ind_status = CPP_CTNL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_cpps_env->operation = CPPS_CTNL_PT_SET_SPAN_LENGTH_OP_CODE;
                    // Span Length
                    p_req_ind->value.span_length = co_read16p(&p_param->value[1]);
                }
            }

        } break;

        case (CPP_CTNL_PT_REQ_SPAN_LENGTH):
        {
            // Optional even if feature not supported
            ind_status = CPP_CTNL_PT_RESP_SUCCESS;
            // Update the environment
            p_cpps_env->operation = CPPS_CTNL_PT_REQ_SPAN_LENGTH_OP_CODE;
        } break;

        case (CPP_CTNL_PT_START_OFFSET_COMP):
        {
            // Check if the Offset Compensation feature is supported
            if (GETB(p_cpps_env->features, CPP_FEAT_OFFSET_COMP_SUP))
            {
                // The request can be handled
                ind_status = CPP_CTNL_PT_RESP_SUCCESS;
                // Update the environment
                p_cpps_env->operation = CPPS_CTNL_PT_START_OFFSET_COMP_OP_CODE;
            }
        } break;

        case (CPP_CTNL_MASK_CP_MEAS_CH_CONTENT):
        {
            // Check if the CP Masking feature is supported
            if (GETB(p_cpps_env->features, CPP_FEAT_CP_MEAS_CH_CONTENT_MASKING_SUP))
            {
                // Provided parameter in not within the defined range
                ind_status = CPP_CTNL_PT_RESP_INV_PARAM;

                if (p_param->length == 3)
                {
                    // The request can be handled
                    ind_status = CPP_CTNL_PT_RESP_SUCCESS;
                    // Update the environment
                    p_cpps_env->operation = CPPS_CTNL_MASK_CP_MEAS_CH_CONTENT_OP_CODE;
                    // Mask content
                    p_req_ind->value.mask_content = co_read16p(&p_param->value[1]);
                }
            }
        } break;

        case (CPP_CTNL_REQ_SAMPLING_RATE):
        {
            // Optional even if feature not supported
            ind_status = CPP_CTNL_PT_RESP_SUCCESS;
            // Update the environment
            p_cpps_env->operation = CPPS_CTNL_REQ_SAMPLING_RATE_OP_CODE;
        } break;

        case (CPP_CTNL_REQ_FACTORY_CALIBRATION_DATE):
        {
            // Optional even if feature not supported
            ind_status = CPP_CTNL_PT_RESP_SUCCESS;
            // Update the environment
            p_cpps_env->operation = CPPS_CTNL_REQ_FACTORY_CALIBRATION_DATE_OP_CODE;
        } break;

        case (CPP_CTNL_START_ENHANCED_OFFSET_COMP):
        {
            // Check if the Enhanced Offset Compensation feature is supported
            if (GETB(p_cpps_env->features, CPP_FEAT_ENHANCED_OFFSET_COMPENS_SUP))
            {
                // The request can be handled
                ind_status = CPP_CTNL_PT_RESP_SUCCESS;
                // Update the environment
                p_cpps_env->operation = CPPS_CTNL_START_ENHANCED_OFFSET_COMP_OP_CODE;
            }
        } break;

        default:
        {
            // Operation Code is invalid, status is already CPP_CTNL_PT_RESP_NOT_SUPP
        } break;
    }

    // If no error raised, inform the application about the request
    if (ind_status == CPP_CTNL_PT_RESP_SUCCESS)
    {
        // Send the request indication to the application
        ke_msg_send(p_req_ind);
        // Go to the Busy status
        ke_state_set(prf_src_task_get(&p_cpps_env->prf_env, conidx), CPPS_BUSY);
    }
    else
    {
        // Free the allocated message
        ke_msg_free(ke_param2msg(p_req_ind));
    }

    return ind_status;
}

uint8_t cpps_pack_ctnl_point_cfm (uint8_t conidx, struct cpps_ctnl_pt_cfm *p_param, uint8_t *p_rsp)
{
    // Get the address of the environment
    struct cpps_env_tag *p_cpps_env = PRF_ENV_GET(CPPS, cpps);
    // Response Length (At least 3)
    uint8_t rsp_len = CPP_CP_CNTL_PT_RSP_MIN_LEN;

    // Set the Response Code
    p_rsp[0] = CPP_CTNL_PT_RSP_CODE;
    // Set the Response Value
    p_rsp[2] = (p_param->status > CPP_CTNL_PT_RESP_FAILED) ? CPP_CTNL_PT_RESP_FAILED : p_param->status;

    switch (p_cpps_env->operation)
    {
        case (CPPS_CTNL_PT_SET_CUMUL_VAL_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_PT_SET_CUMUL_VAL;

            if (p_param->status == CPP_CTNL_PT_RESP_SUCCESS)
            {
                // Save in the environment
                p_cpps_env->cumul_wheel_rev = p_param->value.cumul_wheel_rev;
            }
        } break;

        case (CPPS_CTNL_PT_UPD_SENSOR_LOC_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_PT_UPD_SENSOR_LOC;

            if (p_param->status == CPP_CTNL_PT_RESP_SUCCESS)
            {
                // Store the new value in the environment
                p_cpps_env->sensor_loc = p_param->value.sensor_loc;
            }
        } break;

        case (CPPS_CTNL_PT_REQ_SUPP_SENSOR_LOC_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_PT_REQ_SUPP_SENSOR_LOC;

            if (p_param->status == CPP_CTNL_PT_RESP_SUCCESS)
            {
                // Set the list of supported location
                for (uint8_t counter = 0; counter < CPP_LOC_MAX; counter++)
                {
                    if ((p_param->value.supp_sensor_loc >> counter) & 0x0001)
                    {
                        p_rsp[rsp_len] = counter;
                        rsp_len++;
                    }
                }
            }
        } break;

        case (CPPS_CTNL_PT_SET_CRANK_LENGTH_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_PT_SET_CRANK_LENGTH;
        } break;

        case (CPPS_CTNL_PT_REQ_CRANK_LENGTH_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_PT_REQ_CRANK_LENGTH;
            if (p_param->status == CPP_CTNL_PT_RESP_SUCCESS)
            {
                // Set the response parameter
                co_write16p(&p_rsp[rsp_len], p_param->value.crank_length);
                rsp_len += 2;
            }
        } break;

        case (CPPS_CTNL_PT_SET_CHAIN_LENGTH_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_PT_SET_CHAIN_LENGTH;
        } break;

        case (CPPS_CTNL_PT_REQ_CHAIN_LENGTH_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_PT_REQ_CHAIN_LENGTH;
            if (p_param->status == CPP_CTNL_PT_RESP_SUCCESS)
            {
                // Set the response parameter
                co_write16p(&p_rsp[rsp_len], p_param->value.chain_length);
                rsp_len += 2;
            }
        } break;

        case (CPPS_CTNL_PT_SET_CHAIN_WEIGHT_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_PT_SET_CHAIN_WEIGHT;
        } break;


        case (CPPS_CTNL_PT_REQ_CHAIN_WEIGHT_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_PT_REQ_CHAIN_WEIGHT;
            if (p_param->status == CPP_CTNL_PT_RESP_SUCCESS)
            {
                // Set the response parameter
                co_write16p(&p_rsp[rsp_len], p_param->value.chain_weight);
                rsp_len += 2;
            }
        } break;

        case (CPPS_CTNL_PT_SET_SPAN_LENGTH_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_PT_SET_SPAN_LENGTH;
        } break;

        case (CPPS_CTNL_PT_REQ_SPAN_LENGTH_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_PT_REQ_SPAN_LENGTH;
            if (p_param->status == CPP_CTNL_PT_RESP_SUCCESS)
            {
                // Set the response parameter
                co_write16p(&p_rsp[rsp_len], p_param->value.span_length);
                rsp_len += 2;
            }
        } break;

        case (CPPS_CTNL_PT_START_OFFSET_COMP_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_PT_START_OFFSET_COMP;
            if (p_param->status == CPP_CTNL_PT_RESP_SUCCESS)
            {
                // Set the response parameter
                co_write16p(&p_rsp[rsp_len], p_param->value.offset_comp);
                rsp_len += 2;
            }
        } break;

        case (CPPS_CTNL_MASK_CP_MEAS_CH_CONTENT_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_MASK_CP_MEAS_CH_CONTENT;
            if (p_param->status == CPP_CTNL_PT_RESP_SUCCESS)
            {
                uint16_t cpp_mask_cp_meas_flags [] =
                {
                    CPP_MEAS_PEDAL_POWER_BALANCE_PRESENT_BIT,
                    CPP_MEAS_ACCUM_TORQUE_PRESENT_BIT,
                    CPP_MEAS_WHEEL_REV_DATA_PRESENT_BIT,
                    CPP_MEAS_CRANK_REV_DATA_PRESENT_BIT,
                    CPP_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT_BIT |
                    CPP_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT_BIT,
                    CPP_MEAS_EXTREME_ANGLES_PRESENT_BIT,
                    CPP_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT_BIT,
                    CPP_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT_BIT,
                    CPP_MEAS_ACCUM_ENERGY_PRESENT_BIT,
                };
                uint16_t mask = 0;

                for (uint8_t count = 0; count < 9; count++)
                {
                    if ((p_param->value.mask_meas_content >> count) & 0x0001)
                    {
                        mask |= cpp_mask_cp_meas_flags[count];
                    }
                }
                p_cpps_env->env[conidx].mask_meas_content = mask;
            }
        } break;

        case (CPPS_CTNL_REQ_SAMPLING_RATE_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_REQ_SAMPLING_RATE;
            if (p_param->status == CPP_CTNL_PT_RESP_SUCCESS)
            {
                // Set the response parameter
                p_rsp[rsp_len] = p_param->value.sampling_rate;
                rsp_len ++;
            }
        } break;

        case (CPPS_CTNL_REQ_FACTORY_CALIBRATION_DATE_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_REQ_FACTORY_CALIBRATION_DATE;
            if (p_param->status == CPP_CTNL_PT_RESP_SUCCESS)
            {
                // Set the response parameter
                rsp_len += prf_pack_date_time(&p_rsp[rsp_len], &(p_param->value.factory_calibration));
            }
        } break;

        case (CPPS_CTNL_START_ENHANCED_OFFSET_COMP_OP_CODE):
        {
            // Set the request operation code
            p_rsp[1] = CPP_CTNL_START_ENHANCED_OFFSET_COMP;
            if (p_param->status == CPP_CTNL_PT_RESP_SUCCESS)
            {
                // Set the response parameter
                // Offset
                co_write16p(&p_rsp[rsp_len], p_param->value.enhanced_offset_comp.comp_offset);
                rsp_len += 2;
                // manufacturer company ID
                co_write16p(&p_rsp[rsp_len], p_param->value.enhanced_offset_comp.manu_comp_id);
                rsp_len += 2;
                // length
                p_rsp[rsp_len] = p_param->value.enhanced_offset_comp.length;
                rsp_len++;

                // data
                if ((p_param->value.enhanced_offset_comp.length > 0) &&
                        (p_param->value.enhanced_offset_comp.length <= CPP_CP_CNTL_PT_RSP_MAX_LEN - rsp_len))
                {
                    for (int j = 0; j < p_param->value.enhanced_offset_comp.length; j++)
                    {
                        p_rsp[rsp_len++] = p_param->value.enhanced_offset_comp.data[j];
                    }
                }
                else
                {
                    return 0;
                }
            }
            // Operation results in an error condition
            else
            {
                if (p_param->value.enhanced_offset_comp.rsp_param == CPP_CTNL_PT_ERR_RSP_PARAM_INCORRECT_CALIB_POS)
                {
                    // Response parameter
                    p_rsp[rsp_len] = p_param->value.enhanced_offset_comp.rsp_param;
                    rsp_len++;
                }
                else if (p_param->value.enhanced_offset_comp.rsp_param ==
                        CPP_CTNL_PT_ERR_RSP_PARAM_MANUF_SPEC_ERR_FOLLOWS)
                {
                    // Response parameter
                    p_rsp[rsp_len] = p_param->value.enhanced_offset_comp.rsp_param;
                    rsp_len++;

                    // manufacturer company ID
                    co_write16p(&p_rsp[rsp_len], p_param->value.enhanced_offset_comp.manu_comp_id);
                    rsp_len += 2;

                    // length
                    p_rsp[rsp_len] = p_param->value.enhanced_offset_comp.length;
                    rsp_len++;

                    // data
                    if ((p_param->value.enhanced_offset_comp.length > 0) &&
                            (p_param->value.enhanced_offset_comp.length <= CPP_CP_CNTL_PT_RSP_MAX_LEN - rsp_len))
                    {
                        for (int j = 0; j < p_param->value.enhanced_offset_comp.length; j++)
                        {
                            p_rsp[rsp_len++] = p_param->value.enhanced_offset_comp.data[j];
                        }
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    p_rsp[2] = CPP_CTNL_PT_RESP_INV_PARAM;
                }
            }
        } break;

        default:
        {
            p_rsp[2] = CPP_CTNL_PT_RESP_NOT_SUPP;
        } break;
    }

    return rsp_len;
}

void cpps_send_cmp_evt(uint8_t conidx, uint8_t src_id, uint8_t dest_id, uint8_t operation, uint8_t status)
{
    // Get the address of the environment
    struct cpps_env_tag *p_cpps_env = PRF_ENV_GET(CPPS, cpps);
    struct cpps_cmp_evt *p_evt;

    // Go back to the IDLE if the state is busy
    if (ke_state_get(src_id) == CPPS_BUSY)
    {
        ke_state_set(src_id, CPPS_IDLE);
    }

    // Set the operation code
    p_cpps_env->operation = CPPS_RESERVED_OP_CODE;

    // Send the message
    p_evt = KE_MSG_ALLOC(CPPS_CMP_EVT,dest_id, src_id, cpps_cmp_evt);

    p_evt->conidx = conidx;
    p_evt->operation  = operation;
    p_evt->status     = status;

    ke_msg_send(p_evt);
}

void cpps_exe_operation(uint8_t conidx)
{
    struct cpps_env_tag *p_cpps_env = PRF_ENV_GET(CPPS, cpps);

    ASSERT_ERR(p_cpps_env->p_op_data != NULL);

    bool finished = true;

    while ((p_cpps_env->p_op_data->cursor < BLE_CONNECTION_MAX) && finished)
    {
        uint8_t cursor = p_cpps_env->p_op_data->cursor;

        switch (p_cpps_env->operation)
        {
            case CPPS_NTF_MEAS_OP_CODE:
            {
                // notification is pending, send it first
                if (p_cpps_env->p_op_data->p_ntf_pending != NULL)
                {
                    ke_msg_send(p_cpps_env->p_op_data->p_ntf_pending);
                    p_cpps_env->p_op_data->p_ntf_pending = NULL;
                    finished = false;
                }
                // Check if sending of notifications has been enabled
                else if (GETB(p_cpps_env->env[cursor].prfl_ntf_ind_cfg, CPP_PRF_CFG_FLAG_CP_MEAS_NTF))
                {
                    struct cpps_ntf_cp_meas_cmd *p_meas_cmd =
                            (struct cpps_ntf_cp_meas_cmd *) ke_msg2param(p_cpps_env->p_op_data->p_cmd);

                    struct gattc_send_evt_cmd *p_meas_val;

                    // Save flags value
                    uint16_t flags = p_meas_cmd->parameters.flags;

                    // Mask unwanted fields if supported
                    if (GETB(p_cpps_env->features, CPP_FEAT_CP_MEAS_CH_CONTENT_MASKING_SUP))
                    {
                        p_meas_cmd->parameters.flags &= ~p_cpps_env->env[cursor].mask_meas_content;
                    }

                    // Allocate the GATT notification message
                    p_meas_val = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                                 KE_BUILD_ID(TASK_GATTC, cursor), prf_src_task_get(&(p_cpps_env->prf_env),cursor),
                                 gattc_send_evt_cmd, CPP_CP_MEAS_NTF_MAX_LEN);

                    // Fill in the parameter structure
                    p_meas_val->operation = GATTC_NOTIFY;
                    p_meas_val->handle = CPPS_HANDLE(CPS_IDX_CP_MEAS_VAL);
                    // pack measured value in database
                    p_meas_val->length = cpps_pack_meas_ntf(&p_meas_cmd->parameters, p_meas_val->value);

                    if (p_meas_val->length > gattc_get_mtu(cursor) - 3)
                    {
                        // Split (if necessary)
                        cpps_split_meas_ntf(cursor, p_meas_val);
                    }

                    // Restore flags value
                    p_meas_cmd->parameters.flags = flags;

                    // Send the event
                    ke_msg_send(p_meas_val);

                    finished = false;
                }
                // update cursor only if all notification has been sent
                if (p_cpps_env->p_op_data->p_ntf_pending == NULL)
                {
                    p_cpps_env->p_op_data->cursor++;
                }
            } break;

            case CPPS_NTF_VECTOR_OP_CODE:
            {
                struct cpps_ntf_cp_vector_cmd *p_vector_cmd =
                            (struct cpps_ntf_cp_vector_cmd *) ke_msg2param(p_cpps_env->p_op_data->p_cmd);

                // Check if sending of notifications has been enabled
                if (GETB(p_cpps_env->env[cursor].prfl_ntf_ind_cfg, CPP_PRF_CFG_FLAG_VECTOR_NTF))
                {
                    // Allocate the GATT notification message
                    struct gattc_send_evt_cmd *p_vector_val = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                            KE_BUILD_ID(TASK_GATTC, cursor), prf_src_task_get(&(p_cpps_env->prf_env),cursor),
                            gattc_send_evt_cmd, CPP_CP_VECTOR_MAX_LEN);

                    // Fill in the parameter structure
                    p_vector_val->operation = GATTC_NOTIFY;
                    p_vector_val->handle = CPPS_HANDLE(CPS_IDX_VECTOR_VAL);
                    // pack measured value in database
                    p_vector_val->length = cpps_pack_vector_ntf(&p_vector_cmd->parameters, p_vector_val->value);

                    // Send the event
                    ke_msg_send(p_vector_val);

                    finished = false;
                }

                p_cpps_env->p_op_data->cursor++;
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
        // Inform the application that a procedure has been completed
        cpps_send_cmp_evt(conidx,
                prf_src_task_get(&p_cpps_env->prf_env, conidx),
                prf_dst_task_get(&p_cpps_env->prf_env, conidx),
                p_cpps_env->operation, GAP_ERR_NO_ERROR);

        // free operation data
        ke_msg_free(p_cpps_env->p_op_data->p_cmd);
        ke_free(p_cpps_env->p_op_data);
        p_cpps_env->p_op_data = NULL;
        // Set the operation code
        p_cpps_env->operation = CPPS_RESERVED_OP_CODE;
        // go back to idle state
        ke_state_set(prf_src_task_get(&(p_cpps_env->prf_env), 0), CPPS_IDLE);
    }
}

#endif //(BLE_CP_SENSOR)

/// @} CPPS
