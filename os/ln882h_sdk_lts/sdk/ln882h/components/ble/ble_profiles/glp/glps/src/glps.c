/**
 ****************************************************************************************
 *
 * @file glps.c
 *
 * @brief Glucose Profile Sensor implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GLPS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_GL_SENSOR)
#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "glps.h"
#include "glps_task.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Full GLPS Database Description - Used to add attributes into the database
static const struct attm_desc glps_att_db[GLS_IDX_NB] =
{
    // Glucose Service Declaration
    [GLS_IDX_SVC]                      =   {ATT_DECL_PRIMARY_SERVICE,    PERM(RD, ENABLE), 0, 0},

    // Glucose Measurement Characteristic Declaration
    [GLS_IDX_MEAS_CHAR]                =   {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE), 0, 0},
    // Glucose Measurement Characteristic Value
    [GLS_IDX_MEAS_VAL]                 =   {ATT_CHAR_GLUCOSE_MEAS,       PERM(NTF, ENABLE), PERM(RI, ENABLE), 0},
    // Glucose Measurement Characteristic - Client Characteristic Configuration Descriptor
    [GLS_IDX_MEAS_NTF_CFG]             =   {ATT_DESC_CLIENT_CHAR_CFG,    PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

    // Glucose Measurement Context Characteristic Declaration
    [GLS_IDX_MEAS_CTX_CHAR]            =   {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE), 0, 0},
    // Glucose Measurement Context Characteristic Value
    [GLS_IDX_MEAS_CTX_VAL]             =   {ATT_CHAR_GLUCOSE_MEAS_CTX,   PERM(NTF, ENABLE), PERM(RI, ENABLE), 0},
    // Glucose Measurement Context Characteristic - Client Characteristic Configuration Descriptor
    [GLS_IDX_MEAS_CTX_NTF_CFG]         =   {ATT_DESC_CLIENT_CHAR_CFG,    PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

    // Glucose Features Characteristic Declaration
    [GLS_IDX_FEATURE_CHAR]             =   {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE), 0, 0},
    // Glucose Features Characteristic Value
    [GLS_IDX_FEATURE_VAL]              =   {ATT_CHAR_GLUCOSE_FEATURE,    PERM(RD, ENABLE), PERM(RI, ENABLE) | sizeof(uint16_t), 0},

    // Record Access Control Point characteristic Declaration
    [GLS_IDX_REC_ACCESS_CTRL_CHAR]     =   {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE), 0, 0},
    // Record Access Control Point characteristic Value
    [GLS_IDX_REC_ACCESS_CTRL_VAL]      =   {ATT_CHAR_REC_ACCESS_CTRL_PT, PERM(IND, ENABLE)|PERM(WP, UNAUTH)|PERM(WRITE_REQ, ENABLE),
                                                                         PERM(RI, ENABLE), GLP_REC_ACCESS_CTRL_MAX_LEN},
    // Record Access Control Point characteristic - Client Characteristic Configuration Descriptor
    [GLS_IDX_REC_ACCESS_CTRL_IND_CFG]  =   {ATT_DESC_CLIENT_CHAR_CFG,    PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
};

/*
 * DEFINES
 ****************************************************************************************
 */

