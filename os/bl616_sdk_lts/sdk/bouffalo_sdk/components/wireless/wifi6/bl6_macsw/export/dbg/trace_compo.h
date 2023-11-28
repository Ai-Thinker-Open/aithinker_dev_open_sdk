/**
 ******************************************************************************
 * @file trace_compo.h
 *
 * @brief Declaration for components specific trace level.
 *
 * Copyright (C) RivieraWaves 2011-2021
 *
 ******************************************************************************
 */

#ifndef _TRACE_COMPO_H_
#define _TRACE_COMPO_H_

/**
 ******************************************************************************
 * @addtogroup TRACE
 * @details
 *
 * @b Filterable @b Components: \n
 *
 * To define a new filterable component "compo", in file trace_compo.h: \n
 * - Defined a new value @b TRACE_COMPO_<COMPO> in the @ref trace_compo enum.
 *
 * - set its default trace level in @ref trace_compo_level, initialized in
 * @ref trace_init function.
 *
 * - Defined a new enum name trace_level_<compo> with filters specific to this
 * component.\n
 * All name must start with the @b TRACE_LVL_<COMPO> prefix.\n
 * Although not mandatory, it is good practice to define the first filter as
 * error (so that enabling level 1 on all components enable all error traces).\n
 * It is also good practice to add a small comment to describe which kind of
 * traces are controlled by this filter.
 *
 * - Defined specific macro TRACE_<COMPO> that call the generic @ref
 * TRACE_FILT macro. (You could directly use TRACE_FILT in the code but don't
 * do that)\n
 * Defining a specific macro: \n
 * - Makes the code more compact (compared to calling TRACE_FILT directly).
 * - Allows to easily remove all traces from one component at compilation,
 * usually using a TRACE_<COMPO>_EN macro.
 *
 * @note trace_compo.h is parsed during compilation to extract list of components
 * and filters. To avoid parsing issue try to re-use the same format as other
 * component when adding a new one.
 * @{
 ******************************************************************************
 */

/**
 ******************************************************************************
 *  Filterable trace component
 ******************************************************************************
 */
enum trace_compo
{
    TRACE_COMPO_KERNEL,
    TRACE_COMPO_LMAC,
    TRACE_COMPO_CHAN,
    TRACE_COMPO_STA,
    TRACE_COMPO_AP,
    TRACE_COMPO_P2P,
    TRACE_COMPO_MESH,
    TRACE_COMPO_TDLS,
    TRACE_COMPO_RM,
    TRACE_COMPO_RTOS,
    TRACE_COMPO_FHOST,
    TRACE_COMPO_APP,
    TRACE_COMPO_TWT,
    TRACE_COMPO_FTM,
    /* keep it last */
    TRACE_COMPO_MAX,
};

/// Current trace filters status
extern uint32_t trace_compo_level[TRACE_COMPO_MAX];

/// Macro defining whether a trace component should be compiled or not
#define TRACE_COMPO(compo) NX_TRACE && TRACE_##compo##_EN

/**
 *****************************************************************************
 * Specific filter for KERNEL component
 *****************************************************************************
 */
enum trace_level_kernel
{
    /** Error */
    TRACE_LVL_KERNEL_ERR,
    /** Messages to API layer */
    TRACE_LVL_KERNEL_MSG_API,
    /** Messages from API layer or internal task */
    TRACE_LVL_KERNEL_MSG_LOCAL,
    /** State changes */
    TRACE_LVL_KERNEL_STATE,
};

/** Default trace level for KERNEL component */
#define TRACE_KERNEL_DEFAULT (CO_BIT(TRACE_LVL_KERNEL_ERR) |            \
                              CO_BIT(TRACE_LVL_KERNEL_MSG_API) |        \
                              CO_BIT(TRACE_LVL_KERNEL_MSG_LOCAL))

/** Define whether to include or not KERNEL traces */
#define TRACE_KERNEL_EN 1
#if TRACE_COMPO(KERNEL)
/** add trace point for KERNEL component */
#define TRACE_KERNEL(lvl,...) TRACE_FILT(KERNEL, KERNEL_##lvl, __VA_ARGS__ )
/** add buffer trace point for KERNEL component */
#define TRACE_BUF_KERNEL(lvl,...) TRACE_BUF_FILT(KERNEL, KERNEL_##lvl, __VA_ARGS__ )
/** Test whether KERNEL level is currently enabled or not */
#define TRACE_KERNEL_ON(lvl) TRACE_FILT_ON(KERNEL, KERNEL_##lvl)
#else
#define TRACE_KERNEL(lvl,...)
#define TRACE_BUF_KERNEL(lvl,...)
#define TRACE_KERNEL_ON(lvl) 0
#endif /* TRACE_COMPO(KERNEL) */

