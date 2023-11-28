/**
 ****************************************************************************************
 *
 * @file net_iperf_al.c
 *
 * @brief Definition of the networking stack abstraction layer used for iperf.
 *
 * Copyright (C) RivieraWaves 2017-2020
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/sys.h"
#include "lwip/opt.h"
#include "lwip/ip_addr.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/igmp.h"
#include "lwip/inet_chksum.h"
#include "lwip/etharp.h"

#include "fhost.h"
#include "fhost_ipc.h"
#include "fhost_iperf.h"
#include "net_iperf_al.h"
#include "trace.h"
#include "co_endian.h"

int net_if_get_ip(net_if_t *net_if, uint32_t *ip, uint32_t *mask, uint32_t *gw);
#if NX_IPERF
/*
 * DEFINITIONS
 ****************************************************************************************
 */
/// IPERF buffer len for TCP and UDP transfers
#define IPERF_BUFFER_LEN 1500 - UDP_HLEN - IP_HLEN

/// IPERF minimum duration between two UDP buffers burst transfer (in us)
#define IPERF_UDP_BURST_WAIT_US       10000

/// IPERF UDP frame header size
#define NET_IPERF_HEADER_SIZE         sizeof(struct iperf_UDP_datagram)

/// IPERF frame total header size
#define NET_IPERF_HEADER_TOTAL_SIZE   LWIP_MEM_ALIGN_SIZE( \
        NET_IPERF_HEADER_SIZE + PBUF_LINK_ENCAPSULATION_HLEN + PBUF_LINK_HLEN + \
        PBUF_IP_HLEN + PBUF_TRANSPORT_HLEN)
/// ARP reply message timeout  (in milliseconds)
#define ARP_REPLY_TO                  500 // 500ms
/// IPERF maximum number of polling attempts before closing TCP connection
#define IPERF_TCP_MAX_IDLE_SEC        10

/* IPERF version (2.0.9)*/
/// Iperf Major version (version 2)
#define IPERF_HDR_VER_MAJORHEX 0x00020000
/// Iperf Minor version (version 0.9)
#define IPERF_HDR_VER_MINORHEX 0x000D0000

/// IPERF Client/Server header mask definitions
/// IPERF Header extend type mask
#define IPERF_HDR_EXTEND_TYPE  0x1
/// IPERF Header version 1 mask
#define IPERF_HDR_VER1         0x80000000
/// IPERF Header extend mask
#define IPERF_HDR_EXTEND       0x40000000
/// IPERF Header udp tests mask
#define IPERF_HDR_UDPTESTS     0x20000000
/// IPERF Header timestamp mask
#define IPERF_HDR_TIMESTAMP    0x10000000
/// IPERF Header run now mask
#define IPERF_HDR_RUN_NOW      0x00000001


/// Stateless UDP Client Header (without handshake)
/// This is used for UDP tests that don't require
/// any handshake, i.e they are stateless
struct client_hdr_udp_tests {
    uint16_t testflags;
    uint16_t tlvoffset;
    uint32_t version_u;
    uint32_t version_l;
};

/// Extended UDP Client Header (with handshake)
/// This is used for tests that require
/// the initial handshake
struct client_hdrext {
    uint32_t type;
    uint32_t len;
    uint32_t flags;
    uint32_t version_u;
    uint32_t version_l;
    uint32_t reserved;
    uint32_t rate;
    uint32_t DP_rate_units;
    uint32_t real_time;
};

/// UDP server report
struct server_hdr
{
    uint32_t flags;
    uint32_t total_len1;
    uint32_t total_len2;
    uint32_t stop_sec;
    uint32_t stop_usec;
    uint32_t error_cnt;
    uint32_t outorder_cnt;
    uint32_t datagrams;
    uint32_t jitter1;
    uint32_t jitter2;
};

/// TCP Client Header Base (version 1)
struct client_hdr_v1
{
    uint32_t flags;
    uint32_t num_threads;
    uint32_t port;
    uint32_t bufferlen;
    uint32_t win_band;
    uint32_t amount;
};

/// Top TCP Client Header
struct client_hdr
{
    struct client_hdr_v1 base;
    union {
        struct client_hdrext extend;
        struct client_hdr_udp_tests udp;
    };
};

/// Structure containing the information about the IPERF sending buffer
struct net_iperf_send_info
{
    /// Pointer to IPERF stream
    struct fhost_iperf_stream *stream;
    /// Flag to describe if this buffer is occupied : 1 for occupied, 0 for free
    int flag;
};

/// IPERF sending process environment structure
struct net_iperf_send_buf_tag
{
    /// Network stack buffer element for headers
    struct pbuf_custom h;
    /// Network stack buffer element for IPERF payload
    struct pbuf_custom p;
    /// Structure containing the information about the IPERF sending buffer
    struct net_iperf_send_info info;
    /// Buffer to store all headers (Link/Transport/Iperf)
    char hdr[NET_IPERF_HEADER_TOTAL_SIZE];
};

/// IPERF TCP States
enum net_iperf_tcp_states
{
    IPERF_TCP_NONE = 0,
    IPERF_TCP_ACCEPTED,
    IPERF_TCP_RCV_HDR,
    IPERF_TCP_RECEIVED
};

/// IPERF TCP connection info
struct net_iperf_tcp_info
{
    /// Current state
    u8_t state;
    /// Polling counter
    u8_t poll_count;
    /// Client hdr length
    u8_t client_hdr_len;
    /// Pointer to TCP connection pcb
    struct tcp_pcb *pcb;
    /// Client header
    struct client_hdr client_hdr;
};

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
/// Payload to send (no copy memory) which is pointed by pbuf->payload
static char *net_iperf_nocopy_buf;
static int8_t net_iperf_nocopy_buf_ref_cnt;
/// Pool of IPERF send buffers
static struct net_iperf_send_buf_tag *net_iperf_send_buf_mem[FHOST_IPERF_MAX_STREAMS];

struct fhost_iperf_settings;

/*
 * FUNCTIONS
 ****************************************************************************************
 */
static err_t net_iperf_tcp_server_accept(void * arg, struct tcp_pcb *newpcb, err_t err);

/**
 ****************************************************************************************
 * @brief Callback function to free the pbuf_custom used for IPERF UDP payload
 *
 * It increases credit (i.e signal semaphore) and set flag to 0.
 * Note: It is mandatory to increase the credit and reset flag when freeing the paylaod
 * buffer (and not the header buffer) as it is freed last.
 *
 * @param[in] p pbuf_custom buffer to free (p in struct net_iperf_send_buf_tag)
 ****************************************************************************************
 **/
static void net_iperf_pbuf_custom_payload_free_cb(struct pbuf *p)
{
    struct net_iperf_send_buf_tag *buf;
    struct fhost_iperf_stream *stream;

    buf = (struct net_iperf_send_buf_tag *)((uint8_t *)p -
                                            offsetof(struct net_iperf_send_buf_tag, p));
    stream = buf->info.stream;

    // Avoid sending process to modify it in the same time
    rtos_mutex_lock(stream->iperf_mutex);
    buf->info.flag = 0;
    rtos_mutex_unlock(stream->iperf_mutex);
    // Send a signal to sending process that we have a new credit
    rtos_semaphore_signal(stream->send_buf_semaphore, false);
}

/**
 ****************************************************************************************
 * @brief Callback function to free the pbuf_custom used for IPERF UDP headers
 *
 * Does nothing unless there are no payload buffer attached
 * @param[in] p pbuf_custom buffer to free (h in struct net_iperf_send_buf_tag)
 ****************************************************************************************
 **/
static void net_iperf_pbuf_custom_header_free_cb(struct pbuf *p)
{
    struct net_iperf_send_buf_tag *buf;

    if (p->next)
        return;

    buf = (struct net_iperf_send_buf_tag *)((uint8_t *)p -
                                            offsetof(struct net_iperf_send_buf_tag, h));
    net_iperf_pbuf_custom_payload_free_cb((struct pbuf *)&buf->p);
}

