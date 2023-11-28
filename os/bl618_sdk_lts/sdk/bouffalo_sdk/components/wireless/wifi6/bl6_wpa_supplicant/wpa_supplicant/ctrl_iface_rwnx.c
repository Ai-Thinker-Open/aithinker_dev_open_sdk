/*
 * WPA Supplicant / UDP socket -based control interface
 * Copyright (c) 2004-2016, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"

#include "common.h"
#include "eloop.h"
#include "config.h"
#include "wpa_supplicant_i.h"
#include "ctrl_iface.h"
#include "common/wpa_ctrl.h"
#include "cfgrwnx.h"
#include "fhost.h"
#include "fhost_cntrl.h"

/**
 * Per interface driver data
 * !! keep same with driver_rwnx.c
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
 * !! keep same with driver_rwnx.c
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

static void wpa_rwnx_msg_hdr_init(struct wpa_rwnx_driver_itf_data *drv,
                  struct cfgrwnx_msg_hdr *msg_hdr,
                  uint16_t id, uint16_t len)
{
    msg_hdr->len        = len;
    msg_hdr->id         = id;
    msg_hdr->resp_queue = drv->gdrv->link->queue;
}


#ifndef CONFIG_NO_WPA_MSG
static void wpa_supplicant_ctrl_iface_msg_cb(void *ctx, int level,
					     enum wpa_msg_type type,
					     const char *txt, size_t len)
{
    struct cfgrwnx_status_code_print cmd;
    struct wpa_rwnx_driver_itf_data *drv = ((struct wpa_supplicant *)ctx)->drv_priv;

	if (level >= wpa_debug_level)
		dbg(D_CRT "[WPA] %s\n", txt);

    if (drv->vif_init_type != VIF_STA) {
        return;
    }
    memset(&cmd, 0, sizeof(cmd));

    wpa_rwnx_msg_hdr_init(drv, &cmd.hdr, CFGRWNX_STATUS_CODE_PRINT_CMD, sizeof(cmd));

    cmd.fhost_vif_idx = drv->fhost_vif_idx;
    cmd.sock = drv->gdrv->link->sock_send;
    cmd.msg = txt;

    if (!strncmp(txt, "CTRL-EVENT-NETWORK-NOT-FOUND", strlen("CTRL-EVENT-NETWORK-NOT-FOUND")) ||
            !strncmp(txt, "CTRL-EVENT-CONNECTED", strlen("CTRL-EVENT-CONNECTED"))) {
        if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, NULL)) {
            printf("%s: cmd send failed, txt is %s\r\n", __func__, txt);
        } else {
            printf("%s: cmd send succeded, txt is %s\r\n", __func__, txt);
        }
    }

}
#endif


struct ctrl_iface_priv {
};

struct ctrl_iface_global_priv {
	int sock;
};

/* Per interface ctrl interface not used */
struct ctrl_iface_priv *
wpa_supplicant_ctrl_iface_init(struct wpa_supplicant *wpa_s)
{
	// cannot return NULL
	return (struct ctrl_iface_priv *)1;
}

void wpa_supplicant_ctrl_iface_deinit(struct wpa_supplicant *wpa_s,
				      struct ctrl_iface_priv *priv)
{
}

void wpa_supplicant_ctrl_iface_wait(struct ctrl_iface_priv *priv)
{
}

/* Global ctrl_iface */
static void wpa_supplicant_global_ctrl_iface_receive(int sock, void *eloop_ctx,
						     void *sock_ctx)
{
	struct wpa_global *global = eloop_ctx;
	char buf[256];
	int res;
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);
	char *reply = NULL;
	size_t reply_len;

	res = recvfrom(sock, buf, sizeof(buf) - 1, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		TRACE_FHOST(ERR, "Fail to receive command on WPA ctrl interface");
		return;
	}
	buf[res] = '\0';

	reply = wpa_supplicant_global_ctrl_iface_process(global, buf,
							 &reply_len);

	if (reply) {
		sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from,
		       fromlen);
		os_free(reply);
	} else if (reply_len == 1) {
		sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *) &from,
		       fromlen);
	} else if (reply_len == 2) {
		sendto(sock, "OK\n", 3, 0, (struct sockaddr *) &from,
		       fromlen);
	}
}


struct ctrl_iface_global_priv *
wpa_supplicant_global_ctrl_iface_init(struct wpa_global *global)
{
	struct ctrl_iface_global_priv *priv;
	struct sockaddr_in addr;
	int port = WPA_GLOBAL_CTRL_IFACE_PORT;

	priv = os_zalloc(sizeof(*priv));
	if (priv == NULL)
		goto fail;

	priv->sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (priv->sock < 0)
		goto fail;

	os_memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl((127 << 24) | 1);

try_again:
	addr.sin_port = htons(port);
	if (bind(priv->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		port++;
		if ((port - WPA_GLOBAL_CTRL_IFACE_PORT) <
		    WPA_GLOBAL_CTRL_IFACE_PORT_LIMIT)
			goto try_again;
		goto fail;
	}

	// save port (as dummy char *)
	global->params.ctrl_interface = (char *)port;

	eloop_register_read_sock(priv->sock,
				 wpa_supplicant_global_ctrl_iface_receive,
				 global, priv);
	wpa_msg_register_cb(wpa_supplicant_ctrl_iface_msg_cb);

	return priv;

fail:
	TRACE_FHOST(ERR, "Failed to start WPA ctrl interface");
	if (priv->sock >= 0)
		close(priv->sock);
	os_free(priv);
	return NULL;
}


void
wpa_supplicant_global_ctrl_iface_deinit(struct ctrl_iface_global_priv *priv)
{
	if (priv->sock >= 0) {
		eloop_unregister_read_sock(priv->sock);
		close(priv->sock);
	}
	os_free(priv);
}
