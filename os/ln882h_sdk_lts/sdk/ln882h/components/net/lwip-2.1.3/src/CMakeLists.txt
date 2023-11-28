# COREFILES, CORE4FILES: The minimum set of files needed for lwIP.
set(COREFILES
    core/def.c
    core/dns.c
    core/inet_chksum.c
    core/init.c
    core/ip.c
    core/memp.c
    core/netif.c
    core/pbuf.c
    core/raw.c
    core/stats.c
    core/sys.c
    core/tcp_in.c
    core/tcp_out.c
    core/tcp.c
    core/timeouts.c
    core/udp.c
    # core/mem.c
)

set(CORE4FILES
    core/ipv4/autoip.c
    core/ipv4/dhcp.c
    core/ipv4/etharp.c
    core/ipv4/icmp.c
    core/ipv4/igmp.c
    core/ipv4/ip4_addr.c
    core/ipv4/ip4_frag.c
    core/ipv4/ip4.c
)

set(CORE6FILES
    core/ipv6/dhcp6.c
    core/ipv6/ethip6.c
    core/ipv6/icmp6.c
    core/ipv6/inet6.c
    core/ipv6/ip6_addr.c
    core/ipv6/ip6_frag.c
    core/ipv6/ip6.c
    core/ipv6/mld6.c
    core/ipv6/nd6.c
)

# APIFILES: The files which implement the sequential and socket APIs.
set(APIFILES
    api/api_lib.c
    api/api_msg.c
    api/err.c
    api/netbuf.c
    api/netdb.c
    api/netifapi.c
    api/sockets.c
    api/tcpip.c
)

# NETIFFILES: Files implementing various generic network interface functions
set(NETIFFILES
    netif/ethernet.c
)

# SIXLOWPAN: 6LoWPAN
set(SIXLOWPAN
    netif/lowpan6.c
)

# PPPFILES: PPP
set(PPPFILES
    netif/ppp/polarssl/arc4.c
    netif/ppp/polarssl/des.c
    netif/ppp/polarssl/md4.c
    netif/ppp/polarssl/md5.c
    netif/ppp/polarssl/sha1.c
    netif/ppp/auth.c
    netif/ppp/ccp.c
    netif/ppp/chap_ms.c
    netif/ppp/chap-md5.c
    netif/ppp/chap-new.c
    netif/ppp/demand.c
    netif/ppp/eap.c
    netif/ppp/ecp.c
    netif/ppp/eui64.c
    netif/ppp/fsm.c
    netif/ppp/ipcp.c
    netif/ppp/ipv6cp.c
    netif/ppp/lcp.c
    netif/ppp/magic.c
    netif/ppp/mppe.c
    netif/ppp/multilink.c
    netif/ppp/ppp.c
    netif/ppp/pppapi.c
    netif/ppp/pppcrypt.c
    netif/ppp/pppoe.c
    netif/ppp/pppol2tp.c
    netif/ppp/pppos.c
    netif/ppp/upap.c
    netif/ppp/utils.c
    netif/ppp/vj.c
)

# LWIPNOAPPSFILES: All LWIP files without apps
set(LWIPNOAPPSFILES
    ${COREFILES}
    ${CORE4FILES}
    ${CORE6FILES}
    ${APIFILES}
    ${NETIFFILES}
    ${PPPFILES}
    ${SIXLOWPAN}
)

# HTTPDFILES: HTTP server
set(HTTPDFILES
    apps/httpd/fs.c
    apps/httpd/fsdata.c
    apps/httpd/httpd.c
)

# LWIPERFFILES: IPERF server
set(LWIPERFFILES
    apps/lwiperf/lwiperf.c
)

# MDNSFILES: MDNS responder
set(MDNSFILES
    apps/mdns/mdns.c
)

# MQTTFILES: MQTT client files
set(MQTTFILES
    apps/mqtt/mqtt.c
)

# NETBIOSNSFILES: NetBIOS name server
set(NETBIOSNSFILES
    apps/netbiosns/netbiosns.c
)

# SNMPFILES: SNMPv2c agent
set(SNMPFILES
    apps/snmp/snmp_asn1.c
    apps/snmp/snmp_core.c
    apps/snmp/snmp_mib2_icmp.c
    apps/snmp/snmp_mib2_interfaces.c
    apps/snmp/snmp_mib2_ip.c
    apps/snmp/snmp_mib2_snmp.c
    apps/snmp/snmp_mib2_system.c
    apps/snmp/snmp_mib2_tcp.c
    apps/snmp/snmp_mib2_udp.c
    apps/snmp/snmp_mib2.c
    apps/snmp/snmp_msg.c
    apps/snmp/snmp_netconn.c
    apps/snmp/snmp_pbuf_stream.c
    apps/snmp/snmp_raw.c
    apps/snmp/snmp_scalar.c
    apps/snmp/snmp_table.c
    apps/snmp/snmp_threadsync.c
    apps/snmp/snmp_traps.c
    apps/snmp/snmpv3_dummy.c
    apps/snmp/snmpv3_mbedtls.c
    apps/snmp/snmpv3.c
)

# SNTPFILES: SNTP client
set(SNTPFILES
    apps/sntp/sntp.c
)

# TFTPFILES: TFTP server files
set(TFTPFILES
    apps/tftp/tftp_server.c
)

# LWIPAPPFILES: All LWIP APPs
set(LWIPAPPFILES
    ${SNMPFILES}
    ${HTTPDFILES}
    ${LWIPERFFILES}
    ${SNTPFILES}
    ${MDNSFILES}
    ${NETBIOSNSFILES}
    ${TFTPFILES}
    ${MQTTFILES}
)

set(LWIP_SRC_C
    ${COREFILES}
    ${CORE4FILES}
    ${APIFILES}
    ${NETIFFILES}
    port/ln_osal/netif/ethernetif.c
    port/ln_osal/arch/sys_arch.c
)

set(lwip_static_target "lwip")
add_library(${lwip_static_target} STATIC ${LWIP_SRC_C})

if(NOT DEFINED PROJECT_CFG_DIR)
    message(STATUS "PROJECT_CFG_DIR is not defined, define now!")
    set(PROJECT_CFG_DIR ${LN_SDK_ROOT}/project/${USER_PROJECT}/cfg)
endif()

target_include_directories(${lwip_static_target}
    PUBLIC
    ${LN_SDK_ROOT}/project/${USER_PROJECT}/bsp
    ${PROJECT_CFG_DIR}
    ${LN_SDK_ROOT}/components
    ${COMP_WIFI_DIR}/wifi_lib_export
    ${MCU_LN882X_DIR}/${CHIP_SERIAL}
    ${MCU_LN882X_DIR}/CMSIS_5.3.0
    include
    include/lwip
    include/lwip/prot
    include/lwip/priv
    include/netif
    include/posix
    include/posix/sys
    port/ln_osal/include
    ${COMP_DHCPD_DIR}
    ${COMP_WIFI_DIR}/wifi_manager
    ${COMP_KERNEL_DIR}
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/include
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/portable/GCC/ARM_CM4F
)
