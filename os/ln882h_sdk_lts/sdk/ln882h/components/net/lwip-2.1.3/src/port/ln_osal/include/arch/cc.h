/**
 * Porting by lightningsemi wifi team.
 * Dependencies:
 *     utils/debug/log.h
 *     osal/osal.h
*/

#ifndef __ARCH_CC_H__
#define __ARCH_CC_H__

/* Define platform endianness */
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif /* BYTE_ORDER */

/* Whether to use LWIP error codes */
#define LWIP_PROVIDE_ERRNO  (1)

#if (defined(__GNUC__) && !defined(__CC_ARM))
    /* Use private struct timeval */
    #define LWIP_TIMEVAL_PRIVATE    0
    #if (!LWIP_TIMEVAL_PRIVATE)
    #include <sys/time.h>
    #endif

    /* GNU Compiler */
    #define PACK_STRUCT_BEGIN
    #define PACK_STRUCT_STRUCT __attribute__((__packed__))
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    #define ALIGNED(n) __attribute__((aligned (n)))
#elif defined(__CC_ARM)
    /* ARM Compiler */
    #define PACK_STRUCT_BEGIN __packed
    #define PACK_STRUCT_STRUCT
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    #define ALIGNED(n) __align(n)
#else
    #error "Compiler not supported."
#endif


/* Plaform specific diagnostic output */
#include "utils/debug/log.h"
#define __printf(...)     log_printf(__VA_ARGS__)

#ifndef LWIP_PLATFORM_DIAG
#define LWIP_PLATFORM_DIAG(x)   do {__printf x;} while(0)
#endif

#ifndef LWIP_PLATFORM_ASSERT
#define LWIP_PLATFORM_ASSERT(x)                                                 \
    do {                                                                        \
        __printf("Assertion \"%s\" failed at line %d in %s\n",                  \
            x, __LINE__, __FILE__);                                             \
    } while(0)
#endif

#ifndef LWIP_ERROR
#define LWIP_ERROR(message, expression, handler)                                \
    do {                                                                        \
        if (!(expression)) {                                                    \
            __printf("Assertion \"%s\" failed at line %d in %s\n",              \
                message, __LINE__, __FILE__);                                   \
            fflush(NULL);                                                       \
            handler;                                                            \
        }                                                                       \
    } while(0)
#endif

#include "osal/osal.h"
#define LWIP_RAND()    OS_Rand32()

#endif /* __ARCH_CC_H__ */
