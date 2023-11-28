/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Simon Goldschmidt
 *
 */
#ifndef LWIP_HDR_LWIPOPTS_H__
#define LWIP_HDR_LWIPOPTS_H__

#include "rwnx_config.h"
#include "fhost_rtos.h"
#include "fhost_rx_def.h"
#include "arch/sys_arch.h"

extern uint16_t fhost_ip_chksum(const void *dataptr, int len);
extern sys_mutex_t lock_tcpip_core;
extern int sys_is_inside_interrupt(void);
extern int sys_mutex_is_locked(sys_mutex_t *mutex);
extern int sys_current_is_tcpip(void);

#define LWIP_TCPIP_CORE_LOCKING       1

#if LWIP_TCPIP_CORE_LOCKING
#define LWIP_ASSERT_CORE_LOCKED()       do { if(lock_tcpip_core) { LWIP_ASSERT("api must call with lwip core lock", !sys_is_inside_interrupt() && sys_mutex_is_locked(&lock_tcpip_core)); } } while(0)
#else
#define LWIP_ASSERT_CORE_LOCKED()       do { LWIP_ASSERT("api must call inside lwip task", !sys_is_inside_interrupt() && sys_current_is_tcpip()); } while(0)
#endif

#define LWIP_NETIF_API                1
#define LWIP_DEBUG                    1
#define LWIP_STATS_DISPLAY            1
#define SOCKETS_DEBUG                 LWIP_DBG_OFF
#ifdef BL616_DHCP_DEBUG
#define DHCP_DEBUG                    LWIP_DBG_ON
#else
#define DHCP_DEBUG                    LWIP_DBG_OFF
#endif
#define ICMP_DEBUG                    LWIP_DBG_OFF
#define ETHARP_DEBUG                  LWIP_DBG_OFF
#define LWIP_MULTICAST_PING           1
#define LWIP_BROADCAST_PING           1

#define TCPIP_MBOX_SIZE               64
#define TCPIP_THREAD_STACKSIZE        FHOST_TCPIP_STACK_SIZE
#define TCPIP_THREAD_PRIO             FHOST_TCPIP_PRIORITY

#define DEFAULT_THREAD_STACKSIZE      1024
#define DEFAULT_THREAD_PRIO           1
#define DEFAULT_RAW_RECVMBOX_SIZE     32
#define DEFAULT_UDP_RECVMBOX_SIZE     64
#define DEFAULT_TCP_RECVMBOX_SIZE     64
#define DEFAULT_ACCEPTMBOX_SIZE       32

#define LWIP_NETIF_LOOPBACK           1
#define LWIP_HAVE_LOOPIF              1
#define LWIP_LOOPBACK_MAX_PBUFS       0

#define LWIP_CHKSUM_ALGORITHM         3
#define LWIP_CHKSUM                   fhost_ip_chksum
#define LWIP_TCPIP_CORE_LOCKING_INPUT 1

#define PBUF_LINK_ENCAPSULATION_HLEN  388

#define IP_REASS_MAX_PBUFS            (FHOST_RX_BUF_CNT - 2)

#define MEMP_NUM_NETBUF               32
#define MEMP_NUM_NETCONN              16
#define MEMP_NUM_UDP_PCB              16
#define MEMP_NUM_REASSDATA            LWIP_MIN((IP_REASS_MAX_PBUFS), 5)

#if NX_TXDESC_CNT0 > NX_TXDESC_CNT1
#define MAC_TXQ_DEPTH_0_1             NX_TXDESC_CNT0
#else
#define MAC_TXQ_DEPTH_0_1             NX_TXDESC_CNT1
#endif
#if NX_TXDESC_CNT2 > NX_TXDESC_CNT3
#define MAC_TXQ_DEPTH_2_3             NX_TXDESC_CNT2
#else
#define MAC_TXQ_DEPTH_2_3             NX_TXDESC_CNT3
#endif
#if MAC_TXQ_DEPTH_0_1 > MAC_TXQ_DEPTH_2_3
#define MAC_TXQ_DEPTH                 MAC_TXQ_DEPTH_0_1
#else
#define MAC_TXQ_DEPTH                 MAC_TXQ_DEPTH_2_3
#endif
#if NX_AMPDU_RX
#define MAC_RXQ_DEPTH                 NX_AMPDU_RX_BUF_SIZE
#else
#define MAC_RXQ_DEPTH                 4
#endif

#define TCP_MSS                       1460
#define TCP_WND                       (MAC_RXQ_DEPTH * TCP_MSS)

#if NX_BEACONING
#define TCP_SND_BUF                   (4 * MAC_TXQ_DEPTH * TCP_MSS)
#else
#define TCP_SND_BUF                   (2 * MAC_TXQ_DEPTH * TCP_MSS)
#endif

