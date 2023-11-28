/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

/* This file provides implementation of common (libc) functions that is defined
 * in platform abstraction layer for LN-SEMI Things.
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include <stdlib.h>
#include <string.h>
#include "osal/osal.h"

#if defined(MBEDTLS_PLATFORM_MEMORY)

void * ln_mbedtls_calloc( size_t n, size_t size )
{
    void *buf = NULL;

    buf = OS_Malloc(n * size); // aos_malloc
    if( buf == NULL )
        return( NULL );
    else
        memset(buf, 0, n * size);

    return( buf );
}

void ln_mbedtls_free( void *ptr )
{
    if( ptr == NULL )
        return;

    OS_Free( ptr );
}
#endif /*MBEDTLS_PLATFORM_MEMORY*/
#endif /*MBEDTLS_PLATFORM_C*/
