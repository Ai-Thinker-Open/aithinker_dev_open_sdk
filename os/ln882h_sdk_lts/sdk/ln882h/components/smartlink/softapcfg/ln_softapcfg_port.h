#ifndef __LN_SOFTAPCFG_PORT_H__
#define __LN_SOFTAPCFG_PORT_H__

#include "ln_types.h"
#include "wifi/wifi.h"
#include "lwip/sockets.h"
#include "lwip/ip4_addr.h"
#include "osal/osal.h"

#include "utils/system_parameter.h"
#include "utils/art_string.h"
#include "utils/debug/log.h"


#define APCFG_LOG_ON       (1)

#define apcfg_log(fmt, arg...)                     \
        do {                                       \
            if(APCFG_LOG_ON)                       \
                log_printf("[APCFG] "fmt, ##arg);  \
        } while (0)

#endif /* __LN_SOFTAPCFG_PORT_H__ */
