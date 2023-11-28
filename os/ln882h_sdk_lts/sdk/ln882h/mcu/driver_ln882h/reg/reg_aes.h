#ifndef __REG_AES_H__
#define __REG_AES_H__


#include "ln882h.h"
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ptr0                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ptr0;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ptr1                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ptr1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ptr2                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ptr2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ptr3                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ptr3;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ikr0                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ikr0;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ikr1                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ikr1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ikr2                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ikr2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ikr3                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ikr3;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ikr4                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ikr4;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ikr5                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ikr5;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ikr6                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ikr6;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ikr7                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ikr7;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    iv0                 : 32 ; // 31 : 0
    } bit_field;
} t_aes_iv0;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    iv1                 : 32 ; // 31 : 0
    } bit_field;
} t_aes_iv1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    iv2                 : 32 ; // 31 : 0
    } bit_field;
} t_aes_iv2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    iv3                 : 32 ; // 31 : 0
    } bit_field;
} t_aes_iv3;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    start               : 1  ; // 0  : 0
        uint32_t    key_int_en          : 1  ; // 1  : 1
        uint32_t    data_int_en         : 1  ; // 2  : 2
        uint32_t    cbc                 : 1  ; // 3  : 3
        uint32_t    keylen              : 2  ; // 5  : 4
        uint32_t    opcode              : 2  ; // 7  : 6
        uint32_t    bigendian           : 1  ; // 8  : 8
        uint32_t    bitendian           : 1  ; // 9  : 9
        uint32_t    reserved_0          : 22 ; // 31 : 10
    } bit_field;
} t_aes_ctrl;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    busy                : 1  ; // 0  : 0
        uint32_t    key_int_flg         : 1  ; // 1  : 1
        uint32_t    data_int_flg        : 1  ; // 2  : 2
        uint32_t    reserved_0          : 29 ; // 31 : 3
    } bit_field;
} t_aes_state;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ctr0                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ctr0;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ctr1                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ctr1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ctr2                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ctr2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ctr3                : 32 ; // 31 : 0
    } bit_field;
} t_aes_ctr3;


//--------------------------------------------------------------------------------

typedef struct
{
    volatile        t_aes_ptr0                               ptr0                                   ; // 0x0
    volatile        t_aes_ptr1                               ptr1                                   ; // 0x4
    volatile        t_aes_ptr2                               ptr2                                   ; // 0x8
    volatile        t_aes_ptr3                               ptr3                                   ; // 0xc
    volatile        t_aes_ikr0                               ikr0                                   ; // 0x10
    volatile        t_aes_ikr1                               ikr1                                   ; // 0x14
    volatile        t_aes_ikr2                               ikr2                                   ; // 0x18
    volatile        t_aes_ikr3                               ikr3                                   ; // 0x1c
    volatile        t_aes_ikr4                               ikr4                                   ; // 0x20
    volatile        t_aes_ikr5                               ikr5                                   ; // 0x24
    volatile        t_aes_ikr6                               ikr6                                   ; // 0x28
    volatile        t_aes_ikr7                               ikr7                                   ; // 0x2c
    volatile        t_aes_iv0                                iv0                                    ; // 0x30
    volatile        t_aes_iv1                                iv1                                    ; // 0x34
    volatile        t_aes_iv2                                iv2                                    ; // 0x38
    volatile        t_aes_iv3                                iv3                                    ; // 0x3c
    volatile        t_aes_ctrl                               ctrl                                   ; // 0x40
    volatile        t_aes_state                              state                                  ; // 0x44
    volatile        t_aes_ctr0                               ctr0                                   ; // 0x48
    volatile        t_aes_ctr1                               ctr1                                   ; // 0x4c
    volatile        t_aes_ctr2                               ctr2                                   ; // 0x50
    volatile        t_aes_ctr3                               ctr3                                   ; // 0x54
} t_hwp_aes_t;


//--------------------------------------------------------------------------------

__STATIC_INLINE void aes_ptr0_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->ptr0.val = value;
}

__STATIC_INLINE void aes_ptr0_setf(uint32_t base, uint32_t ptr0)
{
    ((t_hwp_aes_t*)(base))->ptr0.bit_field.ptr0 = ptr0;
}

__STATIC_INLINE void aes_ptr1_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->ptr1.val = value;
}

__STATIC_INLINE void aes_ptr1_setf(uint32_t base, uint32_t ptr1)
{
    ((t_hwp_aes_t*)(base))->ptr1.bit_field.ptr1 = ptr1;
}

