/**
 ****************************************************************************************
 *
 * @file fhost_config.h
 *
 * @brief Definition of configuration for Fully Hosted firmware.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */
#ifndef _FHOST_CONFIG_H_
#define _FHOST_CONFIG_H_

/**
 ****************************************************************************************
 * @addtogroup FHOST
 * @{
 ****************************************************************************************
 */

#include "cfgrwnx.h"

#if NX_FHOST_RX_STATS
struct fhost_rx_conf_tag
{
    /// Number of BW
    uint8_t n_bw;

    /// First HT rate index
    uint16_t first_ht;
    /// Rates per HT MCS
    uint8_t ht_rates_per_mcs;

    /// First VHT rate index
    uint16_t first_vht;
    /// Max VHT MCS
    uint8_t max_vht_mcs;
    /// Rates per VHT MCS
    uint8_t vht_rates_per_mcs;

    /// First HE SU rate index
    uint16_t first_he_su;
    /// Rates per HE SU MCS
    uint8_t he_su_rates_per_mcs;

    /// First HE MU rate index
    uint16_t first_he_mu;
    /// Rates per HE MU MCS
    uint8_t he_mu_rates_per_mcs;

    /// First HE ER rate index
    uint16_t first_he_er;
    /// Rates per HE ER MCS
    uint8_t he_er_rates_per_mcs;

    /// Max HE MCS
    uint8_t max_he_mcs;
    /// Total number of rates
    uint16_t n_rates;
};

extern struct fhost_rx_conf_tag fhost_conf_rx;
#endif

extern struct me_chan_config_req fhost_chan;

/**
 ****************************************************************************************
 * @brief Initialize wifi configuration structure from fhost configuration
 *
 * To be called before initializing the wifi stack.
 * Can also be used to retrieve firmware feature list at run-time. In this case @p init
 * is false.
 *
 * @param[out] me_config     Configuration structure for the UMAC (i.e. ME task)
 * @param[out] start         Configuration structure for the LMAC (i.e. MM task)
 * @param[out] base_mac_addr Base MAC address of the device (from which all VIF MAC
 *                           addresses are computed)
 * @param[in]  init          Whether it is called before firmware initialization or not.
 ****************************************************************************************
 */
void fhost_config_prepare(struct fhost_me_config_req *me_config, struct fhost_mm_start_req *start,
                          bool init);

/**
 ****************************************************************************************
 * @brief Return the channel associated to a given frequency
 *
 * @param[in] freq Channel frequency
 *
 * @return Channel definition whose primary frequency is the requested one and NULL if
 * there no such channel.
 ****************************************************************************************
 */
struct mac_chan_def *fhost_chan_get(int freq);

/**
 * @}
 */
#endif /* _FHOST_CONFIG_H_ */