/**
 *****************************************************************************
 * Specific filter for LMAC
 *****************************************************************************
 */
enum trace_level_lmac
{
    /** Error */
    TRACE_LVL_LMAC_ERR,
    /** Trace all frame sent by LMAC (except beacons) */
    TRACE_LVL_LMAC_TX,
    /** Trace frame received by LMAC from known STA*/
    TRACE_LVL_LMAC_RX,
    /** Trace frame received by LMAC from unknown STA*/
    TRACE_LVL_LMAC_RX_ALL,
    /** Trace when MACHW enter/exit doze mode */
    TRACE_LVL_LMAC_DOZE,
    /** Trace when a block agreement is created/deleted */
    TRACE_LVL_LMAC_BA,
    /** Trace TX power configuration*/
    TRACE_LVL_LMAC_TPC,
    /** Trace Traffic Detection general status */
    TRACE_LVL_LMAC_TD,
    /** Trace Traffic Detection Debug messages*/
    TRACE_LVL_LMAC_TD_DBG,
    /** Trace frame sent by LMAC with TB */
    TRACE_LVL_LMAC_TX_TB,
};

/** Default trace level for LMAC component */
#define TRACE_LMAC_DEFAULT (CO_BIT(TRACE_LVL_LMAC_ERR))

/** Define whether to include or not LMAC traces */
#define TRACE_LMAC_EN 1
#if TRACE_COMPO(LMAC)
/** add trace point for LMAC component */
#define TRACE_LMAC(lvl,...) TRACE_FILT(LMAC, LMAC_##lvl, __VA_ARGS__ )
/** add buffer trace point for LMAC component */
#define TRACE_BUF_LMAC(lvl,...) TRACE_BUF_FILT(LMAC, LMAC_##lvl, __VA_ARGS__ )
/** Test whether LMAC level is currently enabled or not */
#define TRACE_LMAC_ON(lvl) TRACE_FILT_ON(LMAC, LMAC_##lvl)
#else
#define TRACE_LMAC(lvl,...)
#define TRACE_BUF_LMAC(lvl,...)
#define TRACE_LMAC_ON(lvl) 0
#endif /* TRACE_COMPO(LMAC) */

/**
 *****************************************************************************
 * Specific filter for Channel context
 *****************************************************************************
 */
enum trace_level_chan
{
    /** Error */
    TRACE_LVL_CHAN_ERR,
    /** Info (actual channel switch)*/
    TRACE_LVL_CHAN_INF,
    /* create/update/destroy and Link/unlink */
    TRACE_LVL_CHAN_CREATE,
    /** Channel Switch progress */
    TRACE_LVL_CHAN_SWITCH,
    /** Ctxt status update */
    TRACE_LVL_CHAN_STATUS,
    /** Connection-less (SCAN, ROC) channel context */
    TRACE_LVL_CHAN_CON_LESS,
    /** Action related to Notice of Absence */
    TRACE_LVL_CHAN_NOA,
    /** Handling of TBTT for various vifs */
    TRACE_LVL_CHAN_TBTT,
};

/** Default trace level for CHAN component */
#define TRACE_CHAN_DEFAULT (CO_BIT(TRACE_LVL_CHAN_ERR) | \
                            CO_BIT(TRACE_LVL_CHAN_INF))

/** Define whether to include or not CHAN traces */
#define TRACE_CHAN_EN 1
#if TRACE_COMPO(CHAN)
/** add trace point for CHAN component */
#define TRACE_CHAN(lvl,...) TRACE_FILT(CHAN, CHAN_##lvl, __VA_ARGS__ )
/** add buffer trace point for CHAN component */
#define TRACE_BUF_CHAN(lvl,...) TRACE_BUF_FILT(CHAN, CHAN_##lvl, __VA_ARGS__ )
/** Test whether CHAN level is currently enabled or not */
#define TRACE_CHAN_ON(lvl) TRACE_FILT_ON(CHAN, CHAN_##lvl)
#else
#define TRACE_CHAN(lvl,...)
#define TRACE_BUF_CHAN(lvl,...)
#define TRACE_CHAN_ON(lvl) 0
#endif /* TRACE_COMPO(CHAN) */

