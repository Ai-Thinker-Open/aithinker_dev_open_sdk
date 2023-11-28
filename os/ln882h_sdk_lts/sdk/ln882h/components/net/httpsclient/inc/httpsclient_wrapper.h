#ifndef __HTTP_WRAPPER_H__
#define __HTTP_WRAPPER_H__

#if defined(_MSC_VER)
    // for malloc() and free()
    #include <stdlib.h>
    #include <malloc.h>

    #define hc_printf(...)          printf(##__VA_ARGS__)
    #define hc_malloc(numBytes)     malloc(numBytes)
    #define hc_free(ptr)            free(ptr)
    #define strncasecmp(x,y,z)      _strnicmp(x,y,z)

    typedef unsigned char u_char;
    typedef int socklen_t;
#elif defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
    #include "osal/osal.h"
    #include "utils/debug/log.h"

    #define hc_printf(...)          LOG(LOG_LVL_ERROR, ##__VA_ARGS__)
    #define hc_malloc(numBytes)     OS_Malloc(numBytes)
    #define hc_free(ptr)            OS_Free(ptr)

    typedef unsigned char u_char;

#else

    #define hc_printf(...)          do {} while(0)
    #define hc_malloc(numBytes)     do {} while(0)
    #define hc_free(ptr)            do {} while(0)

    typedef unsigned char u_char;
    typedef int socklen_t;
#endif

#ifndef TRUE
    #define TRUE            1
#endif

#ifndef FALSE
    #define FALSE           0
#endif

#endif // !__HTTP_WRAPPER_H__
