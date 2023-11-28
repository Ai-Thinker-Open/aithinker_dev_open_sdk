/*
 * Copyright (C) 2018 Alibaba Group Holding Limited
 */

/* This file provides threading mutex implementation for LN-SEMI Things. */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_THREADING_ALT)
#include "osal/osal.h"
#include "mbedtls/threading.h"

void threading_mutex_init( mbedtls_threading_mutex_t *mutex )
{
    if( mutex == NULL || mutex->is_valid )
        return;

    mutex->is_valid = ( OS_MutexCreate( &mutex->mutex ) == 0 );
}

void threading_mutex_free( mbedtls_threading_mutex_t *mutex )
{
    if( mutex == NULL || !mutex->is_valid )
        return;

    OS_MutexDelete( &mutex->mutex );
    mutex->is_valid = 0;
}

int threading_mutex_lock( mbedtls_threading_mutex_t *mutex )
{
    if( mutex == NULL || !mutex->is_valid )
        return( MBEDTLS_ERR_THREADING_BAD_INPUT_DATA );

    if( OS_MutexLock( &mutex->mutex, OS_WAIT_FOREVER ) != 0 )
        return( MBEDTLS_ERR_THREADING_MUTEX_ERROR );

    return( 0 );
}

int threading_mutex_unlock( mbedtls_threading_mutex_t *mutex )
{
    if( mutex == NULL || ! mutex->is_valid )
        return( MBEDTLS_ERR_THREADING_BAD_INPUT_DATA );

    if( OS_MutexUnlock( &mutex->mutex ) != 0 )
        return( MBEDTLS_ERR_THREADING_MUTEX_ERROR );

    return( 0 );
}
#endif /* MBEDTLS_THREADING_ALT */