/**
 *****************************************************************************
 * Specific filter for STA component
 *****************************************************************************
 */
enum trace_level_sta
{
    /** Error */
    TRACE_LVL_STA_ERR,
    /** Beacon received on STA interfaces */
    TRACE_LVL_STA_BCN,
    /** Power Save status of STA interfaces */
    TRACE_LVL_STA_PS,
    /** Connection/disconnection of STA interfaces */
    TRACE_LVL_STA_CONNECT
};

/** Default trace level for STA component */
#define TRACE_STA_DEFAULT (CO_BIT(TRACE_LVL_STA_ERR))

/** Define whether to include or not STA traces */
#define TRACE_STA_EN 1
#if TRACE_COMPO(STA)
/** add trace point for STA component */
#define TRACE_STA(lvl,...) TRACE_FILT(STA, STA_##lvl, __VA_ARGS__ )
/** add buffer trace point for STA component */
#define TRACE_BUF_STA(lvl,...) TRACE_BUF_FILT(STA, STA_##lvl, __VA_ARGS__ )
/** Test whether STA level is currently enabled or not */
#define TRACE_STA_ON(lvl) TRACE_FILT_ON(STA, STA_##lvl)
#else
#define TRACE_STA(lvl,...)
#define TRACE_BUF_STA(lvl,...)
#define TRACE_STA_ON(lvl) 0
#endif /* TRACE_COMPO(STA) */

/**
 *****************************************************************************
 * Specific filter for AP component
 *****************************************************************************
 */
enum trace_level_ap
{
    /** Error */
    TRACE_LVL_AP_ERR,
    /** Beacon transmitted on AP interfaces */
    TRACE_LVL_AP_BCN,
    /** PS traffic requested/sent by/to connected STA */
    TRACE_LVL_AP_PS,
};

/** Default trace level for AP component */
#define TRACE_AP_DEFAULT (CO_BIT(TRACE_LVL_AP_ERR))

/** Define whether to include or not AP traces */
#define TRACE_AP_EN 1
#if TRACE_COMPO(AP)
/** add trace point for AP component */
#define TRACE_AP(lvl,...) TRACE_FILT(AP, AP_##lvl, __VA_ARGS__ )
/** add buffer trace point for AP component */
#define TRACE_BUF_AP(lvl,...) TRACE_BUF_FILT(AP, AP_##lvl, __VA_ARGS__ )
/** Test whether AP level is currently enabled or not */
#define TRACE_AP_ON(lvl) TRACE_FILT_ON(AP, AP_##lvl)
#else
#define TRACE_AP(lvl,...)
#define TRACE_BUF_AP(lvl,...)
#define TRACE_AP_ON(lvl) 0
#endif /* TRACE_COMPO(AP) */

/**
 *****************************************************************************
 * Specific filter for P2P component
 *****************************************************************************
 */
enum trace_level_p2p
{
    /** Error */
    TRACE_LVL_P2P_ERR,
    /** creation/deletion/activation of P2P interfaces */
    TRACE_LVL_P2P_ITF,
    /** Opportunistic PS information */
    TRACE_LVL_P2P_PS,
    /** Notice of Absence */
    TRACE_LVL_P2P_NOA,
    /** Notice of Absence details*/
    TRACE_LVL_P2P_NOA_DET,
    /** TBTT */
    TRACE_LVL_P2P_TBTT,
};

/** Default trace level for P2P component */
#define TRACE_P2P_DEFAULT (CO_BIT(TRACE_LVL_P2P_ERR))

/** Define whether to include or not P2P traces */
#define TRACE_P2P_EN 1
#if TRACE_COMPO(P2P)
/** add trace point for P2P component */
#define TRACE_P2P(lvl,...) TRACE_FILT(P2P, P2P_##lvl, __VA_ARGS__ )
/** add buffer trace point for P2P component */
#define TRACE_BUF_P2P(lvl,...) TRACE_BUF_FILT(P2P, P2P_##lvl, __VA_ARGS__ )
/** Test whether P2P level is currently enabled or not */
#define TRACE_P2P_ON(lvl) TRACE_FILT_ON(P2P, P2P_##lvl)
#else
#define TRACE_P2P(lvl,...)
#define TRACE_BUF_P2P(lvl,...)
#define TRACE_P2P_ON(lvl) 0
#endif /* TRACE_COMPO(P2P) */

/**
 *****************************************************************************
 * Specific filter for MESH component
 *****************************************************************************
 */
