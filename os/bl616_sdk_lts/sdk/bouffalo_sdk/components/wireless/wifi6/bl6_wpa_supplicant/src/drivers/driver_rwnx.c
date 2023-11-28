/*
 * Driver interface for RWNX platform
 * Copyright (C) RivieraWaves 2017-2021
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"

#include "common.h"
#include "driver.h"
#include "eloop.h"
#include "cfgrwnx.h"
#include "fhost.h"
#include "fhost_cntrl.h"
#include "fhost_wpa.h"
#include "mac_frame.h"
#include "common/ieee802_11_common.h"
#include "rwnx_config.h"
#include "wifi_mgmr_ext.h"
#if RW_MESH_EN
#include "mesh_mpm.h"
#include "mac_types.h"
#endif

#define TX_FRAME_TO_MS 300

#define MAC_FMT "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_LIST(arr) (arr)[0], (arr)[1], (arr)[2], (arr)[3], (arr)[4], (arr)[5]


/**
 * Structure allocated for each frame sent by wpa_supplicant (@ref wpa_rwnx_driver_init_tx_frame)
 * and freed once tx status is received (@ref wpa_rwnx_driver_release_tx_frame).
 */
struct wpa_rwnx_tx_frame {
	// pointer to driver interface data
	struct wpa_rwnx_driver_itf_data *drv;
	// frame data length
	size_t data_len;
	// eapol frame
	bool eapol;
	// dst addr (only for eapol)
	u8 dst_addr[ETH_ALEN];
	// frame data, actual buffer allocated is data_len bytes long
	u8 data[0];
};

/**
 * Per interface driver data
 * !!keep same with ctrl_iface_rwnx.c
 */
struct wpa_rwnx_driver_itf_data {
	// WPA_supplicant global context
	void *ctx;
	// Global driver data
	struct wpa_rwnx_driver_data *gdrv;
	// Index, at FHOST level, of the interface
	int fhost_vif_idx;
	// Initial interface type
	enum mac_vif_type vif_init_type;
	// List of scan results
	struct dl_list scan_res;
	// Driver status
	int status;
	// MAC address of the AP we are connected to
	u8 bssid[ETH_ALEN];
	// SSID of the AP we are connected to
	u8 *ssid;
	// SSID length
	u8 ssid_len;
	// Next authentication alg to try (used when connect with several algos)
	int next_auth_alg;
};

/**
 * Global data driver info
 * !!keep same with ctrl_iface_rwnx.c
 */
struct wpa_rwnx_driver_data {
	// WPA_supplicant context
	void *ctx;
	// List of interface driver data
	struct wpa_rwnx_driver_itf_data itfs[NX_VIRT_DEV_MAX];
	// cntrl link parameters
	struct fhost_cntrl_link *link;
	// Extended capabilities
	u8 extended_capab[10];
	// Extended capabilities mask
	u8 extended_capab_mask[10];
};

struct wpa_rwnx_driver_scan_res {
	struct dl_list list;
	struct wpa_scan_res *res;
};

enum wpa_rwnx_driver_status {
	RWNX_ASSOCIATED = BIT(0),
	RWNX_DISASSOC_PENDING = BIT(1),
	RWNX_COMPLETED = BIT(2),
	RWNX_AP_STARTED = BIT(3),
	RWNX_INITIALIZED = BIT(4),
	RWNX_SCANNING = BIT(5),
	RWNX_EXT_AUTH = BIT(6),
};

// For STA only accept action frames
#define STA_MGMT_RX_FILTER ~CO_BIT(WLAN_FC_STYPE_ACTION)

// For AP accept everyting except beacon
#define AP_MGMT_RX_FILTER CO_BIT(WLAN_FC_STYPE_BEACON)

#if RW_MESH_EN
// Conversion table BETWEEN MESH_STATE defined in WPA and MAC
const int mesh_state_conversion[8] = {-1,
				      MESH_MPM_IDLE,
				      MESH_MPM_OPN_SNT,
				      MESH_MPM_OPN_RCVD,
				      MESH_MPM_CNF_RCVD,
				      MESH_MPM_ESTAB,
				      MESH_MPM_HOLDING,
				      -1
};
#endif

static int wpa_rwnx_driver_vif_update(struct wpa_rwnx_driver_itf_data *drv,
					int type, int p2p);

/******************************************************************************
 * Hostapd to cfgrwnx type conversion and utils functions
 *****************************************************************************/
static void rwnx_to_hostapd_channel(struct mac_chan_def *rwnx,
				    struct hostapd_channel_data *hostapd)
{
	u8 channel;

	memset(hostapd, 0, sizeof(*hostapd));

	hostapd->freq = rwnx->freq;
	if (ieee80211_freq_to_chan(hostapd->freq, &channel) != NUM_HOSTAPD_MODES)
		hostapd->chan = channel;
	hostapd->flag = 0;
	hostapd->dfs_cac_ms = 0;
	hostapd->max_tx_power = rwnx->tx_power;

	if (rwnx->flags & CHAN_NO_IR) {
		hostapd->flag |= HOSTAPD_CHAN_NO_IR;
		hostapd->flag |= HOSTAPD_CHAN_RADAR | HOSTAPD_CHAN_DFS_USABLE;
		hostapd->dfs_cac_ms = 60000;
	}
	if (rwnx->flags & CHAN_DISABLED) {
		hostapd->flag |= HOSTAPD_CHAN_DISABLED;
	}

	// TODO, update this accordingly to regulatory
	hostapd->allowed_bw = ~0;

	dl_list_init(&hostapd->survey_list);
}

static void hostapd_to_rwnx_op_channel(struct hostapd_freq_params *hostapd,
				       struct mac_chan_op *rwnx)
{
	rwnx->band = (hostapd->mode == HOSTAPD_MODE_IEEE80211A) ? PHY_BAND_5G : PHY_BAND_2G4;
	rwnx->prim20_freq = hostapd->freq;
	rwnx->center1_freq = hostapd->center_freq1;
	rwnx->center2_freq = hostapd->center_freq2;
	switch (hostapd->bandwidth)
	{
	case 160:
		rwnx->type = PHY_CHNL_BW_160;
		break;
	case 80:
		if (rwnx->center2_freq)
			rwnx->type = PHY_CHNL_BW_80P80;
		else
			rwnx->type = PHY_CHNL_BW_80;
		break;
	case 40:
		rwnx->type = PHY_CHNL_BW_40;
		break;
	case 20:
		rwnx->type = PHY_CHNL_BW_20;
		break;
	default:
		// HT channel without center freq / bandwidth set
		if (hostapd->sec_channel_offset) {
			rwnx->type = PHY_CHNL_BW_40;
			rwnx->center1_freq = rwnx->prim20_freq  + 10 * hostapd->sec_channel_offset;
		} else {
			rwnx->type = PHY_CHNL_BW_20;
			rwnx->center1_freq = rwnx->prim20_freq;
		}
		break;
	}
	rwnx->tx_power = 0;
	rwnx->flags = 0;
}

static int hostapd_to_rwnx_cipher(enum wpa_alg alg, size_t key_len)
{
	switch (alg) {
	case WPA_ALG_WEP:
		if (key_len == 5)
			return MAC_CIPHER_WEP40;
		return MAC_CIPHER_WEP104;
	case WPA_ALG_TKIP:
		return MAC_CIPHER_TKIP;
	case WPA_ALG_CCMP:
		return MAC_CIPHER_CCMP;
	case WPA_ALG_BIP_CMAC_128:
		return MAC_CIPHER_BIP_CMAC_128;
	case WPA_ALG_SMS4:
		return MAC_CIPHER_WPI_SMS4;
	case WPA_ALG_GCMP:
		return MAC_CIPHER_GCMP_128;
	case WPA_ALG_CCMP_256:
		return MAC_CIPHER_CCMP_256;
	case WPA_ALG_GCMP_256:
		return MAC_CIPHER_GCMP_256;
	case WPA_ALG_BIP_GMAC_128:
	case WPA_ALG_BIP_GMAC_256:
	case WPA_ALG_BIP_CMAC_256:
	case WPA_ALG_KRK:
	case WPA_ALG_NONE:
		return MAC_CIPHER_INVALID;
	}

	return MAC_CIPHER_INVALID;
}

#define MAC_AUTH_ALGO_INVALID 0xffff
static int hostapd_to_rwnx_auth_alg(int auth_alg)
{
	switch (auth_alg)
	{
	case WPA_AUTH_ALG_OPEN:
		return  MAC_AUTH_ALGO_OPEN;
	case WPA_AUTH_ALG_SHARED:
		return MAC_AUTH_ALGO_SHARED;
	case WPA_AUTH_ALG_FT:
		return MAC_AUTH_ALGO_FT;
	case WPA_AUTH_ALG_SAE:
		return MAC_AUTH_ALGO_SAE;
	case WPA_AUTH_ALG_LEAP:
	default:
		return MAC_AUTH_ALGO_INVALID;
	}
}

static void rwnx_to_hostapd_he_capab(struct mac_hecapability *rwnx,
				     struct he_capabilities *hostapd)
{
	hostapd->he_supported = 1;
	os_memcpy(hostapd->phy_cap, rwnx->phy_cap_info, HE_MAX_PHY_CAPAB_SIZE);
	os_memcpy(hostapd->mac_cap, rwnx->mac_cap_info, HE_MAX_MAC_CAPAB_SIZE);
	os_memcpy(hostapd->mcs, &rwnx->mcs_supp, HE_MAX_MCS_CAPAB_SIZE);
	os_memcpy(hostapd->ppet, rwnx->ppe_thres, HE_MAX_PPET_CAPAB_SIZE);
}

static struct wpa_rwnx_tx_frame *
wpa_rwnx_driver_init_tx_frame(struct wpa_rwnx_driver_itf_data *drv, const u8 *data,
			      size_t data_len, const u8 *dst_addr)
{
	struct wpa_rwnx_tx_frame *tx_frame = NULL;

	tx_frame = os_malloc(sizeof(struct wpa_rwnx_tx_frame) + data_len);
	if (!tx_frame) {
		TRACE_FHOST(ERR, "[WPA] Failed to allocate frame buffer");
		return NULL;
	}

	tx_frame->drv = drv;
	tx_frame->data_len = data_len;
	if (dst_addr) {
		tx_frame->eapol = true;
		os_memcpy(tx_frame->dst_addr, dst_addr, ETH_ALEN);
	} else {
		tx_frame->eapol = false;
	}
	os_memcpy(tx_frame->data, data, data_len);

	return tx_frame;
}

static void wpa_rwnx_driver_release_tx_frame(struct wpa_rwnx_tx_frame *tx_frame)
{
	os_free(tx_frame);
}

static int *rwnx_init_rates(int *num)
{
	int leg_rate[] = {10, 20, 55, 110, 60, 90, 120, 180, 240, 360, 480, 540};
	int *rates;

	/* Assume all legacy rates are supported */
	rates = os_malloc(sizeof(leg_rate));
	if (!rates)
		return NULL;

	os_memcpy(rates, leg_rate, sizeof(leg_rate));
	*num = sizeof(leg_rate) / sizeof(int);
	return rates;
}

