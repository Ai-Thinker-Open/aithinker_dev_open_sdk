/**
 ****************************************************************************************
 *
 * @file ln_ble_smp.c
 *
 * @brief smp APIs source code
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
#include "gap.h"                     // GAP Definition
#include "gapm.h"
#include "gapm_task.h"               // GAP Manallger Task API
#include "gapc_task.h"               // GAP Controller Task API
#include "gattm_task.h"               // GAP Manallger Task API
#include "gattc_task.h"               // GAP Controller Task API
#include "co_math.h"                 // Common Maths Definition
#include "co_utils.h"

#include "utils/debug/log.h"

#include "ln_ble_smp.h"
#include "ln_ble_gap.h"
#include "ln_ble_device_manager.h"
#include "ln_ble_connection_manager.h"
#include "ln_ble_rw_app_task.h"
#include "ln_ble_app_kv.h"
#include "ln_ble_app_default_cfg.h"

ln_smp_config_t g_sec_config;

static bt_dh_key_cb_t   g_dh_key_cb = NULL;
static bt_pub_key_cb    g_pub_key_cb = NULL;
static bt_rand_key_cb   g_rand_key_cb = NULL;
static uint8_t          g_irk_gen_key_len = 0;
static uint8_t          g_gen_irk[16] = {0};

const uint8_t debug_irk_key[] =
{
    0xe6, 0x9d, 0x89, 0x15, 0x89, 0x15, 0x15, 0x52,
    0x24, 0x30, 0xdc, 0xac, 0xc2, 0x5a, 0xbf, 0xef,
};

static int ln_smp_bond_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_bond_ind *p_param = (struct gapc_bond_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_smp_bond_ind_handler conidx=0x%x,info=%d \r\n",conidx,p_param->info);

    switch (p_param->info)
    {
        case (GAPC_PAIRING_SUCCEED):
        {
            LOG(LOG_LVL_TRACE,"GAPC_PAIRING_SUCCEED auth=%d,ltk_present=%d\r\n",
                p_param->data.auth.info,p_param->data.auth.ltk_present);
            if(p_param->data.auth.ltk_present)
            {
                ble_dev_conn_manager_t * con_mgr = le_conn_mgr_info_get();
                bt_device_info_t *new_dev = &con_mgr->con_env[conidx].dev_remote;
                ln_ble_dev_insert(new_dev);
                con_mgr->con_env[conidx].is_bonded = true;
            }
        }
            break;

        case (GAPC_REPEATED_ATTEMPT):
            break;

        case (GAPC_IRK_EXCH):
        {
            LOG(LOG_LVL_TRACE,"GAPC_IRK_EXCH\r\n");
            ble_dev_conn_manager_t * con_mgr = le_conn_mgr_info_get();
            bt_device_info_t *new_dev = &con_mgr->con_env[conidx].dev_remote;
            new_dev->addr_type = p_param->data.irk.addr.addr_type;
            memcpy(new_dev->irk.key, p_param->data.irk.irk.key, LN_GAP_KEY_LEN);
            memcpy(new_dev->addr.addr, p_param->data.irk.addr.addr.addr, GAP_BD_ADDR_LEN);
        }
            break;

        case (GAPC_PAIRING_FAILED):
            break;

        // In Secure Connections we get BOND_IND with SMPC calculated LTK
        case (GAPC_LTK_EXCH) :
        {
            LOG(LOG_LVL_TRACE, "GAPC_LTK_EXCH \r\n");

            LOG(LOG_LVL_TRACE, "nvds_put param->ediv=0x%x\r\nparam_randnb:",p_param->data.ltk.ediv);
            for(uint8_t i=0; i<GAP_RAND_NB_LEN; i++)
                LOG(LOG_LVL_TRACE, "%x",p_param->data.ltk.randnb.nb[i]);
            LOG(LOG_LVL_TRACE,"\r\nltk:");
            for(uint8_t i=0; i<GAP_KEY_LEN; i++)
                LOG(LOG_LVL_TRACE, "%x",p_param->data.ltk.ltk.key[i]);
            LOG(LOG_LVL_TRACE,"\r\n");
        }
            break;

        default:
            break;
    }
    return (KE_MSG_CONSUMED);
}

static int ln_smp_bond_req_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_bond_req_ind *p_param = (struct gapc_bond_req_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_smp_bond_req_ind_handler: request=%d, auth_req=%d,conidx:%d\r\n",
        p_param->request,p_param->data.auth_req,conidx);

    // Prepare the GAPC_BOND_CFM message
    struct gapc_bond_cfm *cfm = KE_MSG_ALLOC(GAPC_BOND_CFM,
                                src_id, TASK_APP,
                                gapc_bond_cfm);

    switch (p_param->request)
    {
        case (GAPC_PAIRING_REQ):
        {
            cfm->request = GAPC_PAIRING_RSP;
            cfm->accept  = true;

            cfm->data.pairing_feat.key_size     = g_sec_config.key_size;
            cfm->data.pairing_feat.iocap        = g_sec_config.iocap;
            cfm->data.pairing_feat.oob          = g_sec_config.oob;
            cfm->data.pairing_feat.sec_req      = g_sec_config.sec_req;
            cfm->data.pairing_feat.auth         = g_sec_config.auth;
            cfm->data.pairing_feat.rkey_dist    = g_sec_config.rkey_dist;
            cfm->data.pairing_feat.ikey_dist    = g_sec_config.ikey_dist;

            LOG(LOG_LVL_TRACE,"GAPC_PAIRING_REQ pairing_feat.auth=0x%x\r\n",cfm->data.pairing_feat.auth);
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

            if(cfm->accept)
            {
                ble_dev_conn_manager_t * con_mgr = le_conn_mgr_info_get();
                bt_device_info_t *new_dev = &con_mgr->con_env[conidx].dev_remote;
                memcpy(new_dev->ltk.ltk, cfm->data.ltk.ltk.key, LN_GAP_KEY_LEN);
                new_dev->ltk.ediv = cfm->data.ltk.ediv;
                memcpy(new_dev->ltk.randnb, cfm->data.ltk.randnb.nb, LN_RAND_NB_LEN);
                new_dev->ltk.key_size = cfm->data.ltk.key_size;
                //ln_ble_dev_insert(new_dev);
                //con_mgr->con_env[conidx].is_bonded = true;
            }

            LOG(LOG_LVL_TRACE, "nvds_put cfm->ediv=0x%x\r\nparam_randnb:",cfm->data.ltk.ediv);
            for(uint8_t i=0; i<GAP_RAND_NB_LEN; i++)
                LOG(LOG_LVL_TRACE, "%x",cfm->data.ltk.randnb.nb[i]);
            LOG(LOG_LVL_TRACE,"\r\nltk:");
            for(uint8_t i=0; i<GAP_KEY_LEN; i++)
                LOG(LOG_LVL_TRACE, "%x",cfm->data.ltk.ltk.key[i]);
            LOG(LOG_LVL_TRACE,"\r\n");
        }
            break;

        case (GAPC_IRK_EXCH):
        {
            uint8_t *irk = ln_kv_ble_irk_get();
            bd_addr_t *loc_addr = gapm_get_bdaddr();
            cfm->accept  = true;
            cfm->request = GAPC_IRK_EXCH;

            if(!irk)
                irk = (uint8_t *)debug_irk_key;
            // Load IRK
            memcpy(cfm->data.irk.irk.key, irk, KEY_LEN);

            // load device address
            cfm->data.irk.addr.addr_type = (loc_addr->addr[5] & 0xC0) ? ADDR_RAND : ADDR_PUBLIC;
        }
            break;

        case (GAPC_TK_EXCH):
        {
            // Generate a PIN Code- (Between 100000 and 999999)
            uint32_t pin_code = (100000 + (co_rand_word()%900000));

            if(g_sec_config.static_key > 0)
                pin_code = g_sec_config.static_key;

            LOG(LOG_LVL_TRACE,"app_sec GAPC_TK_EXCH: tk_type=%d\r\n",p_param->data.tk_type);

            cfm->accept  = true;
            cfm->request = GAPC_TK_EXCH;

            // Set the TK value
            memset(cfm->data.tk.key, 0, KEY_LEN);

            cfm->data.tk.key[0] = (uint8_t)((pin_code & 0x000000FF) >>  0);
            cfm->data.tk.key[1] = (uint8_t)((pin_code & 0x0000FF00) >>  8);
            cfm->data.tk.key[2] = (uint8_t)((pin_code & 0x00FF0000) >> 16);
            cfm->data.tk.key[3] = (uint8_t)((pin_code & 0xFF000000) >> 24);

            LOG(LOG_LVL_TRACE,"###GAPC_TK_EXCH pincode=%d\r\n", pin_code);
        }
            break;

        default:
            break;
    }

    ke_msg_send(cfm);
    return (KE_MSG_CONSUMED);
}

static int ln_smp_encrypt_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_encrypt_ind *p_param = (struct gapc_encrypt_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_smp_encrypt_ind_handler conidx=%d,auth=0x%x\r\n",conidx,p_param->auth);

    return (KE_MSG_CONSUMED);
}

static int ln_smp_encrypt_req_ind_handler(ke_msg_id_t const msgid,
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
    le_dev_con_env_t *con_env = &le_conn_mgr_info_get()->con_env[conidx];

    cfm->found = false;
    if(con_env->is_bonded)
    {
        if(p_param->ediv == con_env->dev_remote.ltk.ediv
            && !memcmp(p_param->rand_nb.nb, con_env->dev_remote.ltk.randnb, GAP_RAND_NB_LEN))
        {
            memcpy(cfm->ltk.key, con_env->dev_remote.ltk.ltk, GAP_KEY_LEN);
            cfm->key_size = GAP_KEY_LEN;
            cfm->found = true;
        }
    }

    LOG(LOG_LVL_TRACE,"ln_smp_encrypt_req_ind_handler: conidx=%d,cfm->found=%d\r\n", conidx, cfm->found);

    ke_msg_send(cfm);
    return (KE_MSG_CONSUMED);
}

static int ln_smp_security_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_security_ind *p_param = (struct gapc_security_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_smp_security_ind_handler conidx=%d,auth=0x%x\r\n",conidx, p_param->auth);

    return (KE_MSG_CONSUMED);
}

static int ln_smp_gen_rand_nb_ind_handler(ke_msg_id_t const msgid,
                                        void const *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gapm_gen_rand_nb_ind *p_param = (struct gapm_gen_rand_nb_ind *)param;

    LOG(LOG_LVL_TRACE,"gapm_gen_rand_nb_ind_handler\r\n");

    if(g_rand_key_cb) {
        if(!g_rand_key_cb(p_param->randnb.nb))
            g_rand_key_cb = NULL;
    }

    return (KE_MSG_CONSUMED);
}



static int ln_smp_pub_key_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_pub_key_ind *p_param = (struct gapm_pub_key_ind *)param;
    uint8_t pub_key[GAP_P256_KEY_LEN*2];
    memcpy(pub_key, p_param->pub_key_x, GAP_P256_KEY_LEN);
    memcpy(pub_key+GAP_P256_KEY_LEN, p_param->pub_key_y, GAP_P256_KEY_LEN);
    LOG(LOG_LVL_TRACE,"gapm_pub_key_ind_handler\r\n");
    if(g_pub_key_cb) {
        if(!g_pub_key_cb(pub_key))
            g_pub_key_cb = NULL;
    }


    return (KE_MSG_CONSUMED);
}

static int ln_smp_gen_dh_key_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_gen_dh_key_ind *p_param = (struct gapm_gen_dh_key_ind *)param;
    LOG(LOG_LVL_TRACE,"gapm_gen_dh_key_ind_handler\r\n");
    if(g_dh_key_cb) {
        if(!g_dh_key_cb(p_param->result))
            g_dh_key_cb = NULL;
    }
    return (KE_MSG_CONSUMED);
}

void ln_smp_bond_req(int conidx)
{
    struct gapc_bond_cmd *p_cmd = KE_MSG_ALLOC( GAPC_BOND_CMD,
                                KE_BUILD_ID(TASK_GAPC, conidx), TASK_APP,
                                gapc_bond_cmd);
    p_cmd->operation = GAPC_BOND;
    p_cmd->pairing.iocap        = g_sec_config.iocap;
    p_cmd->pairing.oob          = g_sec_config.oob;
    p_cmd->pairing.auth         = g_sec_config.auth;
    p_cmd->pairing.key_size     = g_sec_config.key_size;
    p_cmd->pairing.ikey_dist    = g_sec_config.ikey_dist;
    p_cmd->pairing.rkey_dist    = g_sec_config.rkey_dist;
    p_cmd->pairing.sec_req      = g_sec_config.sec_req;

    ln_ke_msg_send(p_cmd);
}

void ln_smp_encrypt_start(int conidx, ln_encrypt_info_t *p_param)
{
    struct gapc_encrypt_cmd *p_cmd = KE_MSG_ALLOC(GAPC_ENCRYPT_CMD,
                                                  KE_BUILD_ID(TASK_GAPC, conidx), TASK_APP,
                                                  gapc_encrypt_cmd);
    p_cmd->operation = GAPC_ENCRYPT;
    memcpy(p_cmd->ltk.ltk.key, p_param->ltk.ltk, GAP_KEY_LEN);
    p_cmd->ltk.ediv = p_param->ltk.ediv;
    memcpy(p_cmd->ltk.randnb.nb, p_param->ltk.randnb, GAP_RAND_NB_LEN);
    p_cmd->ltk.key_size = p_param->ltk.key_size;

    ln_ke_msg_send(p_cmd);
}

/// issue from slave
void ln_smp_req_security(int conidx, uint8_t auth)
{
    struct gapc_security_cmd *p_cmd = KE_MSG_ALLOC(GAPC_SECURITY_CMD,
                                                    KE_BUILD_ID(TASK_GAPC, conidx), TASK_APP,
                                                    gapc_security_cmd);

    LOG(LOG_LVL_TRACE,"ln_smp_req_security conidx=%d\r\n", conidx);

    p_cmd->operation = GAPC_SECURITY_REQ;
    p_cmd->auth = auth;//GAP_AUTH_REQ_NO_MITM_BOND;

    ln_ke_msg_send(p_cmd);
}

void ln_app_set_ping_timeout(int conidx, uint16_t timeout)
{
    struct gapc_set_le_ping_to_cmd *p_cmd = KE_MSG_ALLOC(GAPC_SET_LE_PING_TO_CMD,
                                                        KE_BUILD_ID(TASK_GAPC, conidx), TASK_APP,
                                                        gapc_set_le_ping_to_cmd);
    p_cmd->operation = GAPC_SET_LE_PING_TO;
    p_cmd->timeout = timeout;
    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_smp_rd_p256_public_key(void *cdk)
{
    struct gapm_get_pub_key_cmd *p_cmd = KE_MSG_ALLOC(GAPM_GET_PUB_KEY_CMD,
                                                        KE_BUILD_ID(TASK_GAPM, 0), TASK_APP,
                                                        gapm_get_pub_key_cmd);
    p_cmd->operation = GAPM_GET_PUB_KEY;
    g_pub_key_cb = (bt_pub_key_cb)cdk;
    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_smp_gen_dh_key(const uint8_t remote_pk[64], void *cdk)
{
    struct gapm_gen_dh_key_cmd *p_cmd = KE_MSG_ALLOC(GAPM_GEN_DH_KEY_CMD,
                                                        KE_BUILD_ID(TASK_GAPM, 0), TASK_APP,
                                                        gapm_gen_dh_key_cmd);
    p_cmd->operation = GAPM_GEN_DH_KEY;
    memcpy(p_cmd->operand_1, remote_pk, GAP_P256_KEY_LEN);
    memcpy(p_cmd->operand_2, remote_pk+GAP_P256_KEY_LEN, GAP_P256_KEY_LEN);

    g_dh_key_cb = (bt_dh_key_cb_t)cdk;
    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_smp_gen_random_nb(void *cdk)
{
    struct gapm_gen_rand_nb_cmd *p_cmd = KE_MSG_ALLOC(GAPM_GEN_RAND_NB_CMD,
                                         TASK_GAPM, TASK_APP,
                                         gapm_gen_rand_nb_cmd);
    p_cmd->operation = GAPM_GEN_RAND_NB;

    g_rand_key_cb = (bt_rand_key_cb)cdk;
    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_smp_set_irk(ln_gap_irk_t *p_param)
{
    struct gapm_set_irk_cmd *p_cmd = KE_MSG_ALLOC(GAPM_SET_IRK_CMD,
                                     TASK_GAPM, TASK_APP,
                                     gapm_set_irk_cmd);
    p_cmd->operation = GAPM_SET_IRK;
    memcpy(p_cmd->irk.key, p_param->key, GAP_KEY_LEN);
    // Send the message
    ln_ke_msg_send(p_cmd);
}

static int ln_smp_gen_irk_cb(const uint8_t key[8])
{
    uint8_t need_len = (g_irk_gen_key_len > 8) ? (16-g_irk_gen_key_len) : 8;
    memcpy(g_gen_irk+g_irk_gen_key_len, key, need_len);

    g_irk_gen_key_len += need_len;
    if(g_irk_gen_key_len < LN_GAP_KEY_LEN) {
        ln_smp_gen_random_nb(ln_smp_gen_irk_cb);
        return -1;
    } else {
        ln_gap_irk_t s_irk;
        memcpy(s_irk.key, g_gen_irk, LN_GAP_KEY_LEN);
        ln_smp_set_irk(&s_irk);
        ln_kv_ble_irk_store(g_gen_irk);
    }

    return 0;
}

void ln_smp_gen_local_irk(void)
{
    g_irk_gen_key_len = 0;
    ln_smp_gen_random_nb(ln_smp_gen_irk_cb);
}

static struct ke_msg_handler app_smp_msg_tab[] =
{
    {GAPC_BOND_IND, ln_smp_bond_ind_handler},
    {GAPC_SECURITY_IND, ln_smp_security_ind_handler},
    {GAPC_ENCRYPT_IND, ln_smp_encrypt_ind_handler},
    //{GAPC_LE_PING_TO_IND, gapc_le_ping_to_handler},
    //{GAPC_LE_PING_TO_VAL_IND, gapc_le_ping_to_val_ind_handler},
    {GAPM_PUB_KEY_IND, ln_smp_pub_key_ind_handler},
    {GAPM_GEN_DH_KEY_IND, ln_smp_gen_dh_key_ind_handler},
    {GAPM_GEN_RAND_NB_IND, ln_smp_gen_rand_nb_ind_handler},

    {GAPC_BOND_REQ_IND, ln_smp_bond_req_ind_handler},
    {GAPC_ENCRYPT_REQ_IND, ln_smp_encrypt_req_ind_handler},
    //{GAPC_KEY_PRESS_NOTIFICATION_IND, gapc_key_press_notification_ind_handler},
    //{GAPC_SIGN_COUNTER_IND, gapc_sign_counter_ind_handler},
};

int ln_ble_smp_init(void)
{
    int ret = 0;

    g_sec_config.iocap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    g_sec_config.auth = GAP_AUTH_REQ_NO_MITM_BOND;
    g_sec_config.oob = GAP_OOB_AUTH_DATA_NOT_PRESENT;
    g_sec_config.key_size = GAP_KEY_LEN;
    g_sec_config.ikey_dist = GAP_KDIST_NONE;
    g_sec_config.rkey_dist = GAP_KDIST_ENCKEY;
    g_sec_config.sec_req = GAP_SEC1_NOAUTH_PAIR_ENC;
    g_sec_config.static_key = -1;

    ret = ln_app_msg_regester(app_smp_msg_tab, ARRAY_LEN(app_smp_msg_tab));

    return ret;
}