/**
 ****************************************************************************************
 * @brief Print initial header for TCP/UDP Client and Server
 *
 * @param[in] iperf_settings   Pointer to iperf settings
 ****************************************************************************************
 **/
static void net_iperf_print_header(const struct fhost_iperf_settings *iperf_settings,
                                   rtos_task_handle iperf_handle)
{

    fhost_print(iperf_handle, "------------------------------------------------------------\n");
    if (iperf_settings->flags.is_server)
    {
        net_if_t *n = fhost_env.vif[iperf_settings->vif_num].net_if;
        uint32_t ip;

        if (iperf_settings->flags.is_udp)
        {
            fhost_print(iperf_handle, "Server listening on UDP port %d\n",
                        iperf_settings->port);
        }
        else
        {
            fhost_print(iperf_handle, "Server listening on TCP port %d\n",
                        iperf_settings->port);
        }
        net_if_get_ip(n, &ip, NULL, NULL);
        fhost_print(iperf_handle, "IP address : %d.%d.%d.%d\n",
                    ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);
    }
    else
    {
        if (iperf_settings->flags.is_udp)
        {
            fhost_print(iperf_handle, "Client connecting to %s, UDP port %d\n"
                        "Sending %d byte datagrams\n", inet_ntoa(iperf_settings->host_ip),
                        iperf_settings->port, iperf_settings->buf_len);
        }
        else
        {
            fhost_print(iperf_handle, "Client connecting to %s, TCP port %d\n",
                        inet_ntoa(iperf_settings->host_ip), iperf_settings->port);
        }
    }
    fhost_print(iperf_handle, "------------------------------------------------------------\n");
}

/**
 ****************************************************************************************
 * @brief Initialize iperf buffers
 *
 * @param[in] stream   Pointer to iperf stream
 ****************************************************************************************
 **/
static int net_iperf_buf_init(struct fhost_iperf_stream* stream, bool udp)
{
    uint32_t i, j, k;

    i = stream->id;
    if (!udp) {
        goto no_copy_buf;
    }

    // need leave some space for arp
    if(sizeof(struct net_iperf_send_buf_tag) * FHOST_IPERF_SEND_BUF_CNT > MEM_SIZE - 2048) {
        printf("buf no enough \r\n");
        return -1;
    }
    net_iperf_send_buf_mem[i] = mem_malloc(sizeof(struct net_iperf_send_buf_tag) * FHOST_IPERF_SEND_BUF_CNT);
    if (net_iperf_send_buf_mem[i] == NULL) {
        return -1;
    }
    memset(net_iperf_send_buf_mem[i], 0, sizeof(struct net_iperf_send_buf_tag)
            * FHOST_IPERF_SEND_BUF_CNT);
    for (j = 0; j < FHOST_IPERF_SEND_BUF_CNT; j++)
    {
        // Initialize callback function for its pbuf custom
        net_iperf_send_buf_mem[i][j].h.custom_free_function =
                (pbuf_free_custom_fn)net_iperf_pbuf_custom_header_free_cb;
        // Initialize callback function for its pbuf custom
        net_iperf_send_buf_mem[i][j].p.custom_free_function =
                (pbuf_free_custom_fn)net_iperf_pbuf_custom_payload_free_cb;
        // Pointer to IPERF stream
        net_iperf_send_buf_mem[i][j].info.stream = stream;
    }

no_copy_buf:
    if (net_iperf_nocopy_buf == NULL) {
        net_iperf_nocopy_buf = mem_malloc(IPERF_BUFFER_LEN);
        if (net_iperf_nocopy_buf == NULL) {
            mem_free(net_iperf_send_buf_mem[i]);
            net_iperf_send_buf_mem[i] = NULL;
            return -1;
        }
    }

    // Initialize constant buffer
    for (k = 0; k < IPERF_BUFFER_LEN; k++)
    {
        net_iperf_nocopy_buf[k] = 0x30 + k % 10;
    }

    ++net_iperf_nocopy_buf_ref_cnt;

    return 0;
}

/**
 ****************************************************************************************
 * @brief Deinitialize iperf buffers
 *
 * @param[in] stream   Pointer to iperf stream
 ****************************************************************************************
 **/
static void net_iperf_buf_deinit(struct fhost_iperf_stream *stream)
{
    int i;

    i = stream->id;
    mem_free(net_iperf_send_buf_mem[i]);
    net_iperf_send_buf_mem[i] = NULL;

    --net_iperf_nocopy_buf_ref_cnt;

    if (net_iperf_nocopy_buf_ref_cnt == 0) {
        mem_free(net_iperf_nocopy_buf);
        net_iperf_nocopy_buf = NULL;
    }
}

/**
 ****************************************************************************************
 * @brief Look for a free iperf buffer, set its flag to 1 and return it
 *
 * @param[in] stream   Pointer to iperf stream
 * @return  free iperf buffer, NULL if there is none
 ****************************************************************************************
 **/
static struct net_iperf_send_buf_tag *net_iperf_find_free_send_buf(const struct fhost_iperf_stream *stream,
                                                                   int to_time_ms)
{
    int i, j;

    i = stream->id;

    if (rtos_semaphore_wait(stream->send_buf_semaphore, to_time_ms))
    {
        //timeout
        return NULL;
    }

    // Protect net_iperf_send_buf_mem
    rtos_mutex_lock(stream->iperf_mutex);
    for (j = 0; j < FHOST_IPERF_SEND_BUF_CNT; j++)
    {
        if((net_iperf_send_buf_mem[i][j]).info.flag == 0)
        {
            (net_iperf_send_buf_mem[i][j]).info.flag = 1;
            rtos_mutex_unlock(stream->iperf_mutex);
            return &net_iperf_send_buf_mem[i][j];
        }
    }
    rtos_mutex_unlock(stream->iperf_mutex);

    return NULL;
}

/**
 ****************************************************************************************
 * @brief Handle UDP packet and update statistics.
 *
 * @param[in] stream         Pointer to iperf stream
 * @param[in] udp_datagram   Pointer to UDP datagram
 ****************************************************************************************
 **/
static void net_iperf_udp_handle_packet(struct fhost_iperf_stream* stream,
                                        struct iperf_UDP_datagram* udp_datagram)
{
    uint32_t netid;
    struct iperf_time transit, delta_transit;
    struct iperf_report *report = &stream->report;
    int packet_id;
    int delta_transit_us;

    memcpy(&netid, &udp_datagram->id, sizeof(netid));
    packet_id = co_ntohl(netid);

    report->stats.nb_datagrams++;
    memcpy(&netid, &udp_datagram->sec, sizeof(netid));
    report->sent_time.sec = co_ntohl(netid);
    memcpy(&netid, &udp_datagram->usec, sizeof(netid));
    report->sent_time.usec = co_ntohl(netid);
    if (packet_id < 0)
    {
        //For UDP server, set packet id as positive
        packet_id = -packet_id;
    }

    /* Compute jitter
    // from RFC 1889, Real Time Protocol (RTP)
    // J = J + ( | D(i-1,i) | - J ) / 16 */

    // Check if RX time < TX time. The client's and server's clocks do not need to be synchronized
    if (iperf_timerbefore(&report->sent_time, &report->packet_time))
        iperf_timersub(&report->packet_time, &report->sent_time, &transit);
    else
        iperf_timersub(&report->sent_time, &report->packet_time, &transit);