static void rwnx_ht_capabilities_init(struct hostapd_hw_modes *mode,
				      struct mac_htcapability *ht_cap)
{
	mode->flags |= HOSTAPD_MODE_FLAG_HT_INFO_KNOWN;
	mode->ht_capab = ht_cap->ht_capa_info;
	mode->a_mpdu_params = ht_cap->a_mpdu_param;
	os_memcpy(mode->mcs_set, ht_cap->mcs_rate, sizeof(mode->mcs_set));
}

static void rwnx_vht_capabilities_init(struct hostapd_hw_modes *mode,
				       struct mac_vhtcapability *vht_cap)
{
	mode->flags |= HOSTAPD_MODE_FLAG_VHT_INFO_KNOWN;
	mode->vht_capab = vht_cap->vht_capa_info;
	os_memcpy(mode->vht_mcs_set, (u8 *)&vht_cap->rx_mcs_map,
		  sizeof(mode->vht_mcs_set));
}

static void rwnx_he_capabilities_init(struct hostapd_hw_modes *mode,
				      struct mac_hecapability *he_cap)
{
	rwnx_to_hostapd_he_capab(he_cap, &mode->he_capab[IEEE80211_MODE_INFRA]);
#if NX_BEACONING
    // disable twt for ap
    HE_MAC_CAPA_BIT_CLR(he_cap, TWT_REQ);
	rwnx_to_hostapd_he_capab(he_cap, &mode->he_capab[IEEE80211_MODE_AP]);
#if RW_MESH_EN
	rwnx_to_hostapd_he_capab(he_cap, &mode->he_capab[IEEE80211_MODE_MESH]);

#endif
	// In AP mode, always set capab "40/80MHz supported in 5G" as long as HW suuports
	// 40MHz otherwise wpa_supplicant prevent to start a 40MHz AP if 80MHz is not supported.
	// It is assumed that 80MHz will be refused at fhost level if not supported by the HW
	if ((mode->mode == HOSTAPD_MODE_IEEE80211A) &&
	    (mode->ht_capab & HT_CAP_INFO_SUPP_CHANNEL_WIDTH_SET)) {
		mode->he_capab[IEEE80211_MODE_AP].phy_cap[HE_PHYCAP_CHANNEL_WIDTH_SET_IDX] |=
			HE_PHYCAP_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G;
#if RW_MESH_EN
		mode->he_capab[IEEE80211_MODE_MESH].phy_cap[HE_PHYCAP_CHANNEL_WIDTH_SET_IDX] |=
			HE_PHYCAP_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G;
#endif
	}
#endif // NX_BEACONING


}

static void wpa_rwnx_msg_hdr_init(struct wpa_rwnx_driver_itf_data *drv,
				  struct cfgrwnx_msg_hdr *msg_hdr,
				  uint16_t id, uint16_t len)
{
	msg_hdr->len        = len;
	msg_hdr->id         = id;
	msg_hdr->resp_queue = drv->gdrv->link->queue;
}

/******************************************************************************
 * Event processing functions
 *****************************************************************************/
static void wpa_rwnx_driver_process_scan_result(struct wpa_rwnx_driver_data *gdrv)
{
	struct wpa_rwnx_driver_itf_data *drv;
	struct ieee80211_mgmt *mgmt;
	struct cfgrwnx_scan_result res;
	struct wpa_rwnx_driver_scan_res *drv_res, *prev_res = NULL;
	struct wpa_scan_res *wpa_res;
	u16 fc;
	u8 *ie, *dst, *prev_src;
	bool is_beacon = false;
	int len = 0, ie_len;

    // puts("wpa_rwnx_driver_process_scan_result called\r\n");
	if ((fhost_cntrl_cfgrwnx_event_get(gdrv->link, &res, sizeof(res)) < 0) ||
	    !res.payload)
		return;

	drv = &gdrv->itfs[res.fhost_vif_idx];

	if (!(drv->status & RWNX_INITIALIZED) ||
	    (res.length < offsetof(struct ieee80211_mgmt, u.beacon.variable)))
		goto free_payload;

	mgmt = (struct ieee80211_mgmt *)res.payload;
	fc = le_to_host16(mgmt->frame_control);

	if (WLAN_FC_GET_TYPE(fc) != WLAN_FC_TYPE_MGMT)
		goto free_payload;
	if (WLAN_FC_GET_STYPE(fc) == WLAN_FC_STYPE_PROBE_RESP) {
		is_beacon = false;
		ie = mgmt->u.probe_resp.variable;
		ie_len = res.length - offsetof(struct ieee80211_mgmt, u.probe_resp.variable);
	} else if (WLAN_FC_GET_STYPE(fc) == WLAN_FC_STYPE_BEACON) {
		is_beacon = true;
		ie = mgmt->u.beacon.variable;
		ie_len = res.length - offsetof(struct ieee80211_mgmt, u.beacon.variable);
	} else {
		goto free_payload;
	}

	/* TODO: Add suport for filter option in scan request */

	/* Check if result for this bssid is already present */
	dl_list_for_each(drv_res, &drv->scan_res,
			 struct wpa_rwnx_driver_scan_res, list) {
		if (MAC_ADDR_CMP_PACKED(mgmt->bssid, drv_res->res->bssid)) {
			prev_res = drv_res;
			break;
		}
	}

	if (prev_res) {
		if ((is_beacon && prev_res->res->beacon_ie_len) ||
		    (!is_beacon && prev_res->res->ie_len)) {
			/* assume content didn't change */
			goto free_payload;
		} else if (is_beacon) {
			len = prev_res->res->ie_len;
		} else {
			len = prev_res->res->beacon_ie_len;
		}
		prev_src = (u8 *)prev_res->res + sizeof(struct wpa_scan_res);
	}
	len += sizeof(struct wpa_scan_res) + ie_len;

	drv_res = os_malloc(sizeof(struct wpa_rwnx_driver_scan_res));
	if (!drv_res)
		goto free_payload;

	wpa_res = os_malloc(len);
	if (!wpa_res) {
		os_free(drv_res);
		goto free_payload;
	}

	wpa_res->flags = WPA_SCAN_QUAL_INVALID | WPA_SCAN_NOISE_INVALID | WPA_SCAN_LEVEL_DBM;
	os_memcpy(wpa_res->bssid, mgmt->bssid, ETH_ALEN);
	wpa_res->freq = res.freq;
	if (is_beacon) {
		wpa_res->tsf = WPA_GET_LE64(mgmt->u.beacon.timestamp);
		wpa_res->beacon_int = le_to_host16(mgmt->u.beacon.beacon_int);
		wpa_res->caps = le_to_host16(mgmt->u.beacon.capab_info);
	} else {
		wpa_res->tsf = WPA_GET_LE64(mgmt->u.probe_resp.timestamp);
		wpa_res->beacon_int = le_to_host16(mgmt->u.probe_resp.beacon_int);
		wpa_res->caps = le_to_host16(mgmt->u.probe_resp.capab_info);
	}
	wpa_res->level = res.rssi;
	wpa_res->age = 0; /* TODO */
	wpa_res->est_throughput = 0;
	wpa_res->snr = 0;

	dst = (u8 *)wpa_res + sizeof(struct wpa_scan_res);
	if (is_beacon) {
		wpa_res->beacon_ie_len = ie_len;
		if (prev_res) {
			wpa_res->ie_len = prev_res->res->ie_len;
			os_memcpy(dst, prev_src, wpa_res->ie_len);
			dst += wpa_res->ie_len;
		} else {
			wpa_res->ie_len = 0;
		}
		os_memcpy(dst, ie, wpa_res->beacon_ie_len);
	} else {
		wpa_res->ie_len = ie_len;
		os_memcpy(dst, ie, wpa_res->ie_len);
		if (prev_res) {
			dst += wpa_res->ie_len;
			wpa_res->beacon_ie_len = prev_res->res->beacon_ie_len;
			os_memcpy(dst, prev_src, wpa_res->beacon_ie_len);
		} else {
			wpa_res->beacon_ie_len = 0;
		}
	}

	drv_res->res = wpa_res;
	dl_list_add(&drv->scan_res, &drv_res->list);
    // printf("bssid %02x:%02x:%02x:%02x:%02x:%02x added\r\n",
    //     mgmt->bssid[0],
    //     mgmt->bssid[1],
    //     mgmt->bssid[2],
    //     mgmt->bssid[3],
    //     mgmt->bssid[4],
    //     mgmt->bssid[5]
    // );

	if (prev_res) {
		dl_list_del(&prev_res->list);
		os_free(prev_res->res);
		os_free(prev_res);
	}

free_payload:
	rtos_free(res.payload);
}

static void wpa_rwnx_driver_process_scan_done_event(struct wpa_rwnx_driver_data *gdrv)
{
	struct wpa_rwnx_driver_itf_data *drv;
	struct cfgrwnx_scan_completed event;

	if (fhost_cntrl_cfgrwnx_event_get(gdrv->link, &event, sizeof(event)) < 0)
		return;

	drv = &gdrv->itfs[event.fhost_vif_idx];
	drv->status &= ~RWNX_SCANNING;
	if (!(drv->status & RWNX_INITIALIZED))
		return;

	wpa_supplicant_event(drv->ctx, EVENT_SCAN_RESULTS, NULL);
}

static void wpa_rwnx_driver_process_connect_event(struct wpa_rwnx_driver_data *gdrv)
{
	struct wpa_rwnx_driver_itf_data *drv;
	union wpa_event_data data;
	struct cfgrwnx_connect_event event;

	if (fhost_cntrl_cfgrwnx_event_get(gdrv->link, &event, sizeof(event)) < 0)
		return;

	drv = &gdrv->itfs[event.fhost_vif_idx];
	if (!(drv->status & RWNX_INITIALIZED))
		return;

	memset(&data, 0, sizeof(union wpa_event_data));

	if (event.status_code != WLAN_STATUS_SUCCESS) {
		data.assoc_reject.bssid = (u8 *)&event.bssid;
		data.assoc_reject.status_code = event.status_code;

		if ((data.assoc_reject.status_code == WLAN_STATUS_UNSPECIFIED_FAILURE) &&
		    (event.assoc_resp_ie_len == 0)) {
			data.assoc_reject.resp_ies = NULL ;
			data.assoc_reject.resp_ies_len = 0;
			data.assoc_reject.timed_out = 1;
		} else {
			data.assoc_reject.resp_ies = (event.req_resp_ies +
						      event.assoc_req_ie_len);
			data.assoc_reject.resp_ies_len = event.assoc_resp_ie_len;
			data.assoc_reject.timed_out = 0;
		}

		if (drv->next_auth_alg &&
		    (event.status_code == WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG)) {
			// If several authentication algs were specified (i.e.
			// next_auth_alg), then we can remove the one we just
			// tried (MSB) from the list
			drv->next_auth_alg &= ~(1 << (31 - co_clz(drv->next_auth_alg)));
		}

		wpa_supplicant_event(drv->ctx, EVENT_ASSOC_REJECT, &data);
	} else {
		const u8 *ssid_ie;
		data.assoc_info.reassoc = 0;
		data.assoc_info.req_ies = event.req_resp_ies;
		data.assoc_info.req_ies_len = event.assoc_req_ie_len;
		data.assoc_info.resp_ies = event.req_resp_ies + event.assoc_req_ie_len;
		data.assoc_info.resp_ies_len = event.assoc_resp_ie_len;
		data.assoc_info.beacon_ies = NULL;
		data.assoc_info.beacon_ies_len = 0;
		data.assoc_info.freq = event.freq;
		data.assoc_info.wmm_params.info_bitmap = 0;
		data.assoc_info.addr = (u8 *)&event.bssid;
		data.assoc_info.subnet_status = 0;

		drv->status |= RWNX_ASSOCIATED;
		memcpy(drv->bssid, &event.bssid, ETH_ALEN);
		ssid_ie = get_ie(event.req_resp_ies, event.assoc_req_ie_len, WLAN_EID_SSID);
		if (ssid_ie) {
			drv->ssid = os_malloc(ssid_ie[1]);
			if (drv->ssid) {
				drv->ssid_len = ssid_ie[1];
				memcpy(drv->ssid, &ssid_ie[2], ssid_ie[1]);
			}
		}
		wpa_supplicant_event(drv->ctx, EVENT_ASSOC, &data);
	}

	if (event.req_resp_ies)
		rtos_free(event.req_resp_ies);
}

