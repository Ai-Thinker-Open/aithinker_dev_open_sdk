
#ifndef __LN_SHA1_H__
#define __LN_SHA1_H__

#include <stdint.h>
#include <stdlib.h>

#define LN_SW_SHA1_USING_ROM_CODE (1)

#if defined(LITTLE_ENDIAN)
  #define LN_SHA1_LITTLE_ENDIAN
#elif defined(BIG_ENDIAN)
  #define LN_SHA1_BIG_ENDIAN
#else
  #define LN_SHA1_LITTLE_ENDIAN
#endif // !LITTLE_ENDIAN

/** SHA-1 Digest size in bytes */
#define LN_SHA1_DIGEST_SIZE 20

/** SHA-1 Context */
typedef struct {
    uint32_t state[5];  /**< Context state */
    uint32_t count[2];  /**< Counter       */    
    uint8_t buffer[64]; /**< SHA-1 buffer  */
} ln_sha1_ctx_t;

extern void ln_sha1_init(ln_sha1_ctx_t *context);
extern void ln_sha1_update(ln_sha1_ctx_t *context, const void *p, size_t len);
extern void ln_sha1_transform(uint32_t state[5], const uint8_t buffer[64]);
extern void ln_sha1_final(uint8_t digest[LN_SHA1_DIGEST_SIZE], ln_sha1_ctx_t *context);

extern void ln_hmac_sha1(uint8_t  * k,    /* secret key */
                         size_t     lk,   /* length of the key in bytes */
                         uint8_t  * d,    /* data */
                         size_t     ld,   /* length of data in bytes */
                         uint8_t  * out   /* output buffer, at least "t" bytes */
                         );
#endif // !__LN_SHA1_H__