    if ( report->last_transit.sec != 0 || report->last_transit.usec != 0 ) {
        if (iperf_timerbefore(&transit, &report->last_transit))
            iperf_timersub(&report->last_transit, &transit, &delta_transit);
        else
            iperf_timersub(&transit, &report->last_transit, &delta_transit);

        delta_transit_us = iperf_timerusec(&delta_transit);
        report->stats.jitter_us += (int)(delta_transit_us - report->stats.jitter_us) / 16;
    }
    report->last_transit = transit;

    //Check for packet loss (expected packet ID + 1 != RX packet ID)
    if (packet_id != report->packet_id + 1) {
        if (packet_id < report->packet_id + 1) {
            // If RX packet ID < expected packet ID + 1, then,
            // in the past, we received a packet with ID > expected packet ID
            report->stats.nb_out_of_orded++;
        }
        else
        {
            // If RX packet ID > expected packet ID, then missing packets might have been
            // lost, or will be received in the future (out-of-order).
            report->stats.nb_error += packet_id - report->packet_id - 1;
        }
    }

    //Update expected packet ID every time RX packet ID is greater than expected packet ID
    if (packet_id > report->packet_id)
        report->packet_id = packet_id;
}

void net_iperf_tcp_close(struct fhost_iperf_stream *stream)
{
    struct net_iperf_tcp_info *info = stream->arg;
    struct iperf_report *report = &(stream->report);

    if (info == NULL)
        goto end;

    tcp_arg(info->pcb, NULL);
    tcp_sent(info->pcb, NULL);
    tcp_recv(info->pcb, NULL);
    tcp_poll(info->pcb, NULL, 0);
    tcp_err(info->pcb, NULL);
    if (tcp_close(info->pcb) != ERR_OK)
    {
        tcp_abort(info->pcb);
    }

    //Print final stats
    iperf_current_time(&report->end_time);
    fhost_iperf_print_stats(stream, &report->start_time, &report->end_time, &report->stats);

    //Delete tcp info
    info->pcb = NULL;
    rtos_free(info);
    stream->arg = NULL;

  end:
    if (!stream->iperf_settings.flags.is_server || !stream->active)
    {
        // Wakeup IPERF task
        rtos_semaphore_signal(stream->iperf_task_semaphore, false);
    }
}

/**
 ****************************************************************************************
 * @brief Error callback, iperf tcp session aborted
 *
 * @param[in] arg       Pointer to iperf stream
 * @param[in] err       Error value
 ****************************************************************************************
 **/
static void net_iperf_tcp_err(void *arg, err_t err)
{
    struct fhost_iperf_stream *stream = (struct fhost_iperf_stream *) arg;

    fhost_print(stream->iperf_handle, "Abort TCP (error %d)\n", err);
    net_iperf_tcp_close(stream);
}

/**
 ****************************************************************************************
 * @brief Write TCP/UDP client header to a buffer
 *
 * @param[in] hdr       Pointer to client header
 * @param[in] settings  Pointer to iperf settings
 * @return Client Header flags field
 ****************************************************************************************
 **/
static uint32_t net_iperf_write_hdr(struct client_hdr *hdr, struct fhost_iperf_settings *settings)
{
    uint32_t flags = 0;
    struct fhost_iperf_flags *settings_flags = &settings->flags;
    if (settings_flags->is_peer_ver ||
       (settings->test_mode != FHOST_IPERF_TEST_NORMAL && settings_flags->is_bw_set))
    {
        flags |= IPERF_HDR_EXTEND;
    }
    if (settings->test_mode != FHOST_IPERF_TEST_NORMAL)
    {
        flags |= IPERF_HDR_VER1;

        // Set buffer len
        if (settings_flags->is_buf_len_set)
        {
            hdr->base.bufferlen = co_htonl(settings->buf_len);
        }
        else
        {
            hdr->base.bufferlen = 0;
        }

        // Set port
        if (settings->listen_port != 0)
        {
            hdr->base.port = co_htonl(settings->listen_port);
        }
        else
        {
            hdr->base.port = co_htonl(settings->port);
        }

        //Single thread
        hdr->base.num_threads = co_htonl(1);

        // Set amount
        if(settings_flags->is_time_mode)
        {
            hdr->base.amount = co_htonl(-(long) settings->amount);
        }
        else
        {
            hdr->base.amount = co_htonl((long) settings->amount);
            hdr->base.amount &= co_htonl(0x7FFFFFFF);
        }

        if (settings->test_mode == FHOST_IPERF_TEST_DUALTEST)
        {
            flags |= IPERF_HDR_RUN_NOW;
        }
    }

    if (settings_flags->is_udp)
    {
        TRACE_APP(DBG, "TODO: missing write hdr for UDP");
    }

    //Update header flags
    hdr->base.flags = co_htonl(flags);

    if (flags & IPERF_HDR_EXTEND)
    {
        if (settings_flags->is_bw_set)
        {
            hdr->extend.rate = co_htonl(settings->udprate);
        }
        //TODO: Add PPS

        hdr->extend.type = co_htonl(IPERF_HDR_EXTEND_TYPE);
        hdr->extend.len = co_htonl(
                (sizeof(struct client_hdrext) - sizeof(hdr->extend.type) - sizeof(hdr->extend.len)));
        hdr->extend.reserved = 0;
        hdr->extend.version_u = co_htonl(IPERF_HDR_VER_MAJORHEX);
        hdr->extend.version_l = co_htonl(IPERF_HDR_VER_MINORHEX);
        hdr->extend.flags = co_htonl(0);
    }

    return flags;
}

/**
 ****************************************************************************************
 * @brief Read TCP client header
 *
 * @param[in] p         Packet buffer to read header from
 * @param[in] stream    Pointer to iperf stream
 * @return ERR_OK if successful
 ****************************************************************************************
 **/
static err_t net_iperf_tcp_read_client_hdr(struct pbuf *p, struct fhost_iperf_stream* stream)
{
    struct net_iperf_tcp_info *info = (struct net_iperf_tcp_info*) stream->arg;
    uint32_t *flags = &info->client_hdr.base.flags;
    err_t ret = ERR_OK;

    //Check if flags have been already received
    if (info->client_hdr_len <= sizeof(info->client_hdr.base.flags))
    {
        if (pbuf_copy_partial(p, flags, sizeof(info->client_hdr.base.flags), 0)
                    != sizeof(info->client_hdr.base.flags))
        {
            fhost_print(stream->iperf_handle, "TCP RX: too short packet (%d < %d)\n",
                        p->tot_len, sizeof(struct client_hdr_v1));
            ret = ERR_VAL;
            goto end;
        }

        *flags = co_ntohl(*flags);
        info->client_hdr_len = sizeof(struct client_hdr_v1);
        if ((*flags & IPERF_HDR_EXTEND) != 0)
        {
            fhost_print(stream->iperf_handle, "TODO: Header extended\n");
            info->client_hdr_len  = sizeof(struct client_hdr);
        }
        else if ((*flags & IPERF_HDR_VER1) != 0)
        {
            fhost_print(stream->iperf_handle, "TODO: Header version 1 => Client requested parallel transmission test\n");
            info->client_hdr_len = sizeof(struct client_hdr_v1);
        }
        else if ((*flags & IPERF_HDR_TIMESTAMP) != 0)
        {
            fhost_print(stream->iperf_handle, "TODO: Timestamp Client header\n");
            ret = ERR_VAL;
            goto end;
        }
    }

    // Flags already received, check if client hdr has been received
    if (pbuf_copy_partial(p, &info->client_hdr, info->client_hdr_len, 0)
            != info->client_hdr_len)
    {
        ret = ERR_VAL;
        goto end;
    }

    if ((*flags & IPERF_HDR_EXTEND) != 0)
    {
        fhost_print(stream->iperf_handle, "Extended Client HDR => require an ACK back to the client");
        ret = ERR_VAL;
        goto end;
    }
end:
    return ret;
}