static void wpa_rwnx_driver_process_disconnect_event(struct wpa_rwnx_driver_data *gdrv)
{
	struct wpa_rwnx_driver_itf_data *drv;
	union wpa_event_data data;
	struct cfgrwnx_disconnect_event event;

	if (fhost_cntrl_cfgrwnx_event_get(gdrv->link, &event, sizeof(event)) < 0)
		return;

	drv = &gdrv->itfs[event.fhost_vif_idx];
	if (!(drv->status & RWNX_INITIALIZED))
		return;

	data.disassoc_info.addr = drv->bssid;
	data.disassoc_info.reason_code = event.reason_code;
	data.disassoc_info.ie = NULL;
	data.disassoc_info.ie_len = 0;
	data.disassoc_info.locally_generated = !!(drv->status & RWNX_DISASSOC_PENDING);

	drv->status &= ~(RWNX_ASSOCIATED | RWNX_DISASSOC_PENDING);
	if (drv->ssid) {
		os_free(drv->ssid);
		drv->ssid = NULL;
		drv->ssid_len = 0;
	}

	wpa_supplicant_event(drv->ctx, EVENT_DISASSOC, &data);
}


static void wpa_rwnx_driver_process_mic_failure_event(struct wpa_rwnx_driver_data *gdrv)
{
	struct wpa_rwnx_driver_itf_data *drv;
	union wpa_event_data data;
	struct cfgrwnx_mic_failure_event event;

	if (fhost_cntrl_cfgrwnx_event_get(gdrv->link, &event, sizeof(event)) < 0)
		return;

	drv = &gdrv->itfs[event.fhost_vif_idx];
	if (!(drv->status & RWNX_INITIALIZED))
		return;

	data.michael_mic_failure.src = (u8 *)&event.addr;
	data.michael_mic_failure.unicast = event.ga ? 0 : 1;

	wpa_supplicant_event(drv->ctx, EVENT_MICHAEL_MIC_FAILURE, &data);
}

static void wpa_rwnx_driver_process_rx_mgmt_event(struct wpa_rwnx_driver_data *gdrv)
{
	struct wpa_rwnx_driver_itf_data *drv;
	union wpa_event_data data;
	struct cfgrwnx_rx_mgmt_event event;
	struct ieee80211_mgmt *mgmt;

	if ((fhost_cntrl_cfgrwnx_event_get(gdrv->link, &event, sizeof(event)) < 0) ||
	    !event.payload)
		return;

	drv = &gdrv->itfs[event.fhost_vif_idx];
	if (!(drv->status & RWNX_INITIALIZED))
		return;

	mgmt = (struct ieee80211_mgmt *)event.payload;
	data.rx_mgmt.frame = event.payload;
	data.rx_mgmt.frame_len = event.length;
	data.rx_mgmt.datarate = 0;
	data.rx_mgmt.drv_priv = drv;
	data.rx_mgmt.freq = event.freq;
	data.rx_mgmt.ssi_signal = event.rssi;

	if ((WLAN_FC_GET_STYPE(le_to_host16(mgmt->frame_control)) == WLAN_FC_STYPE_AUTH) &&
	    (le_to_host16(mgmt->u.auth.auth_alg) == WLAN_AUTH_SAE)) {
		// Since SAE authentication takes a lot of time to process ignore probe
		// request to avoid overflowed the event socket.
		// Since we are using external authentication in STA mode, wpa_supplicant
		// will call the send_external_auth_status callback when SAE authentication
		// is done even in AP mode, so filters are reset in this function.
		fhost_wpa_set_mgmt_rx_filter(drv->fhost_vif_idx,
					     AP_MGMT_RX_FILTER |
					     CO_BIT(WLAN_FC_STYPE_PROBE_REQ));
	}

	wpa_supplicant_event(drv->ctx, EVENT_RX_MGMT, &data);
	rtos_free(event.payload);
}

static void wpa_rwnx_driver_process_external_auth_event(struct wpa_rwnx_driver_data *gdrv)
{
	struct wpa_rwnx_driver_itf_data *drv;
	union wpa_event_data data;
	struct cfgrwnx_external_auth_event event;

	if (fhost_cntrl_cfgrwnx_event_get(gdrv->link, &event, sizeof(event)) < 0)
		return;

	drv = &gdrv->itfs[event.fhost_vif_idx];
	if (!(drv->status & RWNX_INITIALIZED))
		return;

	drv->status |= RWNX_EXT_AUTH;
	data.external_auth.action = EXT_AUTH_START;
	data.external_auth.key_mgmt_suite = event.akm;
	data.external_auth.bssid = (u8 *)event.bssid.array;
	data.external_auth.ssid = (u8 *)event.ssid.array;
	data.external_auth.ssid_len = event.ssid.length;

	// Need to forward Authentication frame for external authentication procedure
	fhost_wpa_set_mgmt_rx_filter(drv->fhost_vif_idx,
				     STA_MGMT_RX_FILTER ^ CO_BIT(WLAN_FC_STYPE_AUTH));
	wpa_supplicant_event(drv->ctx, EVENT_EXTERNAL_AUTH, &data);
}

static void wpa_rwnx_driver_process_tx_status_event(struct wpa_rwnx_driver_data *gdrv)
{
	union wpa_event_data data;
	struct cfgrwnx_tx_status_event event;
	struct wpa_rwnx_tx_frame *tx_frame;
	enum wpa_event_type wpa_event;

	if (fhost_cntrl_cfgrwnx_event_get(gdrv->link, &event, sizeof(event)) < 0)
		return;

	tx_frame = (struct wpa_rwnx_tx_frame *)event.data;

	if (tx_frame->eapol)
	{
		data.eapol_tx_status.dst = tx_frame->dst_addr;
		data.eapol_tx_status.data = tx_frame->data;
		data.eapol_tx_status.data_len = tx_frame->data_len;
		data.eapol_tx_status.ack = event.acknowledged;
		wpa_event = EVENT_EAPOL_TX_STATUS;
	}
	else
	{
		data.tx_status.type = WLAN_FC_GET_TYPE(tx_frame->data[0]);
		data.tx_status.stype = WLAN_FC_GET_STYPE(tx_frame->data[0]);
		data.tx_status.dst = ((struct ieee80211_hdr *)tx_frame->data)->addr1;
		data.tx_status.data = tx_frame->data;
		data.tx_status.data_len = tx_frame->data_len;
		data.tx_status.ack = event.acknowledged;
		wpa_event = EVENT_TX_STATUS;
	}

	// Interface may have been stopped just after posting the TX_STATUS event
	if (tx_frame->drv->status & RWNX_INITIALIZED)
		wpa_supplicant_event(tx_frame->drv->ctx, wpa_event, &data);

	wpa_rwnx_driver_release_tx_frame(tx_frame);
}

static void wpa_rwnx_driver_tx_status(uint32_t frame_id, bool acknowledged, void *arg)
{
	struct wpa_rwnx_tx_frame *tx_frame = arg;
	struct wpa_rwnx_driver_itf_data *drv = tx_frame->drv;
	struct wpa_rwnx_driver_data *gdrv = drv->gdrv;
	struct cfgrwnx_tx_status_event event;

	// Remember this callback is called in the WIFI task context, so we cannot call
	// wpa_supplicant_event as this may call another driver interface.
	// Instead defer its processing by sending an event to the wpa_supplicant task.
	event.hdr.id = CFGRWNX_TX_STATUS_EVENT;
	event.hdr.len = sizeof(event);
	event.data = (uint8_t *)tx_frame;
	event.acknowledged = acknowledged;

	if (!(drv->status & RWNX_INITIALIZED) ||
	    fhost_cntrl_cfgrwnx_event_send(&event.hdr, gdrv->link->sock_send))
		wpa_rwnx_driver_release_tx_frame(tx_frame);
}

static void wpa_rwnx_driver_process_probe_client_event(struct wpa_rwnx_driver_data *gdrv)
{
	struct wpa_rwnx_driver_itf_data *drv;
	union wpa_event_data data;
	struct cfgrwnx_probe_client_event event;

	if (fhost_cntrl_cfgrwnx_event_get(gdrv->link, &event, sizeof(event)) < 0)
		return;

	drv = &gdrv->itfs[event.fhost_vif_idx];
	if (!(drv->status & RWNX_INITIALIZED) || !event.client_present)
		return;

	memcpy(data.client_poll.addr, event.addr->array, ETH_ALEN);
	wpa_supplicant_event(drv->ctx, EVENT_DRIVER_CLIENT_POLL_OK, &data);
}

static void wpa_rwnx_driver_remain_on_channel_event(struct wpa_rwnx_driver_data *gdrv)
{
	struct wpa_rwnx_driver_itf_data *drv;
	union wpa_event_data data;
	struct cfgrwnx_remain_on_channel_event event;

	if (fhost_cntrl_cfgrwnx_event_get(gdrv->link, &event, sizeof(event)) < 0)
		return;

	data.remain_on_channel.duration = event.duration;
	data.remain_on_channel.freq     = event.freq;

	drv = &gdrv->itfs[event.fhost_vif_idx];
	if (!(drv->status & RWNX_INITIALIZED))
		return;

	wpa_supplicant_event(drv->ctx, EVENT_REMAIN_ON_CHANNEL, &data);
}

static void wpa_rwnx_driver_remain_on_channel_exp_event(struct wpa_rwnx_driver_data *gdrv)
{
	struct wpa_rwnx_driver_itf_data *drv;
	union wpa_event_data data;
	struct cfgrwnx_remain_on_channel_event event;

	if (fhost_cntrl_cfgrwnx_event_get(gdrv->link, &event, sizeof(event)) < 0)
		return;

	data.remain_on_channel.freq = event.freq;

	drv = &gdrv->itfs[event.fhost_vif_idx];
	if (!(drv->status & RWNX_INITIALIZED))
		return;

	wpa_supplicant_event(drv->ctx, EVENT_CANCEL_REMAIN_ON_CHANNEL, &data);
}