__STATIC_INLINE void aes_ptr2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->ptr2.val = value;
}

__STATIC_INLINE void aes_ptr2_setf(uint32_t base, uint32_t ptr2)
{
    ((t_hwp_aes_t*)(base))->ptr2.bit_field.ptr2 = ptr2;
}

__STATIC_INLINE void aes_ptr3_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->ptr3.val = value;
}

__STATIC_INLINE void aes_ptr3_setf(uint32_t base, uint32_t ptr3)
{
    ((t_hwp_aes_t*)(base))->ptr3.bit_field.ptr3 = ptr3;
}

__STATIC_INLINE void aes_ikr0_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->ikr0.val = value;
}

__STATIC_INLINE void aes_ikr0_setf(uint32_t base, uint32_t ikr0)
{
    ((t_hwp_aes_t*)(base))->ikr0.bit_field.ikr0 = ikr0;
}

__STATIC_INLINE void aes_ikr1_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->ikr1.val = value;
}

__STATIC_INLINE void aes_ikr1_setf(uint32_t base, uint32_t ikr1)
{
    ((t_hwp_aes_t*)(base))->ikr1.bit_field.ikr1 = ikr1;
}

__STATIC_INLINE void aes_ikr2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->ikr2.val = value;
}

__STATIC_INLINE void aes_ikr2_setf(uint32_t base, uint32_t ikr2)
{
    ((t_hwp_aes_t*)(base))->ikr2.bit_field.ikr2 = ikr2;
}

__STATIC_INLINE void aes_ikr3_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->ikr3.val = value;
}

__STATIC_INLINE void aes_ikr3_setf(uint32_t base, uint32_t ikr3)
{
    ((t_hwp_aes_t*)(base))->ikr3.bit_field.ikr3 = ikr3;
}

__STATIC_INLINE void aes_ikr4_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->ikr4.val = value;
}

__STATIC_INLINE void aes_ikr4_setf(uint32_t base, uint32_t ikr4)
{
    ((t_hwp_aes_t*)(base))->ikr4.bit_field.ikr4 = ikr4;
}

__STATIC_INLINE void aes_ikr5_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->ikr5.val = value;
}

__STATIC_INLINE void aes_ikr5_setf(uint32_t base, uint32_t ikr5)
{
    ((t_hwp_aes_t*)(base))->ikr5.bit_field.ikr5 = ikr5;
}

__STATIC_INLINE void aes_ikr6_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->ikr6.val = value;
}

__STATIC_INLINE void aes_ikr6_setf(uint32_t base, uint32_t ikr6)
{
    ((t_hwp_aes_t*)(base))->ikr6.bit_field.ikr6 = ikr6;
}

__STATIC_INLINE void aes_ikr7_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->ikr7.val = value;
}

__STATIC_INLINE void aes_ikr7_setf(uint32_t base, uint32_t ikr7)
{
    ((t_hwp_aes_t*)(base))->ikr7.bit_field.ikr7 = ikr7;
}

__STATIC_INLINE void aes_iv0_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->iv0.val = value;
}

__STATIC_INLINE void aes_iv0_setf(uint32_t base, uint32_t iv0)
{
    ((t_hwp_aes_t*)(base))->iv0.bit_field.iv0 = iv0;
}

__STATIC_INLINE void aes_iv1_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->iv1.val = value;
}

__STATIC_INLINE void aes_iv1_setf(uint32_t base, uint32_t iv1)
{
    ((t_hwp_aes_t*)(base))->iv1.bit_field.iv1 = iv1;
}

__STATIC_INLINE void aes_iv2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->iv2.val = value;
}

__STATIC_INLINE void aes_iv2_setf(uint32_t base, uint32_t iv2)
{
    ((t_hwp_aes_t*)(base))->iv2.bit_field.iv2 = iv2;
}

__STATIC_INLINE void aes_iv3_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->iv3.val = value;
}

__STATIC_INLINE void aes_iv3_setf(uint32_t base, uint32_t iv3)
{
    ((t_hwp_aes_t*)(base))->iv3.bit_field.iv3 = iv3;
}

__STATIC_INLINE void aes_ctrl_set(uint32_t base, uint32_t value)
{
    ((t_hwp_aes_t*)(base))->ctrl.val = value;
}