/**
 ****************************************************************************************
 * @brief TCP packet reception callback
 *
 * @param[in] arg       Pointer to iperf stream
 * @param[in] newpcb    Pointer to TCP Protocol Process Block
 * @param[in] p         Received packet buffer
 * @param[in] err       different from ERR_OK if error occurred
 * @return ERR_OK if successful
 ****************************************************************************************
 **/
static err_t net_iperf_tcp_recv_cb(void * arg, struct tcp_pcb *newpcb,
                                   struct pbuf *p, err_t err)
{
    struct fhost_iperf_stream *stream = (struct fhost_iperf_stream*) arg;
    struct net_iperf_tcp_info *info = (struct net_iperf_tcp_info*) stream->arg;
    struct iperf_report *report = &stream->report;
    err_t ret = ERR_OK;

    // We receive an empty frame from client => close connection
    if (p == NULL)
    {
        //TODO: Client requested transmission after end of test
        stream->active = false;
        net_iperf_tcp_close(stream);
        return ERR_OK;
    }

    if (err != ERR_OK || !stream->active)
    {
        net_iperf_tcp_close(stream);
        ret = ERR_OK;
        goto end;
    }

    // If waiting for TCP settings or TX bytes multiple of 128KBytes
    if (info->state == IPERF_TCP_ACCEPTED)
    {
        if (net_iperf_tcp_read_client_hdr(p, stream) != ERR_OK)
        {
            net_iperf_tcp_close(stream);
            ret = ERR_VAL;
            goto end;
        }
        fhost_iperf_init_stats(stream);
        info->state = IPERF_TCP_RECEIVED;
    }
    else if (info->state == IPERF_TCP_RCV_HDR)
    {
        // TODO: read extended header ?
    }

    info->poll_count = 0;
    report->stats.bytes += p->tot_len;
    iperf_current_time(&report->packet_time);

    // interval stats
    fhost_iperf_print_interv_stats(stream);

    // Acknowledge data
    tcp_recved(newpcb, p->tot_len);
end:
    pbuf_free(p);
    return ret;
}

/**
 ****************************************************************************************
 * @brief Send data on an iperf tcp session
 *
 * @param[in] pcb       Pointer to tcp Protocol Control Block
 * @param[in] stream    Pointer to iperf stream
 * @param[in] settings  Pointer to iperf settings
 ****************************************************************************************
 **/
static void net_iperf_tcp_send(struct tcp_pcb* pcb,
                                struct fhost_iperf_stream *stream,
                                struct fhost_iperf_settings* iperf_settings)
{
    struct iperf_report *report = (struct iperf_report *) &stream->report;
    bool write_more = true;

    while (write_more)
    {
        u32_t len = TCP_MSS;
        err_t err;

        iperf_current_time(&report->packet_time);

        if (iperf_settings->flags.is_time_mode)
        {
            // Check if test duration is reached
            if(iperf_timerafter(&report->packet_time, &report->end_time))
            {
                net_iperf_tcp_close(stream);
                return;
            }
        }
        else
        {
            // Check if requested ammount to data has been transmitted
            if (report->stats.bytes >= iperf_settings->amount)
            {
                net_iperf_tcp_close(stream);
                return;
            }
        }

        LWIP_ASSERT("TCP_MSS > buffer size", TCP_MSS <= IPERF_BUFFER_LEN);
        do
        {
            // Queue data for sending
            err = tcp_write(pcb, net_iperf_nocopy_buf, len, 0);
            if (err == ERR_MEM)
            {
                len /= 2;
            }
        } while ((err == ERR_MEM) && (len >= (TCP_MSS / 2)));

        if (err == ERR_OK)
        {
            // Keep sending data
            report->stats.bytes += len;
        }
        else
        {
            write_more = false;
        }

        // Interval stats
        fhost_iperf_print_interv_stats(stream);
    }

    tcp_output(pcb);
}

/**
 ****************************************************************************************
 * @brief TCP polling callback.
 *
 * This function is called periodically (configured when registered with by @ref tcp_poll).
 *
 * For TCP server this function allow to end TCP connection at the end of the test (once
 * inactivity period has been reached).
 *
 * @param[in] arg       Pointer to iperf stream
 * @param[in] pcb       Pointer to TCP Protocol Control Block
 *
 * @return  ERR_OK if successful
 ****************************************************************************************
 **/
static err_t net_iperf_tcp_poll(void *arg, struct tcp_pcb *pcb)
{
    struct fhost_iperf_stream *stream = arg;
    struct net_iperf_tcp_info *info = stream->arg;

    if (info == NULL)
        // wait until connection is completed
        return ERR_OK;

    if (++info->poll_count >= IPERF_TCP_MAX_IDLE_SEC)
    {
        // Close TCP connection:
        // Mutex is used to avoid race condition with @ref net_iperf_tcp_server_accept
        // That may also end previous TCP connection.
        rtos_mutex_lock(stream->iperf_mutex);
        net_iperf_tcp_close(stream);
        rtos_mutex_unlock(stream->iperf_mutex);
    }
    else if (!stream->iperf_settings.flags.is_server)
    {
        // Send more data if we are client
        net_iperf_tcp_send(pcb, stream, &stream->iperf_settings);
    }

    return ERR_OK;
}

/**
 ****************************************************************************************
 * @brief TCP connection acceptance callback. This function is called while listening on a
 * TCP port to accept a connection to a client.
 * Use @ref tcp_accept to register this callback function.
 *
 * @param[in] arg       Pointer to iperf stream
 * @param[in] newpcb    Pointer to TCP Protocol Control Block
 * @param[in] err       different from ERR_OK if error occurred
 * @return  ERR_OK if successful
 ****************************************************************************************
 **/
static err_t net_iperf_tcp_server_accept(void * arg, struct tcp_pcb *newpcb, err_t err)
{
    struct net_iperf_tcp_info *info;
    struct fhost_iperf_stream *stream = (struct fhost_iperf_stream *) arg;
    err_t ret = ERR_OK;

    rtos_mutex_lock(stream->iperf_mutex);
    // Check if previous connection has been cleared (via net_iperf_tcp_poll)
    // otherwise do it here (mutex is to avoid race condition with net_iperf_tcp_poll)
    if (stream->arg)
        net_iperf_tcp_close(stream);
    rtos_mutex_unlock(stream->iperf_mutex);

    if ((err != ERR_OK) || (newpcb == NULL) || (arg == NULL))
    {
        return ERR_VAL;
    }

    tcp_setprio(newpcb, TCP_PRIO_MIN);

    // Create stream TCP info
    info = rtos_malloc(sizeof(struct net_iperf_tcp_info));

    if (info == NULL)
    {
        return ERR_MEM;
    }

    memset(info, 0, sizeof(struct net_iperf_tcp_info));
    info->state = IPERF_TCP_ACCEPTED;
    info->pcb = newpcb;
    stream->arg = info;
    memset(&stream->report, 0, sizeof(struct iperf_report));

    // Set stream as argument for TCP callbacks
    tcp_arg(newpcb, arg);

    // Initialize tcp_recv callback function
    tcp_recv(newpcb, net_iperf_tcp_recv_cb);

    // Initialize tcp_err callback function
    tcp_err(newpcb, net_iperf_tcp_err);

    // Initialize tcp_poll callback function (called every two seconds of inactivity of the TCP connection)
    tcp_poll(newpcb, net_iperf_tcp_poll, 2U);

    return ret;
}

/**
 ****************************************************************************************
 * @brief TCP connected callback.
 *
 * This function is called when the connection to a TCP server is enstablished.
 * Start the test by sending iperf header.
 *
 * @param[in] arg  Pointer to stream
 * @param[in] pcb  Pointer to TCP Protocol Control Block
 * @param[in] err  Different from ERR_OK if error occurred
 * @return  ERR_OK if successful
 ****************************************************************************************
 **/
