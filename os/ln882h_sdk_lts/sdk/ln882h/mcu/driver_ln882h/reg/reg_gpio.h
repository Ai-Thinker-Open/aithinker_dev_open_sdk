#ifndef __REG_GPIO_H__
#define __REG_GPIO_H__

#include "ln882h.h"

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pu                  : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_gpio_pu;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pd                  : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_gpio_pd;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    hp                  : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_gpio_speed;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    dsel                : 16 ; // 15 : 0
        uint32_t    afio_en             : 16 ; // 31 : 16
    } bit_field;
} t_gpio_dsel;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    db_gap              : 4  ; // 3  : 0
        uint32_t    reserved_0          : 28 ; // 31 : 4
    } bit_field;
} t_gpio_db_set;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    db                  : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_gpio_debounce;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ddr                 : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_gpio_ddr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    idr                 : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_gpio_idr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    odr                 : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_gpio_odr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    bs                  : 16 ; // 15 : 0
        uint32_t    br                  : 16 ; // 31 : 16
    } bit_field;
} t_gpio_bsrr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    inten               : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_gpio_inten;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    rise                : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_gpio_rise;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    fall                : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_gpio_fall;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    int_st              : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_gpio_intstatus;


//--------------------------------------------------------------------------------

typedef struct
{
    volatile        t_gpio_pu                                gpio_pu                                ; // 0x0
    volatile        t_gpio_pd                                gpio_pd                                ; // 0x4
    volatile        t_gpio_speed                             gpio_speed                             ; // 0x8
    volatile        t_gpio_dsel                              gpio_dsel                              ; // 0xc
    volatile        t_gpio_db_set                            gpio_db_set                            ; // 0x10
    volatile        t_gpio_debounce                          gpio_debounce                          ; // 0x14
    volatile        t_gpio_ddr                               gpio_ddr                               ; // 0x18
    volatile        t_gpio_idr                               gpio_idr                               ; // 0x1c
    volatile        t_gpio_odr                               gpio_odr                               ; // 0x20
    volatile        t_gpio_bsrr                              gpio_bsrr                              ; // 0x24
    volatile        t_gpio_inten                             gpio_inten                             ; // 0x28
    volatile        t_gpio_rise                              gpio_rise                              ; // 0x2c
    volatile        t_gpio_fall                              gpio_fall                              ; // 0x30
    volatile        t_gpio_intstatus                         gpio_intstatus                         ; // 0x34
} t_hwp_gpio_t;


//--------------------------------------------------------------------------------

__STATIC_INLINE void gpio_pu_set(uint32_t base, uint32_t value)
{
    ((t_hwp_gpio_t*)(base))->gpio_pu.val = value;
}

__STATIC_INLINE void gpio_pu_setf(uint32_t base, uint16_t pu)
{
    ((t_hwp_gpio_t*)(base))->gpio_pu.bit_field.pu = pu;
}

__STATIC_INLINE void gpio_pd_set(uint32_t base, uint32_t value)
{
    ((t_hwp_gpio_t*)(base))->gpio_pd.val = value;
}

__STATIC_INLINE void gpio_pd_setf(uint32_t base, uint16_t pd)
{
    ((t_hwp_gpio_t*)(base))->gpio_pd.bit_field.pd = pd;
}

__STATIC_INLINE void gpio_speed_set(uint32_t base, uint32_t value)
{
    ((t_hwp_gpio_t*)(base))->gpio_speed.val = value;
}

__STATIC_INLINE void gpio_hp_setf(uint32_t base, uint16_t hp)
{
    ((t_hwp_gpio_t*)(base))->gpio_speed.bit_field.hp = hp;
}

__STATIC_INLINE void gpio_dsel_set(uint32_t base, uint32_t value)
{
    ((t_hwp_gpio_t*)(base))->gpio_dsel.val = value;
}

__STATIC_INLINE void gpio_afio_en_setf(uint32_t base, uint16_t afio_en)
{
    ((t_hwp_gpio_t*)(base))->gpio_dsel.bit_field.afio_en = afio_en;
}

__STATIC_INLINE void gpio_dsel_setf(uint32_t base, uint16_t dsel)
{
    ((t_hwp_gpio_t*)(base))->gpio_dsel.bit_field.dsel = dsel;
}

__STATIC_INLINE void gpio_db_set_set(uint32_t base, uint32_t value)
{
    ((t_hwp_gpio_t*)(base))->gpio_db_set.val = value;
}

__STATIC_INLINE void gpio_db_gap_setf(uint32_t base, uint8_t db_gap)
{
    ((t_hwp_gpio_t*)(base))->gpio_db_set.bit_field.db_gap = db_gap;
}

__STATIC_INLINE void gpio_debounce_set(uint32_t base, uint32_t value)
{
    ((t_hwp_gpio_t*)(base))->gpio_debounce.val = value;
}

__STATIC_INLINE void gpio_db_setf(uint32_t base, uint16_t db)
{
    ((t_hwp_gpio_t*)(base))->gpio_debounce.bit_field.db = db;
}

__STATIC_INLINE void gpio_ddr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_gpio_t*)(base))->gpio_ddr.val = value;
}

__STATIC_INLINE void gpio_ddr_setf(uint32_t base, uint16_t ddr)
{
    ((t_hwp_gpio_t*)(base))->gpio_ddr.bit_field.ddr = ddr;
}

__STATIC_INLINE void gpio_odr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_gpio_t*)(base))->gpio_odr.val = value;
}

__STATIC_INLINE void gpio_odr_setf(uint32_t base, uint16_t odr)
{
    ((t_hwp_gpio_t*)(base))->gpio_odr.bit_field.odr = odr;
}

