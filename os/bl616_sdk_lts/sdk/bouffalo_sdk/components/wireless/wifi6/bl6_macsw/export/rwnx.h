/**
 ****************************************************************************************
 *
 * @file rwnx.h
 *
 * @brief Main nX MAC definitions.
 *
 * Copyright (C) RivieraWaves 2011-2021
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @mainpage RW-WLAN-nX MAC SW project index page
 *
 * @section intro_sec Introduction
 *
 * RW-WLAN-nX is the RivieraWaves 802.11a/b/g/n/ac IP.\n
 * This document presents the Low Level Design of the RW-WLAN-nX MAC SW.\n
 * A good entry point to this document is the @ref MACSW "MACSW" module page.
 ****************************************************************************************
 */

#ifndef _RWNXL_H_
#define _RWNXL_H_

#include "export/common/co_bool.h"

#ifdef CFG_BL_WIFI_PS_ENABLE
#include "bl_lp.h"
#endif

/**
 ****************************************************************************************
 * @defgroup MACSW MACSW
 * @brief RW-WLAN-nX root module.
 * @{
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief This function performs all the initializations of the MAC SW.
 *
 ****************************************************************************************
 */
void rwnxl_init(void);

#ifdef CFG_BL_WIFI_PS_ENABLE
/**
 ****************************************************************************************
 * @brief This function performs all the reinit of the MAC SW.
 *
 ****************************************************************************************
 */
void rwnxl_ps_init(void);
#endif

/**
 ****************************************************************************************
 * @brief Start the MAC SW.
 *
 * Start processing wifi event in an endless loop (or the RTOS scheduler).
 * It doesn't returns.
 ****************************************************************************************
 */
void rwnxl_start(void);

/**
 ****************************************************************************************
 * @brief NX reset event handler.
 * This function is part of the recovery mechanism invoked upon an error detection in the
 * LMAC. It performs the full LMAC reset, and restarts the operation.
 *
 * @param[in] dummy Parameter not used but required to follow the kernel event callback
 * format
 ****************************************************************************************
 */
void rwnxl_reset_evt(int dummy);

/**
 ****************************************************************************************
 * @brief This function checks if any kernel event is pending in the MAC SW.
 * If no event is pending, the CPU clock could be gated.
 *
 * @return true if the CPU can be put in sleep, false otherwise.
 ****************************************************************************************
 */
bool rwnxl_cpu_can_sleep(void);

/**
 ****************************************************************************************
 * @brief This function performs the required checks prior to go to DOZE mode.
 * If all these checks pass, then the MAC HW is put in DOZE mode.
 *
 ****************************************************************************************
 */
int rwnxl_sleep(void);

/**
 ****************************************************************************************
 * @brief This function performs the wake up from DOZE mode.
 *
 ****************************************************************************************
 */
void rwnxl_wakeup(void);

#ifdef CFG_BL_WIFI_PS_ENABLE
/**
 ****************************************************************************************
 * @brief This function performs check ps sleep allowed.
 *
 ****************************************************************************************
 */
int rwnxl_ps_sleep_check(void);

/**
 ****************************************************************************************
 * @brief This function performs check ps sleep allowed.
 *
 ****************************************************************************************
 */
int rwnx_get_wifi_ps_state(void);

/**
 ****************************************************************************************
 * @brief This function performs set wifi ps state.
 *
 ****************************************************************************************
 */
void rwnx_set_wifi_ps_state(void);

/**
 ****************************************************************************************
 * @brief This function performs clear wifi ps state.
 *
 ****************************************************************************************
 */
void rwnx_clear_wifi_ps_state(void);

/**
 ****************************************************************************************
 * @brief This function performs notify ap to enter ps.
 *
 ****************************************************************************************
 */
int rwnxl_ps_sleep_enter_ops(void);

/**
 ****************************************************************************************
 * @brief This function performs config wifi ps arguments.
 *
 ****************************************************************************************
 */
int rwnxl_pds_wifi_config(bl_lp_fw_cfg_t *pcfg);

/**
 ****************************************************************************************
 * @brief This function notify to resume wifi.
 *
 ****************************************************************************************
 */
void rwnxl_resume_wifi_evt(int x);
void rwnxl_resume_wifi(void);
void rwnxl_resume_defer(void);

/**
 ****************************************************************************************
 * @brief This function notify to resume wifi.
 *
 ****************************************************************************************
 */
int rwnxl_wifi_notify_resume(void);

/**
 ****************************************************************************************
 * @brief This function perform mm active.
 *
 ****************************************************************************************
 */
void rwnxl_mm_active(void);
#else
void rwnxl_resume_wifi_evt(int x);
#endif


/// @}

#endif // _RWNXL_H_
