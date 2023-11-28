/**
 ****************************************************************************************
 *
 * @file ln_gap_callback.c
 *
 * @brief GAP callback function  source code
 *
 *Copyright (C) 2021. Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup APP
 * @group BLE
 *
 * @brief GAP callback funcion source code
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration
#include "ln_app_gap.h"
#include "ln_app_gatt.h"
#include "usr_app.h"
#include <string.h>

#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manallger Task API
#include "gapc_task.h"               // GAP Controller Task API
#include "gattm_task.h"               // GAP Manallger Task API
#include "gattc_task.h"               // GAP Controller Task API
#include "co_math.h"                 // Common Maths Definition
#include "co_utils.h"

#include "rwip_task.h"      // Task definitions
#include "ke_task.h"        // Kernel Task


#include "usr_ble_app.h"
#include "ln_app_callback.h"
#include "ln_gatt_callback.h"
#if (TRACE_ENABLE)
#include "utils/debug/log.h"
#endif

/// Application Environment Structure
struct app_env_info_tag app_env_info= {0};
extern uint8_t adv_actv_idx;
extern uint8_t init_actv_idx;

uint8_t g_gap_role = GAP_ROLE_NONE;

#define MAX_MTU 260
extern uint8_t svc_uuid[16];
uint8_t con_num=0;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */


static int gapm_activity_created_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_activity_created_ind *p_param = (struct gapm_activity_created_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE, "gapm_activity_created_ind_handler: actv_idx=%d,actv_type=%d \r\n",p_param->actv_idx,p_param->actv_type);
    LOG(LOG_LVL_TRACE, "  ##################app_env_info size   :%d \r\n",sizeof(struct app_env_info_tag));
#endif
    memcpy(&(app_env_info.actv_creat_info),p_param,sizeof(struct gapm_activity_created_ind));
    if(p_param->actv_type  ==GAPM_ACTV_TYPE_ADV)
        adv_actv_idx =p_param->actv_idx;
    if(p_param->actv_type  ==GAPM_ACTV_TYPE_INIT)
        init_actv_idx =p_param->actv_idx;
    return (KE_MSG_CONSUMED);
}


static int gapm_activity_stopped_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_activity_stopped_ind *p_param = (struct gapm_activity_stopped_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_activity_stopped_ind_handler   actv_id=%d   actv_type=%d, reason=0x%x\r\n", p_param->actv_idx, p_param->actv_type,p_param->reason);
#endif
    memcpy(&(app_env_info.actv_stop_info),p_param,sizeof(struct gapm_activity_stopped_ind));
    return (KE_MSG_CONSUMED);
}



static int gapm_ext_adv_report_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_ext_adv_report_ind *p_param = (struct gapm_ext_adv_report_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE, "--------------------adv_report: addr_type=%d, addr=0x%x:0x%x:0x%x:0x%x:0x%x:0x%x, rssi=%d, tx_pwr=%d---------------------------\r\n",
        p_param->trans_addr.addr_type,p_param->trans_addr.addr.addr[0],p_param->trans_addr.addr.addr[1],p_param->trans_addr.addr.addr[2],p_param->trans_addr.addr.addr[3],p_param->trans_addr.addr.addr[4],p_param->trans_addr.addr.addr[5],
        p_param->rssi, p_param->tx_pwr);
#endif
    memcpy(&(app_env_info.ext_adv_rep_info),p_param,sizeof(struct gapm_ext_adv_report_ind));
    return KE_MSG_CONSUMED;

}


static int gapm_scan_request_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_scan_request_ind *p_param = (struct gapm_scan_request_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_dev_version_ind_handler\r\n");
#endif
    memcpy(&(app_env_info.scan_req_info),p_param,sizeof(struct gapm_scan_request_ind));
    return (KE_MSG_CONSUMED);
}



static int gapm_sync_established_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_sync_established_ind *p_param = (struct gapm_sync_established_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_sync_established_ind_handler\r\n");
#endif
    memcpy(&(app_env_info.sync_established_info),p_param,sizeof(struct gapm_sync_established_ind));
    return (KE_MSG_CONSUMED);
}




