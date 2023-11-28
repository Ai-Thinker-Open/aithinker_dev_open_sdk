/**
 ****************************************************************************************
 *
 * @file rtos.h
 *
 * @brief Declarations related to the WiFi stack integration within an RTOS.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */

#ifndef RTOS_H_
#define RTOS_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "export/common/co_bool.h"
#include "rtos_al.h"
#include "export/rwnx_config.h"

/*
 * FUNCTIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Create the RTOS WiFi task.
 *
 * Create the wifi task that will execute the wifi UMAC and LMAC layer.
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int rtos_wifi_task_create(void);

/**
 ****************************************************************************************
 * @brief Request the RTOS to suspend the WiFi task.
 * This function will return only when the WiFi task is resumed by the function @ref
 * rtos_wifi_task_resume
 ****************************************************************************************
 */
void rtos_wifi_task_suspend(void);

/**
 ****************************************************************************************
 * @brief Request the RTOS to resume the WiFi task.
 * This function first checks if the task was indeed suspended and then proceed to the
 * resume. Note that currently this function is supposed to be called from interrupt.
 *
 * @param[in] isr Indicates if called from ISR
 ****************************************************************************************
 */
void rtos_wifi_task_resume(bool isr);

#if NX_FULLY_HOSTED
/**
 ****************************************************************************************
 * @brief Request the RTOS to increase the priority of the WiFi task.
 * By default the WiFi task has a low priority compared to other tasks. This function
 * allows increasing temporarily the WiFi task priority to a high value to ensure it is
 * scheduled immediately.
 * This function cannot be called from an ISR.
 ****************************************************************************************
 */
void rtos_wifi_task_prio_high(void);

/**
 ****************************************************************************************
 * @brief Request the RTOS to decrease the priority of the WiFi task.
 * This function puts back the WiFi task priority to its default value.
 * This function cannot be called from an ISR.
 ****************************************************************************************
 */
void rtos_wifi_task_prio_default(void);
#endif

#endif // RTOS_H_