static err_t net_iperf_tcp_client_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    struct fhost_iperf_stream *stream = (struct fhost_iperf_stream *) arg;
    struct net_iperf_tcp_info *info;
    int flags;

    if ((err != ERR_OK) || (pcb == NULL))
        return ERR_VAL;

    // Create stream TCP info
    info = rtos_malloc(sizeof(struct net_iperf_tcp_info));

    if (info == NULL)
        return ERR_MEM;

    memset(info, 0, sizeof(struct net_iperf_tcp_info));
    info->state = IPERF_TCP_ACCEPTED;
    info->pcb = pcb;
    stream->arg = info;

    flags = net_iperf_write_hdr(&info->client_hdr, &stream->iperf_settings);
    if (flags & IPERF_HDR_EXTEND)
        info->state = IPERF_TCP_RCV_HDR;

    // Send TCP client header message
    err = tcp_write(pcb, &info->client_hdr, sizeof(struct client_hdr), TCP_WRITE_FLAG_COPY);
    LWIP_ASSERT("impossible to send client header", err == ERR_OK);
    tcp_output(pcb);

    net_iperf_print_header(&stream->iperf_settings, stream->iperf_handle);
    fhost_iperf_init_stats(stream);
    return ERR_OK;
}

/**
 ****************************************************************************************
 * @brief TCP data sent callback. This function is called when TCP data
 * has been successfully delivered to the remote host. Send more data
 * Use @ref tcp_sent to register this callback function.
 *
 * @param[in] arg       Pointer to iperf stream
 * @param[in] pcb       Pointer to Process Control Block for which data has been acknowledged
 * @param[in] len       amount of bytes acknowledged
 * @return  ERR_OK if successful
 ****************************************************************************************
 **/
static err_t net_iperf_tcp_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    struct fhost_iperf_stream *stream = (struct fhost_iperf_stream *) arg;
    struct net_iperf_tcp_info *info = (struct net_iperf_tcp_info*) stream->arg;

    info->poll_count = 0;
    net_iperf_tcp_send(pcb, stream, &stream->iperf_settings);
    return ERR_OK;
}

/**
 ****************************************************************************************
 * @brief UDP Server Report reception callback.
 *
 * UDP Rx callback configured when client is waiting for server report at the end of
 * the test. The stream->to_semaphore is used to indicate waiting client that a packet
 * has been received.
 *
 * @param[in] arg       Pointer to iperf stream
 * @param[in] pcb       Pointer to UDP Process Control Block
 * @param[in] p         Pointer to packet buffer containing server report
 * @param[in] addr      Pointer to Server IP address
 * @param[in] port      Server connected port
 ****************************************************************************************
 **/
static void net_iperf_udp_server_report_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                                             const ip_addr_t *addr, u16_t port)
{
    struct fhost_iperf_stream* stream = arg;

    if (p == NULL)
        return;

    if (p->tot_len >= (int32_t) (sizeof(struct iperf_UDP_datagram) +
                                 sizeof(struct server_hdr)))
    {
        struct iperf_stats stats;
        struct iperf_time start_time, end_time;
        struct server_hdr hdr_o;
        struct server_hdr* hdr = &hdr_o;

        memcpy(&hdr_o, ((struct iperf_UDP_datagram*)p->payload + 1), sizeof(struct server_hdr));

        if (!(co_ntohl(hdr->flags) & IPERF_HDR_VER1))
        {
            TRACE_APP(ERR, "NET_IPERF_AL: iperf server report version not supported");
            goto end;
        }

        start_time.sec = 0;
        start_time.usec = 0;
        end_time.sec = co_ntohl(hdr->stop_sec);
        end_time.usec = co_ntohl(hdr->stop_usec);
        stats.bytes = (((uint64_t) co_ntohl( hdr->total_len1 )) << 32) + co_ntohl( hdr->total_len2 );
        stats.jitter_us = co_ntohl(hdr->jitter1) * 1000 + co_ntohl(hdr->jitter2);
        stats.nb_error = co_ntohl(hdr->error_cnt);
        stats.nb_datagrams = co_ntohl(hdr->datagrams);

        // reset some variable for correct stats, It's ok as test is finished after this
        stream->report.start_time = start_time;
        stream->report.last_stats.bytes = 0;
        stream->iperf_settings.flags.is_server = true;

        fhost_print(stream->iperf_handle, "Server report:\n");
        fhost_iperf_print_stats(stream, &start_time, &end_time, &stats);
        stream->iperf_settings.flags.is_server = false;
    }

  end:
    rtos_semaphore_signal(stream->to_semaphore, false);
    udp_recv(pcb, NULL, NULL);
    pbuf_free(p);
}

/**
 ****************************************************************************************
 * @brief Send last UDP packet and wait for server report
 *
 * @param[in] pcb           Pointer to UDP Process Control Block
 * @param[in] stream        Pointer to iperf stream
 * @param[in] packet_id     Packet ID of the last UDP packet to send
 * @return ERR_OK if successful
 ****************************************************************************************
 **/
static err_t net_iperf_udp_server_report_req(struct udp_pcb *pcb,
                                             struct fhost_iperf_stream *stream,
                                             int32_t packet_id)
{
    struct iperf_report *report = &stream->report;
    struct net_iperf_send_buf_tag *buf_tag;
    uint8_t cnt = 0;
    net_buf_tx_t *h = NULL, *p = NULL;
    char *hdr;
    uint32_t payload_val;
    err_t err_log;

    iperf_current_time(&report->packet_time)
    udp_recv(pcb, (udp_recv_fn) net_iperf_udp_server_report_recv, stream);

    while ( cnt < 10 )
    {
        // Retrieve a free buffer from buffer pool
        buf_tag = net_iperf_find_free_send_buf(stream, -1);
        if (buf_tag == NULL)
            break;

        h = pbuf_alloced_custom(PBUF_TRANSPORT, NET_IPERF_HEADER_SIZE, PBUF_RAM,
                                &(buf_tag->h), buf_tag->hdr, NET_IPERF_HEADER_TOTAL_SIZE);

        if (stream->iperf_settings.buf_len > NET_IPERF_HEADER_SIZE)
        {
            p = pbuf_alloced_custom(PBUF_RAW, stream->iperf_settings.buf_len - NET_IPERF_HEADER_SIZE,
                                    PBUF_ROM, &(buf_tag->p), net_iperf_nocopy_buf, IPERF_BUFFER_LEN);

            // Sanity check - If buf_tag is not NULL, then h and p should never be NULL
            ASSERT_ERR(p != NULL);
            pbuf_cat(h, p);
        }

        cnt++;
        hdr = h->payload;

        payload_val = co_htonl(packet_id);
        memcpy(&hdr[0], &payload_val, 4);
        payload_val = co_htonl(report->packet_time.sec);
        memcpy(&hdr[4], &payload_val, 4);
        payload_val = co_htonl(report->packet_time.usec);
        memcpy(&hdr[8], &payload_val, 4);

        LOCK_TCPIP_CORE();
        err_log = udp_send(pcb, h);
        UNLOCK_TCPIP_CORE();
        pbuf_free(h);

        if (err_log != ERR_OK)
        {
            fhost_print(rtos_get_task_handle(), "final NET_IPERF_AL : SEND ERR udp_sendto : %d", err_log);
            return ERR_VAL;
        }

        if (rtos_semaphore_wait(stream->to_semaphore, 350))
        {
            //timeout occurred
            continue;
        }

        break;
    }
    return ERR_OK;
}

/**
 ****************************************************************************************
 * @brief UDP receive callback after sending server report
 *
 * If a frame is received after sending the server report, assume that this is a report
 * request (since no check is done on the actual content of the buffer) and then trigger
 * IPERF server task to re-send the report.
 *
 * @param[in] arg           Pointer to iperf stream
 * @param[in] pcb           Pointer to UDP Process Control Block
 * @param[in] p             Buffer pointer of the last UDP packet
 * @param[in] addr          Pointer to Server IP address
 * @param[in] port          Server port
 ****************************************************************************************
 **/
