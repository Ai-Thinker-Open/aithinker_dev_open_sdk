/*
 *  Portable interface to the CPU cycle counter
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_SELF_TEST) && defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include "utils/debug/log.h"
#define mbedtls_printf     log_printf
#endif

#if defined(MBEDTLS_TIMING_C)

#include "mbedtls/timing.h"

#if defined(MBEDTLS_TIMING_ALT)

#include <stdint.h>
#include "osal/osal.h"

struct _hr_time
{
    uint32_t start;
};

/**
 * @brief Get CPU cycle count value.
 * 
 * @return unsigned long 
 */
unsigned long mbedtls_timing_hardclock( void )
{
    uint32_t usecs = 0;
    
    usecs = OS_GetTicks();

    return usecs;
}

/**
 * @brief 
 * 
 * @param val 
 * @param reset 
 * @return unsigned long 
 */
unsigned long mbedtls_timing_get_timer( struct mbedtls_timing_hr_time *val, int reset )
{
    struct _hr_time *t = (struct _hr_time *) val;

    if( reset )
    {
        t->start = OS_GetTicks();
        return( 0 );
    }
    else
    {
        unsigned long delta;
        uint32_t now_ticks = OS_GetTicks();
        delta = (unsigned long)( now_ticks - t->start ); // * 1000
        return( delta );
    }
}

volatile int mbedtls_timing_alarmed = 0;
static OS_Timer_t g_alarm_timer;

void alarm_active_cb(void *arg)
{
    mbedtls_timing_alarmed = 1;
    OS_TimerDelete(&g_alarm_timer);
}

void mbedtls_set_alarm( int seconds )
{
    if( seconds == 0 )
    {
        /* No need to create a thread for this simple case.
         * Also, this shorcut is more reliable at least on MinGW32 */
        mbedtls_timing_alarmed = 1;
        return;
    }

    mbedtls_timing_alarmed = 0;
    memset(&g_alarm_timer, 0, sizeof(OS_Timer_t));
    OS_TimerCreate(&g_alarm_timer, OS_TIMER_ONCE, (OS_TimerCallback_t) alarm_active_cb, NULL, seconds * 1000);
}


/*
 * Set delays to watch
 */
void mbedtls_timing_set_delay( void *data, uint32_t int_ms, uint32_t fin_ms )
{
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *) data;

    ctx->int_ms = int_ms;
    ctx->fin_ms = fin_ms;

    if( fin_ms != 0 )
        (void) mbedtls_timing_get_timer( &ctx->timer, 1 );
}

/*
 * Get number of delays expired
 */
int mbedtls_timing_get_delay( void *data )
{
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *) data;
    unsigned long elapsed_ms;

    if( ctx->fin_ms == 0 )
        return( -1 );

    elapsed_ms = mbedtls_timing_get_timer( &ctx->timer, 0 );

    if( elapsed_ms >= ctx->fin_ms )
        return( 2 );

    if( elapsed_ms >= ctx->int_ms )
        return( 1 );

    return( 0 );
}

#endif /* !MBEDTLS_TIMING_ALT */

#if defined(MBEDTLS_SELF_TEST)

/*
 * Busy-waits for the given number of milliseconds.
 * Used for testing mbedtls_timing_hardclock.
 */
static void busy_msleep( unsigned long msec )
{
    struct mbedtls_timing_hr_time hires;
    unsigned long i = 0; /* for busy-waiting */
    volatile unsigned long j; /* to prevent optimisation */

    (void) mbedtls_timing_get_timer( &hires, 1 );

    while( mbedtls_timing_get_timer( &hires, 0 ) < msec )
        i++;

    j = i;
    (void) j;
}

#define FAIL    do                                                      \
    {                                                                   \
        if( verbose != 0 )                                              \
        {                                                               \
            mbedtls_printf( "failed at line %d\n", __LINE__ );          \
            mbedtls_printf( " cycles=%lu ratio=%lu millisecs=%lu secs=%lu hardfail=%d a=%lu b=%lu\n", \
                            cycles, ratio, millisecs, secs, hardfail,   \
                            (unsigned long) a, (unsigned long) b );     \
            mbedtls_printf( " elapsed(hires)=%lu elapsed(ctx)=%lu status(ctx)=%d\n", \
                            mbedtls_timing_get_timer( &hires, 0 ),      \
                            mbedtls_timing_get_timer( &ctx.timer, 0 ),  \
                            mbedtls_timing_get_delay( &ctx ) );         \
        }                                                               \
        return( 1 );                                                    \
    } while( 0 )