enum trace_level_mesh
{
    /** Error */
    TRACE_LVL_MESH_ERR,
    /** General status */
    TRACE_LVL_MESH_INF,
    /** Transition of power save mode */
    TRACE_LVL_MESH_PS,
    /** Start/end of Service period */
    TRACE_LVL_MESH_SP,
    /** Start/end of Awake window (local and peer) */
    TRACE_LVL_MESH_AWIN,
    /** When a VIF/PEER prevent to enter sleep mode */
    TRACE_LVL_MESH_SLEEP,
    /** Received frames from peers */
    TRACE_LVL_MESH_RX,
    /** Received beacons */
    TRACE_LVL_MESH_BCN_RX,
    /** Transmitted beacons */
    TRACE_LVL_MESH_BCN_TX,
    /** Postponed traffic (because of Power Save) */
    TRACE_LVL_MESH_POST_TRAFFIC,
    /** Frame send/receiving for Hybrid Wireless Mesh Protocol */
    TRACE_LVL_MESH_HWMP,
    /** Detailed information about mesh path */
    TRACE_LVL_MESH_MPATH,
    /** Mesh Peering Management */
    TRACE_LVL_MESH_MPM,
    /** TBTT of local mesh interfaces and connected peers */
    TRACE_LVL_MESH_TBTT,
};

/** Default trace level for MESH component */
#define TRACE_MESH_DEFAULT (CO_BIT(TRACE_LVL_MESH_ERR)| \
                            CO_BIT(TRACE_LVL_MESH_INF))

/** Define whether to include or not MESH traces */
#define TRACE_MESH_EN 1
#if TRACE_COMPO(MESH)
/** add trace point for MESH component */
#define TRACE_MESH(lvl,...) TRACE_FILT(MESH, MESH_##lvl, __VA_ARGS__ )
/** add buffer trace point for MESH component */
#define TRACE_BUF_MESH(lvl,...) TRACE_BUF_FILT(MESH, MESH_##lvl, __VA_ARGS__ )
/** Test whether MESH level is currently enabled or not */
#define TRACE_MESH_ON(lvl) TRACE_FILT_ON(MESH, MESH_##lvl)
#else
#define TRACE_MESH(lvl,...)
#define TRACE_BUF_MESH(lvl,...)
#define TRACE_MESH_ON(lvl) 0
#endif /* TRACE_COMPO(MESH) */

/**
 *****************************************************************************
 * Specific filter for TDLS component
 *****************************************************************************
 */
enum trace_level_tdls
{
    /** Error */
    TRACE_LVL_TDLS_ERR,
    /** TDLS channel Switch progress  */
    TRACE_LVL_TDLS_CHSWITCH,

};

/** Default trace level for TDLS component */
#define TRACE_TDLS_DEFAULT (CO_BIT(TRACE_LVL_TDLS_ERR)| \
                            CO_BIT(TRACE_LVL_TDLS_CHSWITCH))

/** Define whether to include or not TDLS traces */
#define TRACE_TDLS_EN 1
#if TRACE_COMPO(TDLS)
/** add trace point for TDLS component */
#define TRACE_TDLS(lvl,...) TRACE_FILT(TDLS, TDLS_##lvl, __VA_ARGS__ )
/** add buffer trace point for TDLS component */
#define TRACE_BUF_TDLS(lvl,...) TRACE_BUF_FILT(TDLS, TDLS_##lvl, __VA_ARGS__ )
/** Test whether TDLS level is currently enabled or not */
#define TRACE_TDLS_ON(lvl) TRACE_FILT_ON(TDLS, TDLS_##lvl)
#else
#define TRACE_TDLS(lvl,...)
#define TRACE_BUF_TDLS(lvl,...)
#define TRACE_TDLS_ON(lvl) 0
#endif /* TRACE_COMPO(TDLS) */

/**
 *****************************************************************************
 * Specific filter for Radio Measurement component
 *****************************************************************************
 */
enum trace_level_rm
{
    /** Error */
    TRACE_LVL_RM_ERR,
    /** Warning */
    TRACE_LVL_RM_WARN,
    /** Radio Measurement  */
    TRACE_LVL_RM_MEASURE,
    /** Radio Measurement Report */
    TRACE_LVL_RM_REPORT,
    /** Radio Measurement Request  */
    TRACE_LVL_RM_REQUEST,
};

/** Default trace level for RM component */
#define TRACE_RM_DEFAULT (CO_BIT(TRACE_LVL_RM_ERR))

