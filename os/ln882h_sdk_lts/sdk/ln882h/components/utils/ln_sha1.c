
#include "ln_sha1.h"
#include <string.h>

#if defined (__CC_ARM)
#pragma push
#pragma O2
#pragma Otime
#elif defined (__GNUC__)
#pragma GCC push_options
#pragma GCC optimize(2)
#endif /* __CC_ARM __GNUC__ */

/** SHA-1 Digest size in bytes (OpenSSL compat) */
#define SHA_DIGEST_LENGTH LN_SHA1_DIGEST_SIZE

/** SHA-1 Block size */
#ifndef SHA_BLOCKSIZE
#define SHA_BLOCKSIZE   (64)
#endif /* SHA_BLOCKSIZE */

/** SHA-1 Context (OpenSSL compat) */
typedef ln_sha1_ctx_t SHA_CTX;

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

#if !defined(LN_SHA1_LITTLE_ENDIAN)
#define WORDS_BIGENDIAN 1
#endif /* LN_SHA1_LITTLE_ENDIAN */


/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
/* FIXME: can we do this in an endian-proof way? */
#ifdef WORDS_BIGENDIAN
#define blk0(i) block->l[i]
#else
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xff00ff00) |(rol(block->l[i],8)&0x00ff00ff))
#endif /* WORDS_BIGENDIAN */

#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15]^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v, w, x, y, z, i) \
    z+=((w&(x^y))^y)+blk0(i)+0x5a827999+rol(v,5);w=rol(w,30);
#define R1(v, w, x, y, z, i) \
    z+=((w&(x^y))^y)+blk(i)+0x5a827999+rol(v,5);w=rol(w,30);
#define R2(v, w, x, y, z, i) \
    z+=(w^x^y)+blk(i)+0x6ed9eba1+rol(v,5);w=rol(w,30);
#define R3(v, w, x, y, z, i) \
    z+=(((w|x)&y)|(w&x))+blk(i)+0x8f1bbcdc+rol(v,5);w=rol(w,30);
#define R4(v, w, x, y, z, i) \
    z+=(w^x^y)+blk(i)+0xca62c1d6+rol(v,5);w=rol(w,30);

typedef union {
    uint8_t c[64];
    uint32_t l[16];
} CHAR64LONG16;

#if (!(LN_SW_SHA1_USING_ROM_CODE))

/* Hash a single 512-bit block. This is the core of the algorithm. */
void ln_sha1_transform(uint32_t state[5], const uint8_t buffer[64])
{
    uint32_t a, b, c, d, e;
    CHAR64LONG16 *block = NULL, workspace = {0,};

    block = &workspace;

    memcpy(block, buffer, 64);

    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a, b, c, d, e, 0);
    R0(e, a, b, c, d, 1);
    R0(d, e, a, b, c, 2);
    R0(c, d, e, a, b, 3);
    R0(b, c, d, e, a, 4);
    R0(a, b, c, d, e, 5);
    R0(e, a, b, c, d, 6);
    R0(d, e, a, b, c, 7);
    R0(c, d, e, a, b, 8);
    R0(b, c, d, e, a, 9);
    R0(a, b, c, d, e, 10);
    R0(e, a, b, c, d, 11);
    R0(d, e, a, b, c, 12);
    R0(c, d, e, a, b, 13);
    R0(b, c, d, e, a, 14);
    R0(a, b, c, d, e, 15);
    R1(e, a, b, c, d, 16);
    R1(d, e, a, b, c, 17);
    R1(c, d, e, a, b, 18);
    R1(b, c, d, e, a, 19);
    R2(a, b, c, d, e, 20);
    R2(e, a, b, c, d, 21);
    R2(d, e, a, b, c, 22);
    R2(c, d, e, a, b, 23);
    R2(b, c, d, e, a, 24);
    R2(a, b, c, d, e, 25);
    R2(e, a, b, c, d, 26);
    R2(d, e, a, b, c, 27);
    R2(c, d, e, a, b, 28);
    R2(b, c, d, e, a, 29);
    R2(a, b, c, d, e, 30);
    R2(e, a, b, c, d, 31);
    R2(d, e, a, b, c, 32);
    R2(c, d, e, a, b, 33);
    R2(b, c, d, e, a, 34);
    R2(a, b, c, d, e, 35);
    R2(e, a, b, c, d, 36);
    R2(d, e, a, b, c, 37);
    R2(c, d, e, a, b, 38);
    R2(b, c, d, e, a, 39);
    R3(a, b, c, d, e, 40);
    R3(e, a, b, c, d, 41);
    R3(d, e, a, b, c, 42);
    R3(c, d, e, a, b, 43);
    R3(b, c, d, e, a, 44);
    R3(a, b, c, d, e, 45);
    R3(e, a, b, c, d, 46);
    R3(d, e, a, b, c, 47);
    R3(c, d, e, a, b, 48);
    R3(b, c, d, e, a, 49);
    R3(a, b, c, d, e, 50);
    R3(e, a, b, c, d, 51);
    R3(d, e, a, b, c, 52);
    R3(c, d, e, a, b, 53);
    R3(b, c, d, e, a, 54);
    R3(a, b, c, d, e, 55);
    R3(e, a, b, c, d, 56);
    R3(d, e, a, b, c, 57);
    R3(c, d, e, a, b, 58);
    R3(b, c, d, e, a, 59);
    R4(a, b, c, d, e, 60);
    R4(e, a, b, c, d, 61);
    R4(d, e, a, b, c, 62);
    R4(c, d, e, a, b, 63);
    R4(b, c, d, e, a, 64);
    R4(a, b, c, d, e, 65);
    R4(e, a, b, c, d, 66);
    R4(d, e, a, b, c, 67);
    R4(c, d, e, a, b, 68);
    R4(b, c, d, e, a, 69);
    R4(a, b, c, d, e, 70);
    R4(e, a, b, c, d, 71);
    R4(d, e, a, b, c, 72);
    R4(c, d, e, a, b, 73);
    R4(b, c, d, e, a, 74);
    R4(a, b, c, d, e, 75);
    R4(e, a, b, c, d, 76);
    R4(d, e, a, b, c, 77);
    R4(c, d, e, a, b, 78);
    R4(b, c, d, e, a, 79);

    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;

    /* Wipe variables */
    a = b = c = d = e = 0;
}


