/**
 ****************************************************************************************
 *
 * @file ln_ble_gatt.c
 *
 * @brief GATT APIs source code
 *
 * Copyright (C) 2021.Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>

#include "gattm_task.h"               // GATT Manager Task API
#include "gattc_task.h"               // GATT Controller Task API
#include "co_utils.h"

#include "utils/debug/log.h"

#include "ln_ble_gatt.h"
#include "ln_ble_gatt_ind_handler.h"
#include "ln_ble_rw_app_task.h"


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void ln_gatt_exc_mtu(int conidx)
{
    struct gattc_exc_mtu_cmd *p_cmd = KE_MSG_ALLOC(GATTC_EXC_MTU_CMD,
                                      KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                                      gattc_exc_mtu_cmd);
    p_cmd->operation = GATTC_MTU_EXCH;
    /// identify the command
    p_cmd->seq_num = 0;

    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_gatt_sdp(int conidx, ln_gatt_sdp_cmd_t *p_param)
{
    struct gattc_sdp_svc_disc_cmd *p_cmd = KE_MSG_ALLOC_DYN(GATTC_SDP_SVC_DISC_CMD,
                                           KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                                           gattc_sdp_svc_disc_cmd, p_param->uuid_len);
    p_cmd->operation=p_param->operation;
    p_cmd->seq_num = 5;
    p_cmd->start_hdl = p_param->start_hdl;
    p_cmd->end_hdl = p_param->end_hdl;
    p_cmd->uuid_len = p_param->uuid_len;
    memcpy(p_cmd->uuid, p_param->uuid, p_cmd->uuid_len);

    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_gatt_discovery(int conidx, ln_gatt_disc_cmd_t *p_param)
{
    struct gattc_disc_cmd *p_cmd = KE_MSG_ALLOC_DYN(GATTC_DISC_CMD,
                                   KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                                   gattc_disc_cmd, p_param->uuid_len);
    p_cmd->operation = p_param->operation;
    p_cmd->seq_num = 1;
    p_cmd->start_hdl = p_param->start_hdl;
    p_cmd->end_hdl = p_param->end_hdl;
    p_cmd->uuid_len = p_param->uuid_len;
    if (p_cmd->uuid_len)
    {
        memcpy(p_cmd->uuid, p_param->uuid, p_cmd->uuid_len);
    }
    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_gatt_add_svc(ln_gatt_add_svc_req_t *p_param)
{
    struct gattm_add_svc_req *p_cmd = KE_MSG_ALLOC_DYN( GATTM_ADD_SVC_REQ,
                                      TASK_GATTM, TASK_APP,
                                      gattm_add_svc_req, (p_param->svc_desc.nb_att * sizeof(struct gattm_att_desc)));
    p_cmd->svc_desc.start_hdl = p_param->svc_desc.start_hdl;
    p_cmd->svc_desc.task_id = TASK_APP;
    p_cmd->svc_desc.perm = p_param->svc_desc.perm;
    memcpy(p_cmd->svc_desc.uuid, p_param->svc_desc.uuid, ATT_UUID_128_LEN);
    p_cmd->svc_desc.nb_att = p_param->svc_desc.nb_att;
    for (uint32_t i = 0; i < p_param->svc_desc.nb_att; i++)
    {
        memcpy(p_cmd->svc_desc.atts[i].uuid, p_param->svc_desc.atts[i].uuid, ATT_UUID_128_LEN);
        p_cmd->svc_desc.atts[i].perm = p_param->svc_desc.atts[i].perm;
        p_cmd->svc_desc.atts[i].max_len = p_param->svc_desc.atts[i].max_len;
        p_cmd->svc_desc.atts[i].ext_perm = p_param->svc_desc.atts[i].ext_perm;
    }
    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_gatt_read(int conidx, ln_gatt_read_cmd_t *p_param)
{
    struct gattc_read_cmd *p_cmd;
    if ((p_param->operation== GATTC_READ) ||(p_param->operation== GATTC_READ_LONG))
    {
        p_cmd = KE_MSG_ALLOC(GATTC_READ_CMD,
                             KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                             gattc_read_cmd);
        p_cmd->operation = p_param->operation;
        p_cmd->nb = 0;
        p_cmd->seq_num = 2;
        p_cmd->req.simple.handle = p_param->req.simple.handle;
        if (p_param->operation== GATTC_READ)    //For simple read, offset always 0. (Refer to 'gattc_read_cmd_handler')
            p_cmd->req.simple.offset = 0;
        else
            p_cmd->req.simple.offset = p_param->req.simple.offset;
        p_cmd->req.simple.length = p_param->req.simple.length;
    }
    else if (p_param->operation== GATTC_READ_BY_UUID)
    {
        p_cmd = KE_MSG_ALLOC_DYN( GATTC_READ_CMD,
                                  KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                                  gattc_read_cmd, p_param->req.by_uuid.uuid_len);
        p_cmd->operation = GATTC_READ_BY_UUID;
        p_cmd->nb = 0;
        p_cmd->seq_num = 2;
        p_cmd->req.by_uuid.start_hdl = p_param->req.by_uuid.start_hdl;
        p_cmd->req.by_uuid.end_hdl = p_param->req.by_uuid.end_hdl;
        p_cmd->req.by_uuid.uuid_len = p_param->req.by_uuid.uuid_len;
        memcpy(p_cmd->req.by_uuid.uuid, p_param->req.by_uuid.uuid, p_cmd->req.by_uuid.uuid_len);

    }
    else if (p_param->operation== GATTC_READ_MULTIPLE)
    {
        p_cmd = KE_MSG_ALLOC_DYN( GATTC_READ_CMD,
                                  KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                                  gattc_read_cmd, (p_param->nb * sizeof(struct gattc_read_multiple)));
        p_cmd->operation = GATTC_READ_MULTIPLE;
        p_cmd->nb = p_param->nb;
        p_cmd->seq_num = 2;
        for (uint32_t i = 0; i < p_cmd->nb; i++)
        {
            p_cmd->req.multiple[i].handle = p_param->req.multiple[i].handle;
            p_cmd->req.multiple[i].len = p_param->req.multiple[i].len;
        }
    }
    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_gatt_write(int conidx, ln_gatt_write_cmd_t *p_param)
{
    struct gattc_write_cmd *p_cmd = KE_MSG_ALLOC_DYN( GATTC_WRITE_CMD,
                                    KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                                    gattc_write_cmd, p_param->length);
    p_cmd->operation = p_param->operation;
    p_cmd->seq_num = 3;
    p_cmd->cursor = 0;
    p_cmd->auto_execute = p_param->auto_execute;
    p_cmd->handle = p_param->handle;
    p_cmd->offset = p_param->offset;
    p_cmd->length = p_param->length;
    memcpy(p_cmd->value, p_param->value, p_cmd->length);
    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_gatt_excute_write(int conidx, bool execute)
{
    struct gattc_execute_write_cmd *p_cmd = KE_MSG_ALLOC( GATTC_EXECUTE_WRITE_CMD,
                                            KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                                            gattc_execute_write_cmd);
    p_cmd->operation = GATTC_EXEC_WRITE;
    p_cmd->execute = execute;
    p_cmd->seq_num = 4;
    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_gatt_send_ind(int conidx, ln_gatt_send_evt_cmd_t *p_param )
{
    struct gattc_send_evt_cmd *p_cmd = KE_MSG_ALLOC_DYN( GATTC_SEND_EVT_CMD,
                                       KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                                       gattc_send_evt_cmd, p_param->length);
    p_cmd->operation = GATTC_INDICATE;
    p_cmd->seq_num = 6;
    p_cmd->handle = p_param->handle;
    p_cmd->length = p_param->length;
    memcpy(p_cmd->value, p_param->value, p_param->length);
    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_gatt_send_ntf(int conidx, ln_gatt_send_evt_cmd_t *p_param )
{
    struct gattc_send_evt_cmd *p_cmd = KE_MSG_ALLOC_DYN( GATTC_SEND_EVT_CMD,
                                       KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                                       gattc_send_evt_cmd, p_param->length);
    p_cmd->operation = GATTC_NOTIFY;
    p_cmd->seq_num = 7;
    p_cmd->handle = p_param->handle;
    p_cmd->length = p_param->length;
    memcpy(p_cmd->value, p_param->value, p_param->length);
    ln_ke_msg_send(p_cmd);
}

void ln_gatt_read_req_cfm(uint8_t conidx, ln_gatt_read_cfm_t *p_param)
{
    struct gattc_read_cfm *p_cmd = KE_MSG_ALLOC_DYN( GATTC_WRITE_CFM,
                                    KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                                    gattc_read_cfm, p_param->length);
    p_cmd->handle = p_param->handle;
    p_cmd->status = p_param->status;
    if(!p_param->length && NULL != p_param->value)
    {
        p_cmd->length = p_param->length;
        memcpy(p_cmd->value, p_param->value, p_param->length);
    }
    // Send the message
    ke_msg_send(p_cmd);
}

void ln_gatt_write_req_cfm(uint8_t conidx, ln_gatt_write_cfm_t *p_param)
{
    struct gattc_write_cfm *p_cmd = KE_MSG_ALLOC( GATTC_WRITE_CFM,
                                    KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                                    gattc_write_cfm);
    p_cmd->handle = p_param->handle;
    p_cmd->status = p_param->status;
    // Send the message
    ke_msg_send(p_cmd);
}

int ln_gatt_app_init(void)
{
    int ret = 0;

    ret = ln_app_msg_regester(app_gatt_general_msg_tab, app_gatt_msg_size());

    return ret;
}

