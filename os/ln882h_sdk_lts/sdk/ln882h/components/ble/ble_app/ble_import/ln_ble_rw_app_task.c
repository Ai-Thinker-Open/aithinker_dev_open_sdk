/**
 ****************************************************************************************
 *
 * @file ln_ble_rw_app_task.c
 *
 * @brief RivieraWaves app task source code
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

#include "rwip_config.h"             // SW configuration
#include "rwip_task.h"      // Task definitions
#include "ke_task.h"        // Kernel Task
#include "co_utils.h"

#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manallger Task API
#include "gapc_task.h"               // GAP Controller Task API
#include "gattm_task.h"               // GAP Manallger Task API
#include "gattc_task.h"               // GAP Controller Task API

#include "utils/debug/log.h"

#include "ln_ble_rw_app_task.h"
#include "ln_ble_app_default_cfg.h"
#include "ln_ble_app_err_code.h"


/// Number of APP Task Instances
#define APP_IDX_MAX                     (1)
#define APP_MSG_NUM_MAX                 (100)

static int app_msg_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);

static uint8_t g_app_msg_num = 1;
ke_state_t app_state[APP_IDX_MAX];

static struct ke_msg_handler app_default_msg_tab[APP_MSG_NUM_MAX] = 
{
   // Note: first message is latest message checked by kernel so default is put on top.
   {KE_MSG_DEFAULT_HANDLER,    (ke_msg_func_t)app_msg_handler},
};

struct ke_task_desc TASK_DESC_APP = {(const struct ke_msg_handler* )app_default_msg_tab, app_state, APP_IDX_MAX, ARRAY_LEN(app_default_msg_tab)};

static int app_msg_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    // Retrieve identifier of the task from received message
    ke_task_id_t src_task_id = MSG_T(msgid);

    LOG(LOG_LVL_TRACE,"app_msg_handler src_task_id=%d, msgid=0x%x\r\n",src_task_id,msgid);

    return (KE_MSG_CONSUMED);
}

int ln_app_msg_regester(struct ke_msg_handler *handler, uint8_t msg_num)
{
    if((g_app_msg_num+msg_num) > APP_MSG_NUM_MAX)
        return BLE_ERR_SIZE_EXCEEDED;

    GLOBAL_INT_DISABLE();
    memcpy(app_default_msg_tab+g_app_msg_num, handler, msg_num*sizeof(struct ke_msg_handler));
    g_app_msg_num +=msg_num;
    TASK_DESC_APP.msg_cnt = g_app_msg_num;
    GLOBAL_INT_RESTORE();

    return BLE_ERR_NONE;
}

void ln_rw_app_task_init(void)
{
    // Create APP task
    ke_task_create(TASK_APP, (const struct ke_task_desc*)(&TASK_DESC_APP));

    // Initialize Task state
    ke_state_set(TASK_APP, APP_INIT);
}