static int gapm_peer_name_ind_handler(ke_msg_id_t const msgid,
                                      void const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    struct gapm_peer_name_ind *p_param = (struct gapm_peer_name_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_peer_name_ind_handler\r\n");
#endif
    memcpy(&(app_env_info.peer_name_info),p_param,sizeof(struct gapm_peer_name_ind));
    return (KE_MSG_CONSUMED);
}


static int gapm_le_test_end_ind_handler(ke_msg_id_t const msgid,
                                        void const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    struct gapm_le_test_end_ind *p_param = (struct gapm_le_test_end_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_le_test_end_ind_handler\r\n");
#endif
    memcpy(&(app_env_info.le_test_end_info),p_param,sizeof(struct gapm_le_test_end_ind));
    return (KE_MSG_CONSUMED);
}


static int gapm_pub_key_ind_handler(ke_msg_id_t const msgid,
                                    void const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    struct gapm_pub_key_ind *p_param = (struct gapm_pub_key_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_pub_key_ind_handler\r\n");
#endif
    memcpy(&(app_env_info.pub_key_info),p_param,sizeof(struct gapm_pub_key_ind));
    return (KE_MSG_CONSUMED);
}


static int gapm_gen_dh_key_ind_handler(ke_msg_id_t const msgid,
                                       void const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_gen_dh_key_ind_handler\r\n");
#endif

    return (KE_MSG_CONSUMED);
}







static int gapm_profile_added_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_profile_added_ind *p_param = (struct gapm_profile_added_ind *)param;
    // Current State
    ke_state_t state = ke_state_get(dest_id);

#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_profile_added_ind_handler state=0x%x\r\n",state);
#endif
    memcpy(&(app_env_info.prof_add),p_param,sizeof(struct gapm_profile_added_ind));

    return (KE_MSG_CONSUMED);
}





static int gapm_cmp_evt_handler(ke_msg_id_t const msgid,
                                void const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    struct gapm_cmp_evt *p_param = (struct gapm_cmp_evt *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"app_gapm_cmp_evt_handler: operation=0x%x, status=0x%x\r\n", p_param->operation,p_param->status);
#endif
    ke_msg_sync_lock_release();
    //memcpy(&(app_env_info.cmp_evt_info),p_param,sizeof(struct gapm_cmp_evt));
    switch(p_param->operation)
    {
    // Reset completed
    case (GAPM_RESET):
    {
        if(p_param->status == GAP_ERR_NO_ERROR)
        {
            struct ln_gapm_set_dev_config_cmd cfg_param;
            memset(&cfg_param,0,sizeof(struct ln_gapm_set_dev_config_cmd));
            // Set Data length parameters
            cfg_param.sugg_max_tx_octets = BLE_MIN_OCTETS;
            cfg_param.sugg_max_tx_time   = BLE_MIN_TIME;
            // Host privacy enabled by default
            cfg_param.privacy_cfg = 0;
            memset((void *)&cfg_param.irk.key[0], 0x00, KEY_LEN);
            cfg_param.role    = GAP_ROLE_ALL;
            //cfg_param.max_mtu = 1200;//2048;
            //cfg_param.max_mps = 1200;//2048;
            ln_app_set_dev_config(&cfg_param);

        }
        else
        {
            ASSERT_ERR(0);
        }
    }
    break;

    // Device Configuration updated
    case (GAPM_SET_DEV_CONFIG):
    {
        ASSERT_INFO(p_param->status == GAP_ERR_NO_ERROR, p_param->operation, p_param->status);
        // Go to the create db state
        ke_state_set(TASK_APP, APP_CREATE_DB);
    }
    break;
    default:
        break;
    }
    return (KE_MSG_CONSUMED);
}


static int gapm_dev_version_ind_handler(ke_msg_id_t const msgid,
                                        void const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    struct gapm_dev_version_ind *p_param = (struct gapm_dev_version_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_dev_version_ind_handler\r\n");
#endif
    memcpy(&(app_env_info.dev_version_info),p_param,sizeof(struct gapm_dev_version_ind));
    return (KE_MSG_CONSUMED);
}







static int gapm_dev_bdaddr_ind_handler(ke_msg_id_t const msgid,
                                       void const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    struct gapm_dev_bdaddr_ind *p_param = (struct gapm_dev_bdaddr_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"app_gapm_dev_bdaddr_ind_handler \r\n");
#endif
    memcpy(&(app_env_info.dev_bdadr_info),p_param,sizeof(struct gapm_dev_bdaddr_ind));
    return (KE_MSG_CONSUMED);
}




static int gapm_dev_adv_tx_power_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_dev_adv_tx_power_ind *p_param = (struct gapm_dev_adv_tx_power_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_dev_adv_tx_power_ind_handler \r\n");
#endif
    memcpy(&(app_env_info.adv_tx_power),p_param,sizeof(struct gapm_dev_adv_tx_power_ind));
    return (KE_MSG_CONSUMED);
}





static int gapm_sugg_dflt_data_len_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_sugg_dflt_data_len_ind *p_param = (struct gapm_sugg_dflt_data_len_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_sugg_dflt_data_len_ind_handler \r\n");
#endif
    memcpy(&(app_env_info.dflt_data_len),p_param,sizeof(struct gapm_sugg_dflt_data_len_ind));
    return (KE_MSG_CONSUMED);
}




static int gapm_max_data_len_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_max_data_len_ind *p_param = (struct gapm_max_data_len_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_max_data_len_ind_handler \r\n");
#endif
    memcpy(&(app_env_info.max_data_len),p_param,sizeof(struct gapm_max_data_len_ind));
    return (KE_MSG_CONSUMED);
}





static int gapm_list_size_ind_handler(ke_msg_id_t const msgid,
                                      void const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    struct gapm_list_size_ind *p_param = (struct gapm_list_size_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_list_size_ind_handler \r\n");
#endif
    memcpy(&(app_env_info.list_size),p_param,sizeof(struct gapm_list_size_ind));
    return (KE_MSG_CONSUMED);
}


static int gapm_nb_adv_sets_ind_handler(ke_msg_id_t const msgid,
                                        void const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    struct gapm_nb_adv_sets_ind *p_param = (struct gapm_nb_adv_sets_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_nb_adv_sets_ind_handler \r\n");
#endif
    memcpy(&(app_env_info.nb_adv_sets),p_param,sizeof(struct gapm_nb_adv_sets_ind));
    return (KE_MSG_CONSUMED);
}


static int gapm_max_adv_data_len_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_max_adv_data_len_ind *p_param = (struct gapm_max_adv_data_len_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_max_adv_data_len_ind_handler \r\n");
#endif
    memcpy(&(app_env_info.max_adv_data_len),p_param,sizeof(struct gapm_max_adv_data_len_ind));
    return (KE_MSG_CONSUMED);
}


static int gapm_dev_tx_pwr_ind_handler(ke_msg_id_t const msgid,
                                       void const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    struct gapm_dev_tx_pwr_ind *p_param = (struct gapm_dev_tx_pwr_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_dev_tx_pwr_ind_handler \r\n");
#endif
    memcpy(&(app_env_info.dev_tx_pwr),p_param,sizeof(struct gapm_dev_tx_pwr_ind));
    return (KE_MSG_CONSUMED);
}




static int gapm_addr_solved_ind_handler(ke_msg_id_t const msgid,
                                        void const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    struct gapm_addr_solved_ind *p_param = (struct gapm_addr_solved_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_addr_solved_ind_handler \r\n");
#endif
    memcpy(&(app_env_info.addr_solv_info),p_param,sizeof(struct gapm_addr_solved_ind));
    return (KE_MSG_CONSUMED);
}




static int gapm_use_enc_block_handler(ke_msg_id_t const msgid,
                                      void const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    struct gapm_use_enc_block_ind *p_param = (struct gapm_use_enc_block_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_use_enc_block_handler \r\n");
#endif
    memcpy(&(app_env_info.use_enc_block),p_param,sizeof(struct gapm_use_enc_block_ind));
    return (KE_MSG_CONSUMED);
}





static int gapm_gen_rand_nb_ind_handler(ke_msg_id_t const msgid,
                                        void const *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gapm_gen_rand_nb_ind *p_param = (struct gapm_gen_rand_nb_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_gen_rand_nb_ind_handler\r\n");
#endif
    memcpy(&(app_env_info.gen_rand),p_param,sizeof(struct gapm_gen_rand_nb_ind));
    return (KE_MSG_CONSUMED);
}





static int gapm_ral_addr_ind_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gapm_ral_addr_ind *p_param = (struct gapm_ral_addr_ind *)param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapm_ral_addr_ind_handler\r\n");
#endif
    memcpy(&(app_env_info.ral_addr_info),p_param,sizeof(struct gapm_ral_addr_ind));
    return (KE_MSG_CONSUMED);
}


static int gapc_get_dev_info_req_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_get_dev_info_req_ind *p_param = (struct gapc_get_dev_info_req_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_get_dev_info_req_ind_handler param->req=0x%x,conidx=0x%x\r\n",p_param->req,conidx);
#endif
    memcpy(&(app_env_info.get_dev_info_req_info),p_param,sizeof(struct gapc_get_dev_info_req_ind));
    switch(p_param->req)
    {
    case GAPC_DEV_NAME:
    {
        struct gapc_get_dev_info_cfm * cfm = KE_MSG_ALLOC_DYN(GAPC_GET_DEV_INFO_CFM,
                                             src_id, dest_id,
                                             gapc_get_dev_info_cfm, APP_DEVICE_NAME_MAX_LEN);
        cfm->req = p_param->req;
        //zhongbo
        //cfm->info.name.length = app_get_dev_name(cfm->info.name.value);

        // Send message
        ke_msg_send(cfm);
    }
    break;

    case GAPC_DEV_APPEARANCE:
    {
        // Allocate message
        struct gapc_get_dev_info_cfm *cfm = KE_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM,
                                            src_id, dest_id,
                                            gapc_get_dev_info_cfm);
        cfm->req = p_param->req;
        // Set the device appearance
        // Send message
        ke_msg_send(cfm);
    }
    break;

    case GAPC_DEV_SLV_PREF_PARAMS:
    {
        // Allocate message
        struct gapc_get_dev_info_cfm *cfm = KE_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM,
                                            src_id, dest_id,
                                            gapc_get_dev_info_cfm);
        cfm->req = p_param->req;
        // Slave preferred Connection interval Min
        cfm->info.slv_pref_params.con_intv_min = 8;
        // Slave preferred Connection interval Max
        cfm->info.slv_pref_params.con_intv_max = 10;
        // Slave preferred Connection latency
        cfm->info.slv_pref_params.slave_latency  = 0;
        // Slave preferred Link supervision timeout
        cfm->info.slv_pref_params.conn_timeout    = 200;  // 2s (500*10ms)

        // Send message
        ke_msg_send(cfm);
    }
    break;

    default: /* Do Nothing */
        break;
    }

    return (KE_MSG_CONSUMED);
}


static int gapc_set_dev_info_req_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_set_dev_info_req_ind *p_param = (struct gapc_set_dev_info_req_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_set_dev_info_req_ind_handler  param->req=0x%x,conidx=0x%x\r\n",p_param->req,conidx);
#endif
    memcpy(&(app_env_info.set_dev_info_req_info),p_param,sizeof(struct gapc_set_dev_info_req_ind));
    // Set Device configuration
    struct gapc_set_dev_info_cfm* cfm = KE_MSG_ALLOC(GAPC_SET_DEV_INFO_CFM, src_id, dest_id,
                                        gapc_set_dev_info_cfm);
    // Reject to change parameters
    cfm->status = 0;
    cfm->req = p_param->req;
    // Send message  //zhongbo
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}


static int gapc_connection_req_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{


    struct gapc_connection_req_ind *p_param = (struct gapc_connection_req_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE," #######    gapc_connection_req_ind_handler conhdl=0x%x,  conidx=0x%x , conn_intv:%d, role=%d    addr: 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x  ###########\r\n",p_param->conhdl,conidx,p_param->con_interval,p_param->role,p_param->peer_addr.addr[0],p_param->peer_addr.addr[1],p_param->peer_addr.addr[2],p_param->peer_addr.addr[3],p_param->peer_addr.addr[4],p_param->peer_addr.addr[5]);
#endif
    // Check if the received connection index is valid
    if (conidx != GAP_INVALID_CONIDX)
    {
        // Allocate connection confirmation
        struct gapc_connection_cfm *cfm = KE_MSG_ALLOC(GAPC_CONNECTION_CFM,
                                          KE_BUILD_ID(TASK_GAPC,conidx), TASK_APP,
                                          gapc_connection_cfm);
        app_env_info.conn_req_info.clk_accuracy=p_param->clk_accuracy;
        app_env_info.conn_req_info.conhdl=p_param->conhdl;
        app_env_info.conn_req_info.conidx=conidx;
        app_env_info.conn_req_info.con_interval=p_param->con_interval;
        app_env_info.conn_req_info.con_latency=p_param->con_latency;
        memcpy(app_env_info.conn_req_info.peer_addr.addr,p_param->peer_addr.addr,GAP_BD_ADDR_LEN);
        app_env_info.conn_req_info.peer_addr_type=p_param->peer_addr_type;
        app_env_info.conn_req_info.role=p_param->role;
        app_env_info.conn_req_info.sup_to=p_param->sup_to;
        // Send the message
        ke_msg_send(cfm);
    }
    con_num++;
    struct ln_gapc_connection_req_info *data = blib_malloc(sizeof(struct ln_gapc_connection_req_info ));
    data->conidx=conidx;
    usr_queue_msg_send(BLE_MSG_CONN_IND,sizeof(struct ln_gapc_connection_req_info ),data);
    return (KE_MSG_CONSUMED);
}


static int gapc_param_update_req_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_param_update_req_ind *p_param = (struct gapc_param_update_req_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_param_update_req_ind_handler  conidx=0x%x,intv_max=0x%x,intv_min=0x%x,latency=0x%x,time_out=0x%x\r\n",conidx,p_param->intv_max,p_param->intv_min,p_param->latency,p_param->time_out);
#endif
    // Check if the received Connection Handle was valid
    if (conidx != GAP_INVALID_CONIDX)
    {
        // Send connection confirmation
        struct gapc_param_update_cfm *cfm = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CFM,
                                            KE_BUILD_ID(TASK_GAPC,conidx), TASK_APP,
                                            gapc_param_update_cfm);
        cfm->accept = true;
        cfm->ce_len_min = 0;
        cfm->ce_len_max = 0xFFFF;
        memcpy(&(app_env_info.param_upd_info),p_param,sizeof(struct gapc_param_update_req_ind));
        // Send message
        ke_msg_send(cfm);
    }
    return (KE_MSG_CONSUMED);
}


static int gapc_param_updated_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_param_updated_ind *p_param = (struct gapc_param_updated_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_param_updated_ind_handler conidx=0x%x,con_interval=0x%x,con_latency=0x%x,sup_to=0x%x\r\n",conidx,p_param->con_interval,p_param->con_latency,p_param->sup_to);
#endif
    memcpy(&(app_env_info.param_updated_info),p_param,sizeof(struct gapc_param_updated_ind));
    return (KE_MSG_CONSUMED);
}





static int gapc_cmp_evt_handler(ke_msg_id_t const msgid,
                                void const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    struct gapc_cmp_evt const *p_param = (struct gapc_cmp_evt const *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_cmp_evt_handler   operation=0x%x, status=0x%x   conidx=0x%x\r\n",p_param->operation,p_param->status,conidx);
#endif
    memcpy(&(app_env_info.gapc_cmp_info),p_param,sizeof(struct gapc_cmp_evt));
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}

static int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                       void const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    struct gapc_disconnect_ind *p_param = (struct gapc_disconnect_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_disconnect_ind_handler conidx=0x%x, reason=0x%x\r\n",conidx,p_param->reason);
#endif
    app_env_info.dis_conn_info.conhdl=p_param->conhdl;
    app_env_info.dis_conn_info.reason=p_param->reason;
    app_env_info.dis_conn_info.conidx=conidx;
    con_num--;
    ke_msg_sync_lock_release();
#if (SLAVE)
    app_restart_adv();
#endif
#if (MASTER)
    app_restart_init();
#endif
    return (KE_MSG_CONSUMED);
}



static int gapc_peer_att_info_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_peer_att_info_ind *p_param = (struct gapc_peer_att_info_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"app_gapc_peer_att_info_ind_handler handle=0x%x, req=0x%x,conidx=0x%x\r\n",p_param->handle,p_param->req,conidx);
#endif

    memcpy(&(app_env_info.peer_att_info),p_param,sizeof(struct gapc_peer_att_info_ind));
    return (KE_MSG_CONSUMED);
}



static int gapc_peer_version_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_peer_version_ind *p_param = (struct gapc_peer_version_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_peer_version_ind_handler conidx=0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.peer_version_info),p_param,sizeof(struct gapc_peer_version_ind));
    return (KE_MSG_CONSUMED);
}



static int gapc_peer_features_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_peer_features_ind *p_param = (struct gapc_peer_features_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_peer_features_ind_handler conidx=0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.peer_features_info),p_param,sizeof(struct gapc_peer_features_ind));
    return (KE_MSG_CONSUMED);
}



static int gapc_con_rssi_ind_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    struct gapc_con_rssi_ind *p_param = (struct gapc_con_rssi_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_con_rssi_ind_handler conidx=0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.con_rssi_info),p_param,sizeof( struct gapc_con_rssi_ind));
    return (KE_MSG_CONSUMED);
}



