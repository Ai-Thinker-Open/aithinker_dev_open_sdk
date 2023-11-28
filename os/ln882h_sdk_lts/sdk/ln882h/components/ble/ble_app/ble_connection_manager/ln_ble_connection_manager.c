/**
 ****************************************************************************************
 *
 * @file ln_ble_connection_manager.c
 *
 * @brief connection manager APIs source code
 *
 * Copyright (C) 2021. Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>

#include "gapm_task.h"               // GAP Manallger Task API
#include "gapc_task.h"               // GAP Controller Task API
#include "rwip_config.h"
#include "ble_port.h"

#include "utils/debug/log.h"

#include "ln_ble_connection_manager.h"
#include "ln_ble_device_manager.h"
#include "ln_ble_event_manager.h"
#include "ln_ble_gap.h"
#include "ln_ble_smp.h"
#include "ln_ble_app_kv.h"
#include "ln_ble_rw_app_task.h"
#include "ln_ble_app_default_cfg.h"


ble_dev_conn_manager_t ble_conn_mgr;

/**
 ****************************************************************************************
 *
 * @intv_min N x 1.25ms (7.5ms--4s)
 *
 * @intv_max N x 1.25ms (7.5ms--4s)
 *
 ****************************************************************************************
 */
const ln_ble_conn_param_t conn_param_table[LN_BLE_CONN_MODE_MAX] =
{
    {80, 90, 10, 3000},
    {50, 60, 5,  3000},
    {30, 40, 0,  3000},
};

static void ln_ble_conn_mgr_env_update(uint8_t con_idx, struct gapc_connection_req_ind *p_param);
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
static int ln_ble_conn_req_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_connection_req_ind *p_param = (struct gapc_connection_req_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_ble_conn_req_ind_handler conhdl=0x%x,conidx=0x%x,conn_intv:%d,role=%d,peer_addr_type=%d,addr:0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\r\n",
            p_param->conhdl, conidx, p_param->con_interval, p_param->role,
            p_param->peer_addr_type, p_param->peer_addr.addr[0], p_param->peer_addr.addr[1], p_param->peer_addr.addr[2],
            p_param->peer_addr.addr[3], p_param->peer_addr.addr[4], p_param->peer_addr.addr[5]);

    // Check if the received connection index is valid
    if (conidx != GAP_INVALID_CONIDX)
    {
        // Allocate connection confirmation
        struct gapc_connection_cfm *cfm = KE_MSG_ALLOC(GAPC_CONNECTION_CFM,
                                                KE_BUILD_ID(TASK_GAPC,conidx), TASK_APP,
                                                gapc_connection_cfm);

        ke_msg_send(cfm);
    }

    if(conidx != GAP_INVALID_CONIDX)
    {
        ble_conn_mgr.conn_num++;
        ln_ble_conn_mgr_env_update(conidx, (struct gapc_connection_req_ind *)p_param);

#if (BLE_CONFIG_SECURITY)
        if(!ble_conn_mgr.con_env[conidx].is_bonded && (GAPM_STATIC_ADDR == ble_conn_mgr.con_env[conidx].dev_remote.addr_type)
                && (ln_kv_ble_usr_data_get()->ble_role & BLE_ROLE_PERIPHERAL))
            ln_smp_req_security(conidx, GAP_AUTH_REQ_NO_MITM_BOND);
#endif
    }

    if (conidx != GAP_INVALID_CONIDX)
    {
        ble_evt_connected_t evt_conn = {0};
        evt_conn.conn_idx       = conidx;
        evt_conn.con_interval   = p_param->con_interval;
        evt_conn.con_latency    = p_param->con_latency;
        evt_conn.sup_to         = p_param->sup_to;
        evt_conn.peer_addr_type = p_param->peer_addr_type;
        evt_conn.role           = p_param->role;
        memcpy(evt_conn.peer_addr, p_param->peer_addr.addr, 6);

        ln_ble_evt_mgr_process(BLE_EVT_ID_CONNECTED, &evt_conn);
    }

    return (KE_MSG_CONSUMED);
}

static int ln_ble_disc_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_disconnect_ind *p_param = (struct gapc_disconnect_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_ble_disc_ind_handler conidx=0x%x, reason=0x%x\r\n", conidx, p_param->reason);

    if(conidx != GAP_INVALID_CONIDX)
    {
        ble_conn_mgr.conn_num--;
        memset(&ble_conn_mgr.con_env[conidx], 0, sizeof(le_dev_con_env_t));
        ble_conn_mgr.con_env[conidx].state       = LE_CONN_STATE_DISCONNECTED;
        ble_conn_mgr.con_env[conidx].is_bonded   = false;
        ble_conn_mgr.con_env[conidx].conn_handle = APP_CONN_INVALID_HANDLE;
    }

    if (conidx != GAP_INVALID_CONIDX)
    {
        ble_evt_disconnected_t evt_disconn = {0};
        evt_disconn.conn_idx = conidx;
        evt_disconn.reason   = p_param->reason;

        ln_ble_evt_mgr_process(BLE_EVT_ID_DISCONNECTED, &evt_disconn);
    }

    return (KE_MSG_CONSUMED);
}

