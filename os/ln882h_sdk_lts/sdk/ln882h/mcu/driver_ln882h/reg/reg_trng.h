#ifndef __REG_TRNG_H__
#define __REG_TRNG_H__

#include "ln882h.h"

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    trng_data_1         : 32 ; // 31 : 0
    } bit_field;
} t_trng_data1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    trng_data_2         : 32 ; // 31 : 0
    } bit_field;
} t_trng_data2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    trng_data_3         : 32 ; // 31 : 0
    } bit_field;
} t_trng_data3;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    trng_data_4         : 32 ; // 31 : 0
    } bit_field;
} t_trng_data4;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    trng_en             : 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_trng_enr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    trng_gap            : 4  ; // 3  : 0
        uint32_t    fast_trng           : 1  ; // 4  : 4
        uint32_t    reserved_0          : 27 ; // 31 : 5
    } bit_field;
} t_trng_cr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    trng_ie             : 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_trng_ier;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    trng_done           : 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_trng_isr;


//--------------------------------------------------------------------------------

typedef struct
{
    volatile        t_trng_data1                             trng_data1                             ; // 0x0
    volatile        t_trng_data2                             trng_data2                             ; // 0x4
    volatile        t_trng_data3                             trng_data3                             ; // 0x8
    volatile        t_trng_data4                             trng_data4                             ; // 0xc
    volatile        t_trng_enr                               trng_enr                               ; // 0x10
    volatile        t_trng_cr                                trng_cr                                ; // 0x14
    volatile        t_trng_ier                               trng_ier                               ; // 0x18
    volatile        t_trng_isr                               trng_isr                               ; // 0x1c
} t_hwp_trng_t;


//--------------------------------------------------------------------------------

__STATIC_INLINE void trng_enr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_trng_t*)(base))->trng_enr.val = value;
}

__STATIC_INLINE void trng_en_setf(uint32_t base, uint8_t trng_en)
{
    ((t_hwp_trng_t*)(base))->trng_enr.bit_field.trng_en = trng_en;
}

__STATIC_INLINE void trng_cr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_trng_t*)(base))->trng_cr.val = value;
}

__STATIC_INLINE void trng_fast_trng_setf(uint32_t base, uint8_t fast_trng)
{
    ((t_hwp_trng_t*)(base))->trng_cr.bit_field.fast_trng = fast_trng;
}

__STATIC_INLINE void trng_gap_setf(uint32_t base, uint8_t trng_gap)
{
    ((t_hwp_trng_t*)(base))->trng_cr.bit_field.trng_gap = trng_gap;
}

__STATIC_INLINE void trng_ier_set(uint32_t base, uint32_t value)
{
    ((t_hwp_trng_t*)(base))->trng_ier.val = value;
}

__STATIC_INLINE void trng_ie_setf(uint32_t base, uint8_t trng_ie)
{
    ((t_hwp_trng_t*)(base))->trng_ier.bit_field.trng_ie = trng_ie;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE uint32_t trng_data1_get(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_data1.val;
}

__STATIC_INLINE uint32_t trng_data_1_getf(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_data1.bit_field.trng_data_1;
}

__STATIC_INLINE uint32_t trng_data2_get(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_data2.val;
}

__STATIC_INLINE uint32_t trng_data_2_getf(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_data2.bit_field.trng_data_2;
}

__STATIC_INLINE uint32_t trng_data3_get(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_data3.val;
}

__STATIC_INLINE uint32_t trng_data_3_getf(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_data3.bit_field.trng_data_3;
}

__STATIC_INLINE uint32_t trng_data4_get(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_data4.val;
}

__STATIC_INLINE uint32_t trng_data_4_getf(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_data4.bit_field.trng_data_4;
}

__STATIC_INLINE uint32_t trng_enr_get(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_enr.val;
}

__STATIC_INLINE uint8_t trng_en_getf(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_enr.bit_field.trng_en;
}

__STATIC_INLINE uint32_t trng_cr_get(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_cr.val;
}

__STATIC_INLINE uint8_t trng_fast_trng_getf(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_cr.bit_field.fast_trng;
}

__STATIC_INLINE uint8_t trng_gap_getf(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_cr.bit_field.trng_gap;
}

__STATIC_INLINE uint32_t trng_ier_get(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_ier.val;
}

__STATIC_INLINE uint8_t trng_ie_getf(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_ier.bit_field.trng_ie;
}

__STATIC_INLINE uint32_t trng_isr_get(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_isr.val;
}

__STATIC_INLINE uint8_t trng_done_getf(uint32_t base)
{
    return ((t_hwp_trng_t*)(base))->trng_isr.bit_field.trng_done;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void trng_cr_pack(uint32_t base, uint8_t fast_trng, uint8_t trng_gap)
{
    ((t_hwp_trng_t*)(base))->trng_cr.val = ( \
                                    ((uint32_t )fast_trng         << 4) \
                                  | ((uint32_t )trng_gap) \
                                  );
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void trng_cr_unpack(uint32_t base, uint8_t * fast_trng, uint8_t * trng_gap)
{
    t_trng_cr local_val = ((t_hwp_trng_t*)(base))->trng_cr;

    *fast_trng          = local_val.bit_field.fast_trng;
    *trng_gap           = local_val.bit_field.trng_gap;
}

#endif