static int gapc_con_channel_map_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_con_channel_map_ind *p_param = (struct gapc_con_channel_map_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_con_channel_map_ind_handler conidx=0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.con_channel_map_info),p_param,sizeof( struct gapc_con_channel_map_ind));
    return (KE_MSG_CONSUMED);
}



static int gapc_le_ping_to_val_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_le_ping_to_val_ind *p_param = (struct gapc_le_ping_to_val_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_le_ping_to_val_ind_handler conidx=0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.le_ping_to_val_info),p_param,sizeof( struct gapc_le_ping_to_val_ind));
    return (KE_MSG_CONSUMED);
}




static int gapc_le_phy_ind_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    struct gapc_le_phy_ind *p_param = (struct gapc_le_phy_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_le_phy_ind_handler conidx=0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.le_phy_info),p_param,sizeof( struct gapc_le_phy_ind));
    return (KE_MSG_CONSUMED);
}



static int gapc_chan_sel_algo_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_chan_sel_algo_ind *p_param = (struct gapc_chan_sel_algo_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_chan_sel_algo_ind_handler conidx=0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.chan_sel_algo_info),p_param,sizeof( struct gapc_chan_sel_algo_ind));
    return (KE_MSG_CONSUMED);
}


static int gapc_bond_ind_handler(ke_msg_id_t const msgid,
                                 void const *param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id)
{
    struct gapc_bond_ind *p_param = (struct gapc_bond_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_bond_ind_handler conidx=0x%x,info=%d \r\n",conidx,p_param->info);
#endif
    memcpy(&(app_env_info.bond_ind_info),p_param,sizeof( struct gapc_bond_ind));

    switch (p_param->info)
    {
    case (GAPC_PAIRING_SUCCEED):
    {
#if (TRACE_ENABLE)
        LOG(LOG_LVL_TRACE,"GAPC_PAIRING_SUCCEED auth=%d,ltk_present=%d\r\n",
            p_param->data.auth.info,p_param->data.auth.ltk_present);
#endif
    }
    break;

    case (GAPC_REPEATED_ATTEMPT):
    {

    } break;

    case (GAPC_IRK_EXCH):
    {
    } break;

    case (GAPC_PAIRING_FAILED):
    {

    } break;

    // In Secure Connections we get BOND_IND with SMPC calculated LTK
    case (GAPC_LTK_EXCH) :
    {
#if (TRACE_ENABLE)
        LOG(LOG_LVL_TRACE, "GAPC_LTK_EXCH \r\n");
#endif

#if (TRACE_ENABLE)
        LOG(LOG_LVL_TRACE, "nvds_put param->ediv=0x%x\r\nparam_randnb:",p_param->data.ltk.ediv);
        for(uint8_t i=0; i<GAP_RAND_NB_LEN; i++)
            LOG(LOG_LVL_TRACE, "%x",p_param->data.ltk.randnb.nb[i]);
        LOG(LOG_LVL_TRACE,"\r\nltk:");
        for(uint8_t i=0; i<GAP_KEY_LEN; i++)
            LOG(LOG_LVL_TRACE, "%x",p_param->data.ltk.ltk.key[i]);
        LOG(LOG_LVL_TRACE,"\r\n");
#endif

    }
    break;

    default:
    {

    } break;
    }
    return (KE_MSG_CONSUMED);
}


static int gapc_bond_req_ind_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    struct gapc_bond_req_ind *p_param = (struct gapc_bond_req_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_bond_req_ind_handler: request=%d, auth_req=%d,conidx:%d\r\n",
        p_param->request,p_param->data.auth_req,conidx);
#endif
    memcpy(&(app_env_info.bond_req_info),p_param,sizeof( struct gapc_bond_req_ind));


    // Prepare the GAPC_BOND_CFM message
    struct gapc_bond_cfm *cfm = KE_MSG_ALLOC(GAPC_BOND_CFM,
                                src_id, TASK_APP,
                                gapc_bond_cfm);


    switch (p_param->request)
    {
    case (GAPC_PAIRING_REQ):
    {
        cfm->request = GAPC_PAIRING_RSP;
        {
            cfm->accept  = true;

            cfm->data.pairing_feat.key_size  = 16;
            cfm->data.pairing_feat.oob       = GAP_OOB_AUTH_DATA_NOT_PRESENT;
            cfm->data.pairing_feat.sec_req   = GAP_SEC1_NOAUTH_PAIR_ENC;//GAP_SEC1_AUTH_PAIR_ENC;//GAP_NO_SEC;

            cfm->data.pairing_feat.rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY;
            cfm->data.pairing_feat.ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY;

#if (TRACE_ENABLE)
            LOG(LOG_LVL_TRACE,"GAPC_PAIRING_REQ pairing_feat.auth=0x%x\r\n",cfm->data.pairing_feat.auth);
#endif
        }
    }
    break;

    case (GAPC_LTK_EXCH):
    {
        // Counter
        uint8_t counter;

        cfm->accept  = true;
        cfm->request = GAPC_LTK_EXCH;

        // Generate all the values
        cfm->data.ltk.ediv = (uint16_t)co_rand_word();

        for (counter = 0; counter < RAND_NB_LEN; counter++)
        {
            cfm->data.ltk.ltk.key[counter]    = (uint8_t)co_rand_word();
            cfm->data.ltk.randnb.nb[counter] = (uint8_t)co_rand_word();
        }

        for (counter = RAND_NB_LEN; counter < KEY_LEN; counter++)
        {
            cfm->data.ltk.ltk.key[counter]    = (uint8_t)co_rand_word();
        }

#if (TRACE_ENABLE)
        LOG(LOG_LVL_TRACE, "nvds_put cfm->ediv=0x%x\r\nparam_randnb:",cfm->data.ltk.ediv);
        for(uint8_t i=0; i<GAP_RAND_NB_LEN; i++)
            LOG(LOG_LVL_TRACE, "%x",cfm->data.ltk.randnb.nb[i]);
        LOG(LOG_LVL_TRACE,"\r\nltk:");
        for(uint8_t i=0; i<GAP_KEY_LEN; i++)
            LOG(LOG_LVL_TRACE, "%x",cfm->data.ltk.ltk.key[i]);
        LOG(LOG_LVL_TRACE,"\r\n");
#endif

    }
    break;


    case (GAPC_IRK_EXCH):
    {

        cfm->accept  = true;
        cfm->request = GAPC_IRK_EXCH;

        // Load IRK
    //  memcpy(cfm->data.irk.irk.key, app_env.loc_irk, KEY_LEN);


        // load device address
        cfm->data.irk.addr.addr_type = (cfm->data.irk.addr.addr.addr[5] & 0xC0) ? ADDR_RAND : ADDR_PUBLIC;
    }
    break;


    //#if (BLE_APP_HT)
    case (GAPC_TK_EXCH):
    {
        // Generate a PIN Code- (Between 100000 and 999999)
        uint32_t pin_code = (100000 + (co_rand_word()%900000));
#if (TRACE_ENABLE)
        LOG(LOG_LVL_TRACE,"app_sec GAPC_TK_EXCH: tk_type=%d\r\n",p_param->data.tk_type);
#endif


        cfm->accept  = true;
        cfm->request = GAPC_TK_EXCH;

        // Set the TK value
        memset(cfm->data.tk.key, 0, KEY_LEN);

        cfm->data.tk.key[0] = (uint8_t)((pin_code & 0x000000FF) >>  0);
        cfm->data.tk.key[1] = (uint8_t)((pin_code & 0x0000FF00) >>  8);
        cfm->data.tk.key[2] = (uint8_t)((pin_code & 0x00FF0000) >> 16);
        cfm->data.tk.key[3] = (uint8_t)((pin_code & 0xFF000000) >> 24);
#if (TRACE_ENABLE)
        LOG(LOG_LVL_TRACE,"###GAPC_TK_EXCH pincode=%d\r\n", pin_code);
#endif
    }
    break;
    //#endif //(BLE_APP_HT)

    default:
        break;
    }

    // Send the message
    ke_msg_send(cfm);
    return (KE_MSG_CONSUMED);
}