static int ln_ble_conn_param_update_req_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_param_update_req_ind *p_param = (struct gapc_param_update_req_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gap_param_update_req_ind_handler  conidx=0x%x,intv_max=0x%x,intv_min=0x%x,latency=0x%x,time_out=0x%x\r\n",conidx,p_param->intv_max,p_param->intv_min,p_param->latency,p_param->time_out);

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
        // Send message
        ke_msg_send(cfm);
    }
    return (KE_MSG_CONSUMED);
}

static int ln_ble_conn_param_updated_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_param_updated_ind *p_param = (struct gapc_param_updated_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_ble_conn_param_updated_ind_handler conidx=0x%x,con_interval=0x%x,con_latency=0x%x,sup_to=0x%x\r\n",conidx,p_param->con_interval,p_param->con_latency,p_param->sup_to);

    return (KE_MSG_CONSUMED);
}

static void ln_ble_conn_mgr_env_update(uint8_t con_idx, struct gapc_connection_req_ind *p_param)
{
    bt_device_info_t *dev_info = NULL;
    le_dev_con_env_t *con_env = &ble_conn_mgr.con_env[con_idx];

    con_env->state = LE_CONN_STATE_CONNECTED;
    con_env->conn_handle = p_param->conhdl;
    con_env->dev_remote.role = p_param->role;
    con_env->dev_remote.addr_type = p_param->peer_addr_type;
    memcpy(&con_env->dev_remote.addr, p_param->peer_addr.addr, GAP_BD_ADDR_LEN);

    dev_info = ln_ble_get_dev_by_addr(&con_env->dev_remote.addr);
    if(dev_info && (dev_info->mru > 0 && dev_info->mru < MAX_DEV_BOND_MUN))
    {
        con_env->is_bonded = true;
        memcpy(&con_env->dev_remote.ltk, &dev_info->ltk, sizeof(ln_gap_ltk_t));
        /* device is not lastest, update mru*/
        if(dev_info->mru > 1)
            ln_ble_dev_insert(&con_env->dev_remote);
    }
    else
    {
        memset(&con_env->dev_remote.ltk, 0, sizeof(struct gapc_ltk));
        con_env->is_bonded = false;
    }
}

void ln_ble_conn_mode_set(int conidx, ln_ble_conn_mode_t mode)
{
    // Prepare the GAPC_PARAM_UPDATE_CMD message
    struct gapc_param_update_cmd *p_cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
                                        KE_BUILD_ID(TASK_GAPC,conidx), TASK_APP,
                                        gapc_param_update_cmd);
    p_cmd->operation    = GAPC_UPDATE_PARAMS;
    p_cmd->intv_min = conn_param_table[mode].intv_min;
    p_cmd->intv_max = conn_param_table[mode].intv_max;
    p_cmd->latency  = conn_param_table[mode].latency;
    p_cmd->time_out = conn_param_table[mode].time_out;
    // not used by a slave device
    p_cmd->ce_len_min = 0;
    p_cmd->ce_len_max = 0xFFFF;

    ln_ke_msg_send(p_cmd);
}

void ln_ble_conn_param_update(int conidx, ln_ble_conn_param_t *p_conn_param)
{
    // Prepare the GAPC_PARAM_UPDATE_CMD message
    struct gapc_param_update_cmd *p_cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
                                          KE_BUILD_ID(TASK_GAPC,conidx), TASK_APP,
                                          gapc_param_update_cmd);
    p_cmd->operation = GAPC_UPDATE_PARAMS;
    p_cmd->intv_min = p_conn_param->intv_min;
    p_cmd->intv_max = p_conn_param->intv_max;
    p_cmd->latency  = p_conn_param->latency;
    p_cmd->time_out = p_conn_param->time_out;
    // not used by a slave device
    p_cmd->ce_len_min = 0;
    p_cmd->ce_len_max = 0xFFFF;

    ln_ke_msg_send(p_cmd);

    memcpy(&ble_conn_mgr.con_param, p_conn_param, sizeof(ln_ble_conn_param_t));
}

void ln_ble_disc_req(int conidx)
{
    struct gapc_disconnect_cmd *p_cmd = KE_MSG_ALLOC(GAPC_DISCONNECT_CMD,
                                        KE_BUILD_ID(TASK_GAPC,conidx), TASK_APP,
                                        gapc_disconnect_cmd);
    p_cmd->operation = GAPC_DISCONNECT;
    p_cmd->reason = CO_ERROR_NO_ERROR;

    ln_ke_msg_send(p_cmd);
}