static void net_iperf_udp_server_report_resend(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                                               const ip_addr_t *addr, u16_t port)
{
    if (p != NULL)
    {
        struct fhost_iperf_stream* stream = (struct fhost_iperf_stream*) arg;
        rtos_semaphore_signal(stream->to_semaphore, false);
        pbuf_free(p);
    }
}

/**
 ****************************************************************************************
 * @brief Send UDP server report to client
 *
 * @param[in] pcb           Pointer to UDP Process Control Block
 * @param[in] stream        Buffer pointer of the last UDP packet
 ****************************************************************************************
 **/
static void net_iperf_udp_server_report_send(struct udp_pcb *pcb,
                                             struct fhost_iperf_stream *stream)
{
    uint8_t cnt = 0;
    net_buf_tx_t *p = NULL;
    struct server_hdr *hdr;
    struct iperf_UDP_datagram *UDP_buf;
    struct iperf_report *report = &stream->report;
    struct iperf_time duration;

    if (!stream->active)
        return;

    // Open a UDP connection for sending report
    LOCK_TCPIP_CORE();
    udp_connect(pcb, &report->addr, report->port);
    // Set callback for receiving server report requests from client
    udp_recv(pcb, net_iperf_udp_server_report_resend, stream);
    UNLOCK_TCPIP_CORE();

    iperf_timersub(&report->end_time, &report->start_time, &duration);

    while ( cnt < 10 )
    {
        // Allocate a pbuf for transmitting the server report
        p = pbuf_alloc(PBUF_TRANSPORT, NET_IPERF_HEADER_TOTAL_SIZE +
                       sizeof(struct iperf_UDP_datagram) + sizeof(struct server_hdr), PBUF_RAM);

        // Sanity check - p should never be NULL
        ASSERT_ERR(p != NULL);

        UDP_buf = (struct iperf_UDP_datagram *) p->payload;
        UDP_buf->id = report->packet_id;
        UDP_buf->sec = report->sent_time.sec;
        UDP_buf->usec = report->sent_time.usec;

        hdr = (struct server_hdr *) (UDP_buf + 1);

        hdr->flags = co_htonl(IPERF_HDR_VER1);
        hdr->total_len1 = co_htonl((long) (report->stats.bytes >> 32));
        hdr->total_len2 = co_htonl((long) (report->stats.bytes & 0xFFFFFFFF));
        hdr->stop_sec = co_htonl((long) duration.sec);
        hdr->stop_usec = co_htonl((long) duration.usec);
        hdr->error_cnt = co_htonl(report->stats.nb_error);
        hdr->outorder_cnt = co_htonl(report->stats.nb_out_of_orded);
        hdr->datagrams = co_htonl(report->stats.nb_datagrams);
        hdr->jitter1 = report->stats.jitter_us / 1000000;
        hdr->jitter2 = co_htonl((long) (report->stats.jitter_us - (hdr->jitter1 * 1000000)));
        hdr->jitter1 = co_htonl((long) hdr->jitter1);
        cnt++;

        LOCK_TCPIP_CORE();
        if (udp_send(pcb, p))
            fhost_print(stream->iperf_handle, "NET_IPERF_AL : SEND ERR udp_sendto\n");
        UNLOCK_TCPIP_CORE();
        pbuf_free(p);

        if (rtos_semaphore_wait(stream->to_semaphore, 250))
        {
            //timeout occurred -> Report has been received by the client
            break;
        }
    }

    LOCK_TCPIP_CORE();
    udp_disconnect(pcb);
    UNLOCK_TCPIP_CORE();
}

/**
 ****************************************************************************************
 * @brief UDP packet reception callback
 *
 * @param[in] arg           Pointer to iperf stream
 * @param[in] pcb           Pointer to UDP Process Control Block
 * @param[in] p             Buffer pointer to the received UDP packet
 * @param[in] addr          Pointer to Server IP address
 * @param[in] port          Server port
 ****************************************************************************************
 **/
static void net_iperf_udp_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                                  const ip_addr_t *addr, u16_t port)
{
    int packet_id;
    uint32_t netid;
    struct fhost_iperf_stream *stream = arg;
    struct iperf_UDP_datagram *UDP_buf;
    struct iperf_report *report = &stream->report;

    if (p == NULL)
        return;

    if (!stream->active)
        goto end;

    iperf_current_time(&report->packet_time);

    // Get the counter in the received buffer (normally equal to counter_send)
    // Pointer pointed to counter_send in payload
    UDP_buf = p->payload;
    //Convert network format to host format
    memcpy(&netid, &UDP_buf->id, sizeof(netid));
    packet_id = co_ntohl(netid);

    if (report->packet_id == -1)
    {
        // First packet of the test, init stats.
        // Cannot just test packetID == 0 as we may miss this packet ...
        net_if_t *n = fhost_env.vif[stream->iperf_settings.vif_num].net_if;
        uint32_t ip;

        net_if_get_ip(n, &ip, NULL, NULL);
        fhost_print(stream->iperf_handle, "local %d.%d.%d.%d port %d connected with %d.%d.%d.%d port %d\n",
                    ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff,
                    stream->iperf_settings.port, addr->addr & 0xff, (addr->addr >> 8) & 0xff,
                    (addr->addr >> 16) & 0xff, (addr->addr >> 24) & 0xff, port);
        fhost_iperf_init_stats(stream);
    }

    if (packet_id != 0)
    {
        report->stats.bytes += p->tot_len;

        net_iperf_udp_handle_packet(stream, UDP_buf);

        // Interval stats
        fhost_iperf_print_interv_stats(stream);

        if (packet_id < 0) //End of reception
        {
            udp_recv(pcb, NULL, NULL);
            report->stats.nb_datagrams = report->packet_id;
            if (report->stats.nb_error > report->stats.nb_out_of_orded)
                report->stats.nb_error -= report->stats.nb_out_of_orded;

            //Print final stats
            report->end_time = report->packet_time;
            fhost_iperf_print_stats(stream, &report->start_time, &report->end_time, &report->stats);

            //Prepare packet for UDP report sent to client
            memcpy(&report->packet_id, &UDP_buf->id, sizeof(report->packet_id));
            report->port = port;
            report->addr = *addr;
            stream->active = false;
            rtos_semaphore_signal(stream->iperf_task_semaphore, false);
        }
    }
    else
    {
        report->packet_id = 0;
    }

  end:
    pbuf_free(p);
}

/**
 ****************************************************************************************
 * @brief Configure PCB
 *
 * @param[in] pcb           Pointer Process Control Block (UDP and TCP)
 * @param[in] stream        Pointer to iperf stream
 * @return ERR_OK if successful
 ****************************************************************************************
 **/