static int gapc_key_press_notification_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_key_press_notif_ind *p_param = (struct gapc_key_press_notif_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_key_press_notification_ind_handler conidx=0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.key_press_notif_info),p_param,sizeof( struct gapc_key_press_notif_ind));
    return (KE_MSG_CONSUMED);
}




static int gapc_encrypt_ind_handler(ke_msg_id_t const msgid,
                                    void const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    struct gapc_encrypt_ind *p_param = (struct gapc_encrypt_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_encrypt_ind_handler conidx=0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.encrypt_ind_info),p_param,sizeof( struct gapc_encrypt_ind));
    return (KE_MSG_CONSUMED);
}


static int gapc_encrypt_req_ind_handler(ke_msg_id_t const msgid,
                                        void const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    struct gapc_encrypt_req_ind *p_param = (struct gapc_encrypt_req_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
    // Prepare the GAPC_ENCRYPT_CFM message
    struct gapc_encrypt_cfm *cfm = KE_MSG_ALLOC(GAPC_ENCRYPT_CFM,
                                   src_id, TASK_APP,
                                   gapc_encrypt_cfm);
    cfm->found    = false;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_encrypt_req_ind_handler: conidx=%d\r\n", conidx);
#endif
    memcpy(&(app_env_info.encrypt_req_info),p_param,sizeof( struct gapc_encrypt_req_ind));
    ke_msg_send(cfm);
    return (KE_MSG_CONSUMED);
}




static int gapc_security_ind_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    struct gapc_security_ind *p_param = (struct gapc_security_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_security_ind_handler conidx=0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.security_ind_info),p_param,sizeof( struct gapc_security_ind));
    return (KE_MSG_CONSUMED);
}


