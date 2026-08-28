 //中间件相关
#include "aiio_system.h"

//SDK相关
#include "ameba_soc.h"
#include "os_wrapper.h"
#include "sys_api.h"
// #include "random.h"

void aiio_restart(void)
{
    sys_reset();
}

int aiio_random(void)
{
    return rand();
}