/** Define whether to include or not RM traces */
#define TRACE_RM_EN 1
#if TRACE_COMPO(RM)
/** add trace point for RM component */
#define TRACE_RM(lvl,...) TRACE_FILT(RM, RM_##lvl, __VA_ARGS__ )
/** add buffer trace point for RM component */
#define TRACE_BUF_RM(lvl,...) TRACE_BUF_FILT(RM, RM_##lvl, __VA_ARGS__ )
/** Test whether RM level is currently enabled or not */
#define TRACE_RM_ON(lvl) TRACE_FILT_ON(RM, RM_##lvl)
#else
#define TRACE_RM(lvl,...)
#define TRACE_BUF_RM(lvl,...)
#define TRACE_RM_ON(lvl) 0
#endif /* TRACE_COMPO(RM) */

/**
 *****************************************************************************
 * Specific filter for TWT component
 *****************************************************************************
 */
enum trace_level_twt
{
    /** Error */
    TRACE_LVL_TWT_ERR,
    /** General status */
    TRACE_LVL_TWT_INF,
    /** Received frames from peers */
    TRACE_LVL_TWT_RX,
    /** Transmitted frames to peers */
    TRACE_LVL_TWT_TX,
    /** Debug traces*/
    TRACE_LVL_TWT_DBG,
};

/** Default trace level for TWT component */
#define TRACE_TWT_DEFAULT (CO_BIT(TRACE_LVL_TWT_ERR) | CO_BIT(TRACE_LVL_TWT_INF))

/** Define whether to include or not TWT traces */
#define TRACE_TWT_EN 1
#if TRACE_TWT_EN
/** add trace point for TWT component */
#define TRACE_TWT(lvl,...) TRACE_FILT(TWT, TWT_##lvl, __VA_ARGS__ )
/** add buffer trace point for TWT component */
#define TRACE_BUF_TWT(lvl,...) TRACE_BUF_FILT(TWT, TWT_##lvl, __VA_ARGS__ )
/** Test whether TWT level is currently enabled or not */
#define TRACE_TWT_ON(lvl) TRACE_FILT_ON(TWT, TWT_##lvl)
#else
#define TRACE_TWT(lvl,...)
#define TRACE_BUF_TWT(lvl,...)
#define TRACE_TWT_ON(lvl) 0
#endif /* TRACE_TWT_EN */

/**
 *****************************************************************************
 * Specific filter for RTOS component
 *****************************************************************************
 */
enum trace_level_rtos
{
    /** Error */
    TRACE_LVL_RTOS_ERR,
    /** Memory allocation */
    TRACE_LVL_RTOS_ALLOC,
    /** Memory liberation */
    TRACE_LVL_RTOS_FREE,
    /** Creation/destruction of RTOS tasks */
    TRACE_LVL_RTOS_CREATE,
    /** Suspension/resumption of RTOS tasks */
    TRACE_LVL_RTOS_SUSPEND,
    /** RTOS Task becomes active */
    TRACE_LVL_RTOS_SWITCH_IN,
    /** RTOS Task is no longer active */
    TRACE_LVL_RTOS_SWITCH_OUT,
};

/** Default trace level for RTOS component */
#define TRACE_RTOS_DEFAULT (CO_BIT(TRACE_LVL_RTOS_ERR) |                     \
                            CO_BIT(TRACE_LVL_RTOS_CREATE))

/** Define whether to include or not RTOS traces */
#define TRACE_RTOS_EN 1
#if TRACE_COMPO(RTOS)
/** add trace point for RTOS component */
#define TRACE_RTOS(lvl,...) TRACE_FILT(RTOS, RTOS_##lvl, __VA_ARGS__ )
/** add buffer trace point for RTOS component */
#define TRACE_BUF_RTOS(lvl,...) TRACE_BUF_FILT(RTOS, RTOS_##lvl, __VA_ARGS__ )
/** Test whether RTOS level is currently enabled or not */
#define TRACE_RTOS_ON(lvl) TRACE_FILT_ON(RTOS, RTOS_##lvl)
#else
#define TRACE_RTOS(lvl,...)
#define TRACE_BUF_RTOS(lvl,...)
#define TRACE_RTOS_ON(lvl) 0
#endif /* TRACE_COMPO(RTOS) */

/**
 *****************************************************************************
 * Specific filter for FHOST component
 *****************************************************************************
 */
