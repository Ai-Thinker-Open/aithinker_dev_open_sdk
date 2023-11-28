/**
 ****************************************************************************************
 *
 * @file ln_gatt_callback.h
 *
 * @brief GATT callback function.
 *
 *Copyright (C) 2021.Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

#ifndef _LN_GATT_CALLBACK_H_
#define _LN_GATT_CALLBACK_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @group BLE
 *
 * @brief GATT callback function.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include <stdbool.h>
#include "gattc_task.h"
#include "gattm_task.h"
#include "att.h"
#include "rwip_task.h" // Task definitions
#include "ke_msg.h"
/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


int gattm_add_svc_rsp_handler(ke_msg_id_t const msgid,
                              void const *p_param,
                              ke_task_id_t const dest_id,
                              ke_task_id_t const src_id);

int gattm_svc_get_permission_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);

int gattm_svc_set_permission_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);

int gattm_att_get_permission_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);

int gattm_att_set_permission_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);

int gattm_att_get_value_rsp_handler(ke_msg_id_t const msgid,
                                    void const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);

int gattm_att_set_value_rsp_handler(ke_msg_id_t const msgid,
                                    void const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);

int gattm_unknown_msg_ind_handler(ke_msg_id_t const msgid,
                                  void const *p_param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id);

int gattm_destroy_db_rsp_handler(ke_msg_id_t const msgid,
                                 void const *p_param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id);

int gattm_svc_get_list_rsp_handler(ke_msg_id_t const msgid,
                                   void const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);

int gattm_att_get_info_rsp_handler(ke_msg_id_t const msgid,
                                   void const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);

int gattm_att_db_hash_comp_rsp_handler(ke_msg_id_t const msgid,
                                       void const *p_param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id);

int gattc_cmp_evt_handler(ke_msg_id_t const msgid,
                          void const *p_param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id);

int gattc_mtu_changed_ind_handler(ke_msg_id_t const msgid,
                                  void const *p_param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id);

int gattc_app_disc_svc_ind_handler(ke_msg_id_t const msgid,
                                   void const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);

int gattc_app_disc_svc_incl_ind_handler(ke_msg_id_t const msgid,
                                        void const *p_param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id);

int gattc_app_disc_char_ind_handler(ke_msg_id_t const msgid,
                                    void const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);

int gattc_app_disc_char_desc_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);

int gattc_read_ind_handler(ke_msg_id_t const msgid,
                           void const *p_param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id);

int gattc_event_ind_handler(ke_msg_id_t const msgid,
                            void const *p_param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id);

int gattc_read_req_ind_handler(ke_msg_id_t const msgid,
                               void const *p_param,
                               ke_task_id_t const dest_id,
                               ke_task_id_t const src_id);

int gattc_write_req_ind_handler(ke_msg_id_t const msgid,
                                void const *p_param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id);

int gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
                                   void const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);

int gattc_sdp_svc_ind_handler(ke_msg_id_t const msgid,
                              void const *p_param,
                              ke_task_id_t const dest_id,
                              ke_task_id_t const src_id);

int gattc_transaction_to_error_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);

int gattc_event_req_ind_handler(ke_msg_id_t const msgid,
                                void const *p_param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id);

#endif // _LN_GATT_CALLBACK_H_