static void wpa_rwnx_driver_new_peer_candidate_event(struct wpa_rwnx_driver_data *gdrv)
{
	struct wpa_rwnx_driver_itf_data *drv;
	union wpa_event_data data;
	struct cfgrwnx_new_peer_candidate_event event;

	if (fhost_cntrl_cfgrwnx_event_get(gdrv->link, &event, sizeof(event)) < 0)
		return;

	data.mesh_peer.ie_len = event.ie_len;
	data.mesh_peer.ies = event.ies;
	data.mesh_peer.peer = event.peer;

	drv = &gdrv->itfs[event.fhost_vif_idx];
	if (!(drv->status & RWNX_INITIALIZED))
		return;

	wpa_supplicant_event(drv->ctx, EVENT_NEW_PEER_CANDIDATE, &data);

	if (event.ies) {
		rtos_free(event.ies);
		rtos_free(event.peer);
	}

}

/******************************************************************************
 * Send / Receive functions
 *****************************************************************************/
static void wpa_rwnx_driver_event(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct wpa_rwnx_driver_data *gdrv = eloop_ctx;
	struct cfgrwnx_msg_hdr msg_hdr;

	if (fhost_cntrl_cfgrwnx_event_peek_header(gdrv->link, &msg_hdr) < 0)
		return;

	switch (msg_hdr.id) {
	case CFGRWNX_SCAN_RESULT_EVENT:
		wpa_rwnx_driver_process_scan_result(gdrv);
		break;
	case CFGRWNX_SCAN_DONE_EVENT:
		wpa_rwnx_driver_process_scan_done_event(gdrv);
		break;
	case CFGRWNX_CONNECT_EVENT:
		wpa_rwnx_driver_process_connect_event(gdrv);
		break;
	case CFGRWNX_DISCONNECT_EVENT:
		wpa_rwnx_driver_process_disconnect_event(gdrv);
		break;
	case CFGRWNX_MIC_FAILURE_EVENT:
		wpa_rwnx_driver_process_mic_failure_event(gdrv);
		break;
	case CFGRWNX_RX_MGMT_EVENT:
		wpa_rwnx_driver_process_rx_mgmt_event(gdrv);
		break;
	case CFGRWNX_EXTERNAL_AUTH_EVENT:
		wpa_rwnx_driver_process_external_auth_event(gdrv);
		break;
	case CFGRWNX_TX_STATUS_EVENT:
		wpa_rwnx_driver_process_tx_status_event(gdrv);
		break;
	case CFGRWNX_PROBE_CLIENT_EVENT:
		wpa_rwnx_driver_process_probe_client_event(gdrv);
		break;
	case CFGRWNX_REMAIN_ON_CHANNEL_EVENT:
		wpa_rwnx_driver_remain_on_channel_event(gdrv);
		break;
	case CFGRWNX_REMAIN_ON_CHANNEL_EXP_EVENT:
		wpa_rwnx_driver_remain_on_channel_exp_event(gdrv);
		break;
	case CFGRWNX_NEW_PEER_CANDIDATE_EVENT:
		wpa_rwnx_driver_new_peer_candidate_event(gdrv);
		break;
	default:
		fhost_cntrl_cfgrwnx_event_discard(gdrv->link, &msg_hdr);
		break;
	}
}

/******************************************************************************
 * Drivers interface implemenation
 *****************************************************************************/
static void *wpa_rwnx_driver_init2(void *ctx, const char *ifname, void *global_priv)
{
	struct wpa_rwnx_driver_data *gdrv = global_priv;
	struct wpa_rwnx_driver_itf_data *drv = NULL;
	struct fhost_vif_status vif_status;
	int fhost_vif_idx;
    int type;

	TRACE_FHOST(WPA_INFO, "Driver RWNX init for %s", TR_STR_8(ifname));

	fhost_vif_idx = fhost_vif_idx_from_name(ifname);
	if (fhost_vif_idx < 0)
		goto err;

	if (fhost_get_vif_status(fhost_vif_idx, &vif_status) ||
	    ((vif_status.type != VIF_STA) && (vif_status.type != VIF_AP) &&
	     (vif_status.type != VIF_MESH_POINT)))
		goto err;

	drv = &gdrv->itfs[fhost_vif_idx];
	memset(drv, 0, sizeof(*drv));

	drv->ctx = ctx;
	drv->gdrv = gdrv;
	drv->fhost_vif_idx = fhost_vif_idx;
	drv->vif_init_type = vif_status.type;
	dl_list_init(&drv->scan_res);
	drv->status = RWNX_INITIALIZED;
	drv->ssid = NULL;
	drv->ssid_len = 0;

    // XXX TODO
    // Check here if adding mesh/p2p support
    if (fhost_vif_idx == MGMR_VIF_STA) {
        type = VIF_STA;
    } else {
        type = VIF_AP;
    }
	if (wpa_rwnx_driver_vif_update(drv, type, 0)) {
        goto err;
    }

	// Configure default RX filters (whatever initial interface type is)
	fhost_wpa_set_mgmt_rx_filter(fhost_vif_idx, STA_MGMT_RX_FILTER);

	fhost_wpa_send_event(FHOST_WPA_INTERFACE_ADDED, NULL, 0, drv->fhost_vif_idx);
	return drv;

err:
	TRACE_FHOST(ERR, "Failed to initialize RWNX driver for %s: "
		    "fhost_vif_idx=%d type=%d drv=%p", TR_STR_8(ifname),
		    fhost_vif_idx, vif_status.type, TR_PTR(drv));
	if (drv)
		os_free(drv);

	return NULL;
}

static void wpa_rwnx_driver_deinit(void *priv)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct wpa_rwnx_driver_scan_res *cur, *next;

	TRACE_FHOST(WPA_INFO, "{FVIF-%d} Driver RWNX deinit", drv->fhost_vif_idx);

	drv->status &= ~RWNX_INITIALIZED;

	if (drv->status & RWNX_SCANNING) {
		// TODO: abort scan
		TRACE_FHOST(ERR, "Need to implement abort scan");
	}

	dl_list_for_each_safe(cur, next, &drv->scan_res,
			      struct wpa_rwnx_driver_scan_res, list) {
		dl_list_del(&cur->list);
		os_free(cur->res);
		os_free(cur);
	}
	if (drv->ssid) {
		os_free(drv->ssid);
		drv->ssid = NULL;
	}

	fhost_wpa_send_event(FHOST_WPA_INTERFACE_REMOVED, NULL, 0, drv->fhost_vif_idx);
}

static void *wpa_rwnx_driver_global_init(void *ctx)
{
	struct wpa_rwnx_driver_data *gdrv;

	TRACE_FHOST(WPA_INFO, "Driver RWNX Global init");

	gdrv = os_zalloc(sizeof(struct wpa_rwnx_driver_data));
	if (gdrv == NULL)
		goto err;

	gdrv->ctx = ctx;

	// Open link with cntrl task to send cfgrwnx commands and retrieve events
	gdrv->link = fhost_cntrl_cfgrwnx_link_open();
	if (gdrv->link == NULL)
		goto err;

	eloop_register_read_sock(gdrv->link->sock_recv, wpa_rwnx_driver_event, gdrv, NULL);

	// Extended Capabilities
#define SET_EXT_CAPA_BIT(x)					\
	gdrv->extended_capab[(x)/8] |= 1 << ((x) % 8);		\
	gdrv->extended_capab_mask[(x)/8] |= 1 << ((x) % 8)

#if NX_TWT
	SET_EXT_CAPA_BIT(77); // TWT requester
#endif

#undef SET_EXT_CAPA_BIT

	return gdrv;

err:
	TRACE_FHOST(ERR, "Failed to initialize Global RWNX driver: gdrv=%p link=%p",
		    TR_PTR(gdrv), (gdrv) ? TR_PTR(gdrv->link) : 0);

	if (gdrv) {
		if (gdrv->link)
			fhost_cntrl_cfgrwnx_link_close(gdrv->link);
		os_free(gdrv);
	}
	return NULL;
}

static void wpa_rwnx_driver_global_deinit(void *priv)
{
	struct wpa_rwnx_driver_data *gdrv = priv;
	unsigned int i;

	TRACE_FHOST(WPA_INFO, "Driver RWNX Global deinit");

	for (i = 0; i < ARRAY_SIZE(gdrv->itfs); i++) {
		if (gdrv->itfs[i].status & RWNX_INITIALIZED)
			wpa_rwnx_driver_deinit(&gdrv->itfs[i]);
	}

	eloop_unregister_read_sock(gdrv->link->sock_recv);
	fhost_cntrl_cfgrwnx_link_close(gdrv->link);
	os_free(gdrv);
}

static uint8_t *wpa_rwnx_build_bcn(struct wpa_driver_ap_params *params, int *bcn_len,
				   int *tim_oft, int *tim_len)
{
	uint8_t *bcn_start, *bcn;

	*bcn_len = params->head_len + params->tail_len + MAC_TIM_MIN_LEN;
	bcn_start = os_malloc(*bcn_len);
	if (!bcn_start)
		return NULL;

	bcn = bcn_start;
	memcpy(bcn, params->head, params->head_len);
	bcn += params->head_len;
	// TIM element
	bcn[0] = MAC_ELTID_TIM;
	bcn[1] = MAC_TIM_MIN_LEN - 2;
	bcn[2] = 0;
	bcn[3] = (uint8_t)params->dtim_period;
	bcn[4] = 0;
	bcn[5] = 0;
	bcn += MAC_TIM_MIN_LEN;
	// TAIL
	memcpy(bcn, params->tail, params->tail_len);

	*tim_oft = params->head_len;
	*tim_len = MAC_TIM_MIN_LEN;

	return bcn_start;
}
static int wpa_rwnx_driver_update_bcn(struct wpa_rwnx_driver_itf_data *drv,
				      struct wpa_driver_ap_params *params)
{
	struct cfgrwnx_bcn_update cmd;
	struct cfgrwnx_resp resp;
	int res = 0;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_BCN_UPDATE_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_BCN_UPDATE_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	cmd.bcn = wpa_rwnx_build_bcn(params, &cmd.bcn_len, &cmd.tim_oft, &cmd.tim_len);
	if (!cmd.bcn)
		return -1;

	for (int i = 0; i < BCN_MAX_CSA_CPT; i++)
	{
		cmd.csa_oft[i] = 0;
	}

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) && (resp.status != CFGRWNX_SUCCESS))
		res = -1;

	os_free(cmd.bcn);
	return res;
}