/**
* Initialize new context
*
* @param context SHA1-Context
*/
void ln_sha1_init(ln_sha1_ctx_t *context)
{
    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
    context->state[4] = 0xc3d2e1f0;
    context->count[0] = context->count[1] = 0;
}


/**
* Run your data through this
*
* @param context SHA1-Context
* @param p       Buffer to run SHA1 on
* @param len     Number of bytes
*/
void ln_sha1_update(ln_sha1_ctx_t *context, const void *p, size_t len)
{
    const uint8_t *data = p;
    size_t i, j;

    j = (context->count[0] >> 3) & 63;
    if ((context->count[0] += (uint32_t) (len << 3)) < (len << 3)) {
        context->count[1]++;
    }
    context->count[1] += (uint32_t) (len >> 29);
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64 - j));
        ln_sha1_transform(context->state, context->buffer);
        for (; i + 63 < len; i += 64) {
            ln_sha1_transform(context->state, data + i);
        }
        j = 0;
    }else{
        i = 0;
    }
    memcpy(&context->buffer[j], &data[i], len - i);
}


/**
* Add padding and return the message digest
*
* @param digest  Generated message digest
* @param context SHA1-Context
*/
void ln_sha1_final(uint8_t digest[LN_SHA1_DIGEST_SIZE], ln_sha1_ctx_t *context)
{
    uint32_t i;
    uint8_t finalcount[8] = {0,};

    for (i = 0; i < 8; i++) {
        finalcount[i] = (uint8_t) ((context->count[(i >= 4 ? 0 : 1)]
                >> ((3 - (i & 3)) * 8)) & 255);
    }
    ln_sha1_update(context, (uint8_t *) "\200", 1);
    while ((context->count[0] & 504) != 448) {
        ln_sha1_update(context, (uint8_t *) "\0", 1);
    }
    ln_sha1_update(context, finalcount, 8); /* Should cause ln_sha1_transform */
    for (i = 0; i < LN_SHA1_DIGEST_SIZE; i++) {
        digest[i] = (uint8_t)
                ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }

    /* Wipe variables */
    i = 0;
    memset(context->buffer, 0, 64);
    memset(context->state, 0, 20);
    memset(context->count, 0, 8);
    memset(finalcount, 0, 8);    /* SWR */

    ln_sha1_transform(context->state, context->buffer);
}

