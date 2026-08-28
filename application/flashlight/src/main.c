#include "aiio_log.h"
#include "aiio_os_port.h"
#include "axk_app_init.h"

void aiio_main(void *params)
{
    g_app_init.init();

    aiio_os_thread_delete(NULL);
}