static struct hostapd_hw_modes *wpa_rwnx_driver_get_hw_feature_data(void *priv,
								    u16 *num_modes,
								    u16 *flags, u8 *dfs)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_msg cmd;
	struct cfgrwnx_hw_feature feat;
	struct hostapd_hw_modes *modes, *mode;
	struct hostapd_channel_data *chan;
	struct mac_chan_def *chan_tag;
	int i;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_HW_FEATURE_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &feat.hdr, CFGRWNX_HW_FEATURE_RESP, sizeof(feat));

	*flags = 0;
	*dfs = 0;

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &feat.hdr))
		return NULL;

	/* Don't create mode in B */
	if (feat.chan->chan2G4_cnt && feat.chan->chan5G_cnt) {
		modes = os_zalloc(2 * sizeof(struct hostapd_hw_modes));
		*num_modes = 2;
	} else {
		modes = os_zalloc(sizeof(struct hostapd_hw_modes));
		*num_modes = 1;
	}
	if (!modes)
		return NULL;

	mode = modes;
	if (feat.chan->chan2G4_cnt) {
		mode->mode = HOSTAPD_MODE_IEEE80211G;
		mode->num_channels = feat.chan->chan2G4_cnt;
		mode->channels = os_malloc(feat.chan->chan2G4_cnt *
					   sizeof(struct hostapd_channel_data));
		if (!mode->channels)
			goto err;

		chan = mode->channels;
		chan_tag = feat.chan->chan2G4;
		for (i = 0 ; i < feat.chan->chan2G4_cnt ; i++, chan++, chan_tag++) {
			rwnx_to_hostapd_channel(chan_tag, chan);
		}

		mode->rates = rwnx_init_rates(&mode->num_rates);
		if (!mode->rates)
			goto err;

		if (feat.me_config.ht_supp) {
			rwnx_ht_capabilities_init(mode, &feat.me_config.ht_cap);

			if (feat.me_config.he_supp)
				rwnx_he_capabilities_init(mode, &feat.me_config.he_cap);
		}
		mode++;
	}

	if (feat.chan->chan5G_cnt) {
		mode->mode = HOSTAPD_MODE_IEEE80211A;
		mode->num_channels = feat.chan->chan5G_cnt;
		mode->channels = os_malloc(feat.chan->chan5G_cnt *
					   sizeof(struct hostapd_channel_data));
		if (!mode->channels)
			goto err;

		chan = mode->channels;
		chan_tag = feat.chan->chan5G;
		for (i = 0 ; i < feat.chan->chan5G_cnt ; i++, chan++, chan_tag++) {
			rwnx_to_hostapd_channel(chan_tag, chan);
		}

		mode->rates = rwnx_init_rates(&mode->num_rates);
		if (!mode->rates)
			goto err;

		if (feat.me_config.ht_supp) {
			rwnx_ht_capabilities_init(mode, &feat.me_config.ht_cap);

			if (feat.me_config.vht_supp)
				rwnx_vht_capabilities_init(mode, &feat.me_config.vht_cap);

			if (feat.me_config.he_supp)
				rwnx_he_capabilities_init(mode, &feat.me_config.he_cap);
		}
		mode++;
	}

	return modes;

err:
	for (i = 0 ; i < *num_modes; i++) {
		if (modes[i].channels)
			os_free(modes[i].channels);
		if (modes[i].rates)
			os_free(modes[i].rates);
	}

	os_free(modes);
	return NULL;
}

static int wpa_rwnx_driver_get_capa(void *priv, struct wpa_driver_capa *capa)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_msg cmd;
	struct cfgrwnx_msg resp;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_GET_CAPA_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_GET_CAPA_RESP, sizeof(resp));

	os_memset(capa, 0, sizeof(*capa));

	capa->key_mgmt = WPA_DRIVER_CAPA_KEY_MGMT_WPA |
			 WPA_DRIVER_CAPA_KEY_MGMT_WPA_PSK |
			 WPA_DRIVER_CAPA_KEY_MGMT_WPA2 |
			 WPA_DRIVER_CAPA_KEY_MGMT_WPA2_PSK |
			 WPA_DRIVER_CAPA_KEY_MGMT_SUITE_B |
			 WPA_DRIVER_CAPA_KEY_MGMT_SUITE_B_192;
	capa->enc = WPA_DRIVER_CAPA_ENC_WEP40 |
		    WPA_DRIVER_CAPA_ENC_WEP104 |
		    WPA_DRIVER_CAPA_ENC_TKIP |
#if NX_MFP
		    WPA_DRIVER_CAPA_ENC_BIP |
#endif
		    WPA_DRIVER_CAPA_ENC_CCMP;

        capa->auth = WPA_DRIVER_AUTH_OPEN |
                     WPA_DRIVER_AUTH_SHARED |
                     WPA_DRIVER_AUTH_LEAP;

	capa->flags = WPA_DRIVER_FLAGS_SET_KEYS_AFTER_ASSOC_DONE |
		      WPA_DRIVER_FLAGS_HT_2040_COEX |
		      WPA_DRIVER_FLAGS_SANE_ERROR_CODES;

#if NX_BEACONING
	capa->flags |= WPA_DRIVER_FLAGS_AP |
		       WPA_DRIVER_FLAGS_EAPOL_TX_STATUS |
#ifndef CFG_ENABLE_AP_PS
		       WPA_DRIVER_FLAGS_AP_MLME;
#else
		       WPA_DRIVER_FLAGS_AP_MLME |
		       WPA_DRIVER_FLAGS_AP_UAPSD;
#endif
		       //WPA_DRIVER_FLAGS_AP_CSA;
#endif
#if NX_CRYPTOLIB
	capa->flags |= WPA_DRIVER_FLAGS_SAE;
#endif
#if NX_P2P
	capa->flags |= WPA_DRIVER_FLAGS_P2P_CONCURRENT;
	capa->flags |= WPA_DRIVER_FLAGS_P2P_CAPABLE;
#endif
#if NX_TDLS
	capa->flags |= WPA_DRIVER_FLAGS_TDLS_SUPPORT;
#endif
#if RW_MESH_EN
	capa->flags |= WPA_DRIVER_FLAGS_MESH;
#endif

	capa->wmm_ac_supported = 0;
	//capa->mac_addr_rand_scan_supported = 0;
	//capa->mac_addr_rand_sched_scan_supported = 0;
	capa->max_scan_ssids = 1;
	//capa->max_sched_scan_ssids = 0;
	//capa->max_sched_scan_plans = 0;
	//capa->max_sched_scan_plan_interval = 0;
	//capa->max_sched_scan_plan_iterations = 0;
	//capa->sched_scan_supported = 0;
	//capa->max_match_sets = 0;
	//capa->max_remain_on_chan = 100;
	capa->max_stations = NX_REMOTE_STA_MAX;
	//capa->probe_resp_offloads;
	//capa->max_acl_mac_addrs;
	capa->num_multichan_concurrent = 2;
	capa->extended_capa = drv->gdrv->extended_capab;
	capa->extended_capa_mask = drv->gdrv->extended_capab_mask;
	capa->extended_capa_len = sizeof(drv->gdrv->extended_capab);
	//capa->wowlan_triggers;
	//capa->rrm_flags = 0;
	//capa->conc_capab = 0;
	//capa->max_conc_chan_2_4 = 0;
	//capa->max_conc_chan_5_0 = 0;
	capa->max_csa_counters = 2;

	fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr);
	return 0;
}

static int wpa_rwnx_driver_set_key(void *priv, struct wpa_driver_set_key_params *params)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_set_key cmd;
	struct cfgrwnx_resp resp;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_SET_KEY_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_SET_KEY_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	cmd.addr = (const struct mac_addr *)params->addr;
	if (params->alg == WPA_ALG_NONE) {
		cmd.cipher_suite = MAC_CIPHER_INVALID;
	} else {
		cmd.cipher_suite = hostapd_to_rwnx_cipher(params->alg, params->key_len);
		if (cmd.cipher_suite == MAC_CIPHER_INVALID)
			return -1;
	}
	cmd.key_idx = params->key_idx;
	cmd.key = params->key;
	cmd.key_len = params->key_len;
	cmd.seq = params->seq;
	cmd.seq_len = params->seq_len;
	cmd.pairwise = !!(params->key_flag & KEY_FLAG_PAIRWISE);

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) || (resp.status != CFGRWNX_SUCCESS))
		return -1;

	return 0;
}

static int wpa_rwnx_driver_scan2(void *priv, struct wpa_driver_scan_params *params)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_scan cmd;
	struct cfgrwnx_resp resp;

	memset(&cmd, 0, sizeof(cmd));

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_SCAN_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_SCAN_RESP, sizeof(resp));

	if (params->num_ssids > SCAN_SSID_MAX)
		return -1;

	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	cmd.passive = false;
	if (params->num_ssids == 0) {
		params->num_ssids = 1;
		params->ssids[0].ssid = NULL;
		params->ssids[0].ssid_len = 0;
		cmd.passive = true;
	}
	cmd.ssids = (struct cfgrwnx_scan_ssid *)params->ssids;
	cmd.ssid_cnt = params->num_ssids;
	cmd.extra_ies = params->extra_ies;
	cmd.extra_ies_len = params->extra_ies_len;
	cmd.freqs = params->freqs;
	cmd.no_cck = params->p2p_probe;
	cmd.bssid = params->bssid;
	if (params->duration_mandatory)
		cmd.duration = params->duration;
	else
		cmd.duration = 0;
	cmd.sock = drv->gdrv->link->sock_send;
	cmd.is_cntrl_link = false;

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) || (resp.status != CFGRWNX_SUCCESS))
		return -1;

	drv->status |= RWNX_SCANNING;

	return 0;
}

static struct wpa_scan_results * wpa_rwnx_driver_get_scan_results2(void *priv)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	int nb_res = dl_list_len(&drv->scan_res);
	struct wpa_scan_results *res;
	struct wpa_rwnx_driver_scan_res *cur, *next;
	int i = 0;


	res = os_malloc(sizeof(struct wpa_scan_results));
	if (!res) {
		return NULL;
	}
	res->num = nb_res;

	if (nb_res) {
		res->res = os_malloc(sizeof(struct wpa_scan_res *) * nb_res);
		if (!res->res) {
			os_free(res);
			return NULL;
		}

		dl_list_for_each_safe(cur, next, &drv->scan_res,
				      struct wpa_rwnx_driver_scan_res, list) {
			if (!cur->res->ie_len) {
				cur->res->ie_len = cur->res->beacon_ie_len;
				cur->res->beacon_ie_len = 0;
			}
			res->res[i] = cur->res;
			dl_list_del(&cur->list);
			os_free(cur);
			i++;
		}
		os_get_reltime(&res->fetch_time);
	} else {
		res->res = NULL;
	}

	return res;
}

static int wpa_rwnx_driver_vif_update(struct wpa_rwnx_driver_itf_data *drv,
					int type, int p2p)
{
	struct cfgrwnx_set_vif_type cmd;
	struct cfgrwnx_resp resp;

	// Update the interface with correct parameters
	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_SET_VIF_TYPE_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_SET_VIF_TYPE_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	cmd.type = type;
	cmd.p2p = p2p;

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) || (resp.status != CFGRWNX_SUCCESS))
		return -1;

	return 0;
}

static int wpa_rwnx_driver_associate_ap(struct wpa_rwnx_driver_itf_data *drv,
					struct wpa_driver_associate_params *params)
{
	if (params->uapsd == -1)
		params->uapsd = 1;

	fhost_wpa_set_mgmt_rx_filter(drv->fhost_vif_idx, AP_MGMT_RX_FILTER);

	return 0;
}