enum trace_level_fhost
{
    /** Error */
    TRACE_LVL_FHOST_ERR,
    /** wpa_supplicant task state machine */
    TRACE_LVL_FHOST_WPA_FSM,
    /** Trace command sent to wpa_supplicant */
    TRACE_LVL_FHOST_WPA_CMD,
    /** Trace update of wpa_supplicant status */
    TRACE_LVL_FHOST_WPA_INFO,
    /** DHCP */
    TRACE_LVL_FHOST_DHCP,
    /** FHOST Tasks initialization */
    TRACE_LVL_FHOST_TASK_INIT,
    /** smart_config */
    TRACE_LVL_FHOST_SMARTCONF,
};

/** Default trace level for FHOST component */
#define TRACE_FHOST_DEFAULT (255)

/** Define whether to include or not FHOST traces */
#define TRACE_FHOST_EN 1
#if TRACE_COMPO(FHOST)
/** add trace point for FHOST component */
#define TRACE_FHOST(lvl,...) TRACE_FILT(FHOST, FHOST_##lvl, __VA_ARGS__ )
/** add buffer trace point for RTOS component */
#define TRACE_BUF_FHOST(lvl,...) TRACE_BUF_FILT(FHOST, FHOST_##lvl, __VA_ARGS__ )
/** Test whether FHOST level is currently enabled or not */
#define TRACE_FHOST_ON(lvl) TRACE_FILT_ON(FHOST, FHOST_##lvl)
#else
#define TRACE_FHOST(lvl,...)
#define TRACE_BUF_FHOST(lvl,...)
#define TRACE_FHOST_ON(lvl) 0
#endif /* TRACE_COMPO(FHOST) */

/**
 *****************************************************************************
 * Specific filter for FHOST APPLICATION component
 *****************************************************************************
 */
enum trace_level_app
{
    /** Error */
    TRACE_LVL_APP_ERR,
    /** Warning */
    TRACE_LVL_APP_WARN,
    /** Information (depends of the application used) */
    TRACE_LVL_APP_INF,
    /** Debug (depends of the application used) */
    TRACE_LVL_APP_DBG,
};

/** Default trace level for APP component */
#define TRACE_APP_DEFAULT (CO_BIT(TRACE_LVL_APP_ERR))

/** Define whether to include or not APP traces */
#define TRACE_APP_EN 1
#if TRACE_COMPO(APP)
/** add trace point for APP component */
#define TRACE_APP(lvl,...) TRACE_FILT(APP, APP_##lvl, __VA_ARGS__ )
/** add buffer trace point for APP component */
#define TRACE_BUF_APP(lvl,...) TRACE_BUF_FILT(APP, APP_##lvl, __VA_ARGS__ )
/** Test whether APP level is currently enabled or not */
#define TRACE_APP_ON(lvl) TRACE_FILT_ON(APP, APP_##lvl)
#else
#define TRACE_APP(lvl,...)
#define TRACE_BUF_APP(lvl,...)
#define TRACE_APP_ON(lvl) 0
#endif /* TRACE_COMPO(APP) */

/**
 *****************************************************************************
 * Specific filter for FTM component
 *****************************************************************************
 */
enum trace_level_ftm
{
    /** Error */
    TRACE_LVL_FTM_ERR,
    /** General status */
    TRACE_LVL_FTM_INF,
};

/** Default trace level for FTM component */
#define TRACE_FTM_DEFAULT (CO_BIT(TRACE_LVL_FTM_ERR) |                      \
                           CO_BIT(TRACE_LVL_FTM_INF))

/** Define whether to include or not MESH traces */
#define TRACE_FTM_EN 1
#if TRACE_COMPO(FTM)
/** add trace point for FTM component */
#define TRACE_FTM(lvl,...) TRACE_FILT(FTM, FTM_##lvl, __VA_ARGS__ )
/** add buffer trace point for MESH component */
#define TRACE_BUF_FTM(lvl,...) TRACE_BUF_FILT(FTM, FTM_##lvl, __VA_ARGS__ )
/** Test whether FTM level is currently enabled or not */
#define TRACE_FTM_ON(lvl) TRACE_FILT_ON(FTM, FTM_##lvl)
#else
#define TRACE_FTM(lvl,...)
#define TRACE_BUF_FTM(lvl,...)
#define TRACE_FTM_ON(lvl) 0
#endif /* TRACE_COMPO(FTM) */

#endif /* _TRACE_COMPO_H_ */
/**
 * @} end of group
 */