static int gapc_le_pkt_size_ind_handler(ke_msg_id_t const msgid,
                                        void const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    struct gapc_le_pkt_size_ind *p_param = (struct gapc_le_pkt_size_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_le_pkt_size_ind_handler conidx=0x%x,max_rx_octets:%d,max_rx_time:%d,max_tx_octets:%d, max_tx_time :%d\r\n",conidx,p_param->max_rx_octets,p_param->max_rx_time,p_param->max_tx_octets,p_param->max_tx_time);
#endif
    memcpy(&(app_env_info.le_pkt_size_info),p_param,sizeof( struct gapc_le_pkt_size_ind));
    return (KE_MSG_CONSUMED);
}






static int gapc_le_ping_to_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_le_ping_to_handler conidx=0x%x\r\n",conidx);
#endif
    return (KE_MSG_CONSUMED);
}



static int gapc_sign_counter_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_sign_counter_ind *p_param = (struct gapc_sign_counter_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gapc_sign_counter_ind_handler conidx=0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.sign_counter_info),p_param,sizeof( struct gapc_sign_counter_ind));
    return (KE_MSG_CONSUMED);
}





static int app_msg_handler(ke_msg_id_t const msgid,
                           void const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    // Retrieve identifier of the task from received message
    ke_task_id_t src_task_id = MSG_T(msgid);
    // Message policy
    //uint8_t msg_pol = KE_MSG_CONSUMED;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"app_msg_handler src_task_id=%d, msgid=0x%x\r\n",src_task_id,msgid);
#endif
    return (KE_MSG_CONSUMED);
}