static int wpa_rwnx_driver_associate(void *priv,
				     struct wpa_driver_associate_params *params)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_connect cmd;
	struct cfgrwnx_resp resp;

	if (params->mode == IEEE80211_MODE_AP)
		return wpa_rwnx_driver_associate_ap(drv, params);

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_CONNECT_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_CONNECT_RESP, sizeof(resp));

	if (!params->bssid)
		return -1;
	cmd.bssid = params->bssid;
	cmd.ssid.ssid = params->ssid;
	cmd.ssid.len = params->ssid_len;

	cmd.chan.freq = params->freq.freq;
	if (params->freq.freq < 5000)
		cmd.chan.band = PHY_BAND_2G4;
	else
		cmd.chan.band = PHY_BAND_5G;
	cmd.chan.flags = 0;
	cmd.chan.tx_power = 0;
    cmd.wpa = 0;

	cmd.flags = CONTROL_PORT_HOST;
	if ((params->pairwise_suite == WPA_CIPHER_WEP40) ||
	    (params->pairwise_suite == WPA_CIPHER_TKIP) ||
	    (params->pairwise_suite == WPA_CIPHER_WEP104))
		cmd.flags |= DISABLE_HT;
	if (params->wpa_proto) {
		cmd.flags |= WPA_WPA2_IN_USE;
        cmd.wpa = params->wpa_proto;
    }
	if (params->key_mgmt_suite == WPA_KEY_MGMT_IEEE8021X_NO_WPA &&
	    (params->pairwise_suite == WPA_CIPHER_NONE ||
	     params->pairwise_suite == WPA_CIPHER_WEP104 ||
	     params->pairwise_suite == WPA_CIPHER_WEP40))
		cmd.flags |= CONTROL_PORT_NO_ENC;

	if (params->mgmt_frame_protection == MGMT_FRAME_PROTECTION_REQUIRED)
		cmd.flags |= MFP_IN_USE;

	cmd.ctrl_port_ethertype = htons(ETH_P_PAE);

	// Only consider authentication algo that are supported
	params->auth_alg &= (WPA_AUTH_ALG_OPEN | WPA_AUTH_ALG_SHARED |
			     WPA_AUTH_ALG_FT | WPA_AUTH_ALG_SAE);

	if (params->auth_alg == 0)
		return -1;

	cmd.auth_alg = hostapd_to_rwnx_auth_alg(params->auth_alg);
	if (cmd.auth_alg == MAC_AUTH_ALGO_INVALID)  {
		// Multiple Authentication algos (as we already filter out unsupported algo).
		int auth_alg;

		if (drv->next_auth_alg & params->auth_alg)
			params->auth_alg &= drv->next_auth_alg;
		else
			drv->next_auth_alg = params->auth_alg;

		// drv->next_auth_alg contains the list of auth algs. Try with
		// the first one (i.e. with the MSB) and if it is not supported
		// it will be removed in wpa_rwnx_driver_process_connect_event
		auth_alg = (1 << (31 - co_clz(params->auth_alg)));
		cmd.auth_alg = hostapd_to_rwnx_auth_alg(auth_alg);
	}
    if (cmd.auth_alg == MAC_AUTH_ALGO_SAE) {
        cmd.wpa = MAC_AUTH_ALGO_SAE;
    }

	cmd.fhost_vif_idx = drv->fhost_vif_idx;

	/* for now only support station role */
	if (params->mode != IEEE80211_MODE_INFRA)
		return -1;
	cmd.uapsd = params->uapsd;

	cmd.ie = params->wpa_ie;
	cmd.ie_len = params->wpa_ie_len;

	cmd.sock = drv->gdrv->link->sock_send;

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) || (resp.status != CFGRWNX_SUCCESS))
		return -1;

	return 0;
}

static int wpa_rwnx_driver_get_bssid(void *priv, u8 *bssid)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;

	if (drv->status & RWNX_ASSOCIATED) {
		memcpy(bssid, drv->bssid, ETH_ALEN);
	} else {
		memset(bssid, 0, ETH_ALEN);
	}

	return 0;
}

static int wpa_rwnx_driver_get_ssid(void *priv, u8 *ssid)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	int ret = 0;

	if (drv->status & RWNX_ASSOCIATED) {
		if (drv->ssid) {
			memcpy(ssid, drv->ssid, drv->ssid_len);
			ret = drv->ssid_len;
		} else {
			ret = -1;
		}
	}

	return ret;
}

static int wpa_rwnx_driver_set_supp_port(void *priv, int authorized)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_ctrl_port cmd;
	struct cfgrwnx_resp resp;

	if (!(drv->status & RWNX_ASSOCIATED))
		return 0;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_CTRL_PORT_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_CTRL_PORT_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	cmd.authorized = authorized;

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) || (resp.status != CFGRWNX_SUCCESS))
		return -1;

	return 0;
}

static int wpa_rwnx_driver_deauthenticate(void *priv, const u8 *addr, u16 reason_code)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_disconnect cmd;
	struct cfgrwnx_resp resp;

	if (!(drv->status & RWNX_ASSOCIATED))
		return -1;

	if (memcmp(addr, drv->bssid, ETH_ALEN))
		return -1;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_DISCONNECT_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_DISCONNECT_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	cmd.reason_code = reason_code;
    cmd.status_code = wpa_supplicant_get_status_code(drv->ctx);

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) || (resp.status != CFGRWNX_SUCCESS))
		return -1;

	drv->status |= RWNX_DISASSOC_PENDING;

	return 0;
}

static int wpa_rwnx_driver_set_operstate(void *priv, int state, int  old_state, int new_state)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;

	if ((old_state == -1) && (new_state == -1)) {
		if (state == 1) {
			drv->status |= RWNX_COMPLETED;
			drv->next_auth_alg = 0;
			fhost_wpa_send_event(FHOST_WPA_CONNECTED, NULL, 0, drv->fhost_vif_idx);
		} else if (drv->status & RWNX_COMPLETED) {
			// set_operstate is called with state = 0 when wpa state machine
			// enters WPA_ASSOCIATING, WPA_ASSOCIATED or WPA_DISCONNECTED
			// We just want to send disconnected when WPA_DISCONNECTED state is entered
			// (i.e. when WPA_COMPLETED was first entered)
			drv->status &= ~RWNX_COMPLETED;
			fhost_wpa_send_event(FHOST_WPA_DISCONNECTED, NULL, 0, drv->fhost_vif_idx);
		}
	} else {
		if (state == 1) {
			drv->status |= RWNX_COMPLETED;
			drv->next_auth_alg = 0;
			fhost_wpa_send_event(FHOST_WPA_CONNECTED, NULL, 0, drv->fhost_vif_idx);
		} else if ( (old_state > WPA_INACTIVE) && 
					(new_state == WPA_DISCONNECTED)) {
			// set_operstate is called with state = 0 when wpa state machine
			// enters WPA_ASSOCIATING, WPA_ASSOCIATED or WPA_DISCONNECTED
			// We just want to send disconnected when WPA_DISCONNECTED state is entered
			// (i.e. when WPA_COMPLETED was first entered)
			drv->status &= ~RWNX_COMPLETED;
			fhost_wpa_send_event(FHOST_WPA_DISCONNECTED, NULL, 0, drv->fhost_vif_idx);
		}
	}

	return 0;
}

static int wpa_rwnx_driver_send_mlme(void *priv, const u8 *data, size_t data_len,
				     int noack, unsigned int freq, const u16 *csa_offs,
				     size_t csa_offs_len, int no_encrypt,
				     unsigned int wait)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct wpa_rwnx_tx_frame *tx_frame = NULL;
	cb_fhost_tx cb = NULL;
    uint32_t ret = 0;


	if (freq || csa_offs_len) {
		TRACE_FHOST(ERR, "[WPA] TODO: support freq/csa_offs_len in send_mlme");
	}

	if (!noack) {
		tx_frame = wpa_rwnx_driver_init_tx_frame(drv, data, data_len, NULL);
		if (!tx_frame) {
            wpa_bl_printf("[WPA]%s No tx frame!\r\n", __func__);
			return -1;
        }
		cb = wpa_rwnx_driver_tx_status;
	}

    ret = fhost_send_80211_frame(drv->fhost_vif_idx, data, data_len, 0, cb, tx_frame);
    if (ret < 4) {
        wpa_bl_printf("[WPA]Fhost send failed, ret is %u\r\n", ret);
        return -1;
    }

	return 0;
}

static int wpa_rwnx_driver_send_external_auth_status(void *priv,
						     struct external_auth *params)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_external_auth_status resp;

	if (!(drv->status & RWNX_EXT_AUTH))
	{
		// If external authentication has not been started then this is an
		// AP or MESH interface.
		// Now that SAE processing is done we can re-start processing probe request
		fhost_wpa_set_mgmt_rx_filter(drv->fhost_vif_idx, AP_MGMT_RX_FILTER);
		return 0;
	}

	drv->status &= ~RWNX_EXT_AUTH;
	fhost_wpa_set_mgmt_rx_filter(drv->fhost_vif_idx, STA_MGMT_RX_FILTER);
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_EXTERNAL_AUTH_STATUS_RESP, sizeof(resp));
	resp.fhost_vif_idx = drv->fhost_vif_idx;
	resp.status = params->status;

	if (fhost_cntrl_cfgrwnx_cmd_send(&resp.hdr, NULL))
		return -1;

	return 0;
}

//In order to do coexistence test, Made a temporary patch
#ifdef CFG_FOR_COEXISTENCE_TEST_STOPAP_PATCH
struct wpa_rwnx_driver_itf_data *g_ap_drv = NULL;
#endif
static int wpa_rwnx_driver_set_ap(void *priv, struct wpa_driver_ap_params *params)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_start_ap cmd;
	struct cfgrwnx_resp resp;
	int res = -1;

//In order to do coexistence test, Made a temporary patch
#ifdef CFG_FOR_COEXISTENCE_TEST_STOPAP_PATCH
    g_ap_drv = drv;
#endif

	if (drv->status & RWNX_AP_STARTED)
		return wpa_rwnx_driver_update_bcn(drv, params);

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_START_AP_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_START_AP_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	cmd.basic_rates.length = 0;
	if (params->basic_rates) {
		int i = 0;
		while (params->basic_rates[i] != -1) {
			cmd.basic_rates.array[i] = ((uint8_t)(params->basic_rates[i] / 5) |
						    MAC_BASIC_RATE);
			i++;
		}
		cmd.basic_rates.length = i;
	}
	hostapd_to_rwnx_op_channel(params->freq, &cmd.chan);

	cmd.bcn = wpa_rwnx_build_bcn(params, &cmd.bcn_len, &cmd.tim_oft, &cmd.tim_len);
	if (!cmd.bcn)
		return -1;
	cmd.bcn_int = params->beacon_int;
	cmd.flags = CONTROL_PORT_HOST;
	if (params->key_mgmt_suites & WPA_KEY_MGMT_IEEE8021X_NO_WPA &&
	    (!params->pairwise_ciphers ||
	     params->pairwise_ciphers & (WPA_CIPHER_WEP104 | WPA_CIPHER_WEP40)))
		cmd.flags |= CONTROL_PORT_NO_ENC;
	if (params->wpa_version)
		cmd.flags |= WPA_WPA2_IN_USE;
	cmd.ctrl_ethertype = htons(ETH_P_PAE);
	cmd.sock = drv->gdrv->link->sock_send;

	if (!fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) &&
	    (resp.status == CFGRWNX_SUCCESS)) {
		res = 0;
		drv->status |= RWNX_AP_STARTED;
	}

	os_free(cmd.bcn);
	return res;
}

