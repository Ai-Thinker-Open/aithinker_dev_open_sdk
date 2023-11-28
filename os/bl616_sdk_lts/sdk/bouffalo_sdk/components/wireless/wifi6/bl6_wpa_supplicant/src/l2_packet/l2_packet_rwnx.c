/*
 * Layer2 packet handling for rwnx system
 * Copyright (C) RivieraWaves 2017-2021
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"

#include "common.h"
#include "l2_packet.h"
#include "eloop.h"
#include "net_al.h"
#include "mac_frame.h"

struct l2_packet_data {
	int l2_hdr; /* whether TX buffer already contain L2 (Ethernet) header */
	int sock;
	net_if_t *net_if;
	void (*rx_callback)(void *ctx, const u8 *src_addr,
			    const u8 *buf, size_t len);
	void *rx_callback_ctx;
};

static void l2_packet_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct l2_packet_data *l2 = eloop_ctx;
	struct mac_eth_hdr *eth;
	uint8_t buf[500];
	uint8_t *payload;
	int len;

	len = recv(sock, buf, sizeof(buf), 0);
	if (len < (signed)sizeof(*eth)) {
		return;
	}

	eth = (struct mac_eth_hdr *)buf;
	payload = &buf[sizeof(*eth)];
	len -= sizeof(*eth);

	l2->rx_callback(l2->rx_callback_ctx, (u8 *)(&eth->sa),
			payload, len);
}

struct l2_packet_data * l2_packet_init(
	const char *ifname, const u8 *own_addr, unsigned short protocol,
	void (*rx_callback)(void *ctx, const u8 *src_addr,
			    const u8 *buf, size_t len),
	void *rx_callback_ctx, int l2_hdr)
{
	struct l2_packet_data *l2;

	l2 = os_zalloc(sizeof(struct l2_packet_data));
	if (l2 == NULL)
		return NULL;

	l2->rx_callback = rx_callback;
	l2->rx_callback_ctx = rx_callback_ctx;
	l2->l2_hdr = l2_hdr;

	l2->net_if = net_if_find_from_name(ifname);
	if (!l2->net_if)
		goto err;

	l2->sock = net_l2_socket_create(l2->net_if, protocol);
	if (l2->sock < 0)
		goto err;

	eloop_register_read_sock(l2->sock, l2_packet_receive, l2, NULL);
	return l2;

err:
	os_free(l2);
	return NULL;
}

struct l2_packet_data * l2_packet_init_bridge(
	const char *br_ifname, const char *ifname, const u8 *own_addr,
	unsigned short protocol,
	void (*rx_callback)(void *ctx, const u8 *src_addr,
			    const u8 *buf, size_t len),
	void *rx_callback_ctx, int l2_hdr)
{
	return l2_packet_init(br_ifname, own_addr, protocol, rx_callback,
			      rx_callback_ctx, l2_hdr);
}

void l2_packet_deinit(struct l2_packet_data *l2)
{
	if (l2 == NULL)
		return;

	eloop_unregister_read_sock(l2->sock);
	net_l2_socket_delete(l2->sock);
	os_free(l2);
}

int l2_packet_get_own_addr(struct l2_packet_data *l2, u8 *addr)
{
	os_memcpy(addr, net_if_get_mac_addr(l2->net_if), ETH_ALEN);
	return 0;
}

int l2_packet_send(struct l2_packet_data *l2, const u8 *dst_addr, u16 proto,
		   const u8 *buf, size_t len)
{
    bool ack;
    if (l2 == NULL)
        return -1;

    if (l2->l2_hdr)
        dst_addr = NULL;

    if (net_l2_send(l2->net_if, buf, len, proto, dst_addr, &ack))
        return -1;
    if (!ack)
        return -1;

    return 0;
}

int l2_packet_get_ip_addr(struct l2_packet_data *l2, char *buf, size_t len)
{
	/* TODO: get interface IP address (This function is not essential) */
	return -1;
}

void l2_packet_notify_auth_start(struct l2_packet_data *l2)
{
	/* This function can be left empty */
}

int l2_packet_set_packet_filter(struct l2_packet_data *l2,
				enum l2_packet_filter_type type)
{
	/* Only needed for advanced AP features */
	return -1;
}
