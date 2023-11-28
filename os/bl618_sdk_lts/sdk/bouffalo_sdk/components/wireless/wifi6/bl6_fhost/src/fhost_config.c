/**
 ****************************************************************************************
 *
 * @file fhost_config.c
 *
 * @brief Definition of configuration for Fully Hosted firmware.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */

#include "fhost_config.h"
#include "fhost_api.h"
#include "fhost.h"
#include "mac_frame.h"
#include "ipc_emb.h"
#include "wifi_mgmr_ext.h"
#if NX_FHOST_RX_STATS
#include  "me_utils.h"
#endif
#include "cfgrwnx.h"

/**
 ****************************************************************************************
 * @addtogroup FHOST_CONFIG
 * @{
 ****************************************************************************************
 */
/// define a channel
#define CHAN(_freq, _band, _flags, _pwr) {         \
        .freq = (_freq),                           \
        .band = (_band),                           \
        .flags = (_flags),                         \
        .tx_power = (_pwr),                        \
    }

/// define a channel in 2.4GHz band
#define CHAN_24(_freq, _flag, _pwr) CHAN(_freq, PHY_BAND_2G4, _flag, _pwr)
/// define a channel in 5GHz band
#define CHAN_5(_freq, _flag, _pwr) CHAN(_freq, PHY_BAND_5G, _flag, _pwr)

/** List of supported Channel */
struct me_chan_config_req fhost_chan = {
    .chan2G4_cnt = 14,
    .chan2G4[0] = CHAN_24(2412, 0, 20),
    .chan2G4[1] = CHAN_24(2417, 0, 20),
    .chan2G4[2] = CHAN_24(2422, 0, 20),
    .chan2G4[3] = CHAN_24(2427, 0, 20),
    .chan2G4[4] = CHAN_24(2432, 0, 20),
    .chan2G4[5] = CHAN_24(2437, 0, 20),
    .chan2G4[6] = CHAN_24(2442, 0, 20),
    .chan2G4[7] = CHAN_24(2447, 0, 20),
    .chan2G4[8] = CHAN_24(2452, 0, 20),
    .chan2G4[9] = CHAN_24(2457, 0, 20),
    .chan2G4[10] = CHAN_24(2462, 0, 20),
    .chan2G4[11] = CHAN_24(2467, 0, 20),
    .chan2G4[12] = CHAN_24(2472, 0, 20),
    .chan2G4[13] = CHAN_24(2484, 0, 20),

