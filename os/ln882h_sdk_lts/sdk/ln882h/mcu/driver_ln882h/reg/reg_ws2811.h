#ifndef __REG_WS2811_H__
#define __REG_WS2811_H__

#include "ln882h.h"


typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ws_en               : 1  ; // 0  : 0
        uint32_t    int_en              : 1  ; // 1  : 1
        uint32_t    dma_en              : 1  ; // 2  : 2
        uint32_t    reserved_0          : 29 ; // 31 : 3
    } bit_field;
} t_ws2811_ctrl;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    empty               : 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_ws2811_status;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    intr                : 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_ws2811_isr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    br                  : 7  ; // 6  : 0
        uint32_t    reserved_0          : 25 ; // 31 : 7
    } bit_field;
} t_ws2811_br;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    dr                  : 8  ; // 7  : 0
        uint32_t    reserved_0          : 24 ; // 31 : 8
    } bit_field;
} t_ws2811_dr;


//--------------------------------------------------------------------------------

typedef struct
{
    volatile        t_ws2811_ctrl                            ctrl                                   ; // 0x0
    volatile        t_ws2811_status                          status                                 ; // 0x4
    volatile        t_ws2811_isr                             isr                                    ; // 0x8
    volatile        t_ws2811_br                              br                                     ; // 0xc
    volatile        t_ws2811_dr                              dr                                     ; // 0x10
} t_hwp_ws2811_t;


//--------------------------------------------------------------------------------

__STATIC_INLINE void ws2811_ctrl_set(uint32_t base, uint32_t value)
{
    ((t_hwp_ws2811_t*)(base))->ctrl.val = value;
}

__STATIC_INLINE void ws2811_dma_en_setf(uint32_t base, uint8_t dma_en)
{
    ((t_hwp_ws2811_t*)(base))->ctrl.bit_field.dma_en = dma_en;
}

__STATIC_INLINE void ws2811_int_en_setf(uint32_t base, uint8_t int_en)
{
    ((t_hwp_ws2811_t*)(base))->ctrl.bit_field.int_en = int_en;
}

__STATIC_INLINE void ws2811_ws_en_setf(uint32_t base, uint8_t ws_en)
{
    ((t_hwp_ws2811_t*)(base))->ctrl.bit_field.ws_en = ws_en;
}

__STATIC_INLINE void ws2811_br_set(uint32_t base, uint32_t value)
{
    ((t_hwp_ws2811_t*)(base))->br.val = value;
}

__STATIC_INLINE void ws2811_br_setf(uint32_t base, uint8_t br)
{
    ((t_hwp_ws2811_t*)(base))->br.bit_field.br = br;
}

__STATIC_INLINE void ws2811_dr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_ws2811_t*)(base))->dr.val = value;
}

__STATIC_INLINE void ws2811_dr_setf(uint32_t base, uint8_t dr)
{
    ((t_hwp_ws2811_t*)(base))->dr.bit_field.dr = dr;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE uint32_t ws2811_ctrl_get(uint32_t base)
{
    return ((t_hwp_ws2811_t*)(base))->ctrl.val;
}

__STATIC_INLINE uint8_t ws2811_dma_en_getf(uint32_t base)
{
    return ((t_hwp_ws2811_t*)(base))->ctrl.bit_field.dma_en;
}

__STATIC_INLINE uint8_t ws2811_int_en_getf(uint32_t base)
{
    return ((t_hwp_ws2811_t*)(base))->ctrl.bit_field.int_en;
}

__STATIC_INLINE uint8_t ws2811_ws_en_getf(uint32_t base)
{
    return ((t_hwp_ws2811_t*)(base))->ctrl.bit_field.ws_en;
}

__STATIC_INLINE uint32_t ws2811_status_get(uint32_t base)
{
    return ((t_hwp_ws2811_t*)(base))->status.val;
}

__STATIC_INLINE uint8_t ws2811_empty_getf(uint32_t base)
{
    return ((t_hwp_ws2811_t*)(base))->status.bit_field.empty;
}

__STATIC_INLINE uint32_t ws2811_isr_get(uint32_t base)
{
    return ((t_hwp_ws2811_t*)(base))->isr.val;
}

__STATIC_INLINE uint8_t ws2811_intr_getf(uint32_t base)
{
    return ((t_hwp_ws2811_t*)(base))->isr.bit_field.intr;
}

__STATIC_INLINE uint32_t ws2811_br_get(uint32_t base)
{
    return ((t_hwp_ws2811_t*)(base))->br.val;
}

__STATIC_INLINE uint8_t ws2811_br_getf(uint32_t base)
{
    return ((t_hwp_ws2811_t*)(base))->br.bit_field.br;
}

__STATIC_INLINE uint32_t ws2811_dr_get(uint32_t base)
{
    return ((t_hwp_ws2811_t*)(base))->dr.val;
}

__STATIC_INLINE uint8_t ws2811_dr_getf(uint32_t base)
{
    return ((t_hwp_ws2811_t*)(base))->dr.bit_field.dr;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void ws2811_ctrl_pack(uint32_t base, uint8_t dma_en, uint8_t int_en, uint8_t ws_en)
{
    ((t_hwp_ws2811_t*)(base))->ctrl.val = ( \
                                    ((uint32_t )dma_en            << 2) \
                                  | ((uint32_t )int_en            << 1) \
                                  | ((uint32_t )ws_en) \
                                  );
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void ws2811_ctrl_unpack(uint32_t base, uint8_t * dma_en, uint8_t * int_en, uint8_t * ws_en)
{
    t_ws2811_ctrl local_val = ((t_hwp_ws2811_t*)(base))->ctrl;

    *dma_en             = local_val.bit_field.dma_en;
    *int_en             = local_val.bit_field.int_en;
    *ws_en              = local_val.bit_field.ws_en;
}

#endif