/*
 * Checkup routine
 *
 * Warning: this is work in progress, some tests may not be reliable enough
 * yet! False positives may happen.
 */
int mbedtls_timing_self_test( int verbose )
{
    unsigned long cycles = 0, ratio = 0;
    unsigned long millisecs = 0, secs = 0;
    int hardfail = 0;
    struct mbedtls_timing_hr_time hires;
    uint32_t a = 0, b = 0;
    mbedtls_timing_delay_context ctx;

    if( verbose != 0 )
        mbedtls_printf( "  TIMING tests note: will take some time!\n" );

    if( verbose != 0 )
        mbedtls_printf( "  TIMING test #1 (set_alarm / get_timer): " );

    {
        secs = 1;

        (void) mbedtls_timing_get_timer( &hires, 1 );

        mbedtls_set_alarm( (int) secs );
        while( !mbedtls_timing_alarmed )
            ;

        millisecs = mbedtls_timing_get_timer( &hires, 0 );

        /* For some reason on Windows it looks like alarm has an extra delay
         * (maybe related to creating a new thread). Allow some room here. */
        if( millisecs < 800 * secs || millisecs > 1200 * secs + 300 )
            FAIL;
    }

    if( verbose != 0 )
        mbedtls_printf( "passed\n" );

    if( verbose != 0 )
        mbedtls_printf( "  TIMING test #2 (set/get_delay        ): " );

    {
        a = 800;
        b = 400;
        mbedtls_timing_set_delay( &ctx, a, a + b );          /* T = 0 */

        busy_msleep( a - a / 4 );                      /* T = a - a/4 */
        if( mbedtls_timing_get_delay( &ctx ) != 0 )
            FAIL;

        busy_msleep( a / 4 + b / 4 );                  /* T = a + b/4 */
        if( mbedtls_timing_get_delay( &ctx ) != 1 )
            FAIL;

        busy_msleep( b );                          /* T = a + b + b/4 */
        if( mbedtls_timing_get_delay( &ctx ) != 2 )
            FAIL;
    }

    mbedtls_timing_set_delay( &ctx, 0, 0 );
    busy_msleep( 200 );
    if( mbedtls_timing_get_delay( &ctx ) != -1 )
        FAIL;

    if( verbose != 0 )
        mbedtls_printf( "passed\n" );

    if( verbose != 0 )
        mbedtls_printf( "  TIMING test #3 (hardclock / get_timer): " );

    /*
     * Allow one failure for possible counter wrapping.
     * On a 4Ghz 32-bit machine the cycle counter wraps about once per second;
     * since the whole test is about 10ms, it shouldn't happen twice in a row.
     */

hard_test:
    if( hardfail > 1 )
    {
        if( verbose != 0 )
            mbedtls_printf( "failed (ignored)\n" );

        goto hard_test_done;
    }

    /* Get a reference ratio cycles/ms */
    millisecs = 1;
    cycles = mbedtls_timing_hardclock();
    busy_msleep( millisecs );
    cycles = mbedtls_timing_hardclock() - cycles;
    ratio = cycles / millisecs;

    /* Check that the ratio is mostly constant */
    for( millisecs = 2; millisecs <= 4; millisecs++ )
    {
        cycles = mbedtls_timing_hardclock();
        busy_msleep( millisecs );
        cycles = mbedtls_timing_hardclock() - cycles;

        /* Allow variation up to 20% */
        if( cycles / millisecs < ratio - ratio / 5 ||
            cycles / millisecs > ratio + ratio / 5 )
        {
            hardfail++;
            goto hard_test;
        }
    }

    if( verbose != 0 )
        mbedtls_printf( "passed\n" );

hard_test_done:

    if( verbose != 0 )
        mbedtls_printf( "\n" );

    return( 0 );
}

#endif /* MBEDTLS_SELF_TEST */

#endif /* MBEDTLS_TIMING_C */
