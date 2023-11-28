/**
 ****************************************************************************************
 *
 * @file ln_ble_smp.h
 *
 * @brief GAP API.
 *
 * Copyright (C) 2021.Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

#ifndef LN_BLE_SMP_H_
#define LN_BLE_SMP_H_

#include <stdint.h>
#include <stdbool.h>

#include "ln_ble_app_defines.h"


/// Start Encryption command procedure
/*@TRACE*/
typedef struct 
{
    /// Long Term Key information
    ln_gap_ltk_t ltk;
} ln_encrypt_info_t;

/*  @typedef bt_dh_key_cb_t
 *  @brief Callback type for DH Key calculation.
 *
 *  Used to notify of the calculated DH Key.
 *
 *  @param key The DH Key, or NULL in case of failure.
 */
typedef int (*bt_dh_key_cb_t)(const uint8_t dhkey[32]);
typedef int (*bt_pub_key_cb)(const uint8_t pkey[64]);
typedef int (*bt_rand_key_cb)(const uint8_t rkey[8]);

typedef struct
{
    /// IO capabilities (@see gap_io_cap)
    uint8_t iocap;
    /// OOB information (@see gap_oob)
    uint8_t oob;
    /// Authentication (@see gap_auth)
    /// Note in BT 4.1 the Auth Field is extended to include 'Key Notification' and
    /// and 'Secure Connections'.
    uint8_t auth;
    /// Encryption key size (7 to 16)
    uint8_t key_size;
    ///Initiator key distribution (@see gap_kdist)
    uint8_t ikey_dist;
    ///Responder key distribution (@see gap_kdist)
    uint8_t rkey_dist;
    /// Device security requirements (minimum security level). (@see gap_sec_req)
    uint8_t sec_req;
    /// pair static number
    int static_key;
} ln_smp_config_t;


void ln_smp_bond_req(int conidx);
void ln_smp_encrypt_start(int conidx, ln_encrypt_info_t *p_param);
void ln_smp_req_security(int conidx, uint8_t auth);
void ln_app_set_ping_timeout(int conidx, uint16_t timeout);
void ln_smp_rd_p256_public_key(void *cdk);
void ln_smp_gen_dh_key(const uint8_t remote_pk[64], void *cdk);
void ln_smp_gen_random_nb(void *cdk);
void ln_smp_gen_local_irk(void);
int ln_ble_smp_init(void);

#endif

