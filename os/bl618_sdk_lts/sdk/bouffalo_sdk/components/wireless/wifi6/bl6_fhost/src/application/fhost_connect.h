/**
 ****************************************************************************************
 *
 * @file fhost_connect.h
 *
 * @brief Definitions of the fully hosted Connect task.
 *
 * Copyright (C) RivieraWaves 2019-2021
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup FHOST_CONNECT FHOST_CONNECT
 * @ingroup FHOST_IPC
 * @brief Task started by @p connect command. This task is active until the connection is
 * completed or canceled.
 * @{
 ****************************************************************************************
 */

#include "fhost_api.h"
#include "fhost_ipc.h"

/// Connect Task parameter
struct fhost_ipc_connect_cfg
{
    /// Index of the FHOST interface
    int fhost_vif_idx;
    /// Connection parameters
    struct fhost_vif_sta_cfg cfg;
    /// Link to CNTRL task
    struct fhost_cntrl_link *link;
};


/**
 ****************************************************************************************
 * @brief Start a task to complete the connection
 *
 * @param[in] arg  Task parameter (@ref fhost_ipc_connect_cfg)
 *
 * @return Hnadle of the task that is doing the connection and RTOS_TASK_NULL in case
 * of error
 ****************************************************************************************
 **/
rtos_task_handle fhost_ipc_connect_task_start(void *arg);

/**
 ****************************************************************************************
 * @brief Kill pending connection
 *
 * @param[in] task  Task handle
 * @return always FHOST_IPC_SUCCESS
 ****************************************************************************************
 **/
int fhost_ipc_connect_task_kill(rtos_task_handle task);

/**
 * @}
 */