    .chan5G_cnt = 28,
    .chan5G[0] = CHAN_5(5180, 0, 20),
    .chan5G[1] = CHAN_5(5200, 0, 20),
    .chan5G[2] = CHAN_5(5220, 0, 20),
    .chan5G[3] = CHAN_5(5240, 0, 20),
    .chan5G[4] = CHAN_5(5260, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[5] = CHAN_5(5280, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[6] = CHAN_5(5300, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[7] = CHAN_5(5320, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[8] = CHAN_5(5500, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[9] = CHAN_5(5520, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[10] = CHAN_5(5540, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[11] = CHAN_5(5560, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[12] = CHAN_5(5580, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[13] = CHAN_5(5600, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[14] = CHAN_5(5620, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[15] = CHAN_5(5640, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[16] = CHAN_5(5660, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[17] = CHAN_5(5680, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[18] = CHAN_5(5700, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[19] = CHAN_5(5720, CHAN_NO_IR|CHAN_RADAR, 20),
    .chan5G[20] = CHAN_5(5745, 0, 20),
    .chan5G[21] = CHAN_5(5765, 0, 20),
    .chan5G[22] = CHAN_5(5785, 0, 20),
    .chan5G[23] = CHAN_5(5805, 0, 20),
    .chan5G[24] = CHAN_5(5825, 0, 20),
    .chan5G[25] = CHAN_5(5845, 0, 20),
    .chan5G[26] = CHAN_5(5865, 0, 20),
    .chan5G[27] = CHAN_5(5885, 0, 20),
};

#if NX_FHOST_RX_STATS
struct fhost_rx_conf_tag fhost_conf_rx;
#endif

#undef CHAN_5
#undef CHAN_24
#undef CHAN

/**
 * Wifi configuration
 */


static uint8_t FHOST_CFG_HE_MCS_data[1] = {1}; // 0=MCS0-7, 1=MCS0-9, 2=MCS0-11
static uint16_t tx_agg_lifetime = 0x4000;
#ifdef CFG_BL_WIFI_UAPSD_ENABLE
static uint32_t PS_UAPSD_TIMEOUT = 50;
static uint32_t PS_UAPSD_QUEUES = ((1 << AC_VO) | (1 << AC_VI) | (1 << AC_BE) | (1 << AC_BK));
#endif
struct fhost_config_item example_config[] = {
    {FHOST_CFG_TX_LFT,          2, &tx_agg_lifetime},
    {FHOST_CFG_HE_MCS,          1, FHOST_CFG_HE_MCS_data},
#ifdef CFG_BL_WIFI_UAPSD_ENABLE
    {FHOST_CFG_UAPSD_TIMEOUT,   4, &PS_UAPSD_TIMEOUT},
    {FHOST_CFG_UAPSD_QUEUES,    4, &PS_UAPSD_QUEUES},
#endif
    {FHOST_CFG_END,             0, NULL},
};

/*
 * fhost_config_get_next_item: Must return configuration item one by one.
 * In this example confguration is saved in the global variable example_config
 */
void fhost_config_get_next_item(struct fhost_config_item *item)
{
    static int index;

    // First call, initialize index
    if (item->data == NULL)
        index = 0;

    // Check that we don't read outside of example_config table
    if (index > (sizeof(example_config) / sizeof(struct fhost_config_item)))
    {
        item->id = FHOST_CFG_END;
    }
    else
    {
        // Copy one configuration parameter
        *item = example_config[index];
    }

    // Update index for next call
    index++;
}

void fhost_config_prepare(struct fhost_me_config_req *me_config, struct fhost_mm_start_req *start,
                          bool init)
{
    struct fhost_config_item item;
    uint32_t phy_ver1, phy_ver2, sec = 0, usec = 0;
    uint8_t nss, vht_mcs, mcs_map_tx, mcs_map_rx;
    bool sgi, sgi80, stbc, ht40, ldpc_rx, bfmee, bfmer, mu_rx, mu_tx, vht80;
    #if NX_HE
    uint8_t he_mcs = 0;
    #endif

    memset(me_config, 0, sizeof(*me_config));
    memset(start, 0, sizeof(*start));
    item.data = NULL;

    /* Default value */
    stbc = true;
    sgi = true;
    ht40 = phy_get_bw() >= PHY_CHNL_BW_40 ? true : false;
    vht80 = phy_get_bw() >= PHY_CHNL_BW_80 ? true : false;
    sgi80 = vht80;
    vht_mcs = 0;
    me_config->ht_supp = true;
    #if NX_VHT
    me_config->vht_supp = phy_vht_supported();
    #endif
    me_config->he_supp = inline_hal_machw_he_support();
    nss = phy_get_nss() + 1;
    ldpc_rx = phy_ldpc_rx_supported();
    #if defined(CFG_BL_WIFI_PS_ENABLE)
    bfmee = false;
    #else
    bfmee = phy_bfmee_supported();
    #endif
    bfmer = phy_bfmer_supported();
    mu_rx = phy_mu_mimo_rx_supported();
    mu_tx = phy_mu_mimo_tx_supported();
    phy_get_version(&phy_ver1, &phy_ver2);

    fhost_config_get_next_item(&item);
    while (item.id != FHOST_CFG_END)
    {
        switch (item.id)
        {
            case FHOST_CFG_PS:
                me_config->ps_on = *(bool *)item.data;
                break;
            case FHOST_CFG_DPSM:
                me_config->dpsm = *(bool *)item.data;
                break;
            case FHOST_CFG_SGI:
                sgi = *(bool *)item.data;
                break;
            case FHOST_CFG_SGI80:
                if (vht80)
                    sgi80 = *(bool *)item.data;
                break;
            case FHOST_CFG_NSS:
            {
                uint8_t val = *(uint8_t *)item.data;
                if (val && val < nss)
                    nss = val;
            } break;
            case FHOST_CFG_TX_LFT:
                me_config->tx_lft = *(uint16_t *)item.data;
                break;
            case FHOST_CFG_UAPSD_TIMEOUT:
                start->uapsd_timeout = *(uint32_t *)item.data;
                break;
            case FHOST_CFG_UAPSD_QUEUES:
                if (init)
                    fhost_vif_set_uapsd_queues(-1, *(uint8_t *)item.data);
                break;
            case FHOST_CFG_LP_CLK_ACCURACY:
                start->lp_clk_accuracy = *(uint16_t *)item.data;
                break;
            case FHOST_CFG_HT:
                me_config->ht_supp = *(bool *)item.data;
                break;
            case FHOST_CFG_40MHZ:
                if (ht40)
                    ht40 = *(bool *)item.data;
                break;
            case FHOST_CFG_80MHZ:
                if (vht80)
                    vht80 = *(bool *)item.data;
                break;
            case FHOST_CFG_VHT:
                #if NX_VHT
                if (phy_vht_supported())
                    me_config->vht_supp = *(bool *)item.data;
                else
                    me_config->vht_supp = false;
                #endif
                break;
            case FHOST_CFG_VHT_MCS:
                vht_mcs = *(uint8_t *)item.data;
                if (vht_mcs > 2)
                    vht_mcs = 0;
                break;
            #if NX_HE
            case FHOST_CFG_HE:
                if (inline_hal_machw_he_support())
                    me_config->he_supp = *(bool *)item.data;
                else
                    me_config->he_supp = false;
                break;
            case FHOST_CFG_HE_MCS:
                he_mcs = *(uint8_t *)item.data;
                if (he_mcs > 2)
                    he_mcs = 0;
                break;
            #endif
            case FHOST_CFG_LDPC:
                if (!*(bool *)item.data)
                    ldpc_rx = false;
                break;
            case FHOST_CFG_STBC:
                stbc = *(bool *)item.data;
                break;
            case FHOST_CFG_BFMEE:
                if (!*(bool *)item.data)
                    bfmee = false;
                break;
            case FHOST_CFG_BFMER:
                if (!*(bool *)item.data)
                    bfmer = false;
                break;
            case FHOST_CFG_MURX:
                if (!*(bool *)item.data)
                    mu_rx = false;
                break;
            case FHOST_CFG_MUTX:
                if (!*(bool *)item.data)
                    mu_tx = false;
                break;
            case FHOST_CFG_PHY_TRD:
                if (((phy_ver1 & 0x030000) >> 16) == 0) {
                    memcpy(&start->phy_cfg.parameters, item.data, item.len);
                }
                break;
            case FHOST_CFG_PHY_KARST:
                if (((phy_ver1 & 0x030000) >> 16) == 2)
                {
                    memcpy(&start->phy_cfg.parameters, item.data, item.len);
                }
                break;
            case FHOST_CFG_ANT_DIV:
                me_config->ant_div_on = *(bool *)item.data;
                break;
            case FHOST_CFG_EPOCH_SEC:
                sec = *(uint32_t *)item.data;
                break;
            case FHOST_CFG_EPOCH_USEC:
                usec = *(uint32_t *)item.data;
                break;
            default :
                break;
        }
        fhost_config_get_next_item(&item);
    }

    if (init)
    {
        if (sec && usec)
        {
            time_init(sec, usec);
        }

        get_time_SINCE_EPOCH(&sec, &usec);
        co_random_init((sec & 0xffff) * (usec & 0x1ffff));
    }

    if (!ht40)
    {
        vht80 = false;
        sgi80 = false;
    }

    #if NX_FHOST_RX_STATS
    fhost_conf_rx.n_bw = phy_get_bw() + 1;
    fhost_conf_rx.n_rates = N_CCK + N_OFDM;
    #endif
    if (me_config->ht_supp)
    {
        int max_rate;
        me_config->ht_cap.ht_capa_info = 0;
        if (ldpc_rx)
            me_config->ht_cap.ht_capa_info |= MAC_HTCAPA_LDPC;
        if (ht40)
            me_config->ht_cap.ht_capa_info |= MAC_HTCAPA_40_MHZ;
        #if !NX_HE
        me_config->ht_cap.ht_capa_info |= MAC_HTCAPA_GREEN_FIELD;
        #endif
        if (sgi)
        {
            me_config->ht_cap.ht_capa_info |= MAC_HTCAPA_SHORTGI_20;
            if (ht40)
                me_config->ht_cap.ht_capa_info |= MAC_HTCAPA_SHORTGI_40;
        }
        if (nss > 1)
            me_config->ht_cap.ht_capa_info |= MAC_HTCAPA_TX_STBC;
        me_config->ht_cap.ht_capa_info |= (1 << MAC_HTCAPA_RX_STBC_OFT);

        #if (RWNX_MAX_AMSDU_RX > 4096)
        me_config->ht_cap.ht_capa_info |= MAC_HTCAPA_AMSDU;
        #endif

        me_config->ht_cap.a_mpdu_param = ((3 << MAC_AMPDU_LEN_EXP_OFT) |
                                         (7 << MAC_AMPDU_MIN_SPACING_OFT));
        me_config->ht_cap.mcs_rate[0] = 0xff; /* RX MCS0-7*/
        if (nss > 1)
            me_config->ht_cap.mcs_rate[1] = 0xff; /* RX MCS8-15 */
        if (ht40)
            me_config->ht_cap.mcs_rate[4] = 1; /* RX MCS32 */

        if (sgi && ht40)
            max_rate = 150;
        else if (sgi)
            max_rate = 72;
        else if (ht40)
            max_rate = 135;
        else
            max_rate = 65;
        max_rate = max_rate * nss;

        //FIXME why unaligned access
#if 0
        *((uint16_t *)(&me_config->ht_cap.mcs_rate[10])) = max_rate; /* highest supported rate */
#else
        me_config->ht_cap.mcs_rate[10] = max_rate; /* highest supported rate */
        me_config->ht_cap.mcs_rate[11] = (max_rate >> 8); /* highest supported rate */
#endif
        me_config->ht_cap.mcs_rate[12] = 1;              /* TX mcs same as RX mcs */

        me_config->ht_cap.ht_extended_capa = 0;
        me_config->ht_cap.tx_beamforming_capa = 0;
        me_config->ht_cap.asel_capa = 0;

        #if NX_FHOST_RX_STATS
        fhost_conf_rx.first_ht = fhost_conf_rx.n_rates;
        fhost_conf_rx.ht_rates_per_mcs = fhost_conf_rx.n_bw * 2;
        fhost_conf_rx.n_rates += 8 * nss * fhost_conf_rx.ht_rates_per_mcs;
        #endif
    }
    else
    {
        ht40 = false;
    }

    if (me_config->vht_supp)
    {
        uint16_t vht_base_rate[3] = {0x4100, 0x4E00, 0x56AB}; // VHT base rate per mcs 65, 78 and 86.66 in Q8 notation
        uint16_t vht_mult;
        int i;

        if (vht80)
            vht_mult = 0x0480; // x4.5 in Q8 notation
        else if (ht40)
            vht_mult = 0x0214; // x2.07 in Q8 notation
        else
            vht_mult = 0x0100; // x1 in Q8 notation

        #define VHT_RATE(_mcs, _nss) (((uint32_t)(vht_base_rate[(_mcs)] * vht_mult) >> 16) * (_nss))

        #if (RWNX_MAX_AMSDU_RX > 8192)
        me_config->vht_cap.vht_capa_info |= MAC_VHTCAPA_MAX_MPDU_LENGTH_11454;
        #elif (RWNX_MAX_AMSDU_RX > 4096)
        me_config->vht_cap.vht_capa_info |= MAC_VHTCAPA_MAX_MPDU_LENGTH_7991;
        #else
        me_config->vht_cap.vht_capa_info |= MAC_VHTCAPA_MAX_MPDU_LENGTH_3895;
        #endif
        me_config->vht_cap.vht_capa_info |= MAC_VHTCAPA_SUPP_CHAN_WIDTH_80;

        if (ldpc_rx)
            me_config->vht_cap.vht_capa_info |= MAC_VHTCAPA_RXLDPC;
        if (sgi80)
            me_config->vht_cap.vht_capa_info |= MAC_VHTCAPA_SHORT_GI_80;
        if (nss > 1)
            me_config->vht_cap.vht_capa_info |= MAC_VHTCAPA_TXSTBC;
        if (stbc)
            me_config->vht_cap.vht_capa_info |= MAC_VHTCAPA_RXSTBC_1;
        if (bfmer)
            me_config->vht_cap.vht_capa_info |= MAC_VHTCAPA_SU_BEAMFORMER_CAPABLE;
        if (bfmee)
            me_config->vht_cap.vht_capa_info |= MAC_VHTCAPA_SU_BEAMFORMEE_CAPABLE;
        me_config->vht_cap.vht_capa_info |= (nss - 1 ) << MAC_VHTCAPA_SOUNDING_DIMENSIONS_OFT;
        if (mu_tx)
            me_config->vht_cap.vht_capa_info |= MAC_VHTCAPA_MU_BEAMFORMER_CAPABLE;
        if (mu_rx)
            me_config->vht_cap.vht_capa_info |= MAC_VHTCAPA_MU_BEAMFORMEE_CAPABLE;

        me_config->vht_cap.vht_capa_info = (7 << MAC_VHTCAPA_MAX_A_MPDU_LENGTH_EXP_OFT);

        // RX/TX MCS
        if (!ht40)
            // MCS9 not allowed in 20MHZ
            vht_mcs = co_min(vht_mcs, MAC_VHT_MCS_MAP_0_8);

        mcs_map_rx = vht_mcs;
        mcs_map_tx = vht_mcs;

        // 1 SS
        me_config->vht_cap.rx_mcs_map = mcs_map_rx;
        me_config->vht_cap.tx_mcs_map = mcs_map_tx;
        // 2 SS and above
        if (nss > 1)
        {
            if (vht80)
            {
                // limit rate @80MHz because of FPGA platform limitation
                mcs_map_rx = MAC_VHT_MCS_MAP_0_7;
                mcs_map_tx = co_min(mcs_map_tx, MAC_VHT_MCS_MAP_0_8);
            }
            for (i = 1; i < nss; i++)
            {
                me_config->vht_cap.rx_mcs_map |= mcs_map_rx << (2 * i);
                me_config->vht_cap.tx_mcs_map |= mcs_map_tx << (2 * i);
            }
        }
        // Non supported NSS
        for (i = nss; i < 8; i++)
        {
            me_config->vht_cap.rx_mcs_map |= MAC_VHT_MCS_MAP_NONE << (2 * i);
            me_config->vht_cap.tx_mcs_map |= MAC_VHT_MCS_MAP_NONE << (2 * i);
        }

        me_config->vht_cap.rx_highest = VHT_RATE(mcs_map_rx, nss);
        me_config->vht_cap.tx_highest = VHT_RATE(mcs_map_tx, nss);

        #if NX_FHOST_RX_STATS
        fhost_conf_rx.max_vht_mcs = me_11ac_mcs_max(vht_mcs) + 1;
        fhost_conf_rx.first_vht = fhost_conf_rx.n_rates;
        fhost_conf_rx.vht_rates_per_mcs = fhost_conf_rx.n_bw * 2;
        fhost_conf_rx.n_rates += fhost_conf_rx.max_vht_mcs * nss * fhost_conf_rx.vht_rates_per_mcs;
        #endif
    }
    else
    {
        vht80 = false;
    }

    #if NX_HE
    if (me_config->he_supp)
    {
        struct mac_hecapability *he_cap = &me_config->he_cap;
        int i, ppe_ru_cnt;

        memset(he_cap, 0, sizeof(*he_cap));

        HE_MAC_CAPA_BIT_SET(he_cap, ALL_ACK);
        #if NX_TWT
        HE_MAC_CAPA_BIT_SET(he_cap, TWT_REQ);
        #endif

        if (ht40)
            HE_PHY_CAPA_VAL_SET(he_cap, CHAN_WIDTH_SET, 40MHZ_IN_2G);
        if (vht80)
            HE_PHY_CAPA_VAL_SET(he_cap, CHAN_WIDTH_SET, 40MHZ_80MHZ_IN_5G);
        if (ldpc_rx)
            HE_PHY_CAPA_BIT_SET(he_cap, LDPC_CODING_IN_PAYLOAD);
        HE_PHY_CAPA_BIT_SET(he_cap, HE_SU_PPDU_1x_LTF_AND_GI_0_8US);
        HE_PHY_CAPA_VAL_SET(he_cap, MIDAMBLE_RX_MAX_NSTS, 4_STS);
        HE_PHY_CAPA_BIT_SET(he_cap, NDP_4x_LTF_AND_3_2US);
        if (stbc)
            HE_PHY_CAPA_BIT_SET(he_cap, STBC_RX_UNDER_80MHZ);
        HE_PHY_CAPA_BIT_SET(he_cap, DOPPLER_RX);
        // HE_PHY_CAPA_VAL_SET(he_cap, DCM_MAX_CONST_TX, NO_DCM);
        // HE_PHY_CAPA_VAL_SET(he_cap, DCM_MAX_CONST_RX, 16_QAM);
        HE_PHY_CAPA_VAL_SET(he_cap, DCM_MAX_CONST_TX, 16_QAM);
        HE_PHY_CAPA_VAL_SET(he_cap, DCM_MAX_CONST_RX, 16_QAM);
        if (nss > 1)
            HE_PHY_CAPA_BIT_SET(he_cap, DCM_MAX_NSS_RX);
        HE_PHY_CAPA_BIT_SET(he_cap, RX_HE_MU_PPDU_FRM_NON_AP);
        if (bfmer)
            HE_PHY_CAPA_BIT_SET(he_cap, SU_BEAMFORMER);
        if (bfmee)
        {
            HE_PHY_CAPA_BIT_SET(he_cap, SU_BEAMFORMEE);
            HE_PHY_CAPA_VAL_SET(he_cap, BFMEE_MAX_STS_UNDER_80MHZ, 4);
        }
        HE_PHY_CAPA_BIT_SET(he_cap, NG16_SU_FEEDBACK);
        HE_PHY_CAPA_BIT_SET(he_cap, NG16_MU_FEEDBACK);
        HE_PHY_CAPA_BIT_SET(he_cap, CODEBOOK_SIZE_42_SU);
        HE_PHY_CAPA_BIT_SET(he_cap, CODEBOOK_SIZE_75_MU);
        HE_PHY_CAPA_BIT_SET(he_cap, TRIG_SU_BEAMFORMER_FB);
        HE_PHY_CAPA_BIT_SET(he_cap, TRIG_MU_BEAMFORMER_FB);
        #if (NX_MDM_VER > 30)
        HE_PHY_CAPA_BIT_SET(he_cap, PARTIAL_BW_EXT_RANGE);
        #endif
        HE_PHY_CAPA_BIT_SET(he_cap, PARTIAL_BW_DL_MUMIMO);
        HE_PHY_CAPA_BIT_SET(he_cap, PPE_THRESHOLD_PRESENT);
        #if (NX_MDM_VER > 30)
        HE_PHY_CAPA_BIT_SET(he_cap, HE_ER_SU_PPDU_4x_LTF_AND_08_US_GI);
        #endif
        HE_PHY_CAPA_BIT_SET(he_cap, HE_SU_MU_PPDU_4x_LTF_AND_08_US_GI);
        HE_PHY_CAPA_BIT_SET(he_cap, 20MHZ_IN_40MHZ_HE_PPDU_IN_2G);
        #if (NX_MDM_VER > 30)
        HE_PHY_CAPA_BIT_SET(he_cap, HE_ER_SU_1x_LTF_AND_08_US_GI);
        #endif
        if (vht80)
            HE_PHY_CAPA_VAL_SET(he_cap, DCM_MAX_BW, 80MHZ);
        else if (ht40)
            HE_PHY_CAPA_VAL_SET(he_cap, DCM_MAX_BW, 40MHZ);
        HE_PHY_CAPA_BIT_SET(he_cap, RX_FULL_BW_SU_COMP_SIGB);
        HE_PHY_CAPA_BIT_SET(he_cap, RX_FULL_BW_SU_NON_COMP_SIGB);
        HE_PHY_CAPA_VAL_SET(he_cap, NOMINAL_PACKET_PADDING, 16US);

        if (!ldpc_rx)
            // If no LDPC is supported, we have to limit to MCS0_9, as LDPC is mandatory
            // for MCS 10 and 11
            he_mcs = co_min(he_mcs, MAC_HE_MCS_MAP_0_9);

        memset(&he_cap->mcs_supp, 0, sizeof(he_cap->mcs_supp));

        he_cap->mcs_supp.rx_mcs_80 = he_mcs;
        for (i = 1; i < nss; i++) {
            uint16_t unsup_for_ss = MAC_HE_MCS_MAP_NONE << (i*2);
            he_cap->mcs_supp.rx_mcs_80 |= MAC_HE_MCS_MAP_0_7 << (i*2);
            he_cap->mcs_supp.rx_mcs_160 |= unsup_for_ss;
            he_cap->mcs_supp.rx_mcs_80p80 |= unsup_for_ss;
        }
        for (; i < 8; i++) {
            uint16_t unsup_for_ss = MAC_HE_MCS_MAP_NONE << (i*2);
            he_cap->mcs_supp.rx_mcs_80 |= unsup_for_ss;
            he_cap->mcs_supp.rx_mcs_160 |= unsup_for_ss;
            he_cap->mcs_supp.rx_mcs_80p80 |= unsup_for_ss;
        }
        he_cap->mcs_supp.tx_mcs_80 = he_mcs;
        for (i = 1; i < nss; i++) {
            uint16_t unsup_for_ss = MAC_HE_MCS_MAP_NONE << (i*2);
            he_cap->mcs_supp.tx_mcs_80 |= MAC_HE_MCS_MAP_0_7 << (i*2);
            he_cap->mcs_supp.tx_mcs_160 |= unsup_for_ss;
            he_cap->mcs_supp.tx_mcs_80p80 |= unsup_for_ss;
        }
        for (; i < 8; i++) {
            uint16_t unsup_for_ss = MAC_HE_MCS_MAP_NONE << (i*2);
            he_cap->mcs_supp.tx_mcs_80 |= unsup_for_ss;
            he_cap->mcs_supp.tx_mcs_160 |= unsup_for_ss;
            he_cap->mcs_supp.tx_mcs_80p80 |= unsup_for_ss;
        }

        // PPE threshold
        if (vht80)
            ppe_ru_cnt = 3;
        else if (ht40)
            ppe_ru_cnt = 2;
        else
            ppe_ru_cnt = 1;
        co_val_set(he_cap->ppe_thres, HE_PPE_CAPA_NSTS_OFT, HE_PPE_CAPA_NSTS_WIDTH, nss - 1);
        co_val_set(he_cap->ppe_thres, HE_PPE_CAPA_RU_INDEX_BITMAP_OFT,
                   HE_PPE_CAPA_RU_INDEX_BITMAP_WIDTH, (1 << ppe_ru_cnt) - 1);

        for (i = HE_PPE_CAPA_PPE_THRES_INFO_OFT;
             i < HE_PPE_CAPA_PPE_THRES_INFO_OFT + (ppe_ru_cnt * 6 * nss);
             i += 6)
        {
            co_val_set(he_cap->ppe_thres, i, 6, HE_PPE_CAPA_BPSK | (HE_PPE_CAPA_NONE << 3));
        }

        #if NX_FHOST_RX_STATS
        fhost_conf_rx.max_he_mcs = me_11ax_mcs_max(he_mcs) + 1;

        fhost_conf_rx.first_he_su = fhost_conf_rx.n_rates;
        fhost_conf_rx.he_su_rates_per_mcs = fhost_conf_rx.n_bw * 3;
        fhost_conf_rx.n_rates += fhost_conf_rx.max_he_mcs * nss * fhost_conf_rx.he_su_rates_per_mcs;

        fhost_conf_rx.first_he_mu = fhost_conf_rx.n_rates;
        fhost_conf_rx.he_mu_rates_per_mcs = (phy_get_bw() + 4) * 3;
        fhost_conf_rx.n_rates += fhost_conf_rx.max_he_mcs * nss * fhost_conf_rx.he_mu_rates_per_mcs;

        fhost_conf_rx.first_he_er = fhost_conf_rx.n_rates;
        fhost_conf_rx.he_er_rates_per_mcs = 3;
        fhost_conf_rx.n_rates += 3 * fhost_conf_rx.he_er_rates_per_mcs + fhost_conf_rx.he_er_rates_per_mcs;
        #endif
    }
    #endif

    if (vht80)
        me_config->phy_bw_max = PHY_CHNL_BW_80;
    else if (ht40)
        me_config->phy_bw_max = PHY_CHNL_BW_40;
    else
        me_config->phy_bw_max = PHY_CHNL_BW_20;
}


/**
 ****************************************************************************************
 * @brief Return the channel associated to a given frequency
 *
 * @param[in] freq Channel frequency
 *
 * @return Channel definition whose primary frequency is the requested one and NULL
 * no such channel doesn't exist.
 ****************************************************************************************
 */
struct mac_chan_def *fhost_chan_get(int freq)
{
    int i, nb_chan;
    struct mac_chan_def *chans, *chan = NULL;

    if (freq < PHY_FREQ_5G)
    {
        chans = fhost_chan.chan2G4;
        nb_chan = fhost_chan.chan2G4_cnt;
    }
    else
    {
        chans = fhost_chan.chan5G;
        nb_chan = fhost_chan.chan5G_cnt;
    }

    for (i = 0; i < nb_chan; i++, chans++)
    {
        if (freq == chans->freq)
        {
            chan = chans;
            break;
        }
    }

    return chan;
}


/**
 * @}
 */
