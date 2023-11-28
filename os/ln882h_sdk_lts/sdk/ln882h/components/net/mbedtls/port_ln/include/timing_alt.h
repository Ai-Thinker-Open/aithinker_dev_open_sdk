#ifndef __TIMING_ALT_H__
#define __TIMING_ALT_H__

/**
 * \brief          timer structure
 */
struct mbedtls_timing_hr_time
{
    unsigned char opaque[32];
};

/**
 * \brief          Context for mbedtls_timing_set/get_delay()
 */
typedef struct mbedtls_timing_delay_context
{
    struct mbedtls_timing_hr_time   timer;
    uint32_t                        int_ms;
    uint32_t                        fin_ms;
} mbedtls_timing_delay_context;

#endif // __TIMING_ALT_H__
