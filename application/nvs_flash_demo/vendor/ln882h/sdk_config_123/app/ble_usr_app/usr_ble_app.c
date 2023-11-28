#include <stdlib.h>         // standard lib functions
#include <stdint.h>         // standard integer definition

#include "rwip_config.h"    // RW SW configuration TASK_APP
#include "ke_task.h"        // Kernel Task
#include "ln_app_gap.h"     // gapm reset

#include "usr_ble_app.h"

void ble_app_init(void)
{
    /// Application Task Descriptor
    extern const struct ke_task_desc TASK_DESC_APP;

    // Create APP task
    ke_task_create(TASK_APP, &TASK_DESC_APP);

    // Initialize Task state
    ke_state_set(TASK_APP, APP_INIT);
    // Reset the stack
    ln_app_gapm_reset();
}