KE_MSG_HANDLER_TAB(app)
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER,    (ke_msg_func_t)app_msg_handler},
    //GAPM
    {GAPM_CMP_EVT, gapm_cmp_evt_handler},
    {GAPM_DEV_VERSION_IND, gapm_dev_version_ind_handler},
    {GAPM_DEV_BDADDR_IND, gapm_dev_bdaddr_ind_handler},
    {GAPM_DEV_ADV_TX_POWER_IND, gapm_dev_adv_tx_power_ind_handler},
    {GAPM_ADDR_SOLVED_IND, gapm_addr_solved_ind_handler},
    {GAPM_USE_ENC_BLOCK_IND, gapm_use_enc_block_handler},
    {GAPM_GEN_RAND_NB_IND, gapm_gen_rand_nb_ind_handler},
    {GAPM_SUGG_DFLT_DATA_LEN_IND, gapm_sugg_dflt_data_len_ind_handler},
    {GAPM_MAX_DATA_LEN_IND, gapm_max_data_len_ind_handler},
    {GAPM_LIST_SIZE_IND, gapm_list_size_ind_handler},
    {GAPM_NB_ADV_SETS_IND, gapm_nb_adv_sets_ind_handler},
    {GAPM_MAX_ADV_DATA_LEN_IND, gapm_max_adv_data_len_ind_handler},
    {GAPM_DEV_TX_PWR_IND, gapm_dev_tx_pwr_ind_handler},
    {GAPM_RAL_ADDR_IND, gapm_ral_addr_ind_handler},
    {GAPM_ACTIVITY_CREATED_IND, gapm_activity_created_ind_handler},
    {GAPM_ACTIVITY_STOPPED_IND, gapm_activity_stopped_ind_handler},
    {GAPM_EXT_ADV_REPORT_IND,  gapm_ext_adv_report_ind_handler},
    {GAPM_SCAN_REQUEST_IND, gapm_scan_request_ind_handler},
    {GAPM_SYNC_ESTABLISHED_IND, gapm_sync_established_ind_handler},
    {GAPM_PEER_NAME_IND, gapm_peer_name_ind_handler},
    {GAPM_PROFILE_ADDED_IND, gapm_profile_added_ind_handler},
    {GAPM_LE_TEST_END_IND, gapm_le_test_end_ind_handler},
    {GAPM_PUB_KEY_IND, gapm_pub_key_ind_handler},
    {GAPM_GEN_DH_KEY_IND, gapm_gen_dh_key_ind_handler},
    //GAPC
    {GAPC_CMP_EVT, gapc_cmp_evt_handler },
    {GAPC_CONNECTION_REQ_IND, gapc_connection_req_ind_handler},
    {GAPC_DISCONNECT_IND, gapc_disconnect_ind_handler},
    {GAPC_PEER_ATT_INFO_IND, gapc_peer_att_info_ind_handler},
    {GAPC_PEER_VERSION_IND, gapc_peer_version_ind_handler},
    {GAPC_PEER_FEATURES_IND, gapc_peer_features_ind_handler},
    {GAPC_CON_RSSI_IND, gapc_con_rssi_ind_handler},
    {GAPC_CON_CHANNEL_MAP_IND, gapc_con_channel_map_ind_handler},
    {GAPC_LE_PING_TO_VAL_IND, gapc_le_ping_to_val_ind_handler},
    {GAPC_LE_PHY_IND, gapc_le_phy_ind_handler},
    {GAPC_CHAN_SEL_ALGO_IND, gapc_chan_sel_algo_ind_handler},
    {GAPC_PARAM_UPDATED_IND, gapc_param_updated_ind_handler},
    {GAPC_BOND_IND, gapc_bond_ind_handler},
    {GAPC_LE_PKT_SIZE_IND, gapc_le_pkt_size_ind_handler},
    {GAPC_SECURITY_IND, gapc_security_ind_handler},
    {GAPC_ENCRYPT_IND, gapc_encrypt_ind_handler},
    {GAPC_LE_PING_TO_IND, gapc_le_ping_to_handler},
    {GAPC_GET_DEV_INFO_REQ_IND, gapc_get_dev_info_req_ind_handler},
    {GAPC_SET_DEV_INFO_REQ_IND, gapc_set_dev_info_req_ind_handler},
    {GAPC_PARAM_UPDATE_REQ_IND, gapc_param_update_req_ind_handler},
    {GAPC_BOND_REQ_IND, gapc_bond_req_ind_handler},
    {GAPC_ENCRYPT_REQ_IND, gapc_encrypt_req_ind_handler},
    {GAPC_KEY_PRESS_NOTIFICATION_IND, gapc_key_press_notification_ind_handler},
    {GAPC_SIGN_COUNTER_IND, gapc_sign_counter_ind_handler},
    //GATTM
    {GATTM_ADD_SVC_RSP, gattm_add_svc_rsp_handler },
    {GATTM_SVC_GET_PERMISSION_RSP, gattm_svc_get_permission_rsp_handler},
    {GATTM_SVC_SET_PERMISSION_RSP, gattm_svc_set_permission_rsp_handler},
    {GATTM_ATT_GET_PERMISSION_RSP, gattm_att_get_permission_rsp_handler},
    {GATTM_ATT_SET_PERMISSION_RSP, gattm_att_set_permission_rsp_handler},
    {GATTM_ATT_GET_VALUE_RSP, gattm_att_get_value_rsp_handler},
    {GATTM_ATT_SET_VALUE_RSP, gattm_att_set_value_rsp_handler},
    {GATTM_UNKNOWN_MSG_IND, gattm_unknown_msg_ind_handler},
    {GATTM_DESTROY_DB_RSP, gattm_destroy_db_rsp_handler},
    {GATTM_SVC_GET_LIST_RSP, gattm_svc_get_list_rsp_handler},
    {GATTM_ATT_GET_INFO_RSP, gattm_att_get_info_rsp_handler},
    {GATTM_ATT_DB_HASH_COMP_RSP, gattm_att_db_hash_comp_rsp_handler},
    //GATTC
    {GATTC_CMP_EVT, gattc_cmp_evt_handler },
    {GATTC_MTU_CHANGED_IND, gattc_mtu_changed_ind_handler},
    {GATTC_DISC_SVC_IND, gattc_app_disc_svc_ind_handler},
    {GATTC_DISC_SVC_INCL_IND, gattc_app_disc_svc_incl_ind_handler},
    {GATTC_DISC_CHAR_IND, gattc_app_disc_char_ind_handler},
    {GATTC_DISC_CHAR_DESC_IND, gattc_app_disc_char_desc_ind_handler},
    {GATTC_READ_IND, gattc_read_ind_handler},
    {GATTC_EVENT_IND, gattc_event_ind_handler},
    {GATTC_EVENT_REQ_IND, gattc_event_req_ind_handler},
    {GATTC_READ_REQ_IND, gattc_read_req_ind_handler},
    {GATTC_WRITE_REQ_IND, gattc_write_req_ind_handler},
    {GATTC_ATT_INFO_REQ_IND, gattc_att_info_req_ind_handler},
    {GATTC_SDP_SVC_IND, gattc_sdp_svc_ind_handler},
    {GATTC_TRANSACTION_TO_ERROR_IND, gattc_transaction_to_error_ind_handler},
};


/// Number of APP Task Instances
#define APP_IDX_MAX                 (1)

ke_state_t app_state[APP_IDX_MAX];

const struct ke_task_desc TASK_DESC_APP = {app_msg_handler_tab, app_state, APP_IDX_MAX, ARRAY_LEN(app_msg_handler_tab)};
/// @} APP