/**
* Function to compute the digest
*
* @param k   Secret key
* @param lk  Length of the key in bytes
* @param d   Data
* @param ld  Length of data in bytes
* @param out Digest output
* @param t   Size of digest output
*/
void ln_hmac_sha1(uint8_t *k,  /* secret key */
        size_t lk,             /* length of the key in bytes */
        uint8_t *d,            /* data */
        size_t ld,             /* length of data in bytes */
        uint8_t *out           /* output buffer, at least "t" bytes */
        )
{
    SHA_CTX ictx = {0,}, octx = {0,};
    uint8_t isha[SHA_DIGEST_LENGTH] = {0,}, osha[SHA_DIGEST_LENGTH] = {0,};
    uint8_t key[SHA_DIGEST_LENGTH] = {0,};
    uint8_t buf[SHA_BLOCKSIZE] = {0,};
    size_t i;

    if (lk > SHA_BLOCKSIZE) {
        SHA_CTX tctx = {0,};

        ln_sha1_init(&tctx);
        ln_sha1_update(&tctx, k, lk);
        ln_sha1_final(key, &tctx);

        k = key;
        lk = SHA_DIGEST_LENGTH;
    }

    /**** Inner Digest ****/

    ln_sha1_init(&ictx);

    /* Pad the key for inner digest */
    for (i = 0; i < lk; ++i) {
        buf[i] = k[i] ^ 0x36;
    }
    for (i = lk; i < SHA_BLOCKSIZE; ++i) {
        buf[i] = 0x36;
    }

    ln_sha1_update(&ictx, buf, SHA_BLOCKSIZE);
    ln_sha1_update(&ictx, d, ld);

    ln_sha1_final(isha, &ictx);

    /**** Outer Digest ****/

    ln_sha1_init(&octx);

    /* Pad the key for outter digest */

    for (i = 0; i < lk; ++i) {
        buf[i] = k[i] ^ 0x5c;
    }
    for (i = lk; i < SHA_BLOCKSIZE; ++i) {
        buf[i] = 0x5c;
    }

    ln_sha1_update(&octx, buf, SHA_BLOCKSIZE);
    ln_sha1_update(&octx, isha, SHA_DIGEST_LENGTH);

    ln_sha1_final(osha, &octx);
    memcpy(out, osha, SHA_DIGEST_LENGTH);
}
#else
#include "ln882h_rom_fun.h"

typedef void (*rom_func_sha1_init)(ln_sha1_ctx_t *);
typedef void (*rom_func_sha1_update)(ln_sha1_ctx_t *, const void *, size_t);
typedef void (*rom_func_sha1_transform)(uint32_t *, const uint8_t *);
typedef void (*rom_func_sha1_final)(uint8_t *, ln_sha1_ctx_t *);
typedef void (*rom_func_hmac_sha1)(uint8_t *, size_t, uint8_t *, size_t, uint8_t *);

void ln_sha1_init(ln_sha1_ctx_t *context)
{
    rom_func_sha1_init sha1_init = (rom_func_sha1_init)ROM_FUN_SHA1_INIT;
    sha1_init(context);
}

void ln_sha1_update(ln_sha1_ctx_t *context, const void *p, size_t len)
{
    rom_func_sha1_update sha1_update = (rom_func_sha1_update)ROM_FUN_SHA1_UPDATE;
    sha1_update(context, p, len);
}

void ln_sha1_transform(uint32_t state[5], const uint8_t buffer[64])
{
    rom_func_sha1_transform sha1_transform = (rom_func_sha1_transform)ROM_FUN_SHA1_TRANSFORM;
    sha1_transform(state, buffer);
}

void ln_sha1_final(uint8_t digest[LN_SHA1_DIGEST_SIZE], ln_sha1_ctx_t *context)
{
    rom_func_sha1_final sha1_final = (rom_func_sha1_final)ROM_FUN_SHA1_FINAL;
    sha1_final(digest, context);
}

void ln_hmac_sha1(uint8_t  * k,    /* secret key */
                  size_t     lk,   /* length of the key in bytes */
                  uint8_t  * d,    /* data */
                  size_t     ld,   /* length of data in bytes */
                  uint8_t  * out   /* output buffer, at least "t" bytes */
                  )
{
    rom_func_hmac_sha1 hmac_sha1 = (rom_func_hmac_sha1)ROM_FUN_HMAC_SHA1;
    hmac_sha1(k, lk, d, ld, out);
}
#endif /* !(LN_SW_SHA1_USING_ROM_CODE) */

#if defined (__CC_ARM)
#pragma pop
#elif defined (__GNUC__)
#pragma GCC pop_options
#endif