void ln_ble_conn_req(ln_bd_addr_t *addr)
{
    // Prepare the GAPM_ACTIVITY_START_CMD message
    struct gapm_activity_start_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_START_CMD,
                                            TASK_GAPM, TASK_APP,
                                            gapm_activity_start_cmd);
    p_cmd->operation = GAPM_START_ACTIVITY;
    p_cmd->actv_idx = ble_conn_mgr.init_actv_idx;
    p_cmd->u_param.init_param.type = GAPM_INIT_TYPE_DIRECT_CONN_EST;//GAPM_INIT_TYPE_DIRECT_CONN_EST;;
    p_cmd->u_param.init_param.prop = GAPM_INIT_PROP_1M_BIT | GAPM_INIT_PROP_CODED_BIT;
    p_cmd->u_param.init_param.conn_to = 0;
    p_cmd->u_param.init_param.scan_param_1m.scan_intv = 16;
    p_cmd->u_param.init_param.scan_param_1m.scan_wd = 16;
    p_cmd->u_param.init_param.scan_param_coded.scan_intv =16;
    p_cmd->u_param.init_param.scan_param_coded.scan_wd = 16;

    p_cmd->u_param.init_param.conn_param_1m.conn_intv_min = 80;
    p_cmd->u_param.init_param.conn_param_1m.conn_intv_max = 80;
    p_cmd->u_param.init_param.conn_param_1m.conn_latency = 0;
    p_cmd->u_param.init_param.conn_param_1m.ce_len_min = 0;
    p_cmd->u_param.init_param.conn_param_1m.ce_len_max = 0xFFFF;
    p_cmd->u_param.init_param.conn_param_1m.supervision_to= 500;

    p_cmd->u_param.init_param.conn_param_2m.conn_intv_min = 80;
    p_cmd->u_param.init_param.conn_param_2m.conn_intv_max = 80;
    p_cmd->u_param.init_param.conn_param_2m.conn_latency = 0;
    p_cmd->u_param.init_param.conn_param_2m.ce_len_min = 0;
    p_cmd->u_param.init_param.conn_param_2m.ce_len_max = 0xFFFF;
    p_cmd->u_param.init_param.conn_param_2m.supervision_to= 500;

    p_cmd->u_param.init_param.conn_param_coded.conn_intv_min = 80;
    p_cmd->u_param.init_param.conn_param_coded.conn_intv_max = 80;
    p_cmd->u_param.init_param.conn_param_coded.conn_latency = 0;
    p_cmd->u_param.init_param.conn_param_coded.ce_len_min = 0;
    p_cmd->u_param.init_param.conn_param_coded.ce_len_max = 0xFFFF;
    p_cmd->u_param.init_param.conn_param_coded.supervision_to= 500;

    p_cmd->u_param.init_param.peer_addr.addr_type = GAPM_STATIC_ADDR;
    memcpy(&p_cmd->u_param.init_param.peer_addr.addr, addr, GAP_BD_ADDR_LEN);

    ln_ke_msg_send(p_cmd);
}

void ln_ble_init_actv_creat(void)
{
    // Prepare the GAPM_ACTIVITY_CREATE_CMD message
    struct gapm_activity_create_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_CREATE_CMD,
            TASK_GAPM, TASK_APP,
            gapm_activity_create_cmd);

    // Set operation code
    p_cmd->operation = GAPM_CREATE_INIT_ACTIVITY;
    p_cmd->own_addr_type = GAPM_STATIC_ADDR;

    ln_ke_msg_send(p_cmd);
}

bool ln_ble_is_connected(void)
{
    for(int i=0;i<BLE_CONN_DEV_NUM_MAX;i++)
    {
        if(LE_CONN_STATE_CONNECTED == ble_conn_mgr.con_env[i].state)
            return true;
    }

    return false;
}

static struct ke_msg_handler app_conn_msg_tab[] =
{
    {GAPC_CONNECTION_REQ_IND, ln_ble_conn_req_ind_handler},
    {GAPC_DISCONNECT_IND, ln_ble_disc_ind_handler},
    {GAPC_PARAM_UPDATE_REQ_IND, ln_ble_conn_param_update_req_ind_handler},
    {GAPC_PARAM_UPDATED_IND, ln_ble_conn_param_updated_ind_handler},
};

int ln_ble_conn_mgr_init(void)
{
    int ret = 0;

    memset(&ble_conn_mgr, 0, sizeof(ble_dev_conn_manager_t));

    ble_conn_mgr.init_actv_idx = APP_ACTV_INVALID_IDX;
    ble_conn_mgr.con_param = conn_param_table[LN_BLE_CONN_MODE_NORMAL];
    for(int i=0; i<BLE_CONN_DEV_NUM_MAX; i++)
    {
        le_dev_con_env_t *con_env = &ble_conn_mgr.con_env[i];
        con_env->state = LE_CONN_STATE_INITIALIZED;
        con_env->conn_handle = APP_CONN_INVALID_HANDLE;
        con_env->is_bonded = false;
    }
    ret = ln_app_msg_regester(app_conn_msg_tab, ARRAY_LEN(app_conn_msg_tab));

    return ret;
}
