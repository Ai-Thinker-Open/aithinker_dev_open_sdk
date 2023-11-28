#include "utils/ln_misc.h"
#include "ln_types.h"
#include "ln_utils.h"
#include "hal/hal_trng.h"

uint8_t ln_char2hex(char c)
{
    if(c - '0' <= 9) {
        return (c - '0');
    } else if('f' - c <= 6) {
        return (c - 'a' + 10);
    } else {
        return (c - 'A' + 10);
    }
}

int ln_char2hex_safe(char c, uint8_t *hex)
{
    if (!isxdigit(c)) {
        return LN_FALSE;
    }
    *hex = ln_char2hex(c);
    return LN_TRUE;
}

int ln_str2bytes(uint8_t *bytes, const char *str)
{
    uint8_t byte = 0;
    int i = 0;

    while ('\0' != str[i])
    {
        if (!isxdigit((int)str[i])) {
            return LN_FALSE;
        }
        byte = ln_char2hex(str[i]);
        if (0 == i++ % 2) {
            *bytes = (byte << 4) & 0xF0;
        } else {
            *bytes++ += byte & 0x0F;
        }
    }

    return LN_TRUE;
}

int ln_is_valid_mac_str(const char* mac_str)
{
    int i = 0, s = 0;
    char *ptr = (char *)mac_str;

    if (mac_str == NULL) {
        return LN_FALSE;
    }

    if (strlen(mac_str) != 17) {
        return LN_FALSE;
    }

    while (*ptr)
    {
        if (isxdigit( (int) *ptr) ) {
            i++;
        } else if ((*ptr == ':') || (*ptr == '-')) {
            if ((i == 0) || ((i/2 - 1) != s)) {
                break;
            }
            ++s;
        } else {
            s = -1;
        }
        ++ptr;
    }

    return ((i == 12) && (s == 5 || s == 0) ? LN_TRUE : LN_FALSE);
}

int ln_mac_str2hex(const uint8_t *str, uint8_t *hex)
{
    const uint8_t *tmp = str;
    if (LN_TRUE != ln_is_valid_mac_str((const char*)tmp)) {
        return LN_FALSE;
    }

    for (int i = 0; i < 6; ++i) {
        hex[i]  = ln_char2hex(*tmp++) * 16;
        hex[i] += ln_char2hex(*tmp++);
        tmp++;
    }
    return LN_TRUE;
}

int ln_is_valid_mac(const char *mac)
{
    if (mac == NULL) {
        return LN_FALSE;
    }

    if(!memcmp(mac, "\x00\x00\x00\x00\x00\x00", 6)) {
        return LN_FALSE;
    }
    if(!memcmp(mac, "\xFF\xFF\xFF\xFF\xFF\xFF", 6)) {
        return LN_FALSE;
    }

    return LN_TRUE;
}

int ln_is_zero_mem(const void *addr, uint32_t size)
{
    uint8_t *_addr = (uint8_t *)addr;

    if (!addr) {
        return LN_FALSE;
    }

    for (uint32_t i = 0; i < size; i++) {
        if(_addr[i]) {
            return LN_FALSE;
        }
    }
    return LN_TRUE;
}

int ln_generate_true_random_words(uint32_t *words, uint32_t words_len)
{
    LN_UNUSED(words_len);

    if(!words && ((((uint32_t)words) % sizeof(uint32_t)) != 0)){
        return LN_FALSE;
    }

//    trng_init_t_def trng_init;
//    trng_init.fast_mode_en = TRNG_FAST_MODE_DIS;
//    trng_init.gap = 10;
//    hal_trng_init(REG_TRNG_BASE, &trng_init);
//
//    for (uint32_t i = 0; i < words_len; i++) {
//        hal_trng_en(REG_TRNG_BASE, HAL_ENABLE);
//        while(!hal_trng_is_complate(REG_TRNG_BASE));
//        *words++ = hal_trng_get_data_0_31(REG_TRNG_BASE);
//    }
    return LN_TRUE;
}

int ln_generate_random_mac(uint8_t *addr)
{
    #define LN_MAC_OUI0    0x00
    #define LN_MAC_OUI1    0x50
    #define LN_MAC_OUI2    0xC2

    if(!addr){
        return -1;
    }

    trng_init_t_def trng_init;
    memset(&trng_init, 0, sizeof(trng_init));
    trng_init.trng_fast_mode_en_status = TRNG_FAST_MODE_DIS;
    trng_init.trng_gap = 10;
    hal_trng_init(TRNG_BASE, &trng_init);
    hal_trng_en(TRNG_BASE, HAL_ENABLE);
    while(hal_trng_get_en_status(TRNG_BASE) == 1);
    while(hal_trng_get_it_flag(TRNG_BASE, TRNG_IT_FLAG_TRNG_DONE) == 0);

#if defined(LN_MAC_OUI0) && defined(LN_MAC_OUI1) && defined(LN_MAC_OUI2)
    *addr++ = LN_MAC_OUI0;
    *addr++ = LN_MAC_OUI1;
    *addr++ = LN_MAC_OUI2;

   *addr++ = (uint8_t)hal_trng_get_data_0_31(TRNG_BASE);
   *addr++ = (uint8_t)hal_trng_get_data_32_63(TRNG_BASE);
   *addr   = (uint8_t)hal_trng_get_data_64_95(TRNG_BASE);
#else
    *addr++ = (uint8_t) hal_trng_get_data_0_31(TRNG_BASE);
    *addr++ = (uint8_t)(hal_trng_get_data_0_31(TRNG_BASE) >> 8U);

    *addr++ = (uint8_t) hal_trng_get_data_32_63(TRNG_BASE);
    *addr++ = (uint8_t)(hal_trng_get_data_32_63(TRNG_BASE) >> 8U);

    *addr++ = (uint8_t)hal_trng_get_data_64_95(TRNG_BASE);
    *addr   = (uint8_t)hal_trng_get_data_96_127(TRNG_BASE);

    CLR_BIT(addr[0],0);
    CLR_BIT(addr[0],1);
#endif
    hal_trng_en(TRNG_BASE,HAL_DISABLE);
    return 0;
}