__STATIC_INLINE void aes_bitendian_setf(uint32_t base, uint8_t bitendian)
{
    ((t_hwp_aes_t*)(base))->ctrl.bit_field.bitendian = bitendian;
}

__STATIC_INLINE void aes_bigendian_setf(uint32_t base, uint8_t bigendian)
{
    ((t_hwp_aes_t*)(base))->ctrl.bit_field.bigendian = bigendian;
}

__STATIC_INLINE void aes_opcode_setf(uint32_t base, uint8_t opcode)
{
    ((t_hwp_aes_t*)(base))->ctrl.bit_field.opcode = opcode;
}

__STATIC_INLINE void aes_keylen_setf(uint32_t base, uint8_t keylen)
{
    ((t_hwp_aes_t*)(base))->ctrl.bit_field.keylen = keylen;
}

__STATIC_INLINE void aes_cbc_setf(uint32_t base, uint8_t cbc)
{
    ((t_hwp_aes_t*)(base))->ctrl.bit_field.cbc = cbc;
}

__STATIC_INLINE void aes_data_int_en_setf(uint32_t base, uint8_t data_int_en)
{
    ((t_hwp_aes_t*)(base))->ctrl.bit_field.data_int_en = data_int_en;
}

__STATIC_INLINE void aes_key_int_en_setf(uint32_t base, uint8_t key_int_en)
{
    ((t_hwp_aes_t*)(base))->ctrl.bit_field.key_int_en = key_int_en;
}

__STATIC_INLINE void aes_start_setf(uint32_t base, uint8_t start)
{
    ((t_hwp_aes_t*)(base))->ctrl.bit_field.start = start;
}

__STATIC_INLINE void aes_key_int_clr_setf(uint32_t base, uint8_t key_int_flag)
{
    ((t_hwp_aes_t*)(base))->state.bit_field.key_int_flg = key_int_flag;
}

__STATIC_INLINE void aes_data_int_clr_setf(uint32_t base, uint8_t data_int_flag)
{
    ((t_hwp_aes_t*)(base))->state.bit_field.data_int_flg = data_int_flag;
}
//--------------------------------------------------------------------------------

__STATIC_INLINE uint32_t aes_ptr0_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ptr0.val;
}

__STATIC_INLINE uint32_t aes_ptr0_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ptr0.bit_field.ptr0;
}

__STATIC_INLINE uint32_t aes_ptr1_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ptr1.val;
}

__STATIC_INLINE uint32_t aes_ptr1_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ptr1.bit_field.ptr1;
}

__STATIC_INLINE uint32_t aes_ptr2_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ptr2.val;
}

__STATIC_INLINE uint32_t aes_ptr2_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ptr2.bit_field.ptr2;
}

__STATIC_INLINE uint32_t aes_ptr3_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ptr3.val;
}

__STATIC_INLINE uint32_t aes_ptr3_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ptr3.bit_field.ptr3;
}

__STATIC_INLINE uint32_t aes_ikr0_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr0.val;
}

__STATIC_INLINE uint32_t aes_ikr0_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr0.bit_field.ikr0;
}

__STATIC_INLINE uint32_t aes_ikr1_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr1.val;
}

__STATIC_INLINE uint32_t aes_ikr1_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr1.bit_field.ikr1;
}

__STATIC_INLINE uint32_t aes_ikr2_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr2.val;
}

__STATIC_INLINE uint32_t aes_ikr2_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr2.bit_field.ikr2;
}

__STATIC_INLINE uint32_t aes_ikr3_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr3.val;
}

__STATIC_INLINE uint32_t aes_ikr3_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr3.bit_field.ikr3;
}

__STATIC_INLINE uint32_t aes_ikr4_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr4.val;
}

__STATIC_INLINE uint32_t aes_ikr4_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr4.bit_field.ikr4;
}

__STATIC_INLINE uint32_t aes_ikr5_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr5.val;
}

__STATIC_INLINE uint32_t aes_ikr5_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr5.bit_field.ikr5;
}

__STATIC_INLINE uint32_t aes_ikr6_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr6.val;
}

__STATIC_INLINE uint32_t aes_ikr6_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr6.bit_field.ikr6;
}

__STATIC_INLINE uint32_t aes_ikr7_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr7.val;
}

__STATIC_INLINE uint32_t aes_ikr7_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ikr7.bit_field.ikr7;
}

__STATIC_INLINE uint32_t aes_iv0_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->iv0.val;
}

