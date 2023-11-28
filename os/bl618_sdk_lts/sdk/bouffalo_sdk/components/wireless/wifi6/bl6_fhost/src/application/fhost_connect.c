/**
 ****************************************************************************************
 *
 * @file fhost_connect.c
 *
 * @brief Implementation of the fully hosted Connect task.
 *
 * Copyright (C) RivieraWaves 2019-2021
 *
 ****************************************************************************************
 */

#include "fhost.h"
#include "fhost_connect.h"
#include "fhost_wpa.h"
#include "net_al_ext.h"


/**
 ****************************************************************************************
 * @addtogroup FHOST_CONNECT
 * @{
 ****************************************************************************************
 */

/// Table to keep correspondance between fhost_vif_idx and task handle;
rtos_task_handle fhost_ipc_connect_task[NX_VIRT_DEV_MAX];

/**
 ****************************************************************************************
 * @brief Connect task main loop
 *
 * This task is only used to run fhost_sta_cfg outside of IPC task, so that it can be
 * killed if needed
 ****************************************************************************************
 */
#ifdef CONFIG_RA_EN
uint8_t conn_flag = 0;
#endif
static RTOS_TASK_FCT(fhost_ipc_connect_main)
{
    struct fhost_ipc_connect_cfg *cfg = env;
    struct fhost_vif_status status;
    struct net_al_ext_ip_addr_cfg ip_cfg;
    int res = FHOST_IPC_ERROR;

    ip_cfg.mode = IP_ADDR_NONE;

    if (fhost_get_vif_status(cfg->fhost_vif_idx, &status))
        goto end;

    if (status.type != VIF_STA)
    {
        fhost_set_vif_type(cfg->link, cfg->fhost_vif_idx, VIF_STA, false);
    }
    else if (fhost_wpa_get_state(cfg->fhost_vif_idx) != FHOST_WPA_STATE_STOPPED)
    {
        if (net_al_ext_get_vif_ip(cfg->fhost_vif_idx, &ip_cfg))
            goto end;

#ifdef CONFIG_RA_EN
        if (fhost_wpa_get_state(cfg->fhost_vif_idx) == FHOST_WPA_STATE_CONNECTED) {
            conn_flag = 1;
        }
#endif

        if (fhost_wpa_disable_network(cfg->fhost_vif_idx)) {
            goto end;
        }

        // Must wait for scanning done (if is scanning, it means scan isnot done)
        while (fhost_wpa_get_supplicant_state(cfg->fhost_vif_idx) != 0 ||
                fhost_env.vif[cfg->fhost_vif_idx].scanning)
        {
            rtos_task_suspend(100);
        }

        // Reset STA interface (this will end previous wpa_supplicant task)
        if (fhost_set_vif_type(cfg->link, cfg->fhost_vif_idx, VIF_UNKNOWN, false) ||
            fhost_set_vif_type(cfg->link, cfg->fhost_vif_idx, VIF_STA, false))
            goto end;
    }

    // Must wait for scanning done (if is scanning, it means scan isnot done)
    while (fhost_env.vif[cfg->fhost_vif_idx].scanning) {
        rtos_task_suspend(100);
    }

    if (fhost_sta_cfg(cfg->fhost_vif_idx, &(cfg->cfg)))
        goto end;

#if 0
    if (ip_cfg.mode != IP_ADDR_NONE)
    {
        if (ip_cfg.mode == IP_ADDR_DHCP_CLIENT)
            ip_cfg.dhcp.to_ms = 5000;
        ip_cfg.default_output = true;

        if (fhost_set_vif_ip(cfg->fhost_vif_idx, &ip_cfg))
            goto end;
    }
#endif

    res = FHOST_IPC_SUCCESS;

  end:
#ifdef CONFIG_RA_EN
    conn_flag = 0;
#endif
    fhost_ipc_connect_task[cfg->fhost_vif_idx] = RTOS_TASK_NULL;
    if(cfg){
        rtos_free(cfg);
    }
    rtos_task_delete(NULL);
}

static RTOS_TASK_FCT(fhost_ipc_connect_entry)
{
    rtos_task_handle task;
    struct fhost_ipc_connect_cfg *cfg = env;

    while (fhost_ipc_connect_task[cfg->fhost_vif_idx]) {
        rtos_task_suspend(500);
    }

    if (rtos_task_create(fhost_ipc_connect_main, "CONNECT", APPLICATION_TASK, 512,
                         env,  RTOS_TASK_PRIORITY(2), &task))
    {
        rtos_free(env);
        printf("Create CONNECT Task error\r\n");
        return;
    }

    // Ideally we should ensure that we return of this function before
    // fhost_ipc_connect_main returns but this is just a test function and so far
    // IPC task (i.e. the one in which this function is executed) has a higher priority
    fhost_ipc_connect_task[cfg->fhost_vif_idx] = task;
    rtos_task_delete(NULL);
}

rtos_task_handle fhost_ipc_connect_task_start(void *arg)
{
    struct fhost_ipc_connect_cfg *cfg;
    rtos_task_handle task;

    cfg = rtos_malloc(sizeof(*cfg));
    if (!cfg)
        return RTOS_TASK_NULL;

    memcpy(cfg, arg, sizeof(*cfg));

    if (xTaskGetHandle("CONNECT_ENTRY") == NULL) {
        if (rtos_task_create(fhost_ipc_connect_entry, "CONNECT_ENTRY", APPLICATION_TASK, 512,
                            cfg,  10, &task))
        {
            rtos_free(cfg);
            printf("Create CONNECT_ENTRY Task error\r\n");
            return RTOS_TASK_NULL;
        }
    } else {
        /* wait first "CONNECT_ENTRY" task finished */
        while(xTaskGetHandle("CONNECT_ENTRY") != NULL){
            rtos_task_suspend(500);
        }

        if (rtos_task_create(fhost_ipc_connect_entry, "CONNECT_ENTRY", APPLICATION_TASK, 512,
                            cfg,  10, &task))
        {
            rtos_free(cfg);
            printf("Create CONNECT_ENTRY Task error\r\n");
            return RTOS_TASK_NULL;
        }
    }

    return task;
}

int fhost_ipc_connect_task_kill(rtos_task_handle task)
{
    int i;

    for (i = 0; i < NX_VIRT_DEV_MAX ; i ++)
    {
        if (fhost_ipc_connect_task[i] == task)
        {
            fhost_wpa_remove_vif(i);
            break;
        }
    }

    return FHOST_IPC_SUCCESS;
}

/**
 * @}
 */
