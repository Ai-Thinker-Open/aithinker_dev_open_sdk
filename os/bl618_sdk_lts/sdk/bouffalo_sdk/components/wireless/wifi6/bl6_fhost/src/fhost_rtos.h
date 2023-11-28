/**
 ****************************************************************************************
 *
 * @file fhost.h
 *
 * @brief Definitions of the fully hosted module.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */

#ifndef _FHOST_RTOS_H_
#define _FHOST_RTOS_H_

/**
 ****************************************************************************************
 * @defgroup FHOST_RTOS FHOST_RTOS
 * @ingroup FHOST
 * @brief Task priorities and stack size definitions.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rtos_al.h"

/*
 * DEFINITIONS
 ****************************************************************************************
 */
/// Definitions of the different FHOST task priorities
enum
{
    /// Priority of the WiFi task
    FHOST_WIFI_PRIORITY = RTOS_TASK_PRIORITY(27),
    /// Priority of the WiFi task (when high priority is set)
    FHOST_WIFI_PRIORITY_HIGH = RTOS_TASK_PRIORITY(30),
    /// Priority of the TX task
    FHOST_TCPIP_PRIORITY = RTOS_TASK_PRIORITY(28),
    /// Priority of the control task
    FHOST_CNTRL_PRIORITY = RTOS_TASK_PRIORITY(27),
    /// Priority of the RX task
    FHOST_RX_PRIORITY = RTOS_TASK_PRIORITY(27),
    /// Priority of the TX task
    FHOST_TX_PRIORITY = RTOS_TASK_PRIORITY(29),
    /// Priority of the WPA task
    FHOST_WPA_PRIORITY = RTOS_TASK_PRIORITY(26),
    /// Priority of the IPC task
    FHOST_IPC_PRIORITY = RTOS_TASK_PRIORITY(29),
    /// Priority of the Ping send task
    FHOST_PING_SEND_PRIORITY = RTOS_TASK_PRIORITY(27),
    /// Priority of the IPERF task
    FHOST_IPERF_PRIORITY = RTOS_TASK_PRIORITY(27),
};

/// Definitions of the different FHOST task stack size requirements
enum
{
    /// WiFi task stack size
    FHOST_WIFI_STACK_SIZE = 2048,
    /// Control task stack size
    FHOST_CNTRL_STACK_SIZE = 1024,
    /// TCP/IP task stack size
    FHOST_TCPIP_STACK_SIZE = 1024,
    /// RX task stack size
    FHOST_RX_STACK_SIZE = 1024,
    /// TX task stack size
    FHOST_TX_STACK_SIZE = 1024,
    /// WPA task stack size
    FHOST_WPA_STACK_SIZE = 2048,
    /// IPC task stack size
    FHOST_IPC_STACK_SIZE = 512,
    /// TG send stack size
    FHOST_TG_SEND_STACK_SIZE = 1024,
    /// Ping send stack size
    FHOST_PING_SEND_STACK_SIZE = 256,
    /// Smartconfig task stack size
    FHOST_SMARTCONF_STACK_SIZE = 512,
    /// IPERF task stack size
    FHOST_IPERF_STACK_SIZE = 1024,
    /// DOORBELL task stack size
    FHOST_DOORBELL_STACK_SIZE = 1024,
};

/// @}

#endif // _FHOST_H_