#define TCP_QUEUE_OOSEQ               1
#define MEMP_NUM_TCP_SEG              ((4 * TCP_SND_BUF) / TCP_MSS)
#define MEMP_NUM_PBUF                 (TCP_SND_BUF / TCP_MSS)
#define PBUF_POOL_SIZE                0
#define LWIP_WND_SCALE                1
#define TCP_RCV_SCALE                 2
#define TCP_SNDLOWAT                  LWIP_MIN(LWIP_MAX(((TCP_SND_BUF)/4),               \
                                                        (2 * TCP_MSS) + 1),              \
                                               (TCP_SND_BUF) - 1)

#define MEM_MIN_TCP                   (2048 + MEMP_NUM_PBUF * (100 + PBUF_LINK_ENCAPSULATION_HLEN + 1514))
#if NX_TG
#define MEM_MIN_TG                    16384
#else
#define MEM_MIN_TG                    0
#endif
#if MEM_MIN_TCP > MEM_MIN_TG
#define MEM_MIN                       MEM_MIN_TCP
#else
#define MEM_MIN                       MEM_MIN_TG
#endif
#define MEM_ALIGNMENT                 4
#if defined(CFG_SDIOWIFI)
#undef MEM_MIN
#ifdef LP_APP
#define MEM_MIN                       (14 * 1024)
#else
#define MEM_MIN                       (48 * 1024)
#endif
#endif

// heap size upper limit
#if MEM_MIN > (64 * 1024)
#undef MEM_MIN
#define MEM_MIN (64 * 1024)
#endif

#ifdef LWIP_HEAP_SIZE
#define MEM_SIZE LWIP_HEAP_SIZE
#else
#if MEM_MIN > 8192
#define MEM_SIZE                      MEM_MIN
#else
#define MEM_SIZE                      8192
#endif
#endif

#define LWIP_HOOK_FILENAME            "lwiphooks.h"

#define LWIP_RAW                      1
#define LWIP_MULTICAST_TX_OPTIONS     1

#define LWIP_TIMEVAL_PRIVATE          0  // use sys/time.h for struct timeval
                                         //
//#define LWIP_PROVIDE_ERRNO            1
#ifndef PLATFORM_PRIVDE_ERRNO
#define LWIP_PROVIDE_ERRNO            1
#endif

#define LWIP_DHCP                     1
#define LWIP_DNS                      1
#define LWIP_IGMP                     0
#define LWIP_SO_RCVTIMEO              1
#define LWIP_SO_SNDTIMEO              1
#define SO_REUSE                      1
#define LWIP_TCP_KEEPALIVE            1

extern int * __errno(void);
#define errno (*__errno())
#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_API 1

#if 0
/* Prevent having to link sys_arch.c (we don't test the API layers in unit tests) */
#define NO_SYS                          0
#define LWIP_NETCONN                    0
#define LWIP_SOCKET                     0
#define SYS_LIGHTWEIGHT_PROT            0

#define LWIP_IPV6                       1
#define IPV6_FRAG_COPYHEADER            1
#define LWIP_IPV6_DUP_DETECT_ATTEMPTS   0

/* Turn off checksum verification of fuzzed data */
#define CHECKSUM_CHECK_IP               0
#define CHECKSUM_CHECK_UDP              0
#define CHECKSUM_CHECK_TCP              0
#define CHECKSUM_CHECK_ICMP             0
#define CHECKSUM_CHECK_ICMP6            0

/* Minimal changes to opt.h required for tcp unit tests: */
#define MEM_SIZE                        16000
#define TCP_SND_QUEUELEN                40
#define MEMP_NUM_TCP_SEG                TCP_SND_QUEUELEN
#define TCP_SND_BUF                     (12 * TCP_MSS)
#define TCP_WND                         (10 * TCP_MSS)
#define LWIP_WND_SCALE                  1
#define TCP_RCV_SCALE                   0
#define PBUF_POOL_SIZE                  400 /* pbuf tests need ~200KByte */

/* Minimal changes to opt.h required for etharp unit tests: */
#define ETHARP_SUPPORT_STATIC_ENTRIES   1
#endif

#define ETHARP_SUPPORT_STATIC_ENTRIES   1

#define LWIP_SUPPORT_CUSTOM_PBUF        1
#ifdef  HIGH_PERFORMANCE_PBUF
#define LWIP_NETIF_TX_SINGLE_PBUF       0
#else
#define LWIP_NETIF_TX_SINGLE_PBUF       1
#endif
#define LWIP_RAND() ((u32_t)random())
#define LWIP_DECLARE_MEMORY_ALIGNED(variable_name, size) u8_t variable_name[size] __ALIGN4 __SHAREDRAM


#endif /* LWIP_HDR_LWIPOPTS_H__ */
