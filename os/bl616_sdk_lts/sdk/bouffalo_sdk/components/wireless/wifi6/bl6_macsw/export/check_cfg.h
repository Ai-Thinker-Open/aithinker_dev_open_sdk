/* Automatically generated, please do not modify! */
#ifndef _CHECK_MACSW_CFG_
#define _CHECK_MACSW_CFG_
#ifndef CFG_AGG
 #error "Missing CFG_AGG Param."
#endif
#ifndef CFG_AMSDU_4K
 #error "Missing CFG_AMSDU_4K Param."
#endif
#ifdef CFG_BARX
 #if CFG_BARX != 2
  #error "CFG_BARX not compatible"
 #endif
#else
 #error "Missing CFG_BARX Param."
#endif
#ifdef CFG_BATX
 #if CFG_BATX != 1
  #error "CFG_BATX not compatible"
 #endif
#else
 #error "Missing CFG_BATX Param."
#endif
#ifndef CFG_BCN
 #error "Missing CFG_BCN Param."
#endif
#ifndef CFG_BWLEN
 #error "Missing CFG_BWLEN Param."
#endif
#ifndef CFG_COEX
 #error "Missing CFG_COEX Param."
#endif
#ifndef CFG_CRYPTO
 #error "Missing CFG_CRYPTO Param."
#endif
#ifndef CFG_FHOST
 #error "Missing CFG_FHOST Param."
#endif
#ifndef CFG_FHOST_MONITOR
 #error "Missing CFG_FHOST_MONITOR Param."
#endif
#ifndef CFG_FTM_INIT
 #error "Missing CFG_FTM_INIT Param."
#endif
#ifndef CFG_FTM_RSP
 #error "Missing CFG_FTM_RSP Param."
#endif
#ifndef CFG_HE
 #error "Missing CFG_HE Param."
#endif
#ifdef CFG_HSU
 #if CFG_HSU != 0
  #error "CFG_HSU not compatible"
 #endif
#else
 #error "Missing CFG_HSU Param."
#endif
#ifndef CFG_IPERF
 #error "Missing CFG_IPERF Param."
#endif
#ifndef CFG_KEYCFG
 #error "Missing CFG_KEYCFG Param."
#endif
#ifndef CFG_MAC_VER_V21
 #error "Missing CFG_MAC_VER_V21 Param."
#endif
#ifndef CFG_MDM_VER_V30
 #error "Missing CFG_MDM_VER_V30 Param."
#endif
#ifndef CFG_MFP
 #error "Missing CFG_MFP Param."
#endif
#ifdef CFG_MU_CNT
 #if CFG_MU_CNT != 1
  #error "CFG_MU_CNT not compatible"
 #endif
#else
 #error "Missing CFG_MU_CNT Param."
#endif
#ifdef CFG_P2P
 #if CFG_P2P != 0
  #error "CFG_P2P not compatible"
 #endif
#else
 #error "Missing CFG_P2P Param."
#endif
#ifndef CFG_P2P_GO
 #error "Missing CFG_P2P_GO Param."
#endif
#ifndef CFG_PLF_VER_V30
 #error "Missing CFG_PLF_VER_V30 Param."
#endif
#ifndef CFG_PS
 #error "Missing CFG_PS Param."
#endif
#ifdef CFG_REORD_BUF
 #if CFG_REORD_BUF != 12
  #error "CFG_REORD_BUF not compatible"
 #endif
#else
 #error "Missing CFG_REORD_BUF Param."
#endif
#ifndef CFG_SMARTCONFIG
 #error "Missing CFG_SMARTCONFIG Param."
#endif
#ifdef CFG_SPC
 #if CFG_SPC != 16
  #error "CFG_SPC not compatible"
 #endif
#else
 #error "Missing CFG_SPC Param."
#endif
#ifdef CFG_STA_MAX
 #if CFG_STA_MAX != 4
  #error "CFG_STA_MAX not compatible"
 #endif
#else
 #error "Missing CFG_STA_MAX Param."
#endif
#ifndef CFG_STATS
 #error "Missing CFG_STATS Param."
#endif
#ifdef CFG_TXDESC1
 #if CFG_TXDESC1 != 32
  #error "CFG_TXDESC1 not compatible"
 #endif
#else
 #error "Missing CFG_TXDESC1 Param."
#endif
#ifdef CFG_TXDESC4
 #if CFG_TXDESC4 != 4
  #error "CFG_TXDESC4 not compatible"
 #endif
#else
 #error "Missing CFG_TXDESC4 Param."
#endif
#ifndef CFG_UF
 #error "Missing CFG_UF Param."
#endif
#ifndef CFG_UMAC
 #error "Missing CFG_UMAC Param."
#endif
#ifdef CFG_VIF_MAX
 #if CFG_VIF_MAX != 2
  #error "CFG_VIF_MAX not compatible"
 #endif
#else
 #error "Missing CFG_VIF_MAX Param."
#endif
#endif