/**
 ****************************************************************************************
 *
 * @file hogpd.c
 *
 * @brief Hid Over Gatt profile Report host Implementation.
 *
 * Copyright (C) LIGHTNINGSEMI 2020-2026
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup HOGPD
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "rwip.h"
#include "rwprf_config.h"

#if (BLE_HID_REPORT_HOST)

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "hogprh.h"
#include "hogprh_task.h"
#include "prf_utils.h"

#include "ke_mem.h"

#include "utils/debug/log.h"



/**
 ****************************************************************************************
 * @brief Initialization of the HOGPRH module.
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
static uint8_t hogprh_init (struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl, void *p_params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct hogprh_env_tag *p_hogprh_env =
            (struct hogprh_env_tag *) ke_malloc(sizeof(struct hogprh_env_tag), KE_MEM_ATT_DB);

    // allocate DISC required environment variable
    p_env->env = (prf_env_t *) p_hogprh_env;

    p_hogprh_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    p_hogprh_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    p_env->id = TASK_ID_HOGPRH;
    hogprh_task_init(&(p_env->desc));

    for (idx = 0; idx < HOGPRH_IDX_MAX; idx++)
    {
        p_hogprh_env->p_env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(p_env->task, idx), HOGPRH_IDLE);
    }

    return GAP_ERR_NO_ERROR;
}


/**
 ****************************************************************************************
 * @brief Destruction of the HOGPrh module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void hogprh_destroy(struct prf_task_env *p_env)
{
    struct hogprh_env_tag *p_hogprh_env = (struct hogprh_env_tag *) p_env->env;

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_hogprh_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 * @param[in]        conidx       Connection index
 ****************************************************************************************
 */
static void hogprh_create(struct prf_task_env *p_env, uint8_t conidx)
{
    // Nothing to do
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
static void hogprh_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    // Nothing to do
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// hogpd Task interface required by profile manager
const struct prf_task_cbs hogprh_itf =
{
    (prf_init_fnct) hogprh_init,
    hogprh_destroy,
    hogprh_create,
    hogprh_cleanup,
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *hogprh_prf_itf_get(void)
{
    return &hogprh_itf;
}


void hogprh_enable_rsp_send(struct hogprh_env_tag *p_hogprh_env, uint8_t conidx, uint8_t status)
{
    // Send APP the details of the discovered attributes on DISC
    struct hogprh_enable_rsp *p_rsp = KE_MSG_ALLOC(HOGPRH_ENABLE_RSP,
                                                prf_dst_task_get(&(p_hogprh_env->prf_env), conidx),
                                                prf_src_task_get(&(p_hogprh_env->prf_env), conidx),
                                                hogprh_enable_rsp);

    p_rsp->status = status;

    LOG(LOG_LVL_INFO, "hogprh_enable_rsp_send status=0x%x\r\n",status);


    if (status == GAP_ERR_NO_ERROR)
    {
        p_rsp->device= p_hogprh_env->p_env[conidx]->device;
        
        // Register HOGPRH task in gatt for indication/notifications
        prf_register_atthdl2gatt(&(p_hogprh_env->prf_env), conidx, &(p_hogprh_env->p_env[conidx]->device.svc));

        // Go to connected state
        ke_state_set(prf_src_task_get(&(p_hogprh_env->prf_env), conidx), HOGPRH_IDLE);
    }

    ke_msg_send(p_rsp);
}

void hogprh_send_cmp_evt(struct hogprh_env_tag *p_hogprh_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct hogprh_cmp_evt *p_evt;
    ke_task_id_t src_id = prf_src_task_get(&(p_hogprh_env->prf_env), conidx);
    ke_task_id_t dest_id = prf_dst_task_get(&(p_hogprh_env->prf_env), conidx);

    // Come back to the Connected state if the state was busy.
    if (ke_state_get(src_id) == HOGPRH_BUSY)
    {
        ke_state_set(src_id, HOGPRH_IDLE);
    }

    // Send the message to the application
    p_evt = KE_MSG_ALLOC(HOGPRH_CMP_EVT, dest_id, src_id, hogprh_cmp_evt);

    p_evt->operation   = operation;
    p_evt->status      = status;

    ke_msg_send(p_evt);
}


#endif///#if (BLE_HID_REPORT_HOST)



