#ifndef LN_BLE_RW_APP_TASK_H_
#define LN_BLE_RW_APP_TASK_H_

#include "ke_task.h"
#include "co_utils.h"

/// States of APP task
enum app_state
{
    /// Initialization state
    APP_INIT,
    /// Database create state
    APP_CREATE_DB,
    /// Ready State
    APP_READY,
    /// Connected state
    APP_CONNECTED,
    /// advertising
    APP_ADVERTISING,

    /// Number of defined states.
    APP_STATE_MAX
};


void ln_rw_app_task_init(void);
int ln_app_msg_regester(struct ke_msg_handler *handler, uint8_t msg_num);

#endif