#define GLPS_FILTER_USER_FACING_TIME_SIZE (7)

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialization of the GLPS module.
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
static uint8_t glps_init (struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl, struct glps_db_cfg *p_params)
{
    //------------------ create the attribute database for the profile -------------------
    uint8_t idx;
    // Service content flag
    uint32_t cfg_flag = GLPS_MANDATORY_MASK;
    // DB Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;

    // Set Configuration Flag Value
    if (p_params->meas_ctx_supported)
    {
        cfg_flag |= GLPS_MEAS_CTX_PRES_MASK;
    }

    // Create Glucose Database
    status = attm_svc_create_db(p_start_hdl, ATT_SVC_GLUCOSE, (uint8_t *)&cfg_flag,
            GLS_IDX_NB, NULL, p_env->task, &glps_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_MI, ENABLE));

    //-------------------- allocate memory required for the profile  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {
        // Allocate GLPS required environment variable
        struct glps_env_tag *p_glps_env =
                (struct glps_env_tag *) ke_malloc(sizeof(struct glps_env_tag), KE_MEM_ATT_DB);

        // Initialize GLPS environment
        p_env->env                      = (prf_env_t *) p_glps_env;
        p_glps_env->start_hdl           = *p_start_hdl;
        p_glps_env->meas_ctx_supported  = p_params->meas_ctx_supported;
        p_glps_env->features            = p_params->features;
        p_glps_env->prf_env.app_task    = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        p_glps_env->prf_env.prf_task    = p_env->task | PERM(PRF_MI, ENABLE);

        // initialize environment variable
        p_env->id                     = TASK_ID_GLPS;
        glps_task_init(&(p_env->desc));

        for (idx = 0; idx < GLPS_IDX_MAX; idx++)
        {
            p_glps_env->p_env[idx] = NULL;
            // service is ready, go into an Idle state
            ke_state_set(KE_BUILD_ID(p_env->task, idx), GLPS_FREE);
        }
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the GLPS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void glps_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct glps_env_tag *p_glps_env = (struct glps_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for (idx = 0; idx < GLPS_IDX_MAX; idx++)
    {
        if (p_glps_env->p_env[idx] != NULL)
        {
            ke_free(p_glps_env->p_env[idx]);
        }
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_glps_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void glps_create(struct prf_task_env *p_env, uint8_t conidx)
{
    struct glps_env_tag *p_glps_env = (struct glps_env_tag *) p_env->env;

    p_glps_env->p_env[conidx] = (struct glps_cnx_env *)
            ke_malloc(sizeof(struct glps_cnx_env), KE_MEM_ATT_DB);

    memset(p_glps_env->p_env[conidx], 0, sizeof(struct glps_cnx_env));

    // Put GLS in Idle state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), GLPS_IDLE);
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
static void glps_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct glps_env_tag *p_glps_env = (struct glps_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if (p_glps_env->p_env[conidx] != NULL)
    {
        ke_free(p_glps_env->p_env[conidx]);
        p_glps_env->p_env[conidx] = NULL;
    }

    // Put GLS in Free state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), GLPS_FREE);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// GLPS Task interface required by profile manager
const struct prf_task_cbs glps_itf =
{
    (prf_init_fnct) glps_init,
    glps_destroy,
    glps_create,
    glps_cleanup,
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *glps_prf_itf_get(void)
{
    return &glps_itf;
}

uint8_t glps_pack_meas_value(uint8_t *p_packed_meas, const struct glp_meas* p_meas_val,
                             uint16_t seq_num, struct glps_env_tag *p_glps_env)
{
    uint8_t cursor = 0;

    // Flags
    p_packed_meas[cursor] = p_meas_val->flags;
    cursor += 1;

    // Sequence Number
    co_write16p(p_packed_meas + cursor, seq_num);
    cursor += 2;

    // Base Time
    cursor += prf_pack_date_time(p_packed_meas + cursor, &(p_meas_val->base_time));

    // Time Offset
    if (GETB(p_meas_val->flags, GLP_MEAS_TIME_OFF_PRES) != 0)
    {
        co_write16p(p_packed_meas + cursor, p_meas_val->time_offset);
        cursor += 2;
    }

    // Glucose Concentration, type and location
    if (GETB(p_meas_val->flags, GLP_MEAS_GL_CTR_TYPE_AND_SPL_LOC_PRES) != 0)
    {
        co_write16p(p_packed_meas + cursor, p_meas_val->concentration);
        cursor += 2;

        // type and location are 2 nibble values
        p_packed_meas[cursor] = (p_meas_val->location << 4) | (p_meas_val->type);
        cursor += 1;
    }

    // Sensor Status Annunciation
    if (GETB(p_meas_val->flags, GLP_MEAS_SENS_STAT_ANNUN_PRES) != 0)
    {
        // Use a non-const value
        uint16_t status = p_meas_val->status;

        // If feature not supported, corresponding Flag in the Sensor Status Annunciation Field
        // shall be set to default of 0

        // Low Battery Detection During Measurement Support Bit
        if (!GETB(p_glps_env->features, GLP_FET_LOW_BAT_DET_DUR_MEAS_SUPP))
        {
            SETB(status, GLP_MEAS_STATE_DEV_BAT_LOW, 0);
        }

        // Sensor Malfunction Detection Support Bit
        if (!GETB(p_glps_env->features, GLP_FET_SENS_MFNC_DET_SUPP))
        {
            SETB(status, GLP_MEAS_STATE_SENS_MFNC_OR_FLTING, 0);
        }

        // Sensor Sample Size Support Bit
        if (!GETB(p_glps_env->features, GLP_FET_SENS_SPL_SIZE_SUPP))
        {
            SETB(status, GLP_MEAS_STATE_SPL_SIZE_INSUFF, 0);
        }

        // Sensor Strip Insertion Error Detection Support Bit
        if (!GETB(p_glps_env->features, GLP_FET_SENS_STRIP_INSERT_ERR_DET_SUPP))
        {
            SETB(status, GLP_MEAS_STATE_STRIP_INSERT_ERR, 0);
        }

        // Sensor Result High-Low Detection Support Bit
        if (!GETB(p_glps_env->features, GLP_FET_SENS_RES_HIGH_LOW_DET_SUPP))
        {
            SETB(status, GLP_MEAS_STATE_SENS_RES_HIGHER, 0);
            SETB(status, GLP_MEAS_STATE_SENS_RES_LOWER, 0);
        }

        // Sensor Temperature High-Low Detection Support Bit
        if (!GETB(p_glps_env->features, GLP_FET_SENS_TEMP_HIGH_LOW_DET_SUPP))
        {
            SETB(status, GLP_MEAS_STATE_SENS_TEMP_TOO_HIGH, 0);

            SETB(status, GLP_MEAS_STATE_SENS_TEMP_TOO_LOW, 0);
        }

        // Sensor Read Interrupt Detection Support Bit
        if (!GETB(p_glps_env->features, GLP_FET_SENS_RD_INT_DET_SUPP))
        {
            SETB(status, GLP_MEAS_STATE_SENS_RD_INTED, 0);
        }

        // General Device Fault Support Bit
        if (!GETB(p_glps_env->features, GLP_FET_GEN_DEV_FLT_SUPP))
        {
            SETB(status, GLP_MEAS_STATE_GEN_DEV_FLT, 0);
        }

        // Time Fault Support Bit
        if (!GETB(p_glps_env->features, GLP_FET_TIME_FLT_SUPP))
        {
            SETB(status, GLP_MEAS_STATE_TIME_FLT, 0);
        }

        // Multiple Bond Support Bit
        if (!GETB(p_glps_env->features, GLP_FET_MUL_BOND_SUPP))
        {
            // can determine that the Glucose Sensor supports only a single bond
        }
        else
        {
            // Collector can determine that the Glucose supports multiple bonds
        }

        co_write16p(p_packed_meas + cursor, status);
        cursor += 2;
    }

    return cursor;
}

uint8_t glps_pack_meas_ctx_value(uint8_t *p_packed_meas_ctx,
                                 const struct glp_meas_ctx* p_meas_ctx_val,
                                 uint16_t seq_num)
{
    uint8_t cursor = 0;
    // Flags
    p_packed_meas_ctx[cursor] = p_meas_ctx_val->flags;
    cursor += 1;

    // Sequence Number
    co_write16p(p_packed_meas_ctx + cursor, seq_num);
    cursor += 2;

    // Extended Flags
    if (GETB(p_meas_ctx_val->flags, GLP_CTX_EXTD_F_PRES) != 0)
    {
        p_packed_meas_ctx[cursor] = p_meas_ctx_val->ext_flags;
        cursor += 1;
    }

    // Carbohydrate ID And Carbohydrate Present
    if (GETB(p_meas_ctx_val->flags, GLP_CTX_CRBH_ID_AND_CRBH_PRES) != 0)
    {
        // Carbohydrate ID
        p_packed_meas_ctx[cursor] = p_meas_ctx_val->carbo_id;
        cursor += 1;
        // Carbohydrate Present
        co_write16p(p_packed_meas_ctx + cursor, p_meas_ctx_val->carbo_val);
        cursor += 2;
    }

    // Meal Present
    if (GETB(p_meas_ctx_val->flags, GLP_CTX_MEAL_PRES) != 0)
    {
        p_packed_meas_ctx[cursor] = p_meas_ctx_val->meal;
        cursor += 1;
    }

    // Tester-Health Present
    if (GETB(p_meas_ctx_val->flags, GLP_CTX_TESTER_HEALTH_PRES) != 0)
    {
        // Tester and Health are 2 nibble values
        p_packed_meas_ctx[cursor] = (p_meas_ctx_val->health << 4) | (p_meas_ctx_val->tester);
        cursor += 1;
    }

    // Exercise Duration & Exercise Intensity Present
    if (GETB(p_meas_ctx_val->flags, GLP_CTX_EXE_DUR_AND_EXE_INTENS_PRES) != 0)
    {
        // Exercise Duration
        co_write16p(p_packed_meas_ctx + cursor, p_meas_ctx_val->exercise_dur);
        cursor += 2;

        // Exercise Intensity
        p_packed_meas_ctx[cursor] = p_meas_ctx_val->exercise_intens;
        cursor += 1;
    }

    // Medication ID And Medication Present
    if (GETB(p_meas_ctx_val->flags, GLP_CTX_MEDIC_ID_AND_MEDIC_PRES) != 0)
    {
        // Medication ID
        p_packed_meas_ctx[cursor] = p_meas_ctx_val->med_id;
        cursor += 1;

        // Medication Present
        co_write16p(p_packed_meas_ctx + cursor, p_meas_ctx_val->med_val);
        cursor += 2;
    }

    // HbA1c Present
    if (GETB(p_meas_ctx_val->flags, GLP_CTX_HBA1C_PRES) != 0)
    {
        // HbA1c
        co_write16p(p_packed_meas_ctx + cursor, p_meas_ctx_val->hba1c_val);
        cursor += 2;
    }

    return cursor;
}

uint8_t glps_unpack_racp_req(uint8_t *p_packed_val, uint16_t length,
                                        struct glp_racp_req* p_racp_req)
{
    uint8_t cursor = 0;

    // verify that enough data present to load operation filter
    if (length < 2)
    {
        return PRF_APP_ERROR;
    }

    // retrieve command op code
    p_racp_req->op_code = p_packed_val[cursor];
    cursor++;

    // clear filter structure
    memset(&(p_racp_req->filter), 0, sizeof(struct glp_filter));

    // retrieve operator of the function
    p_racp_req->filter.operator = p_packed_val[cursor];
    cursor++;

    // check if opcode is supported
    if ((p_racp_req->op_code < GLP_REQ_REP_STRD_RECS)
       || (p_racp_req->op_code > GLP_REQ_REP_NUM_OF_STRD_RECS))
    {
        return GLP_RSP_OP_CODE_NOT_SUP;
    }

    // Abort operation don't require any other parameter
    if (p_racp_req->op_code == GLP_REQ_ABORT_OP)
    {
        return GAP_ERR_NO_ERROR;
    }

    // check if operator is valid
    if (p_racp_req->filter.operator < GLP_OP_ALL_RECS)
    {
        return GLP_RSP_INVALID_OPERATOR;
    }
    // check if operator is supported
    else if (p_racp_req->filter.operator > GLP_OP_LAST_REC)
    {
        return GLP_RSP_OPERATOR_NOT_SUP;
    }

    // check if request requires operand (filter)
    if ((p_racp_req->filter.operator >= GLP_OP_LT_OR_EQ)
            && (p_racp_req->filter.operator <= GLP_OP_WITHIN_RANGE_OF))
    {
        // verify that enough data present to load operand filter
        if (length < cursor)
        {
            return  GLP_RSP_INVALID_OPERAND;
        }

        // retrieve command filter type
        p_racp_req->filter.filter_type = p_packed_val[cursor];
        cursor++;

        // filter uses sequence number
        if (p_racp_req->filter.filter_type == GLP_FILTER_SEQ_NUMBER)
        {
            // retrieve minimum value
            if ((p_racp_req->filter.operator == GLP_OP_GT_OR_EQ)
                    || (p_racp_req->filter.operator == GLP_OP_WITHIN_RANGE_OF))
            {
                // check sufficient data available
                if ((length - cursor) < 2)
                {
                    return GLP_RSP_INVALID_OPERAND;
                }

                // retrieve minimum value
                p_racp_req->filter.val.seq_num.min = co_read16p(p_packed_val + cursor);
                cursor +=2;
            }

            // retrieve maximum value
            if ((p_racp_req->filter.operator == GLP_OP_LT_OR_EQ)
                    || (p_racp_req->filter.operator == GLP_OP_WITHIN_RANGE_OF))
            {
                if ((length - cursor) < 2)
                {
                    return GLP_RSP_INVALID_OPERAND;
                }

                // retrieve maximum value
                p_racp_req->filter.val.seq_num.max = co_read16p(p_packed_val + cursor);
                cursor +=2;
            }


            // check that range value is valid
            if ((p_racp_req->filter.operator == GLP_OP_WITHIN_RANGE_OF)
                    && (p_racp_req->filter.val.seq_num.min > p_racp_req->filter.val.seq_num.max))
            {
                return GLP_RSP_INVALID_OPERAND;
            }
        }
        // filter uses user facing time
        else if (p_racp_req->filter.filter_type == GLP_FILTER_USER_FACING_TIME)
        {
            // retrieve minimum value
            if ((p_racp_req->filter.operator == GLP_OP_GT_OR_EQ)
                    || (p_racp_req->filter.operator == GLP_OP_WITHIN_RANGE_OF))
            {
                // check sufficient data available
                if ((length - cursor) < GLPS_FILTER_USER_FACING_TIME_SIZE)
                {
                    return GLP_RSP_INVALID_OPERAND;
                }

                // retrieve minimum facing time
                cursor += prf_unpack_date_time((p_packed_val + cursor),
                        &(p_racp_req->filter.val.time.facetime_min));
                cursor +=2;
            }

            // retrieve maximum value
            if ((p_racp_req->filter.operator == GLP_OP_LT_OR_EQ)
                    || (p_racp_req->filter.operator == GLP_OP_WITHIN_RANGE_OF))
            {
                if ((length - cursor) < GLPS_FILTER_USER_FACING_TIME_SIZE)
                {
                    return GLP_RSP_INVALID_OPERAND;
                }

                // retrieve maximum facing time
                cursor += prf_unpack_date_time((p_packed_val + cursor),
                        &(p_racp_req->filter.val.time.facetime_max));
            }
        }
        else
        {
            return GLP_RSP_OPERAND_NOT_SUP;
        }
    }
    else if ((length - cursor) != 0)
    {
        return GLP_RSP_INVALID_OPERAND;
    }

    // Check that operand is valid
    return GAP_ERR_NO_ERROR;
}

uint8_t glps_pack_racp_rsp(uint8_t *p_packed_val, struct glp_racp_rsp *p_racp_rsp)
{
    uint8_t cursor = 0;

    // set response op code
    p_packed_val[cursor] = p_racp_rsp->op_code;
    cursor++;

    // set operator (null)
    p_packed_val[cursor] = 0;
    cursor++;

    // number of record
    if (p_racp_rsp->op_code == GLP_REQ_NUM_OF_STRD_RECS_RSP)
    {
        co_write16(p_packed_val+cursor, p_racp_rsp->operand.num_of_record);
        cursor += 2;
    }
    else
    {
        // requested opcode
        p_packed_val[cursor] = p_racp_rsp->operand.rsp.op_code_req;;
        cursor++;
        // command status
        p_packed_val[cursor] = p_racp_rsp->operand.rsp.status;
        cursor++;
    }

    return cursor;
}

#endif /* BLE_GL_SENSOR */

/// @} GLPS
