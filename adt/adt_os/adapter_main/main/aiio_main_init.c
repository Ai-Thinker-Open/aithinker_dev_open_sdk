#include "aiio_chip_init.h"
#include "aiio_chip_spec.h"
#include "aiio_os_port.h"
#include "aiio_autoconf.h"
#include "aiio_error.h"
#include "aiio_adapter_include.h"


#define MAIN_ENTRY()                __MAIN_ENTRY()
#define AIIO_OS_PRIORITY_DEFAULT    __AIIO_OS_PRIORITY_DEFAULT

#define AIIO_MAIN_STACK             CONFIG_AIIO_MAIN_STACK


MAIN_ENTRY()
{
    extern void aiio_wsrc_call(void);
    aiio_wsrc_call();

    aiio_chip_init();
    aiio_log_init();
    aiio_manufacturer_info();
    static aiio_os_thread_handle_t *aiio_main_thread = NULL;
    static aiio_os_thread_handle_t *aiio_heap_check_thread = NULL;
    extern aiio_os_function_return_t aiio_main(void *params);

    if (AIIO_OK != aiio_os_thread_create(&aiio_main_thread, "aiio_main", aiio_main, CONFIG_AIIO_MAIN_STACK, NULL, AIIO_OS_PRIORITY_DEFAULT))
    {
    }
#if CONFIG_AIIO_HEAP_LOG_EN
    if (AIIO_OK != aiio_os_thread_create(&aiio_heap_check_thread, "aiio_heap_check", aiio_heap_check, CONFIG_AIIO_MAIN_STACK / 2, NULL, AIIO_OS_PRIORITY_DEFAULT))
    {
    }
#endif
    aiio_os_thread_start_scheduler();
}




