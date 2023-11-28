/**
 ****************************************************************************************
 *
 * @file rscps.c
 *
 * @brief Running Speed and Cadence Profile Sensor implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup RSCPS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_RSC_SENSOR)
#include "rscp_common.h"

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "rscps.h"
#include "rscps_task.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Full RSCPS Database Description - Used to add attributes into the database
static const struct attm_desc rscps_att_db[RSCS_IDX_NB] =
{
    // Running Speed and Cadence Service Declaration
    [RSCS_IDX_SVC]                 =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

    // RSC Measurement Characteristic Declaration
    [RSCS_IDX_RSC_MEAS_CHAR]       =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // RSC Measurement Characteristic Value
    [RSCS_IDX_RSC_MEAS_VAL]        =   {ATT_CHAR_RSC_MEAS, PERM(NTF, ENABLE), PERM(RI, ENABLE), RSCP_RSC_MEAS_MAX_LEN},
    // RSC Measurement Characteristic - Client Characteristic Configuration Descriptor
    [RSCS_IDX_RSC_MEAS_NTF_CFG]    =   {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},

    // RSC Feature Characteristic Declaration
    [RSCS_IDX_RSC_FEAT_CHAR]       =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // RSC Feature Characteristic Value
    [RSCS_IDX_RSC_FEAT_VAL]        =   {ATT_CHAR_RSC_FEAT, PERM(RD, ENABLE), PERM(RI, ENABLE), sizeof(uint16_t)},

    // Sensor Location Characteristic Declaration
    [RSCS_IDX_SENSOR_LOC_CHAR]     =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Sensor Location Characteristic Value
    [RSCS_IDX_SENSOR_LOC_VAL]      =   {ATT_CHAR_SENSOR_LOC, PERM(RD, ENABLE), PERM(RI, ENABLE), sizeof(uint8_t)},

    // SC Control Point Characteristic Declaration
    [RSCS_IDX_SC_CTNL_PT_CHAR]     =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // SC Control Point Characteristic Value - The response has the maximal length
    [RSCS_IDX_SC_CTNL_PT_VAL]      =   {ATT_CHAR_SC_CNTL_PT, PERM(IND, ENABLE) | PERM(WRITE_REQ, ENABLE),
                                                                 PERM(RI, ENABLE), RSCP_SC_CNTL_PT_RSP_MAX_LEN},
    // SC Control Point Characteristic - Client Characteristic Configuration Descriptor
    [RSCS_IDX_SC_CTNL_PT_NTF_CFG]  =   {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the RSCPS module.
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
 * @param[in]     p_param      Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 ****************************************************************************************
 */
static uint8_t rscps_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task, uint8_t sec_lvl,
        struct rscps_db_cfg *p_param)
{
    //------------------ create the attribute database for the profile -------------------
    // Service content flag
    uint32_t cfg_flag= RSCPS_MANDATORY_MASK;
    // DB Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;

    /*
     * Check if the Sensor Location characteristic shall be added.
     * Mandatory if the Multiple Sensor Location feature is supported, otherwise optional.
     */
    if ((p_param->sensor_loc_supp == RSCPS_SENSOR_LOC_SUPP) ||
        (GETB(p_param->rsc_feature, RSCP_FEAT_MULT_SENSOR_LOC_SUPP)))
    {
        cfg_flag |= RSCPS_SENSOR_LOC_MASK;
    }

    /*
     * Check if the SC Control Point characteristic shall be added
     * Mandatory if at least one SC Control Point procedure is supported, otherwise excluded.
     */
    if (GETB(p_param->rsc_feature, RSCP_FEAT_CALIB_PROC_SUPP) ||
        GETB(p_param->rsc_feature, RSCP_FEAT_MULT_SENSOR_LOC_SUPP) ||
        GETB(p_param->rsc_feature, RSCP_FEAT_TOTAL_DST_MEAS_SUPP))
    {
        cfg_flag |= RSCPS_SC_CTNL_PT_MASK;
    }

