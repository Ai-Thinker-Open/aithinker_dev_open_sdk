#include "aiio_type.h"
#include "aiio_log.h"
#include "aiio_error.h"
#include "aiio_chip_init.h"

int32_t aiio_chip_init(void)
{
    return AIIO_OK;
}

char *aiio_get_chipname(void)
{
    return "RTL8720D";
}
