/**
 ****************************************************************************************
 *
 * @file ln_ble_advertising.c
 *
 * @brief advertising APIs source code
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
#include "co_bt_defines.h"

#include "utils/debug/log.h"

#include "ln_ble_advertising.h"
#include "ln_ble_app_default_cfg.h"
#include "ln_ble_app_err_code.h"


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

ln_ble_adv_manager_t ble_adv_mgr;

int ln_ble_adv_actv_creat(adv_param_t *param)
{
    if (param == NULL) { return BLE_ERR_INVALID_PARAM; }
    
    struct gapm_activity_create_adv_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_CREATE_CMD, TASK_GAPM, TASK_APP, gapm_activity_create_adv_cmd);
    p_cmd->operation            = GAPM_CREATE_ADV_ACTIVITY;
    p_cmd->own_addr_type        = param->own_addr_type;
    p_cmd->adv_param.type       = param->adv_type;
    p_cmd->adv_param.disc_mode  = GAPM_ADV_MODE_GEN_DISC;

    p_cmd->adv_param.prop = param->adv_prop;
   
    p_cmd->adv_param.max_tx_pwr = 0;
    p_cmd->adv_param.filter_pol = ADV_ALLOW_SCAN_ANY_CON_ANY;

    p_cmd->adv_param.peer_addr.addr_type = GAPM_STATIC_ADDR;
    memset(p_cmd->adv_param.peer_addr.addr.addr, 0, GAP_BD_ADDR_LEN);

    p_cmd->adv_param.prim_cfg.adv_intv_min  = param->adv_intv_min;
    p_cmd->adv_param.prim_cfg.adv_intv_max  = param->adv_intv_max;
    p_cmd->adv_param.prim_cfg.chnl_map      = APP_ADV_CHMAP;
    p_cmd->adv_param.prim_cfg.phy           = GAP_PHY_1MBPS;

    if(GAPM_ADV_TYPE_LEGACY != p_cmd->adv_param.type)
    {
        p_cmd->adv_param.second_cfg.max_skip    = 0x00;
        p_cmd->adv_param.second_cfg.phy         = GAP_PHY_1MBPS;
        p_cmd->adv_param.second_cfg.adv_sid     = 0x00;
    }
    if(GAPM_ADV_TYPE_PERIODIC == p_cmd->adv_param.type)
    {
        p_cmd->adv_param.period_cfg.adv_intv_min = 0x0400;
        p_cmd->adv_param.period_cfg.adv_intv_max = 0x0400;
    }

    ln_ke_msg_send(p_cmd);
    
    ble_adv_mgr.adv_param.own_addr_type = param->own_addr_type;
    ble_adv_mgr.adv_param.adv_type      = param->adv_type;
    ble_adv_mgr.adv_param.adv_intv_min  = param->adv_intv_min;
    ble_adv_mgr.adv_param.adv_intv_max  = param->adv_intv_max;
    
    return BLE_ERR_NONE;
}

int ln_ble_adv_data_set(ln_adv_data_t *adv_data)
{
    if (ble_adv_mgr.adv_param.adv_type == GAPM_ADV_TYPE_LEGACY && adv_data->length > ADV_DATA_LEGACY_MAX){
        return BLE_ERR_INVALID_PARAM;
    }else if (ble_adv_mgr.adv_param.adv_type == GAPM_ADV_TYPE_EXTENDED && adv_data->length > ADV_DATA_EXTEND_MAX){
        return BLE_ERR_INVALID_PARAM;
    }
    struct gapm_set_adv_data_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_SET_ADV_DATA_CMD, TASK_GAPM, TASK_APP, gapm_set_adv_data_cmd, adv_data->length);
    p_cmd->operation    = GAPM_SET_ADV_DATA;
    p_cmd->actv_idx     = ble_adv_mgr.adv_actv_idx;
    p_cmd->length       = adv_data->length;
    memcpy(p_cmd->data, adv_data->data, adv_data->length);

    ln_ke_msg_send(p_cmd);
    return BLE_ERR_NONE;
}

int ln_ble_adv_scan_rsp_data_set(ln_adv_data_t *scan_rsp_data)
{
    if (ble_adv_mgr.adv_param.adv_type == GAPM_ADV_TYPE_LEGACY && scan_rsp_data->length > ADV_DATA_LEGACY_MAX){
        return BLE_ERR_INVALID_PARAM;
    }else if (ble_adv_mgr.adv_param.adv_type == GAPM_ADV_TYPE_EXTENDED && scan_rsp_data->length > ADV_DATA_EXTEND_MAX){
        return BLE_ERR_INVALID_PARAM;
    }
    struct gapm_set_adv_data_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_SET_ADV_DATA_CMD, TASK_GAPM, TASK_APP, gapm_set_adv_data_cmd, scan_rsp_data->length);
    p_cmd->operation    = GAPM_SET_SCAN_RSP_DATA;
    p_cmd->actv_idx     = ble_adv_mgr.adv_actv_idx;
    p_cmd->length       = scan_rsp_data->length;
    memcpy(p_cmd->data, scan_rsp_data->data, scan_rsp_data->length);

    ln_ke_msg_send(p_cmd);
    return BLE_ERR_NONE;
}

void ln_ble_adv_start(void)
{
    struct gapm_activity_start_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_START_CMD, TASK_GAPM, TASK_APP, gapm_activity_start_cmd);
    p_cmd->operation                            = GAPM_START_ACTIVITY;
    p_cmd->actv_idx                             = ble_adv_mgr.adv_actv_idx;
    p_cmd->u_param.adv_add_param.duration       = 0;
    p_cmd->u_param.adv_add_param.max_adv_evt    = 0;

    ln_ke_msg_send(p_cmd);

    ble_adv_mgr.state = LE_ADV_STATE_STARTING;
}

void ln_ble_adv_stop(void)
{
    struct gapm_activity_stop_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_STOP_CMD, TASK_GAPM, TASK_GAPM, gapm_activity_stop_cmd);
    p_cmd->operation = GAPM_STOP_ACTIVITY;
    p_cmd->actv_idx  = ble_adv_mgr.adv_actv_idx;

    ke_msg_send(p_cmd);

    ble_adv_mgr.state = LE_ADV_STATE_STOPING;
}

int ln_ble_adv_mgr_init(void)
{
    memset(&ble_adv_mgr, 0, sizeof(ble_adv_mgr));

    ble_adv_mgr.state = LE_ADV_STATE_INITIALIZED;
    ble_adv_mgr.adv_actv_idx = APP_ACTV_INVALID_IDX;

    ble_adv_mgr.adv_param.own_addr_type = GAPM_STATIC_ADDR;
#ifdef BLE_USE_LEGACY_ADV
    ble_adv_mgr.adv_param.adv_type      = GAPM_ADV_TYPE_LEGACY;
    ble_adv_mgr.adv_param.adv_prop      = GAPM_ADV_PROP_UNDIR_CONN_MASK;
#else
    ble_adv_mgr.adv_param.adv_type      = GAPM_ADV_TYPE_EXTENDED;
    ble_adv_mgr.adv_param.adv_prop      = GAPM_EXT_ADV_PROP_UNDIR_CONN_MASK;
#endif
    ble_adv_mgr.adv_param.adv_intv_min  = APP_ADV_INT_MIN;
    ble_adv_mgr.adv_param.adv_intv_max  = APP_ADV_INT_MAX;

    return BLE_ERR_NONE;
}