static int wpa_rwnx_driver_deinit_ap(void *priv)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_resp resp;
	struct cfgrwnx_set_vif_type cmd;

	// Always reset to STA filter whatever initial interface type
	fhost_wpa_set_mgmt_rx_filter(drv->fhost_vif_idx, STA_MGMT_RX_FILTER);

	if (drv->status & RWNX_AP_STARTED) {
		struct cfgrwnx_stop_ap stop;
		drv->status &= ~RWNX_AP_STARTED;
		wpa_rwnx_msg_hdr_init(drv, &stop.hdr, CFGRWNX_STOP_AP_CMD, sizeof(stop));
		wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_STOP_AP_RESP, sizeof(resp));
		stop.fhost_vif_idx = drv->fhost_vif_idx;

		if (fhost_cntrl_cfgrwnx_cmd_send(&stop.hdr, &resp.hdr) ||
		    (resp.status != CFGRWNX_SUCCESS))
			return -1;
	}

	if (drv->status & RWNX_SCANNING) {
		// TODO:
		TRACE_FHOST(ERR, "Need to abort scan");
	}

	// switch back to initial interface type
	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_SET_VIF_TYPE_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_SET_VIF_TYPE_RESP, sizeof(resp));
	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	cmd.type = drv->vif_init_type;
	cmd.p2p = false;

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) || (resp.status != CFGRWNX_SUCCESS))
		return -1;

	return 0;
}

//In order to do coexistence test, Made a temporary patch
#ifdef CFG_FOR_COEXISTENCE_TEST_STOPAP_PATCH
void wpa_rwnx_deinitap(void)
{
    if (g_ap_drv != NULL) {
        wpa_rwnx_driver_deinit_ap(g_ap_drv);
    }
}
#endif

static int wpa_rwnx_driver_set_tx_queue_params(void *priv, int queue, int aifs, int cw_min,
					       int cw_max, int burst_time)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_set_edca cmd;
	struct cfgrwnx_resp resp;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_SET_EDCA_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_SET_EDCA_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;

	// In WPA Supplicant the order of the tx queue is inversed compare to the MAC:
	// conf->tx_queue[0] = txq_vo ... conf->tx_queue[3] = txq_bk
	// An inversion of the order is done before copying to the tx_queue of the MAC
	cmd.hw_queue = (AC_MAX - 1) - queue;
	cmd.aifsn = aifs;
	cmd.cwmin = cw_min;
	cmd.cwmax = cw_max;
	cmd.txop = (burst_time * 100 + 16) / 32;

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) || (resp.status != CFGRWNX_SUCCESS))
		return -1;

	return 0;
}

static int wpa_rwnx_driver_hapd_send_eapol(void *priv, const u8 *addr, const u8 *data,
					   size_t data_len, int encrypt,
					   const u8 *own_addr, u32 flags)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_tx_status_event event;
	struct wpa_rwnx_tx_frame *tx_frame;
	bool ack;

	if (net_l2_send(fhost_to_net_if(drv->fhost_vif_idx), data, data_len, ETH_P_PAE,
			addr, &ack))
		return -1;

	tx_frame = wpa_rwnx_driver_init_tx_frame(drv, data, data_len, addr);
	if (!tx_frame)
		return -1;

	event.hdr.id = CFGRWNX_TX_STATUS_EVENT;
	event.hdr.len = sizeof(event);
	event.data = (uint8_t *)tx_frame;
	event.acknowledged = ack;

	if (fhost_cntrl_cfgrwnx_event_send(&event.hdr, drv->gdrv->link->sock_send))
		wpa_rwnx_driver_release_tx_frame(tx_frame);

	return 0;
}

static int wpa_rwnx_driver_sta_add(void *priv, struct hostapd_sta_add_params *params)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_resp resp;

	if (params->set)
	{
		#if RW_MESH_EN
		struct cfgrwnx_mesh_peer_update_ntf cmd;

		if ((params->plink_state == 0) || (params->plink_state > 6))
			return -1;

		memset(&cmd, 0, sizeof(cmd));
		wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_MESH_PEER_UPDATE_NTF_CMD, sizeof(cmd));
		wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_MESH_PEER_UPDATE_NTF_RESP, sizeof(resp));

		cmd.fhost_vif_idx = drv->fhost_vif_idx;
		cmd.addr = (const struct mac_addr *)params->addr;
		cmd.state = mesh_state_conversion[params->plink_state];

		if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) || (resp.status != CFGRWNX_SUCCESS))
			return -1;
		#else
		return -1;
		#endif
	}
	else
	{
		struct cfgrwnx_sta_add cmd;
		memset(&cmd, 0, sizeof(cmd));
		wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_STA_ADD_CMD, sizeof(cmd));
		wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_STA_ADD_RESP, sizeof(resp));

		cmd.fhost_vif_idx = drv->fhost_vif_idx;
		cmd.aid = params->aid;
		cmd.addr = (const struct mac_addr *)params->addr;

		cmd.rate_set.length = params->supp_rates_len;
		if (cmd.rate_set.length > MAC_RATESET_LEN)
			cmd.rate_set.length = MAC_RATESET_LEN;
		os_memcpy(cmd.rate_set.array, params->supp_rates, cmd.rate_set.length);

		if (params->capability & WLAN_CAPABILITY_SHORT_PREAMBLE)
			cmd.flags |= STA_SHORT_PREAMBLE_CAPA;

		if (params->ht_capabilities) {
			cmd.flags |= STA_HT_CAPA;
			os_memcpy(&cmd.ht_cap, params->ht_capabilities, sizeof(cmd.ht_cap));
		}
		if (params->vht_capabilities) {
			cmd.flags |= STA_VHT_CAPA;
			os_memcpy(&cmd.vht_cap, params->vht_capabilities, sizeof(cmd.vht_cap));
		}
		if (params->he_capab) {
			int ppe_idx, ppe_len;

			cmd.flags |= STA_HE_CAPA;
			os_memcpy(&cmd.he_cap.mac_cap_info, params->he_capab->he_mac_capab_info,
				  sizeof(cmd.he_cap.mac_cap_info));
			os_memcpy(&cmd.he_cap.phy_cap_info, params->he_capab->he_phy_capab_info,
				  sizeof(cmd.he_cap.phy_cap_info));

			cmd.he_cap.mcs_supp.rx_mcs_80 = WPA_GET_LE16(&params->he_capab->optional[0]);
			cmd.he_cap.mcs_supp.tx_mcs_80 = WPA_GET_LE16(&params->he_capab->optional[2]);

			if (params->he_capab->he_phy_capab_info[0] &
			    HE_PHYCAP_CHANNEL_WIDTH_SET_160MHZ_IN_5G) {
				cmd.he_cap.mcs_supp.rx_mcs_160 = WPA_GET_LE16(&params->he_capab->optional[4]);
				cmd.he_cap.mcs_supp.tx_mcs_160 = WPA_GET_LE16(&params->he_capab->optional[6]);
				if (params->he_capab->he_phy_capab_info[0] &
				    HE_PHYCAP_CHANNEL_WIDTH_SET_80PLUS80MHZ_IN_5G) {
					cmd.he_cap.mcs_supp.rx_mcs_80p80 = WPA_GET_LE16(&params->he_capab->optional[8]);
					cmd.he_cap.mcs_supp.tx_mcs_80p80 = WPA_GET_LE16(&params->he_capab->optional[10]);
					ppe_idx = 12;
				} else {
					cmd.he_cap.mcs_supp.rx_mcs_80p80 = 0xFFFF;
					cmd.he_cap.mcs_supp.tx_mcs_80p80 = 0xFFFF;
					ppe_idx = 8;
				}
			} else {
				cmd.he_cap.mcs_supp.rx_mcs_160 = 0xFFFF;
				cmd.he_cap.mcs_supp.tx_mcs_160 = 0xFFFF;
				ppe_idx = 4;
			}

			ppe_len = (params->he_capab_len - ppe_idx -
				   offsetof(struct ieee80211_he_capabilities, optional));
			os_memcpy(&cmd.he_cap.ppe_thres, &params->he_capab->optional[ppe_idx], ppe_len);
		}
		if (params->vht_opmode_enabled) {
			cmd.flags |= STA_OPMOD_NOTIF;
			cmd.opmode = params->vht_opmode;
		}
		if (params->flags & WPA_STA_WMM)
			cmd.flags |= STA_QOS_CAPA;
		if (params->flags & WPA_STA_MFP)
			cmd.flags |= STA_MFP_CAPA;
		cmd.uapsd_queues = (params->qosinfo & 0xF);
		cmd.max_sp_len  = (params->qosinfo & 0x60) >> 4;
		cmd.listen_interval = params->listen_interval;

		if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) || (resp.status != CFGRWNX_SUCCESS))
			return -1;
	}

	return 0;
}

static int wpa_rwnx_driver_sta_remove(void *priv, const u8 *addr)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_sta_remove cmd;
	struct cfgrwnx_resp resp;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_STA_REMOVE_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_STA_REMOVE_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	cmd.addr = (const struct mac_addr *)addr;

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) || (resp.status != CFGRWNX_SUCCESS))
		return -1;

	return 0;
}

static int wpa_rwnx_driver_sta_set_flags(void *priv, const u8 *addr,
					 unsigned int total_flags, unsigned int flags_or,
					 unsigned int flags_and)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_ctrl_port cmd;
	struct cfgrwnx_resp resp;
	int authorized = -1;

	// Only support authorized flag for now
	if (flags_or & WPA_STA_AUTHORIZED)
		authorized = 1;
	if (!(flags_and & WPA_STA_AUTHORIZED))
		authorized = 0;

	if (authorized < 0)
		return 0;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_CTRL_PORT_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_CTRL_PORT_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	os_memcpy(cmd.addr.array, addr, ETH_ALEN);
	cmd.authorized = authorized;

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) || (resp.status != CFGRWNX_SUCCESS))
		return -1;

	return 0;
}

static int wpa_rwnx_driver_sta_deauth(void *priv, const u8 *own_addr, const u8 *addr,
				      u16 reason)
{
	struct ieee80211_mgmt mgmt;
    int ret;

	os_memset(&mgmt, 0, sizeof(mgmt));
	mgmt.frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					  WLAN_FC_STYPE_DEAUTH);
	os_memcpy(mgmt.da, addr, ETH_ALEN);
	os_memcpy(mgmt.sa, own_addr, ETH_ALEN);
	os_memcpy(mgmt.bssid, own_addr, ETH_ALEN);
	mgmt.u.deauth.reason_code = host_to_le16(reason);

	ret = wpa_rwnx_driver_send_mlme(priv, (u8 *) &mgmt,
					 IEEE80211_HDRLEN + sizeof(mgmt.u.deauth),
					 0, 0, NULL, 0, 0, 0);
    if (!ret) {
        printf("[WPA]SEND DEAUTH: DA: " MAC_FMT " SA: " MAC_FMT " BSSID: " MAC_FMT \
                " reason_code: %u\r\n", MAC_LIST(mgmt.da), \
                MAC_LIST(mgmt.sa), MAC_LIST(mgmt.bssid),\
                host_to_le16(reason));
    }

    return ret;
}