static err_t net_iperf_pcb_config(void *pcb, struct fhost_iperf_stream *stream)
{
    const struct fhost_iperf_settings *settings = (struct fhost_iperf_settings *) &stream->iperf_settings;
    uint32_t ip;
    ip_addr_t lip;

    if (settings->flags.is_server)
    {
        net_if_t *n = fhost_env.vif[settings->vif_num].net_if;
        uint32_t mask;

        // Bind Server IP address and port to pcb
        net_if_get_ip(n, &ip, &mask, NULL);
        ip_addr_set_ip4_u32(&lip,  ip);
        if (settings->flags.is_udp)
        {
            if (udp_bind((struct udp_pcb *) pcb, &lip, settings->port) != ERR_OK)
            {
                TRACE_APP(ERR, "NET_IPERF_AL: PCB CONFIG ERR udp_bind failed");
                return 1;
            }
        }
        else
        {
            if (tcp_bind((struct tcp_pcb *) pcb, &lip, settings->port))
            {
                TRACE_APP(ERR, "NET_IPERF_AL: PCB CONFIG ERR tcp_bind failed");
                return 1;
            }
        }

        // Print header
        net_iperf_print_header(settings, rtos_get_task_handle());
    }
    else
    {
        ip_addr_t rip;

        ip = settings->host_ip;
        ip_addr_set_ip4_u32(&rip,  ip);

        if (settings->flags.is_udp)
        {
            if (udp_bind((struct udp_pcb *) pcb, IP_ANY_TYPE, 0) != ERR_OK)
            {
                TRACE_APP(ERR, "NET_IPERF_AL : PCB CONFIG ERR udp_bind failed");
                return 1;
            }
            if (udp_connect((struct udp_pcb *) pcb, &rip, settings->port) != ERR_OK)
            {
                TRACE_APP(ERR, "NET_IPERF_AL : PCB CONFIG ERR udp_connect failed");
                return 1;
            }

            ((struct udp_pcb *) pcb)->tos = settings->tos;
            ((struct udp_pcb *) pcb)->ttl = settings->ttl;
        }
        else
        {
            tcp_arg((struct tcp_pcb *) pcb, stream);
            tcp_sent((struct tcp_pcb *) pcb, net_iperf_tcp_sent);
            tcp_poll((struct tcp_pcb *) pcb, net_iperf_tcp_poll, 2U);
            tcp_err((struct tcp_pcb *) pcb, net_iperf_tcp_err);
            tcp_recv((struct tcp_pcb *) pcb, net_iperf_tcp_recv_cb);

            if (tcp_connect((struct tcp_pcb *) pcb, &rip, settings->port,
                            net_iperf_tcp_client_connected) != ERR_OK)
            {
                TRACE_APP(ERR, "NET_IPERF_AL : PCB CONFIG ERR tcp_connect failed");
                return 1;
            }
            ((struct tcp_pcb *) pcb)->tos = settings->tos;
            ((struct tcp_pcb *) pcb)->ttl = settings->ttl;
        }
    }
    return 0;
}

/**
 ****************************************************************************************
 * @brief Create Protocol Control Block and configure it
 *
 * For client test, it first ensure that server is reachable by sending ARP request if
 * needed.
 * It then creates and configures the the connection PCB (TCP or UDP).
 *
 * @param[in] stream  Pointer to iperf stream
 * @return Active Protocol Control Block
 ****************************************************************************************
 **/
static void* net_iperf_init(struct fhost_iperf_stream* stream)
{
    void* pcb;

    // For client ensure that server is reachable first
    if (!stream->iperf_settings.flags.is_server)
    {
        uint32_t ip, tries = 0, found = 0;
        struct eth_addr *eth_ret;
        struct netif *netif;
        const ip4_addr_t *ip_ret;
        ip_addr_t rip;

        ip = stream->iperf_settings.host_ip;
        ip_addr_set_ip4_u32(&rip,  ip);

        /* Don't call ip_route() with IP_ANY_TYPE */
        netif = ip_route(IP46_ADDR_ANY(IP_GET_TYPE(&rip)), &rip);
        if (netif == NULL)
            return NULL;

        found = (etharp_find_addr(netif, &rip, &eth_ret, &ip_ret) != -1);
        while (!found && tries < 3)
        {
            etharp_request(netif, &rip);

            // It is not possible to use a callback for the ARP reply, set a timeout
            rtos_task_suspend(ARP_REPLY_TO);
            tries++;
            found = (etharp_find_addr(netif, &rip, &eth_ret, &ip_ret) != -1);
        }
        if (!found)
            return NULL;
    }

    LOCK_TCPIP_CORE();
    if (stream->iperf_settings.flags.is_udp)
    {
        // Create UDP Protocol Control Block
        pcb = udp_new();
        if (pcb == NULL)
        {
            TRACE_APP(ERR, "NET_IPERF_AL : IPERF INIT ERR udp_new failed");
            goto unlock;
        }
        stream->arg = pcb;
    }
    else
    {
        // Create TCP Protocol Control Block
        pcb = tcp_new();
        if (pcb == NULL)
        {
            TRACE_APP(ERR, "NET_IPERF_AL : IPERF INIT ERR tcp_new failed");
            goto unlock;
        }
    }

    // Configure pcb
    if (net_iperf_pcb_config(pcb, stream) != ERR_OK)
    {
        if (stream->iperf_settings.flags.is_udp)
            udp_remove(pcb);
        else
            tcp_close(pcb);
        pcb = NULL;
    }

  unlock:
    UNLOCK_TCPIP_CORE();
    return pcb;
}

int net_iperf_udp_server_run(struct fhost_iperf_stream* stream)
{
    int ret = -1;
    struct udp_pcb * pcb = NULL;

    if (net_iperf_buf_init(stream, true)) {
        goto exit;
    }
    pcb = net_iperf_init(stream);
    if (pcb == NULL)
    {
        TRACE_APP(ERR, "NET_IPERF_AL : SEND TRANSC ERR pcb null");
        goto cleanup;
    }

    while(stream->active)
    {
        stream->report.packet_id = -1;
        udp_recv(pcb, net_iperf_udp_recv_cb, stream);

        rtos_semaphore_wait(stream->iperf_task_semaphore, -1);
        net_iperf_udp_server_report_send(pcb, stream);
        fhost_print(stream->iperf_handle, "\n");
        memset(&stream->report, 0, sizeof(stream->report));
    }

    ret = 0;

cleanup:
    if (pcb) {
        LOCK_TCPIP_CORE();
        udp_remove(pcb);
        UNLOCK_TCPIP_CORE();
    }
    net_iperf_buf_deinit(stream);

exit:
    return ret;
}