__STATIC_INLINE uint32_t aes_iv0_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->iv0.bit_field.iv0;
}

__STATIC_INLINE uint32_t aes_iv1_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->iv1.val;
}

__STATIC_INLINE uint32_t aes_iv1_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->iv1.bit_field.iv1;
}

__STATIC_INLINE uint32_t aes_iv2_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->iv2.val;
}

__STATIC_INLINE uint32_t aes_iv2_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->iv2.bit_field.iv2;
}

__STATIC_INLINE uint32_t aes_iv3_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->iv3.val;
}

__STATIC_INLINE uint32_t aes_iv3_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->iv3.bit_field.iv3;
}

__STATIC_INLINE uint32_t aes_ctrl_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctrl.val;
}

__STATIC_INLINE uint8_t aes_bitendian_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctrl.bit_field.bitendian;
}

__STATIC_INLINE uint8_t aes_bigendian_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctrl.bit_field.bigendian;
}

__STATIC_INLINE uint8_t aes_opcode_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctrl.bit_field.opcode;
}

__STATIC_INLINE uint8_t aes_keylen_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctrl.bit_field.keylen;
}

__STATIC_INLINE uint8_t aes_cbc_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctrl.bit_field.cbc;
}

__STATIC_INLINE uint8_t aes_data_int_en_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctrl.bit_field.data_int_en;
}

__STATIC_INLINE uint8_t aes_key_int_en_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctrl.bit_field.key_int_en;
}

__STATIC_INLINE uint8_t aes_start_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctrl.bit_field.start;
}

__STATIC_INLINE uint32_t aes_state_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->state.val;
}

__STATIC_INLINE uint8_t aes_busy_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->state.bit_field.busy;
}

__STATIC_INLINE uint8_t aes_key_int_flg_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->state.bit_field.key_int_flg;
}

__STATIC_INLINE uint8_t aes_data_int_flg_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->state.bit_field.data_int_flg;
}

__STATIC_INLINE uint32_t aes_ctr0_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctr0.val;
}

__STATIC_INLINE uint32_t aes_ctr0_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctr0.bit_field.ctr0;
}

__STATIC_INLINE uint32_t aes_ctr1_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctr1.val;
}

__STATIC_INLINE uint32_t aes_ctr1_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctr1.bit_field.ctr1;
}

__STATIC_INLINE uint32_t aes_ctr2_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctr2.val;
}

__STATIC_INLINE uint32_t aes_ctr2_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctr2.bit_field.ctr2;
}

__STATIC_INLINE uint32_t aes_ctr3_get(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctr3.val;
}

__STATIC_INLINE uint32_t aes_ctr3_getf(uint32_t base)
{
    return ((t_hwp_aes_t*)(base))->ctr3.bit_field.ctr3;
}




//--------------------------------------------------------------------------------

__STATIC_INLINE void aes_ctrl_pack(uint32_t base, uint8_t bitendian, uint8_t bigendian, uint8_t opcode, uint8_t keylen, uint8_t cbc, uint8_t data_int_en, uint8_t key_int_en, uint8_t start)
{
    ((t_hwp_aes_t*)(base))->ctrl.val = ( \
                                    ((uint32_t )bitendian         << 9) \
                                  | ((uint32_t )bigendian         << 8) \
                                  | ((uint32_t )opcode            << 6) \
                                  | ((uint32_t )keylen            << 4) \
                                  | ((uint32_t )cbc               << 3) \
                                  | ((uint32_t )data_int_en       << 2) \
                                  | ((uint32_t )key_int_en        << 1) \
                                  | ((uint32_t )start) \
                                  );
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void aes_ctrl_unpack(uint32_t base, uint8_t * bitendian, uint8_t * bigendian, uint8_t * opcode, uint8_t * keylen, uint8_t * cbc, uint8_t * data_int_en, uint8_t * key_int_en, uint8_t * start)
{
    t_aes_ctrl local_val = ((t_hwp_aes_t*)(base))->ctrl;

    *bitendian          = local_val.bit_field.bitendian;
    *bigendian          = local_val.bit_field.bigendian;
    *opcode             = local_val.bit_field.opcode;
    *keylen             = local_val.bit_field.keylen;
    *cbc                = local_val.bit_field.cbc;
    *data_int_en        = local_val.bit_field.data_int_en;
    *key_int_en         = local_val.bit_field.key_int_en;
    *start              = local_val.bit_field.start;
}

#endif

