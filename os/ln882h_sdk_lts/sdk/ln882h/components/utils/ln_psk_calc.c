
#include "ln_compiler.h"
#include "ln_psk_calc.h"
#include "ln_sha1.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#undef  MAX_PSK_PASS_PHRASE_LEN
#define MAX_PSK_PASS_PHRASE_LEN           (64u)

/* Convert char to hex */
__STATIC_INLINE__ uint8_t char_2_hex(uint8_t val)
{
	if(val - '0' <= 9)
	{
		return (val - '0');
	}
	else if('f' - val <= 6)
	{
		return (val - 'a' + 10);
	}
	else
	{
		return (val - 'A' + 10);
	}
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name    : pbkdf2_sha1_f                                         */
/*                                                                           */
/*  Description      : This function iterates the hashing of digest as per   */
/*                     the key                                               */
/*                                                                           */
/*  Inputs           : 1) key                                                */
/*                     2) data                                               */
/*                     3) key len                                            */
/*                     4) iterations                                         */
/*                     5) count                                              */
/*                     6) digest                                             */
/*                                                                           */
/*  Globals          : None                                                  */
/*                                                                           */
/*  Processing       : F(P, S, c, i) = U1 xor U2 xor ... Uc                  */
/*                     U1 = PRF(P, S || Int(i))                              */
/*                     U2 = PRF(P, U1)                                       */
/*                     Uc = PRF(P, Uc-1)                                     */
/*                                                                           */
/*  Outputs          : Iterated hashed password                              */
/*                                                                           */
/*  Returns          : Returns 0 on failure 1 otherwise returns 1            */
/*                                                                           */
/*  Issues           : None                                                  */
/*                                                                           */
/*****************************************************************************/
static uint8_t pbkdf2_sha1_f(uint8_t *key, uint8_t key_length,
                     uint8_t *data, uint32_t data_len,
                   uint32_t iterations, uint32_t count, uint8_t *digest)
{
    uint8_t tmp1[36]    = {0,};
    uint8_t tmp2[LN_SHA1_DIGEST_SIZE]    = {0,};
    uint16_t i, j;

    for (i = 0; i < key_length ; i++)
    {
        if(key[i] < 32)
        {
            return 0;
        }
        else if(key[i] > 126)
        {
            return 0;
        }
    }

    /* U1 = PRF(P, S || int(i)) */
    memcpy(tmp1, data, data_len);

    tmp1[data_len]   = (uint8_t)((count>>24) & 0xFF);
    tmp1[data_len+1] = (uint8_t)((count>>16) & 0xFF);
    tmp1[data_len+2] = (uint8_t)((count>>8) & 0xFF);
    tmp1[data_len+3] = (uint8_t)(count & 0xFF);

    ln_hmac_sha1(key, key_length, tmp1, (data_len + 4), tmp2);

    /* digest = U1 */
    memcpy(digest, tmp2, LN_SHA1_DIGEST_SIZE);

    for (i = 1; i < iterations; i++)
    {
        /* Un = PRF(P, Un-1) */
        ln_hmac_sha1(key, key_length, tmp2, LN_SHA1_DIGEST_SIZE, tmp1);

        memcpy(tmp2, tmp1, LN_SHA1_DIGEST_SIZE);

        /* digest = digest xor Un */
        for (j = 0; j < LN_SHA1_DIGEST_SIZE; j++)
        {
            digest[j] ^= tmp1[j];
        }
    }

    return 1;
}

/* This function runs the PBKDF2 on the data with a given                    */
/* key. To obtain the PSK from the password, PBKDF2 is                       */
/* run on SSID (data) with password (key)                                    */
__STATIC_INLINE__ uint8_t pbkdf2_sha1(uint8_t *key, uint8_t key_len,
                          uint8_t *data, uint8_t data_len,
                     uint8_t *digest)
{
    if ((key_len > 64) || (data_len > 32))
        return 0;

    if(pbkdf2_sha1_f(key, key_len, data, data_len, 4096, 1, digest) == 0)
    {
        return 0;
    }
    return pbkdf2_sha1_f(key, key_len, data, data_len,
                            4096, 2, &digest[LN_SHA1_DIGEST_SIZE]);
}

/**
 * @brief calc psk with ssid and pwd
 *
 * @param ssid
 * @param pwd
 * @param psk_buff
 * @return int    -1: fail; 0: success
 */
int ln_psk_calc(const char *ssid, const char *pwd, uint8_t *psk_buff, uint8_t psk_buff_size)
{
    uint8_t ssid_len = 0;
    uint8_t pwd_len  = 0;

    if ((NULL == psk_buff) || (NULL == ssid) || (NULL == pwd) ||
        (psk_buff_size < MAX_PSK_PASS_PHRASE_LEN/2)) {
        return -1;
    }

    ssid_len = strlen(ssid);
    pwd_len  = strlen(pwd);

    if (MAX_PSK_PASS_PHRASE_LEN > pwd_len) {
        pbkdf2_sha1((void *)pwd, pwd_len, (void *)ssid, ssid_len, (void *)psk_buff);
    } else if (MAX_PSK_PASS_PHRASE_LEN == pwd_len) {
        uint8_t i = 0;
        for (i = 0; i < (MAX_PSK_PASS_PHRASE_LEN/2); i++) {
            psk_buff[i] = char_2_hex(pwd[2*i]) * 16 + char_2_hex(pwd[2*i + 1]);
        }
    } else {
        return -1;
    }

    return 0;
}