__STATIC_INLINE void gpio_bsrr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_gpio_t*)(base))->gpio_bsrr.val = value;
}

__STATIC_INLINE void gpio_br_setf(uint32_t base, uint16_t br)
{
    ((t_hwp_gpio_t*)(base))->gpio_bsrr.bit_field.br = br;
}

__STATIC_INLINE void gpio_bs_setf(uint32_t base, uint16_t bs)
{
    ((t_hwp_gpio_t*)(base))->gpio_bsrr.bit_field.bs = bs;
}

__STATIC_INLINE void gpio_inten_set(uint32_t base, uint32_t value)
{
    ((t_hwp_gpio_t*)(base))->gpio_inten.val = value;
}

__STATIC_INLINE void gpio_inten_setf(uint32_t base, uint16_t inten)
{
    ((t_hwp_gpio_t*)(base))->gpio_inten.bit_field.inten = inten;
}

__STATIC_INLINE void gpio_rise_set(uint32_t base, uint32_t value)
{
    ((t_hwp_gpio_t*)(base))->gpio_rise.val = value;
}

__STATIC_INLINE void gpio_rise_setf(uint32_t base, uint16_t rise)
{
    ((t_hwp_gpio_t*)(base))->gpio_rise.bit_field.rise = rise;
}

__STATIC_INLINE void gpio_fall_set(uint32_t base, uint32_t value)
{
    ((t_hwp_gpio_t*)(base))->gpio_fall.val = value;
}

__STATIC_INLINE void gpio_fall_setf(uint32_t base, uint16_t fall)
{
    ((t_hwp_gpio_t*)(base))->gpio_fall.bit_field.fall = fall;
}

__STATIC_INLINE void gpio_intstatus_set(uint32_t base, uint32_t value)
{
    ((t_hwp_gpio_t*)(base))->gpio_intstatus.val = value;
}

__STATIC_INLINE void gpio_int_st_setf(uint32_t base, uint16_t int_st)
{
    ((t_hwp_gpio_t*)(base))->gpio_intstatus.bit_field.int_st = int_st;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE uint32_t gpio_pu_get(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_pu.val;
}

__STATIC_INLINE uint16_t gpio_pu_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_pu.bit_field.pu;
}

__STATIC_INLINE uint32_t gpio_pd_get(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_pd.val;
}

__STATIC_INLINE uint16_t gpio_pd_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_pd.bit_field.pd;
}

__STATIC_INLINE uint32_t gpio_speed_get(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_speed.val;
}

__STATIC_INLINE uint16_t gpio_hp_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_speed.bit_field.hp;
}

__STATIC_INLINE uint32_t gpio_dsel_get(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_dsel.val;
}

__STATIC_INLINE uint16_t gpio_afio_en_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_dsel.bit_field.afio_en;
}

__STATIC_INLINE uint16_t gpio_dsel_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_dsel.bit_field.dsel;
}

__STATIC_INLINE uint32_t gpio_db_set_get(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_db_set.val;
}

__STATIC_INLINE uint8_t gpio_db_gap_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_db_set.bit_field.db_gap;
}

__STATIC_INLINE uint32_t gpio_debounce_get(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_debounce.val;
}

__STATIC_INLINE uint16_t gpio_db_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_debounce.bit_field.db;
}

__STATIC_INLINE uint32_t gpio_ddr_get(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_ddr.val;
}

__STATIC_INLINE uint16_t gpio_ddr_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_ddr.bit_field.ddr;
}

__STATIC_INLINE uint32_t gpio_idr_get(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_idr.val;
}

__STATIC_INLINE uint16_t gpio_idr_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_idr.bit_field.idr;
}

__STATIC_INLINE uint32_t gpio_odr_get(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_odr.val;
}

__STATIC_INLINE uint16_t gpio_odr_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_odr.bit_field.odr;
}

__STATIC_INLINE uint32_t gpio_inten_get(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_inten.val;
}

__STATIC_INLINE uint16_t gpio_inten_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_inten.bit_field.inten;
}

__STATIC_INLINE uint32_t gpio_rise_get(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_rise.val;
}

__STATIC_INLINE uint16_t gpio_rise_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_rise.bit_field.rise;
}

__STATIC_INLINE uint32_t gpio_fall_get(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_fall.val;
}

__STATIC_INLINE uint16_t gpio_fall_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_fall.bit_field.fall;
}

__STATIC_INLINE uint32_t gpio_intstatus_get(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_intstatus.val;
}

__STATIC_INLINE uint16_t gpio_int_st_getf(uint32_t base)
{
    return ((t_hwp_gpio_t*)(base))->gpio_intstatus.bit_field.int_st;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void gpio_dsel_pack(uint32_t base, uint16_t afio_en, uint16_t dsel)
{
    ((t_hwp_gpio_t*)(base))->gpio_dsel.val = ( \
                                    ((uint32_t )afio_en           << 16) \
                                  | ((uint32_t )dsel) \
                                  );
}

__STATIC_INLINE void gpio_bsrr_pack(uint32_t base, uint16_t br, uint16_t bs)
{
    ((t_hwp_gpio_t*)(base))->gpio_bsrr.val = ( \
                                    ((uint32_t )br                << 16) \
                                  | ((uint32_t )bs) \
                                  );
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void gpio_dsel_unpack(uint32_t base, uint16_t * afio_en, uint16_t * dsel)
{
    t_gpio_dsel local_val = ((t_hwp_gpio_t*)(base))->gpio_dsel;

    *afio_en            = local_val.bit_field.afio_en;
    *dsel               = local_val.bit_field.dsel;
}

#endif