    // Add service in the database
    status = attm_svc_create_db(p_start_hdl, ATT_SVC_RUNNING_SPEED_CADENCE, (uint8_t *)&cfg_flag,
            RSCS_IDX_NB, NULL, p_env->task, &rscps_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_MI, DISABLE));

    // Check if an error has occured
    if (status == ATT_ERR_NO_ERROR)
    {
        // Allocate RSCPS required environment variable
        struct rscps_env_tag *p_rscps_env =
                (struct rscps_env_tag *) ke_malloc(sizeof(struct rscps_env_tag), KE_MEM_ATT_DB);

        // Initialize RSCPS environment

        p_env->env             = (prf_env_t *) p_rscps_env;
        p_rscps_env->shdl      = *p_start_hdl;
        p_rscps_env->prf_cfg   = cfg_flag;
        p_rscps_env->features  = p_param->rsc_feature;
        p_rscps_env->operation = RSCPS_RESERVED_OP_CODE;

        if (RSCPS_IS_FEATURE_SUPPORTED(p_rscps_env->prf_cfg, RSCPS_SENSOR_LOC_MASK))
        {
            p_rscps_env->sensor_loc = (p_param->sensor_loc >= RSCP_LOC_MAX) ?
                    RSCP_LOC_OTHER : p_param->sensor_loc;
        }

        p_rscps_env->prf_env.app_task    = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        // Mono Instantiated task
        p_rscps_env->prf_env.prf_task    = p_env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        p_env->id                        = TASK_ID_RSCPS;
        rscps_task_init(&(p_env->desc));

        p_rscps_env->p_ntf = NULL;
        memset(p_rscps_env->prfl_ntf_ind_cfg, 0, BLE_CONNECTION_MAX);

        // If we are here, database has been fulfilled with success, go to idle state
        ke_state_set(p_env->task, RSCPS_IDLE);
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the RSCPS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void rscps_destroy(struct prf_task_env *p_env)
{
    struct rscps_env_tag *p_rscps_env = (struct rscps_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    if (p_rscps_env->p_ntf != NULL)
    {
        ke_free(p_rscps_env->p_ntf);
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_rscps_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void rscps_create(struct prf_task_env *p_env, uint8_t conidx)
{
    struct rscps_env_tag *p_rscps_env = (struct rscps_env_tag *) p_env->env;

    p_rscps_env->prfl_ntf_ind_cfg[conidx] = 0;

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
static void rscps_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct rscps_env_tag *p_rscps_env = (struct rscps_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    p_rscps_env->prfl_ntf_ind_cfg[conidx] = 0;
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// RSCPS Task interface required by profile manager
const struct prf_task_cbs rscps_itf =
{
    (prf_init_fnct) rscps_init,
    rscps_destroy,
    rscps_create,
    rscps_cleanup,
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *rscps_prf_itf_get(void)
{
    return &rscps_itf;
}

void rscps_send_cmp_evt(uint8_t conidx, uint8_t src_id, uint8_t dest_id, uint8_t operation, uint8_t status)
{
    // Get the address of the environment
    struct rscps_env_tag *p_rscps_env = PRF_ENV_GET(RSCPS, rscps);
    struct rscps_cmp_evt *p_evt;

    // Go back to the Connected state if the state is busy
    if (ke_state_get(src_id) == RSCPS_BUSY)
    {
        ke_state_set(src_id, RSCPS_IDLE);
    }

    // Set the operation code
    p_rscps_env->operation = RSCPS_RESERVED_OP_CODE;

    // Send the message
    p_evt = KE_MSG_ALLOC(RSCPS_CMP_EVT, dest_id, src_id, rscps_cmp_evt);

    p_evt->conidx     = conidx;
    p_evt->operation  = operation;
    p_evt->status     = status;

    ke_msg_send(p_evt);
}


void rscps_exe_operation(uint8_t conidx)
{
    struct rscps_env_tag *p_rscps_env = PRF_ENV_GET(RSCPS, rscps);

    ASSERT_ERR(p_rscps_env->p_ntf != NULL);

    bool finished = true;

    while (p_rscps_env->p_ntf->cursor < BLE_CONNECTION_MAX)
    {
        // Check if notifications are enabled
        if (GETB(p_rscps_env->prfl_ntf_ind_cfg[p_rscps_env->p_ntf->cursor], RSCP_PRF_CFG_FLAG_RSC_MEAS_NTF))
        {
            // Allocate the GATT notification message
            struct gattc_send_evt_cmd *p_meas_val = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                    KE_BUILD_ID(TASK_GATTC, p_rscps_env->p_ntf->cursor),
                    prf_src_task_get(&(p_rscps_env->prf_env), 0),
                    gattc_send_evt_cmd, p_rscps_env->p_ntf->length);

            // Fill in the parameter structure
            p_meas_val->operation = GATTC_NOTIFY;
            p_meas_val->handle = RSCPS_HANDLE(RSCS_IDX_RSC_MEAS_VAL);
            p_meas_val->length = p_rscps_env->p_ntf->length;
            memcpy(p_meas_val->value, p_rscps_env->p_ntf->value, p_rscps_env->p_ntf->length);

            // Send the event
            ke_msg_send(p_meas_val);

            finished = false;
            p_rscps_env->p_ntf->cursor++;
            break;
        }

        p_rscps_env->p_ntf->cursor++;
    }

    // check if operation is finished
    if (finished)
    {
        // Inform the application that a procedure has been completed
        rscps_send_cmp_evt(conidx,
                prf_src_task_get(&p_rscps_env->prf_env, conidx),
                prf_dst_task_get(&p_rscps_env->prf_env, conidx),
                p_rscps_env->operation, GAP_ERR_NO_ERROR);

        // free operation
        ke_free(p_rscps_env->p_ntf);
        p_rscps_env->p_ntf = NULL;
        p_rscps_env->operation = RSCPS_RESERVED_OP_CODE;

        ke_state_set(prf_src_task_get(&(p_rscps_env->prf_env), conidx), RSCPS_IDLE);
    }
}

void rscps_send_rsp_ind(uint8_t conidx, uint8_t req_op_code, uint8_t status)
{
    // Get the address of the environment
    struct rscps_env_tag *p_rscps_env = PRF_ENV_GET(RSCPS, rscps);

    // Allocate the GATT notification message
    struct gattc_send_evt_cmd *p_ctl_pt_rsp = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KE_BUILD_ID(TASK_GATTC, conidx),
            prf_src_task_get(&p_rscps_env->prf_env, conidx),
            gattc_send_evt_cmd, RSCP_SC_CNTL_PT_RSP_MIN_LEN);

    // Fill in the parameter structure
    p_ctl_pt_rsp->operation = GATTC_INDICATE;
    p_ctl_pt_rsp->handle = RSCPS_HANDLE(RSCS_IDX_SC_CTNL_PT_VAL);
    // Pack Control Point confirmation
    p_ctl_pt_rsp->length = RSCP_SC_CNTL_PT_RSP_MIN_LEN;
    // Response Code
    p_ctl_pt_rsp->value[0] = RSCP_CTNL_PT_RSP_CODE;
    // Request Operation Code
    p_ctl_pt_rsp->value[1] = req_op_code;
    // Response value
    p_ctl_pt_rsp->value[2] = status;

    // Send the event
    ke_msg_send(p_ctl_pt_rsp);
}

#endif //(BLE_RSC_SENSOR)

/// @} RSCPS
