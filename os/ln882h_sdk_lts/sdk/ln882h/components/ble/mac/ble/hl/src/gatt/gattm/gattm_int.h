/**
 ****************************************************************************************
 *
 * @file gattm_int.h
 *
 * @brief Internal Header file - GATTM.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef GATTM_INT_H_
#define GATTM_INT_H_

/**
 ****************************************************************************************
 * @addtogroup GATTM_INT Task
 * @ingroup GATTM
 * @brief Internal function required for GATTM usage
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "ke_task.h"
#include "gattm_task.h"

#include "../attm/attm_db.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// number of GATT Process
#define GATTM_IDX_MAX                                 0x01


/// GATT database default features
#define GATT_DB_DEFAULT_FEAT         0x00FF

/// states of GATT task
enum gattm_state_id
{
    /// idle state
    GATTM_IDLE,
    /// busy state
    GATTM_BUSY,
    /// Number of defined states.
    GATTM_STATE_MAX
};

/// GATT General Information Manager
struct gattm_env_tag
{
    #if (BLE_ATTS)
    /// Environment data needed by attribute database
    struct attm_db  db;

    /// GATT service start handle
    uint16_t svc_start_hdl;
    #endif // (BLE_ATTS)
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern struct gattm_env_tag gattm_env;

/*
 * FUNCTIONS DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Informs that database has been updated
 ****************************************************************************************
 */
void gattm_db_updated(void);

/// @} GATTM_INT
#endif // GATTM_INT_H_
