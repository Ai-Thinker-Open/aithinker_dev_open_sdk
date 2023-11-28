#include "aiio_type.h"
#include "aiio_log.h"
#include "aiio_error.h"
#include "aiio_chip_init.h"
#include "bl_sys.h"

int32_t aiio_chip_init(void)
{
    bl_sys_init();
    return AIIO_OK;
}
