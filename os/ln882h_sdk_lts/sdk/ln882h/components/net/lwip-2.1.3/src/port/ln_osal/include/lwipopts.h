#ifndef __LWIP_CFG_H__
#define __LWIP_CFG_H__

#include <sys/time.h>

#define LWIP_MDNS_RESPONDER    (1)
#define LWIP_NUM_NETIF_CLIENT_DATA  (1)

#define LWIP_TIMEVAL_PRIVATE    0

/* ---------- User options ---------- */
#define LWIP_USE_LN_USER_CHANGE    1
/* ---------- User options end ------ */

#define ETHARP_TRUST_IP_MAC        0
#define IP_REASSEMBLY              1
#define IP_FRAG                    1
#define LWIP_NETIF_TX_SINGLE_PBUF  1

#define ARP_QUEUEING               0
#define TCP_LISTEN_BACKLOG         1
#define LWIP_COMPAT_MUTEX          0
#define LWIP_COMPAT_MUTEX_ALLOWED  1

#define LWIP_TCPIP_TIMEOUT         1

/**
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,use lwIP facilities.
 */
#define NO_SYS                  (0)

#define NETIF_MTU               (1500)


/* ---------- Memory options ---------- */
#define MEM_LIBC_MALLOC                     1

/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           (4)

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                (5 * 1024)


/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this should be set high. */
#define MEMP_NUM_PBUF           (50)

/* MEMP_NUM_NETCONN: the number of struct netconns. (only needed if you use the sequential API, like api_lib.c) */
#define MEMP_NUM_NETCONN        (MEMP_NUM_UDP_PCB + MEMP_NUM_TCP_PCB + MEMP_NUM_TCP_PCB_LISTEN)
#define MEMP_NUM_NETBUF         (8)
/* the number of UDP protocol control blocks. One per active RAW "connection". */
#define MEMP_NUM_RAW_PCB        (2)
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        (4)
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP connections. */
#define MEMP_NUM_TCP_PCB        (8)
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP connections. */
#define MEMP_NUM_TCP_PCB_LISTEN (3)
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP segments. */
#define MEMP_NUM_TCP_SEG        TCP_SND_QUEUELEN
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    (10)

#define MEMP_NUM_TCPIP_MSG_INPKT (12)
#define MEMP_NUM_TCPIP_MSG_API   (8)

#define PBUF_LINK_HLEN                  (14 + ETH_PAD_SIZE)

/* ---------- Pbuf options ---------- */
#define IP_REASS_MAX_PBUFS      (10)
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE          14//(2*IP_REASS_MAX_PBUFS) // 40
/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       772 //(NETIF_MTU + PBUF_LINK_HLEN)

#if PBUF_POOL_BUFSIZE > (TCP_MSS+PBUF_IP_HLEN+PBUF_TRANSPORT_HLEN+PBUF_LINK_ENCAPSULATION_HLEN+PBUF_LINK_HLEN)
#undef PBUF_POOL_BUFSIZE
#define PBUF_POOL_BUFSIZE       (TCP_MSS+PBUF_IP_HLEN+PBUF_TRANSPORT_HLEN+PBUF_LINK_ENCAPSULATION_HLEN+PBUF_LINK_HLEN)
#endif

/* ---------- TCP options ---------- */
#define LWIP_TCP                (1)
#define TCP_TTL                 (255)
/* Controls if TCP should queue segments that arrive out of order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         (1)
/* TCP Maximum segment size. */
#define TCP_MSS                 (NETIF_MTU - 40)  //TCP_MSS = (Ethernet MTU - IP header size - TCP header size)
/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             (6 * TCP_MSS)//(7 * TCP_MSS)
/*  TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work. */
#define TCP_SND_QUEUELEN        (4 * TCP_SND_BUF/TCP_MSS)//(5 * TCP_SND_BUF/TCP_MSS)
/* TCP receive window. */
#define TCP_WND                 (3 * TCP_MSS)
/* Maximum number of retransmissions of data segments. */
#define TCP_MAXRTX              (12)
/* Maximum number of retransmissions of SYN segments. */
#define TCP_SYNMAXRTX           (4)

/* ---------- ICMP options ---------- */
#define LWIP_ICMP               1

/* ---------- IGMP options ---------- */
#define LWIP_IGMP               1
#define LWIP_NETIF_LOOPBACK     1
#define LWIP_LOOPIF_MULTICAST   1

/* ---------- DHCP options ---------- */
#define LWIP_DHCP               1
#define DHCP_CREATE_RAND_XID    1


#define LWIP_IPV4               1
#define LWIP_IPV6               0


/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255

/* -----Support netif api------------*/
#define LWIP_NETIF_API          1

/* ---------- Statistics options ---------- */
#define LWIP_STATS              0
#define LWIP_STATS_DISPLAY      0

/* ---------- link callback options ---------- */
#define LWIP_NETIF_LINK_CALLBACK        1
#define LWIP_NETIF_REMOVE_CALLBACK      1
#define LWIP_NETIF_STATUS_CALLBACK      1 //set ip_addr/netif_set_up/down