static int wpa_rwnx_driver_sta_disassoc(void *priv, const u8 *own_addr, const u8 *addr,
					u16 reason)
{
	struct ieee80211_mgmt mgmt;
    int ret;

	os_memset(&mgmt, 0, sizeof(mgmt));
	mgmt.frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					  WLAN_FC_STYPE_DISASSOC);
	os_memcpy(mgmt.da, addr, ETH_ALEN);
	os_memcpy(mgmt.sa, own_addr, ETH_ALEN);
	os_memcpy(mgmt.bssid, own_addr, ETH_ALEN);
	mgmt.u.disassoc.reason_code = host_to_le16(reason);
	ret = wpa_rwnx_driver_send_mlme(priv, (u8 *) &mgmt,
					 IEEE80211_HDRLEN + sizeof(mgmt.u.disassoc),
					 0, 0, NULL, 0, 0, 0);
    if (!ret) {
        printf("[WPA]SEND DISASSO: DA: " MAC_FMT " SA: " MAC_FMT " BSSID: " MAC_FMT \
                " reason_code:%u\r\n", MAC_LIST(mgmt.da), \
                MAC_LIST(mgmt.sa), MAC_LIST(mgmt.bssid),\
                host_to_le16(reason));
    }
    return ret;
}

static int wpa_rwnx_driver_get_seqnum(const char *ifname, void *priv, const u8 *addr,
				      int idx, u8 *seq)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_key_seqnum cmd;
	struct cfgrwnx_key_seqnum_resp resp;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_KEY_SEQNUM_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_KEY_SEQNUM_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	cmd.addr = (const struct mac_addr *)addr;
	cmd.key_idx = idx;

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) || (resp.status != CFGRWNX_SUCCESS))
		return -1;

	// assume buffer is always 8 bytes long
	for (int i = 0 ; i < 8 ; i ++) {
		seq[i] = (resp.seqnum >> (8 * i)) & 0xff;
	}

	return 0;
}

static int wpa_rwnx_get_inact_sec(void *priv, const u8 *addr)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_get_sta_info cmd;
	struct cfgrwnx_get_sta_info_resp resp;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_GET_STA_INFO_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_GET_STA_INFO_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	cmd.addr = (const struct mac_addr *)addr;

	if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr))
		return -1;

	return resp.inactive_msec / 1000;
}

static void wpa_rwnx_poll_client(void *priv, const u8 *own_addr,
				 const u8 *addr, int qos)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_probe_client cmd;
	struct cfgrwnx_resp resp;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_PROBE_CLIENT_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_PROBE_CLIENT_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	cmd.addr = (const struct mac_addr *)addr;

	fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr);
}

static const char *wpa_rwnx_get_radio_name(void *priv)
{
	return wpa_driver_rwnx_ops.name;
}

static int wpa_rwnx_remain_on_channel(void *priv, unsigned int freq, unsigned int duration)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_remain_on_channel cmd;
	struct cfgrwnx_resp resp;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_REMAIN_ON_CHANNEL_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_REMAIN_ON_CHANNEL_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	cmd.duration = duration;
	cmd.freq = freq;
	cmd.sock = drv->gdrv->link->sock_send;

	fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr);

	return 0;
}

static int wpa_rwnx_cancel_remain_on_channel(void *priv)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_cancel_remain_on_channel cmd;
	struct cfgrwnx_resp resp;

	cmd.fhost_vif_idx = drv->fhost_vif_idx;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_CANCEL_REMAIN_ON_CHANNEL_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_CANCEL_REMAIN_ON_CHANNEL_RESP, sizeof(resp));

	fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr);

	return 0;
}

static int wpa_rwnx_send_action(void *priv, unsigned int freq, unsigned int wait,
				const u8 *dst, const u8 *src, const u8 *bssid,
				const u8 *data, size_t data_len, int no_cck)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct wpa_rwnx_tx_frame *tx_frame = NULL;
	struct ieee80211_hdr *hdr;
	cb_fhost_tx cb = NULL;
    uint32_t ret = 0;

	tx_frame = wpa_rwnx_driver_init_tx_frame(drv, NULL, IEEE80211_HDRLEN + data_len, NULL);
	if (!tx_frame)
		return -1;
	cb = wpa_rwnx_driver_tx_status;

	os_memcpy(tx_frame->data + IEEE80211_HDRLEN, data, data_len);

	hdr = (struct ieee80211_hdr *)tx_frame->data;
	hdr->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_ACTION);

	os_memcpy(hdr->addr1, dst, ETH_ALEN);
	os_memcpy(hdr->addr2, src, ETH_ALEN);
	os_memcpy(hdr->addr3, bssid, ETH_ALEN);

    ret = fhost_send_80211_frame(drv->fhost_vif_idx, tx_frame->data, IEEE80211_HDRLEN + data_len, no_cck, cb, tx_frame);
    if (ret < 4)
        return -1;

    return 0;
}

static int wpa_rwnx_probe_req_report(void *priv, int report)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct fhost_vif_status vif_status;
	int rx_filter;

	fhost_get_vif_status(drv->fhost_vif_idx, &vif_status);

	if (vif_status.type != VIF_AP)
	{
		rx_filter = fhost_wpa_get_mgmt_rx_filter(drv->fhost_vif_idx);

		if (report)
			fhost_wpa_set_mgmt_rx_filter(drv->fhost_vif_idx, rx_filter & ~(CO_BIT(WLAN_FC_STYPE_PROBE_REQ)));
		else
			fhost_wpa_set_mgmt_rx_filter(drv->fhost_vif_idx, rx_filter | (CO_BIT(WLAN_FC_STYPE_PROBE_REQ)));
	}

	return 0;
}

#if RW_MESH_EN
static int wpa_rwnx_init_mesh(void *priv)
{
	int rx_filter;

	struct wpa_rwnx_driver_itf_data *drv = priv;
	if (wpa_rwnx_driver_vif_update(drv, VIF_MESH_POINT, 0))
		return -1;

	fhost_wpa_set_mgmt_rx_filter(drv->fhost_vif_idx, AP_MGMT_RX_FILTER);

	return 0;
}

static int wpa_rwnx_join_mesh(void *priv,struct wpa_driver_mesh_join_params *params)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_join_mesh_cmd cmd;
	struct cfgrwnx_resp resp;
	struct hostapd_freq_params *freq = &params->freq;

	int i = 0, basic_rates = 0, rate_oft = 0;
	int rate_len = MAC_RATESET_LEN;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_JOIN_MESH_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_JOIN_MESH_RESP, sizeof(resp));

	cmd.sock = drv->gdrv->link->sock_send;
	cmd.fhost_vif_idx = drv->fhost_vif_idx;
	if (params->beacon_int != 0)
		cmd.bcn_int = params->beacon_int;
	else
		cmd.bcn_int = 1000;
	if (params->dtim_period != 0)
		cmd.dtim_period = params->dtim_period;
	else
		cmd.dtim_period = 2;
	cmd.mesh_id_len = params->meshid_len;
	cmd.mesh_id = params->meshid;
	cmd.user_mpm = 1;
	cmd.is_auth = params->flags & WPA_DRIVER_MESH_FLAG_SAE_AUTH;
	if (params->flags & WPA_DRIVER_MESH_FLAG_SAE_AUTH)
		cmd.auth_id = MESH_CONF_AUTH_PROTO_SAE;
	else
		cmd.auth_id = MESH_CONF_AUTH_PROTO_NO_AUTH;
	cmd.ie_len = params->ie_len;
	cmd.ie = params->ies;

	// Compute the number of legacy rates depending on the band
	if ( params->freq.freq >= PHY_FREQ_5G)
	{
		rate_len = MAC_RATES_ELMT_MAX_LEN;
		rate_oft = 4;
	}

	for (int i = rate_oft; i < rate_len; i++)
	{
		cmd.rates.array[i-rate_oft] = mac_id2rate[i];
	}
	cmd.rates.length = rate_len;

	i = 0;

	while (params->basic_rates[i] > 0)
	{
		for (int j = rate_oft ; j < rate_len; j++)
		{
			if (params->basic_rates[i] / 5 == mac_id2rate[j])
			{
				cmd.rates.array[j] |= MAC_BASIC_RATE;
				break;
			}
		}
		i++;
	}

	hostapd_to_rwnx_op_channel(&params->freq, &cmd.chan);

	fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr);

	return 0;
}

static int wpa_rwnx_leave_mesh(void *priv)
{
	struct wpa_rwnx_driver_itf_data *drv = priv;
	struct cfgrwnx_leave_mesh_cmd cmd;
	struct cfgrwnx_resp resp;

	wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_LEAVE_MESH_CMD, sizeof(cmd));
	wpa_rwnx_msg_hdr_init(drv, &resp.hdr, CFGRWNX_LEAVE_MESH_RESP, sizeof(resp));

	cmd.fhost_vif_idx = drv->fhost_vif_idx;

	fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr);

	return 0;
}
#endif //RW_MESH_EN

const struct wpa_driver_ops wpa_driver_rwnx_ops = {
	.name = "RWNX",
	.desc = "RWNX + LwIP driver",
	.init2 = wpa_rwnx_driver_init2,
	.deinit = wpa_rwnx_driver_deinit,
	.global_init = wpa_rwnx_driver_global_init,
	.global_deinit = wpa_rwnx_driver_global_deinit,
	.get_hw_feature_data = wpa_rwnx_driver_get_hw_feature_data,
	.get_capa = wpa_rwnx_driver_get_capa,
	.set_key = wpa_rwnx_driver_set_key,
	.scan2 = wpa_rwnx_driver_scan2,
	.get_scan_results2 = wpa_rwnx_driver_get_scan_results2,
	.set_supp_port = wpa_rwnx_driver_set_supp_port,
	.associate = wpa_rwnx_driver_associate,
	.get_bssid = wpa_rwnx_driver_get_bssid,
	.get_ssid = wpa_rwnx_driver_get_ssid,
	.deauthenticate = wpa_rwnx_driver_deauthenticate,
	.set_operstate = wpa_rwnx_driver_set_operstate,
	.send_mlme = wpa_rwnx_driver_send_mlme,
	.send_external_auth_status = wpa_rwnx_driver_send_external_auth_status,
	.set_ap = wpa_rwnx_driver_set_ap,
	.deinit_ap = wpa_rwnx_driver_deinit_ap,
	.set_tx_queue_params = wpa_rwnx_driver_set_tx_queue_params,
	.hapd_send_eapol = wpa_rwnx_driver_hapd_send_eapol,
	.sta_add = wpa_rwnx_driver_sta_add,
	.sta_remove = wpa_rwnx_driver_sta_remove,
	.sta_set_flags = wpa_rwnx_driver_sta_set_flags,
	.sta_deauth = wpa_rwnx_driver_sta_deauth,
	.sta_disassoc = wpa_rwnx_driver_sta_disassoc,
	.get_seqnum = wpa_rwnx_driver_get_seqnum,
	.get_inact_sec = wpa_rwnx_get_inact_sec,
	.poll_client = wpa_rwnx_poll_client,
	.get_radio_name = wpa_rwnx_get_radio_name,
	.remain_on_channel = wpa_rwnx_remain_on_channel,
	.cancel_remain_on_channel = wpa_rwnx_cancel_remain_on_channel,
	.send_action = wpa_rwnx_send_action,
	.probe_req_report = wpa_rwnx_probe_req_report,
	#if RW_MESH_EN
	.init_mesh = wpa_rwnx_init_mesh,
	.join_mesh = wpa_rwnx_join_mesh,
	.leave_mesh = wpa_rwnx_leave_mesh,
	#endif
};
