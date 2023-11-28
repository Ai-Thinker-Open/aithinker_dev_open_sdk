/**
 ****************************************************************************************
 *
 * @file ln_ble_scan.c
 *
 * @brief scan APIs source code
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

#include "rwip_config.h"
#include "gapm_task.h"               // GAP Manallger Task API
#include "gapc_task.h"               // GAP Controller Task API
#include "co_utils.h"

#include "utils/debug/log.h"

#include "ln_ble_scan.h"
#include "ln_ble_rw_app_task.h"
#include "ln_ble_event_manager.h"
#include "ln_ble_app_default_cfg.h"


ln_ble_scan_manager_t ble_scan_mgr;

static bool ln_ble_scan_is_filter(struct gapm_ext_adv_report_ind *p_para);

static int ln_ble_scan_report_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_ext_adv_report_ind *p_param = (struct gapm_ext_adv_report_ind *)param;

    LOG(LOG_LVL_TRACE, "ln_ble_scan_report_ind_handler\r\n");
    //hexdump(LOG_LVL_INFO, "adv data: ", (void *)p_param->data, p_param->length);

    if(ln_ble_scan_is_filter(p_param))
    {
        ble_evt_scan_report_t *p_evt_scan = blib_malloc(sizeof(ble_evt_scan_report_t) + p_param->length);
        if(p_evt_scan != NULL)
        {
            p_evt_scan->actv_idx            = p_param->actv_idx;
            p_evt_scan->info                = p_param->info;
            p_evt_scan->trans_addr_type     = p_param->trans_addr.addr_type;
            p_evt_scan->target_addr_type    = p_param->target_addr.addr_type;
            p_evt_scan->tx_pwr              = p_param->tx_pwr;
            p_evt_scan->rssi                = p_param->rssi;
            p_evt_scan->length              = p_param->length;

            memcpy(p_evt_scan->trans_addr,  p_param->trans_addr.addr.addr,  6);
            memcpy(p_evt_scan->target_addr, p_param->target_addr.addr.addr, 6);
            memcpy(p_evt_scan->data,        p_param->data, p_param->length);

            ln_ble_evt_mgr_process(BLE_EVT_ID_SCAN_REPORT, p_evt_scan);
            blib_free(p_evt_scan);
        }
    }

    return KE_MSG_CONSUMED;
}

adv_data_item_t *ln_ble_scan_data_find_item_by_type(uint8_t type, uint8_t *data, uint8_t len)
{
    adv_data_item_t *item = NULL;
    uint8_t data_offset = 0;
    do {
        item = (adv_data_item_t *)(data + data_offset);
        //LOG(LOG_LVL_INFO, "ln_ble_scan_report_data_parser item->len=%d,item->type=%d\r\n", item->len, item->type);
        if(type == item->type)
        {
            return item;
        }
        data_offset += item->len + 1;
    } while(data_offset < len);

    return NULL;
}

void ln_ble_scan_actv_creat(void)
{
    struct gapm_activity_create_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_CREATE_CMD, TASK_GAPM, TASK_APP, gapm_activity_create_cmd);
    p_cmd->operation        = GAPM_CREATE_SCAN_ACTIVITY;
    p_cmd->own_addr_type    = GAPM_STATIC_ADDR;

    ln_ke_msg_send(p_cmd);
}

static bool ln_ble_scan_is_filter(struct gapm_ext_adv_report_ind *p_param)
{
    if(ble_scan_mgr.filter.filter_type & SCAN_FILTER_TYPE_RSSI)
    {
        if(ble_scan_mgr.filter.rssi > p_param->rssi)
            return false;
    }
    if(ble_scan_mgr.filter.filter_type & SCAN_FILTER_TYPE_TX_PWR)
    {
        if(ble_scan_mgr.filter.tx_pwr > p_param->tx_pwr)
            return false;
    }
    if(ble_scan_mgr.filter.filter_type & SCAN_FILTER_TYPE_PHY)
    {
        if(ble_scan_mgr.filter.phy != p_param->phy_prim || ble_scan_mgr.filter.phy != p_param->phy_second)
            return false;
    }
    if(ble_scan_mgr.filter.filter_type & SCAN_FILTER_TYPE_ADDR_TYPE)
    {
        if(ble_scan_mgr.filter.find_addr_type != p_param->trans_addr.addr_type)
            return false;
    }
    if(ble_scan_mgr.filter.filter_type & SCAN_FILTER_TYPE_ADDR)
    {
        if(memcmp(ble_scan_mgr.filter.find_addr, p_param->trans_addr.addr.addr, GAP_BD_ADDR_LEN))
            return false;
    }
    if(ble_scan_mgr.filter.filter_type & SCAN_SFILTER_TYPE_ADV_DATA)
    {
        if(ble_scan_mgr.filter.pattern_len <= p_param->length
                && ble_scan_mgr.filter.pattern_len)
        {
            uint8_t offset = p_param->length - ble_scan_mgr.filter.pattern_len;
            for(int i = 0; i <= offset; i++)
            {
                for(int j = 0;j < ble_scan_mgr.filter.pattern_len; j++)
                {
                    if(ble_scan_mgr.filter.pattern_data[j] != p_param->data[i + j])
                        break;
                    if(ble_scan_mgr.filter.pattern_len == (j + 1))
                        return true;
                }
            }
        }
        return false;
    }
    return true;
}

void ln_ble_scan_set_rpt_filter(le_scan_report_filter_t *filter)
{
    ln_ble_scan_clear_rpt_filter();
    memcpy(&ble_scan_mgr.filter, filter, sizeof(le_scan_report_filter_t));
    if(filter->pattern_len > SCAN_RPT_DATA_MAX_LENGTH) {
        ble_scan_mgr.filter.pattern_len = SCAN_RPT_DATA_MAX_LENGTH;
    }
}

void ln_ble_scan_clear_rpt_filter(void)
{
    memset(&ble_scan_mgr.filter, 0, sizeof(le_scan_report_filter_t));
}

void ln_ble_scan_start(le_scan_parameters_t *param)
{
    struct gapm_activity_start_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_START_CMD, TASK_GAPM, TASK_APP, gapm_activity_start_cmd);
    p_cmd->operation                                        = GAPM_START_ACTIVITY;
    p_cmd->actv_idx                                         = ble_scan_mgr.scan_actv_idx;
    p_cmd->u_param.scan_param.type                          = param->type;
    p_cmd->u_param.scan_param.prop                          = param->prop;
    p_cmd->u_param.scan_param.dup_filt_pol                  = param->dup_filt_pol;
    p_cmd->u_param.scan_param.scan_param_1m.scan_intv       = param->scan_intv; //(T = N * 0.625 ms);
    p_cmd->u_param.scan_param.scan_param_1m.scan_wd         = param->scan_wd;   //(T = N * 0.625 ms);
    p_cmd->u_param.scan_param.duration                      = 0;
    p_cmd->u_param.scan_param.scan_param_coded.scan_intv    = param->scan_intv; //(T = N * 0.625 ms);
    p_cmd->u_param.scan_param.scan_param_coded.scan_wd      = param->scan_wd;   //(T = N * 0.625 ms);
    p_cmd->u_param.scan_param.period                        = 2;

    ln_ke_msg_send(p_cmd);

    ble_scan_mgr.scan_param.type            = param->type;
    ble_scan_mgr.scan_param.prop            = param->prop;
    ble_scan_mgr.scan_param.dup_filt_pol    = param->dup_filt_pol;
    ble_scan_mgr.scan_param.scan_intv       = param->scan_intv;
    ble_scan_mgr.scan_param.scan_wd         = param->scan_wd;
    ble_scan_mgr.state                      = LE_SCAN_STATE_STARTING;
}

void ln_ble_scan_stop(void)
{
    struct gapm_activity_stop_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_STOP_CMD, TASK_GAPM, TASK_GAPM, gapm_activity_stop_cmd);

    p_cmd->operation = GAPM_STOP_ACTIVITY;
    p_cmd->actv_idx  = ble_scan_mgr.scan_actv_idx;

    ke_msg_send(p_cmd);

    ble_scan_mgr.state = LE_SCAN_STATE_STOPING;
}

static struct ke_msg_handler app_scan_msg_tab[] =
{
    {GAPM_EXT_ADV_REPORT_IND,  ln_ble_scan_report_ind_handler},
};

int ln_ble_scan_mgr_init(void)
{
    int ret = 0;

    memset(&ble_scan_mgr, 0, sizeof(ble_scan_mgr));
    ble_scan_mgr.state = LE_SCAN_STATE_INITIALIZED;
    ble_scan_mgr.scan_actv_idx = APP_ACTV_INVALID_IDX;

    ble_scan_mgr.scan_param.type            = GAPM_SCAN_TYPE_OBSERVER;
    ble_scan_mgr.scan_param.prop            = GAPM_SCAN_PROP_PHY_1M_BIT | GAPM_SCAN_PROP_PHY_CODED_BIT;
    ble_scan_mgr.scan_param.dup_filt_pol    = GAPM_DUP_FILT_EN;
    ble_scan_mgr.scan_param.scan_intv       = SCAN_INTERVAL_DEF;
    ble_scan_mgr.scan_param.scan_wd         = SCAN_WINDOW_DEF;

    ret = ln_app_msg_regester(app_scan_msg_tab, ARRAY_LEN(app_scan_msg_tab));

    return ret;
}