//#define LWIP_NETCONN_FULLDUPLEX         1
#define LWIP_TCP_KEEPALIVE              1
#define SO_REUSE                        1


/*---------- Sequential layer options ----------*/
#define LWIP_NETCONN                    1

/*---------- Socket options ----------*/
#define LWIP_SOCKET                     1
#define LWIP_RAW                        1
#define LWIP_SO_SNDTIMEO                1
#define LWIP_SO_RCVTIMEO                1
#define LWIP_SO_LINGER                  1

/* ----------  DNS options  ----------*/
#define LWIP_DNS                        1

/*---------- DEBUG options ----------*/
#define LWIP_DEBUG                      LWIP_DBG_OFF
#define PBUF_DEBUG                      LWIP_DBG_OFF

#if !(LWIP_DEBUG)
#define LWIP_NOASSERT
#endif

// Debug Options
#define NETIF_DEBUG                 LWIP_DBG_OFF
#define PBUF_DEBUG                  LWIP_DBG_OFF
#define API_LIB_DEBUG               LWIP_DBG_OFF
#define API_MSG_DEBUG               LWIP_DBG_OFF
#define SOCKETS_DEBUG               LWIP_DBG_OFF
#define ICMP_DEBUG                  LWIP_DBG_OFF
#define IGMP_DEBUG                  LWIP_DBG_OFF
#define INET_DEBUG                  LWIP_DBG_OFF
#define IP_DEBUG                    LWIP_DBG_OFF
#define IP_REASS_DEBUG              LWIP_DBG_OFF
#define RAW_DEBUG                   LWIP_DBG_OFF
#define MEM_DEBUG                   LWIP_DBG_OFF
#define MEMP_DEBUG                  LWIP_DBG_OFF
#define SYS_DEBUG                   LWIP_DBG_OFF
#define TIMERS_DEBUG                LWIP_DBG_OFF
#define TCP_DEBUG                   LWIP_DBG_OFF
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF
#define TCP_FR_DEBUG                LWIP_DBG_OFF
#define TCP_RTO_DEBUG               LWIP_DBG_OFF
#define TCP_CWND_DEBUG              LWIP_DBG_OFF
#define TCP_WND_DEBUG               LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF
#define TCP_RST_DEBUG               LWIP_DBG_OFF
#define TCP_QLEN_DEBUG              LWIP_DBG_OFF
#define UDP_DEBUG                   LWIP_DBG_OFF
#define TCPIP_DEBUG                 LWIP_DBG_OFF
#define SLIP_DEBUG                  LWIP_DBG_OFF
#define DHCP_DEBUG                  LWIP_DBG_OFF
#define AUTOIP_DEBUG                LWIP_DBG_OFF
#define DNS_DEBUG                   LWIP_DBG_OFF
#define IP6_DEBUG                   LWIP_DBG_OFF

#define ETHARP_DEBUG                LWIP_DBG_OFF

/*----------- OS options ------------*/
#define TCPIP_THREAD_NAME              "TCP/IP"
#define TCPIP_THREAD_PRIO               OS_PRIORITY_ABOVE_NORMAL
#define TCPIP_THREAD_STACKSIZE          (3*1024)

#define TCPIP_MBOX_SIZE                 (MEMP_NUM_TCPIP_MSG_INPKT + MEMP_NUM_TCPIP_MSG_API)
#define DEFAULT_RAW_RECVMBOX_SIZE       (MEMP_NUM_NETBUF)
#define DEFAULT_UDP_RECVMBOX_SIZE       (MEMP_NUM_NETBUF)
#define DEFAULT_TCP_RECVMBOX_SIZE       (MEMP_NUM_NETBUF)
#define DEFAULT_ACCEPTMBOX_SIZE         4

#define DEFAULT_THREAD_STACKSIZE        500

/*----------- Core locking -----------*/
#define LWIP_MPU_COMPATIBLE             0
#define LWIP_TCPIP_CORE_LOCKING         1
#define LWIP_TCPIP_CORE_LOCKING_INPUT   0
#define SYS_LIGHTWEIGHT_PROT            (NO_SYS == 0)


/* ---------- Network Interfaces options ----------*/
#define LWIP_NETIF_HOSTNAME             1

#define LWIP_MBOX_TRACE                 0
#define LWIP_MBOX_DEBUG                 0
#define LWIP_RESOURCE_TRACE             0

/* ---------- Checksum options ----------*/
#define LWIP_CHKSUM_ALGORITHM           3

//#define CHECKSUM_BY_HARDWARE

#ifdef CHECKSUM_BY_HARDWARE
  /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 0
  /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                0
  /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                0
  /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               0
  /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              0
  /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              0
  /* CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               0
#else
  /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 1
  /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                1
  /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                1
  /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               1
  /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              1
  /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              1
  /* CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               1
#endif

#endif /* __LWIP_CFG_H__ */