int net_iperf_udp_client_run(struct fhost_iperf_stream* stream)
{
    int ret = -1;
    struct fhost_iperf_settings* iperf_settings = &stream->iperf_settings;
    struct iperf_report *report = &stream->report;
    struct udp_pcb * pcb = NULL;
    int32_t payload_val, packet_id, remaining_time_ms;
    struct iperf_time elapsed_time, last_sleep_time, expected_sleep_time;
    struct net_iperf_send_buf_tag *buf_tag = NULL;
    err_t err_log;
    net_buf_tx_t *h = NULL, *p = NULL;
    char *payload;
    uint32_t burst_intv_us, burst_size;
    bool done = false;

    if (net_iperf_buf_init(stream, true)) {
        goto exit;
    }
    pcb = net_iperf_init(stream);
    if (pcb == NULL)
    {
        TRACE_APP(ERR, "NET_IPERF_AL : SEND TRANSC ERR pcb null");
        goto cleanup;
    }

    // Init UDP transmission
    net_iperf_print_header(iperf_settings, rtos_get_task_handle());
    fhost_iperf_init_stats(stream);
    packet_id = 0;
    report->packet_time = last_sleep_time = report->start_time;

    // Ensure Buffer length is in a valid range
    if (iperf_settings->buf_len > IPERF_BUFFER_LEN)
        iperf_settings->buf_len = IPERF_BUFFER_LEN;
    else if (iperf_settings->buf_len < NET_IPERF_HEADER_SIZE)
        iperf_settings->buf_len = NET_IPERF_HEADER_SIZE;

    // Compute burst interval and size to reach target throughput
    burst_intv_us = (uint32_t) ((iperf_settings->buf_len) * ((float)(1000000 * 8) / (iperf_settings->udprate)));
    if (burst_intv_us < IPERF_UDP_BURST_WAIT_US)
        // the time between 2 buffer is too short, increase burst size
        burst_size = (IPERF_UDP_BURST_WAIT_US / burst_intv_us);
    else
        // enough time to sleep between 2 pkt
        burst_size = 1;
    expected_sleep_time.sec = 0;
    expected_sleep_time.usec = burst_intv_us * burst_size;

    remaining_time_ms = -1;
    while (!done && stream->active)
    {
        // Retrieve a free buffer from buffer pool
        buf_tag = net_iperf_find_free_send_buf(stream, remaining_time_ms);
        if (buf_tag == NULL)
            break;

        // custom PBUF for everything up to IPERF header
        h = pbuf_alloced_custom(PBUF_TRANSPORT, NET_IPERF_HEADER_SIZE, PBUF_RAM,
                                &(buf_tag->h), buf_tag->hdr, NET_IPERF_HEADER_TOTAL_SIZE);

        if (stream->iperf_settings.buf_len > NET_IPERF_HEADER_SIZE)
        {
            // custom PBUF for IPERF payload
            p = pbuf_alloced_custom(PBUF_RAW, stream->iperf_settings.buf_len - NET_IPERF_HEADER_SIZE,
                                    PBUF_ROM, &(buf_tag->p), net_iperf_nocopy_buf, IPERF_BUFFER_LEN);

            // Sanity check - If buf_tag is not NULL, then p should never be NULL
            LWIP_ASSERT("Initialized pbuf should not be NULL", p != NULL);
            pbuf_cat(h, p);
        }

        // Write IPerf header
        payload = h->payload;
        payload_val = co_htonl(packet_id++);
        memcpy(&payload[0], &payload_val, 4);
        payload_val = co_htonl(report->packet_time.sec);
        memcpy(&payload[4], &payload_val, 4);
        payload_val = co_htonl(report->packet_time.usec);
        memcpy(&payload[8], &payload_val, 4);

        LOCK_TCPIP_CORE();
        // Send UDP frame
        err_log = udp_send(pcb, h);
        UNLOCK_TCPIP_CORE();
        pbuf_free(h);

        if (err_log == ERR_OK)
        {
            // Update statistics
            report->stats.bytes += iperf_settings->buf_len;
            report->stats.nb_datagrams++;
        }
        else
        {
            TRACE_APP(ERR, "NET_IPERF_AL : SEND ERR");
            fhost_print(rtos_get_task_handle(), "NET_IPERF_AL : SEND ERR udp_sendto : %d", err_log);
            goto cleanup;
        }

        if (report->stats.nb_datagrams % burst_size == 0)
        {
            // Burst is over, sleep until next burst

            if (iperf_timerbefore(&report->packet_time, &last_sleep_time))
            {
                // corner case when burst size takes less than sleep adjustment
                elapsed_time.sec = 0;
                elapsed_time.usec = 0;
            }
            else
            {
                iperf_timersub(&report->packet_time, &last_sleep_time, &elapsed_time);
            }

            if (iperf_timerbefore(&elapsed_time, &expected_sleep_time))
            {
                struct iperf_time sleep_time, wakeup_time;
                uint32_t sleep_ms;

                iperf_timersub(&expected_sleep_time, &elapsed_time, &sleep_time);
                iperf_timeradd(&report->packet_time, &sleep_time, &wakeup_time);

                if (iperf_settings->flags.show_int_stats)
                {
                    fhost_iperf_print_interv_stats(stream);

                    // Compute next wake up time to check if intermedaite stats must be print before
                    if (iperf_timerbefore(&report->interval_target, &wakeup_time))
                    {
                        iperf_timersub(&report->interval_target, &report->packet_time, &sleep_time);

                        sleep_ms = iperf_timermsec(&sleep_time);
                        if (sleep_ms)
                            rtos_task_suspend(sleep_ms);

                        // fhost_iperf_print_interv_stats expect current time in packet_time
                        iperf_current_time(&report->packet_time);
                        fhost_iperf_print_interv_stats(stream);

                        if (iperf_timerbefore(&report->packet_time, &wakeup_time))
                        {
                            iperf_timersub(&wakeup_time, &report->packet_time, &sleep_time);
                        }
                        else
                        {
                            sleep_time.sec = 0;
                            sleep_time.usec = 0;
                        }
                    }
                }

                sleep_ms = iperf_timermsec(&sleep_time);
                if (sleep_ms)
                    rtos_task_suspend(sleep_ms);

                // Resetting last_sleep_time to wakeup_time (instead of current time) allow
                // to compensate for sleep duration that doesn't always match what is asked.
                last_sleep_time = wakeup_time;
            }
            else
            {
                // Always Update time of last sleep (even if we skipped it)
                iperf_current_time(&last_sleep_time);
            }
        }

        // Update packet time
        iperf_current_time(&report->packet_time);

        // Interval stats
        fhost_iperf_print_interv_stats(stream);

        if (iperf_settings->flags.is_time_mode)
        {
            // Check if test duration is reached
            if (iperf_timerafter(&report->packet_time, &report->end_time))
                done = true;
            else
            {
                struct iperf_time remaining_time;
                iperf_timersub(&report->end_time, &report->packet_time, &remaining_time);
                remaining_time_ms = iperf_timermsec(&remaining_time);
            }
        }
        else
        {
            // Check if requested ammount to data has been transmitted
            if (report->stats.bytes >= iperf_settings->amount)
                done = true;
        }
    }

    fhost_iperf_print_stats(stream, &report->start_time, &report->packet_time, &report->stats);

    err_log = net_iperf_udp_server_report_req(pcb, stream, -packet_id);
    if (err_log != ERR_OK)
        ret = 1;
    else
        ret = 0;

    // Wait until all of pbuf are freed
    while (rtos_semaphore_get_count(stream->send_buf_semaphore) < FHOST_IPERF_SEND_BUF_CNT)
    {
        rtos_semaphore_wait(stream->send_buf_semaphore, -1);
        rtos_semaphore_signal(stream->send_buf_semaphore, false);
    }

cleanup:
    if (pcb) {
        LOCK_TCPIP_CORE();
        udp_remove(pcb);
        UNLOCK_TCPIP_CORE();
    }
    net_iperf_buf_deinit(stream);

exit:
    return ret;
}

int net_iperf_tcp_client_run(struct fhost_iperf_stream* stream)
{
    int ret = -1;
    struct tcp_pcb *pcb = NULL;

    if (net_iperf_buf_init(stream, false)) {
        goto exit;
    }
    pcb = net_iperf_init(stream);
    if (pcb == NULL)
    {
        TRACE_APP(ERR, "NET_IPERF_AL : SEND TRANSC ERR pcb null");
        goto cleanup;
    }

    // Wait for TCP traffic to end
    rtos_semaphore_wait(stream->iperf_task_semaphore, -1);

    ret = 0;

cleanup:
    net_iperf_buf_deinit(stream);
exit:
    return ret;
}

int net_iperf_tcp_server_run(struct fhost_iperf_stream* stream)
{
    struct tcp_pcb *pcb;

    // Create and configure TCP Protocol Control Block
    pcb = net_iperf_init(stream);
    if (pcb == NULL)
    {
        TRACE_APP(ERR, "NET_IPERF_AL : SEND TRANSC ERR pcb null");
        return -1;
    }

    LOCK_TCPIP_CORE();
    // Listening for TCP clients to connect
    pcb = tcp_listen_with_backlog(pcb, 1);
    if (pcb == NULL)
    {
        fhost_print(stream->iperf_handle, "IPERF: Server pcb is NULLs\n");
        tcp_close(pcb);
        UNLOCK_TCPIP_CORE();
        return -1;
    }

    // Register argument for acceptance callback
    tcp_arg(pcb, stream);
    tcp_accept(pcb, net_iperf_tcp_server_accept);
    UNLOCK_TCPIP_CORE();

    // Wait for TCP traffic to end
    rtos_semaphore_wait(stream->iperf_task_semaphore, -1);
    tcp_close(pcb);
    return 0;
}
#endif
