#ifndef __REG_DMA_H__
#define __REG_DMA_H__

#include "ln882h.h"

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    gif1                : 1  ; // 0  : 0
        uint32_t    tcif1               : 1  ; // 1  : 1
        uint32_t    htif1               : 1  ; // 2  : 2
        uint32_t    teif1               : 1  ; // 3  : 3
        uint32_t    gif2                : 1  ; // 4  : 4
        uint32_t    tcif2               : 1  ; // 5  : 5
        uint32_t    htif2               : 1  ; // 6  : 6
        uint32_t    teif2               : 1  ; // 7  : 7
        uint32_t    gif3                : 1  ; // 8  : 8
        uint32_t    tcif3               : 1  ; // 9  : 9
        uint32_t    htif3               : 1  ; // 10 : 10
        uint32_t    teif3               : 1  ; // 11 : 11
        uint32_t    gif4                : 1  ; // 12 : 12
        uint32_t    tcif4               : 1  ; // 13 : 13
        uint32_t    htif4               : 1  ; // 14 : 14
        uint32_t    teif4               : 1  ; // 15 : 15
        uint32_t    gif5                : 1  ; // 16 : 16
        uint32_t    tcif5               : 1  ; // 17 : 17
        uint32_t    htif5               : 1  ; // 18 : 18
        uint32_t    teif5               : 1  ; // 19 : 19
        uint32_t    gif6                : 1  ; // 20 : 20
        uint32_t    tcif6               : 1  ; // 21 : 21
        uint32_t    htif6               : 1  ; // 22 : 22
        uint32_t    teif6               : 1  ; // 23 : 23
        uint32_t    gif7                : 1  ; // 24 : 24
        uint32_t    tcif7               : 1  ; // 25 : 25
        uint32_t    htif7               : 1  ; // 26 : 26
        uint32_t    teif7               : 1  ; // 27 : 27
        uint32_t    reserved_0          : 4  ; // 31 : 28
    } bit_field;
} t_dma_isr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    cgif1               : 1  ; // 0  : 0
        uint32_t    ctcif1              : 1  ; // 1  : 1
        uint32_t    chtif1              : 1  ; // 2  : 2
        uint32_t    cteif1              : 1  ; // 3  : 3
        uint32_t    cgif2               : 1  ; // 4  : 4
        uint32_t    ctcif2              : 1  ; // 5  : 5
        uint32_t    chtif2              : 1  ; // 6  : 6
        uint32_t    cteif2              : 1  ; // 7  : 7
        uint32_t    cgif3               : 1  ; // 8  : 8
        uint32_t    ctcif3              : 1  ; // 9  : 9
        uint32_t    chtif3              : 1  ; // 10 : 10
        uint32_t    cteif3              : 1  ; // 11 : 11
        uint32_t    cgif4               : 1  ; // 12 : 12
        uint32_t    ctcif4              : 1  ; // 13 : 13
        uint32_t    chtif4              : 1  ; // 14 : 14
        uint32_t    cteif4              : 1  ; // 15 : 15
        uint32_t    cgif5               : 1  ; // 16 : 16
        uint32_t    ctcif5              : 1  ; // 17 : 17
        uint32_t    chtif5              : 1  ; // 18 : 18
        uint32_t    cteif5              : 1  ; // 19 : 19
        uint32_t    cgif6               : 1  ; // 20 : 20
        uint32_t    ctcif6              : 1  ; // 21 : 21
        uint32_t    chtif6              : 1  ; // 22 : 22
        uint32_t    cteif6              : 1  ; // 23 : 23
        uint32_t    cgif7               : 1  ; // 24 : 24
        uint32_t    ctcif7              : 1  ; // 25 : 25
        uint32_t    chtif7              : 1  ; // 26 : 26
        uint32_t    cteif7              : 1  ; // 27 : 27
        uint32_t    reserved_0          : 4  ; // 31 : 28
    } bit_field;
} t_dma_ifcr;



typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    en                 : 1  ; // 0  : 0
        uint32_t    tcie               : 1  ; // 1  : 1
        uint32_t    htie               : 1  ; // 2  : 2
        uint32_t    teie               : 1  ; // 3  : 3
        uint32_t    dir                : 1  ; // 4  : 4
        uint32_t    circ               : 1  ; // 5  : 5
        uint32_t    pinc               : 1  ; // 6  : 6
        uint32_t    minc               : 1  ; // 7  : 7
        uint32_t    psize              : 2  ; // 9  : 8
        uint32_t    msize              : 2  ; // 11 : 10
        uint32_t    pl                 : 2  ; // 13 : 12
        uint32_t    mem2mem            : 1  ; // 14 : 14
        uint32_t    reserved_0         : 17 ; // 31 : 15
    } bit_field;
} t_dma_ccr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ndt                : 16 ; // 15 : 0
        uint32_t    reserved_0         : 16 ; // 31 : 16
    } bit_field;
} t_dma_cndtr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pa                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cpar;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ma                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cmar;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    en1                 : 1  ; // 0  : 0
        uint32_t    tcie1               : 1  ; // 1  : 1
        uint32_t    htie1               : 1  ; // 2  : 2
        uint32_t    teie1               : 1  ; // 3  : 3
        uint32_t    dir1                : 1  ; // 4  : 4
        uint32_t    circ1               : 1  ; // 5  : 5
        uint32_t    pinc1               : 1  ; // 6  : 6
        uint32_t    minc1               : 1  ; // 7  : 7
        uint32_t    psize1              : 2  ; // 9  : 8
        uint32_t    msize1              : 2  ; // 11 : 10
        uint32_t    pl1                 : 2  ; // 13 : 12
        uint32_t    mem2mem1            : 1  ; // 14 : 14
        uint32_t    reserved_0          : 17 ; // 31 : 15
    } bit_field;
} t_dma_ccr1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ndt1                : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_dma_cndtr1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pa1                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cpar1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ma1                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cmar1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    en2                 : 1  ; // 0  : 0
        uint32_t    tcie2               : 1  ; // 1  : 1
        uint32_t    htie2               : 1  ; // 2  : 2
        uint32_t    teie2               : 1  ; // 3  : 3
        uint32_t    dir2                : 1  ; // 4  : 4
        uint32_t    circ2               : 1  ; // 5  : 5
        uint32_t    pinc2               : 1  ; // 6  : 6
        uint32_t    minc2               : 1  ; // 7  : 7
        uint32_t    psize2              : 2  ; // 9  : 8
        uint32_t    msize2              : 2  ; // 11 : 10
        uint32_t    pl2                 : 2  ; // 13 : 12
        uint32_t    mem2mem2            : 1  ; // 14 : 14
        uint32_t    reserved_0          : 17 ; // 31 : 15
    } bit_field;
} t_dma_ccr2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ndt2                : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_dma_cndtr2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pa2                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cpar2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ma2                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cmar2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    en3                 : 1  ; // 0  : 0
        uint32_t    tcie3               : 1  ; // 1  : 1
        uint32_t    htie3               : 1  ; // 2  : 2
        uint32_t    teie3               : 1  ; // 3  : 3
        uint32_t    dir3                : 1  ; // 4  : 4
        uint32_t    circ3               : 1  ; // 5  : 5
        uint32_t    pinc3               : 1  ; // 6  : 6
        uint32_t    minc3               : 1  ; // 7  : 7
        uint32_t    psize3              : 2  ; // 9  : 8
        uint32_t    msize3              : 2  ; // 11 : 10
        uint32_t    pl3                 : 2  ; // 13 : 12
        uint32_t    mem2mem3            : 1  ; // 14 : 14
        uint32_t    reserved_0          : 17 ; // 31 : 15
    } bit_field;
} t_dma_ccr3;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ndt3                : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_dma_cndtr3;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pa3                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cpar3;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ma3                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cmar3;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    en4                 : 1  ; // 0  : 0
        uint32_t    tcie4               : 1  ; // 1  : 1
        uint32_t    htie4               : 1  ; // 2  : 2
        uint32_t    teie4               : 1  ; // 3  : 3
        uint32_t    dir4                : 1  ; // 4  : 4
        uint32_t    circ4               : 1  ; // 5  : 5
        uint32_t    pinc4               : 1  ; // 6  : 6
        uint32_t    minc4               : 1  ; // 7  : 7
        uint32_t    psize4              : 2  ; // 9  : 8
        uint32_t    msize4              : 2  ; // 11 : 10
        uint32_t    pl4                 : 2  ; // 13 : 12
        uint32_t    mem2mem4            : 1  ; // 14 : 14
        uint32_t    reserved_0          : 17 ; // 31 : 15
    } bit_field;
} t_dma_ccr4;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ndt4                : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_dma_cndtr4;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pa4                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cpar4;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ma4                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cmar4;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    en5                 : 1  ; // 0  : 0
        uint32_t    tcie5               : 1  ; // 1  : 1
        uint32_t    htie5               : 1  ; // 2  : 2
        uint32_t    teie5               : 1  ; // 3  : 3
        uint32_t    dir5                : 1  ; // 4  : 4
        uint32_t    circ5               : 1  ; // 5  : 5
        uint32_t    pinc5               : 1  ; // 6  : 6
        uint32_t    minc5               : 1  ; // 7  : 7
        uint32_t    psize5              : 2  ; // 9  : 8
        uint32_t    msize5              : 2  ; // 11 : 10
        uint32_t    pl5                 : 2  ; // 13 : 12
        uint32_t    mem2mem5            : 1  ; // 14 : 14
        uint32_t    reserved_0          : 17 ; // 31 : 15
    } bit_field;
} t_dma_ccr5;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ndt5                : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_dma_cndtr5;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pa5                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cpar5;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ma5                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cmar5;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    en6                 : 1  ; // 0  : 0
        uint32_t    tcie6               : 1  ; // 1  : 1
        uint32_t    htie6               : 1  ; // 2  : 2
        uint32_t    teie6               : 1  ; // 3  : 3
        uint32_t    dir6                : 1  ; // 4  : 4
        uint32_t    circ6               : 1  ; // 5  : 5
        uint32_t    pinc6               : 1  ; // 6  : 6
        uint32_t    minc6               : 1  ; // 7  : 7
        uint32_t    psize6              : 2  ; // 9  : 8
        uint32_t    msize6              : 2  ; // 11 : 10
        uint32_t    pl6                 : 2  ; // 13 : 12
        uint32_t    mem2mem6            : 1  ; // 14 : 14
        uint32_t    reserved_0          : 17 ; // 31 : 15
    } bit_field;
} t_dma_ccr6;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ndt6                : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_dma_cndtr6;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pa6                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cpar6;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ma6                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cmar6;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    en7                 : 1  ; // 0  : 0
        uint32_t    tcie7               : 1  ; // 1  : 1
        uint32_t    htie7               : 1  ; // 2  : 2
        uint32_t    teie7               : 1  ; // 3  : 3
        uint32_t    dir7                : 1  ; // 4  : 4
        uint32_t    circ7               : 1  ; // 5  : 5
        uint32_t    pinc7               : 1  ; // 6  : 6
        uint32_t    minc7               : 1  ; // 7  : 7
        uint32_t    psize7              : 2  ; // 9  : 8
        uint32_t    msize7              : 2  ; // 11 : 10
        uint32_t    pl7                 : 2  ; // 13 : 12
        uint32_t    mem2mem7            : 1  ; // 14 : 14
        uint32_t    reserved_0          : 17 ; // 31 : 15
    } bit_field;
} t_dma_ccr7;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ndt7                : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_dma_cndtr7;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pa7                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cpar7;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ma7                 : 32 ; // 31 : 0
    } bit_field;
} t_dma_cmar7;


//--------------------------------------------------------------------------------
typedef struct
{                        
    volatile        t_dma_ccr                               dma_ccr                               ; // base + 0x8
    volatile        t_dma_cndtr                             dma_cndtr                             ; // base + 0xc
    volatile        t_dma_cpar                              dma_cpar                              ; // base + 0x10
    volatile        t_dma_cmar                              dma_cmar                              ; // base + 0x14
    volatile        uint32_t                                reserved[1]                           ; // base + 0x18
                   
} t_hwp_dma_tmp_t;

typedef struct
{
    volatile        t_dma_isr                                dma_isr                                ; // 0x0
    volatile        t_dma_ifcr                               dma_ifcr                               ; // 0x4
    volatile        t_dma_ccr1                               dma_ccr1                               ; // 0x8
    volatile        t_dma_cndtr1                             dma_cndtr1                             ; // 0xc
    volatile        t_dma_cpar1                              dma_cpar1                              ; // 0x10
    volatile        t_dma_cmar1                              dma_cmar1                              ; // 0x14
    volatile        uint32_t                                 reserved_1[1]                          ; // 0x18
    volatile        t_dma_ccr2                               dma_ccr2                               ; // 0x1c
    volatile        t_dma_cndtr2                             dma_cndtr2                             ; // 0x20
    volatile        t_dma_cpar2                              dma_cpar2                              ; // 0x24
    volatile        t_dma_cmar2                              dma_cmar2                              ; // 0x28
    volatile        uint32_t                                 reserved_2[1]                          ; // 0x2c
    volatile        t_dma_ccr3                               dma_ccr3                               ; // 0x30
    volatile        t_dma_cndtr3                             dma_cndtr3                             ; // 0x34
    volatile        t_dma_cpar3                              dma_cpar3                              ; // 0x38
    volatile        t_dma_cmar3                              dma_cmar3                              ; // 0x3c
    volatile        uint32_t                                 reserved_3[1]                          ; // 0x40
    volatile        t_dma_ccr4                               dma_ccr4                               ; // 0x44
    volatile        t_dma_cndtr4                             dma_cndtr4                             ; // 0x48
    volatile        t_dma_cpar4                              dma_cpar4                              ; // 0x4c
    volatile        t_dma_cmar4                              dma_cmar4                              ; // 0x50
    volatile        uint32_t                                 reserved_4[1]                          ; // 0x54
    volatile        t_dma_ccr5                               dma_ccr5                               ; // 0x58
    volatile        t_dma_cndtr5                             dma_cndtr5                             ; // 0x5c
    volatile        t_dma_cpar5                              dma_cpar5                              ; // 0x60
    volatile        t_dma_cmar5                              dma_cmar5                              ; // 0x64
    volatile        uint32_t                                 reserved_5[1]                          ; // 0x68
    volatile        t_dma_ccr6                               dma_ccr6                               ; // 0x6c
    volatile        t_dma_cndtr6                             dma_cndtr6                             ; // 0x70
    volatile        t_dma_cpar6                              dma_cpar6                              ; // 0x74
    volatile        t_dma_cmar6                              dma_cmar6                              ; // 0x78
    volatile        uint32_t                                 reserved_6[1]                          ; // 0x7c
    volatile        t_dma_ccr7                               dma_ccr7                               ; // 0x80
    volatile        t_dma_cndtr7                             dma_cndtr7                             ; // 0x84
    volatile        t_dma_cpar7                              dma_cpar7                              ; // 0x88
    volatile        t_dma_cmar7                              dma_cmar7                              ; // 0x8c
} t_hwp_dma_t;


//--------------------------------------------------------------------------------

__STATIC_INLINE void dma_ifcr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.val = value;
}

__STATIC_INLINE void dma_cteif7_setf(uint32_t base, uint8_t cteif7)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif7 = cteif7;
}

__STATIC_INLINE void dma_chtif7_setf(uint32_t base, uint8_t chtif7)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif7 = chtif7;
}

__STATIC_INLINE void dma_ctcif7_setf(uint32_t base, uint8_t ctcif7)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif7 = ctcif7;
}

__STATIC_INLINE void dma_cgif7_setf(uint32_t base, uint8_t cgif7)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif7 = cgif7;
}

__STATIC_INLINE void dma_cteif6_setf(uint32_t base, uint8_t cteif6)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif6 = cteif6;
}

__STATIC_INLINE void dma_chtif6_setf(uint32_t base, uint8_t chtif6)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif6 = chtif6;
}

__STATIC_INLINE void dma_ctcif6_setf(uint32_t base, uint8_t ctcif6)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif6 = ctcif6;
}

__STATIC_INLINE void dma_cgif6_setf(uint32_t base, uint8_t cgif6)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif6 = cgif6;
}

__STATIC_INLINE void dma_cteif5_setf(uint32_t base, uint8_t cteif5)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif5 = cteif5;
}

__STATIC_INLINE void dma_chtif5_setf(uint32_t base, uint8_t chtif5)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif5 = chtif5;
}

__STATIC_INLINE void dma_ctcif5_setf(uint32_t base, uint8_t ctcif5)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif5 = ctcif5;
}

__STATIC_INLINE void dma_cgif5_setf(uint32_t base, uint8_t cgif5)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif5 = cgif5;
}

__STATIC_INLINE void dma_cteif4_setf(uint32_t base, uint8_t cteif4)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif4 = cteif4;
}

__STATIC_INLINE void dma_chtif4_setf(uint32_t base, uint8_t chtif4)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif4 = chtif4;
}

__STATIC_INLINE void dma_ctcif4_setf(uint32_t base, uint8_t ctcif4)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif4 = ctcif4;
}

__STATIC_INLINE void dma_cgif4_setf(uint32_t base, uint8_t cgif4)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif4 = cgif4;
}

__STATIC_INLINE void dma_cteif3_setf(uint32_t base, uint8_t cteif3)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif3 = cteif3;
}

__STATIC_INLINE void dma_chtif3_setf(uint32_t base, uint8_t chtif3)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif3 = chtif3;
}

__STATIC_INLINE void dma_ctcif3_setf(uint32_t base, uint8_t ctcif3)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif3 = ctcif3;
}

__STATIC_INLINE void dma_cgif3_setf(uint32_t base, uint8_t cgif3)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif3 = cgif3;
}

__STATIC_INLINE void dma_cteif2_setf(uint32_t base, uint8_t cteif2)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif2 = cteif2;
}

__STATIC_INLINE void dma_chtif2_setf(uint32_t base, uint8_t chtif2)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif2 = chtif2;
}

__STATIC_INLINE void dma_ctcif2_setf(uint32_t base, uint8_t ctcif2)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif2 = ctcif2;
}

__STATIC_INLINE void dma_cgif2_setf(uint32_t base, uint8_t cgif2)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif2 = cgif2;
}

__STATIC_INLINE void dma_cteif1_setf(uint32_t base, uint8_t cteif1)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif1 = cteif1;
}

__STATIC_INLINE void dma_chtif1_setf(uint32_t base, uint8_t chtif1)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif1 = chtif1;
}

__STATIC_INLINE void dma_ctcif1_setf(uint32_t base, uint8_t ctcif1)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif1 = ctcif1;
}

__STATIC_INLINE void dma_cgif1_setf(uint32_t base, uint8_t cgif1)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif1 = cgif1;
}

__STATIC_INLINE void dma_ccr1_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.val = value;
}

__STATIC_INLINE void dma_mem2mem1_setf(uint32_t base, uint8_t mem2mem1)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.mem2mem1 = mem2mem1;
}

__STATIC_INLINE void dma_pl1_setf(uint32_t base, uint8_t pl1)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.pl1 = pl1;
}

__STATIC_INLINE void dma_msize1_setf(uint32_t base, uint8_t msize1)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.msize1 = msize1;
}

__STATIC_INLINE void dma_psize1_setf(uint32_t base, uint8_t psize1)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.psize1 = psize1;
}

__STATIC_INLINE void dma_minc1_setf(uint32_t base, uint8_t minc1)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.minc1 = minc1;
}

__STATIC_INLINE void dma_pinc1_setf(uint32_t base, uint8_t pinc1)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.pinc1 = pinc1;
}

__STATIC_INLINE void dma_circ1_setf(uint32_t base, uint8_t circ1)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.circ1 = circ1;
}

__STATIC_INLINE void dma_dir1_setf(uint32_t base, uint8_t dir1)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.dir1 = dir1;
}

__STATIC_INLINE void dma_teie1_setf(uint32_t base, uint8_t teie1)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.teie1 = teie1;
}

__STATIC_INLINE void dma_htie1_setf(uint32_t base, uint8_t htie1)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.htie1 = htie1;
}

__STATIC_INLINE void dma_tcie1_setf(uint32_t base, uint8_t tcie1)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.tcie1 = tcie1;
}

__STATIC_INLINE void dma_en1_setf(uint32_t base, uint8_t en1)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.en1 = en1;
}

__STATIC_INLINE void dma_cndtr1_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr1.val = value;
}

__STATIC_INLINE void dma_ndt1_setf(uint32_t base, uint16_t ndt1)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr1.bit_field.ndt1 = ndt1;
}

__STATIC_INLINE void dma_cpar1_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cpar1.val = value;
}

__STATIC_INLINE void dma_pa1_setf(uint32_t base, uint32_t pa1)
{
    ((t_hwp_dma_t*)(base))->dma_cpar1.bit_field.pa1 = pa1;
}

__STATIC_INLINE void dma_cmar1_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cmar1.val = value;
}

__STATIC_INLINE void dma_ma1_setf(uint32_t base, uint32_t ma1)
{
    ((t_hwp_dma_t*)(base))->dma_cmar1.bit_field.ma1 = ma1;
}

__STATIC_INLINE void dma_ccr2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.val = value;
}

__STATIC_INLINE void dma_mem2mem2_setf(uint32_t base, uint8_t mem2mem2)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.mem2mem2 = mem2mem2;
}

__STATIC_INLINE void dma_pl2_setf(uint32_t base, uint8_t pl2)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.pl2 = pl2;
}

__STATIC_INLINE void dma_msize2_setf(uint32_t base, uint8_t msize2)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.msize2 = msize2;
}

__STATIC_INLINE void dma_psize2_setf(uint32_t base, uint8_t psize2)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.psize2 = psize2;
}

__STATIC_INLINE void dma_minc2_setf(uint32_t base, uint8_t minc2)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.minc2 = minc2;
}

__STATIC_INLINE void dma_pinc2_setf(uint32_t base, uint8_t pinc2)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.pinc2 = pinc2;
}

__STATIC_INLINE void dma_circ2_setf(uint32_t base, uint8_t circ2)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.circ2 = circ2;
}

__STATIC_INLINE void dma_dir2_setf(uint32_t base, uint8_t dir2)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.dir2 = dir2;
}

__STATIC_INLINE void dma_teie2_setf(uint32_t base, uint8_t teie2)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.teie2 = teie2;
}

__STATIC_INLINE void dma_htie2_setf(uint32_t base, uint8_t htie2)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.htie2 = htie2;
}

__STATIC_INLINE void dma_tcie2_setf(uint32_t base, uint8_t tcie2)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.tcie2 = tcie2;
}

__STATIC_INLINE void dma_en2_setf(uint32_t base, uint8_t en2)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.en2 = en2;
}

__STATIC_INLINE void dma_cndtr2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr2.val = value;
}

__STATIC_INLINE void dma_ndt2_setf(uint32_t base, uint16_t ndt2)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr2.bit_field.ndt2 = ndt2;
}

__STATIC_INLINE void dma_cpar2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cpar2.val = value;
}

__STATIC_INLINE void dma_pa2_setf(uint32_t base, uint32_t pa2)
{
    ((t_hwp_dma_t*)(base))->dma_cpar2.bit_field.pa2 = pa2;
}

__STATIC_INLINE void dma_cmar2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cmar2.val = value;
}

__STATIC_INLINE void dma_ma2_setf(uint32_t base, uint32_t ma2)
{
    ((t_hwp_dma_t*)(base))->dma_cmar2.bit_field.ma2 = ma2;
}

__STATIC_INLINE void dma_ccr3_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.val = value;
}

__STATIC_INLINE void dma_mem2mem3_setf(uint32_t base, uint8_t mem2mem3)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.mem2mem3 = mem2mem3;
}

__STATIC_INLINE void dma_pl3_setf(uint32_t base, uint8_t pl3)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.pl3 = pl3;
}

__STATIC_INLINE void dma_msize3_setf(uint32_t base, uint8_t msize3)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.msize3 = msize3;
}

__STATIC_INLINE void dma_psize3_setf(uint32_t base, uint8_t psize3)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.psize3 = psize3;
}

__STATIC_INLINE void dma_minc3_setf(uint32_t base, uint8_t minc3)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.minc3 = minc3;
}

__STATIC_INLINE void dma_pinc3_setf(uint32_t base, uint8_t pinc3)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.pinc3 = pinc3;
}

__STATIC_INLINE void dma_circ3_setf(uint32_t base, uint8_t circ3)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.circ3 = circ3;
}

__STATIC_INLINE void dma_dir3_setf(uint32_t base, uint8_t dir3)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.dir3 = dir3;
}

__STATIC_INLINE void dma_teie3_setf(uint32_t base, uint8_t teie3)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.teie3 = teie3;
}

__STATIC_INLINE void dma_htie3_setf(uint32_t base, uint8_t htie3)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.htie3 = htie3;
}

__STATIC_INLINE void dma_tcie3_setf(uint32_t base, uint8_t tcie3)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.tcie3 = tcie3;
}

__STATIC_INLINE void dma_en3_setf(uint32_t base, uint8_t en3)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.en3 = en3;
}

__STATIC_INLINE void dma_cndtr3_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr3.val = value;
}

__STATIC_INLINE void dma_ndt3_setf(uint32_t base, uint16_t ndt3)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr3.bit_field.ndt3 = ndt3;
}

__STATIC_INLINE void dma_cpar3_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cpar3.val = value;
}

__STATIC_INLINE void dma_pa3_setf(uint32_t base, uint32_t pa3)
{
    ((t_hwp_dma_t*)(base))->dma_cpar3.bit_field.pa3 = pa3;
}

__STATIC_INLINE void dma_cmar3_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cmar3.val = value;
}

__STATIC_INLINE void dma_ma3_setf(uint32_t base, uint32_t ma3)
{
    ((t_hwp_dma_t*)(base))->dma_cmar3.bit_field.ma3 = ma3;
}

__STATIC_INLINE void dma_ccr4_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.val = value;
}

__STATIC_INLINE void dma_mem2mem4_setf(uint32_t base, uint8_t mem2mem4)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.mem2mem4 = mem2mem4;
}

__STATIC_INLINE void dma_pl4_setf(uint32_t base, uint8_t pl4)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.pl4 = pl4;
}

__STATIC_INLINE void dma_msize4_setf(uint32_t base, uint8_t msize4)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.msize4 = msize4;
}

__STATIC_INLINE void dma_psize4_setf(uint32_t base, uint8_t psize4)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.psize4 = psize4;
}

__STATIC_INLINE void dma_minc4_setf(uint32_t base, uint8_t minc4)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.minc4 = minc4;
}

__STATIC_INLINE void dma_pinc4_setf(uint32_t base, uint8_t pinc4)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.pinc4 = pinc4;
}

__STATIC_INLINE void dma_circ4_setf(uint32_t base, uint8_t circ4)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.circ4 = circ4;
}

__STATIC_INLINE void dma_dir4_setf(uint32_t base, uint8_t dir4)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.dir4 = dir4;
}

__STATIC_INLINE void dma_teie4_setf(uint32_t base, uint8_t teie4)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.teie4 = teie4;
}

__STATIC_INLINE void dma_htie4_setf(uint32_t base, uint8_t htie4)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.htie4 = htie4;
}

__STATIC_INLINE void dma_tcie4_setf(uint32_t base, uint8_t tcie4)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.tcie4 = tcie4;
}

__STATIC_INLINE void dma_en4_setf(uint32_t base, uint8_t en4)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.en4 = en4;
}

__STATIC_INLINE void dma_cndtr4_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr4.val = value;
}

__STATIC_INLINE void dma_ndt4_setf(uint32_t base, uint16_t ndt4)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr4.bit_field.ndt4 = ndt4;
}

__STATIC_INLINE void dma_cpar4_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cpar4.val = value;
}

__STATIC_INLINE void dma_pa4_setf(uint32_t base, uint32_t pa4)
{
    ((t_hwp_dma_t*)(base))->dma_cpar4.bit_field.pa4 = pa4;
}

__STATIC_INLINE void dma_cmar4_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cmar4.val = value;
}

__STATIC_INLINE void dma_ma4_setf(uint32_t base, uint32_t ma4)
{
    ((t_hwp_dma_t*)(base))->dma_cmar4.bit_field.ma4 = ma4;
}

__STATIC_INLINE void dma_ccr5_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.val = value;
}

__STATIC_INLINE void dma_mem2mem5_setf(uint32_t base, uint8_t mem2mem5)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.mem2mem5 = mem2mem5;
}

__STATIC_INLINE void dma_pl5_setf(uint32_t base, uint8_t pl5)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.pl5 = pl5;
}

__STATIC_INLINE void dma_msize5_setf(uint32_t base, uint8_t msize5)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.msize5 = msize5;
}

__STATIC_INLINE void dma_psize5_setf(uint32_t base, uint8_t psize5)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.psize5 = psize5;
}

__STATIC_INLINE void dma_minc5_setf(uint32_t base, uint8_t minc5)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.minc5 = minc5;
}

__STATIC_INLINE void dma_pinc5_setf(uint32_t base, uint8_t pinc5)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.pinc5 = pinc5;
}

__STATIC_INLINE void dma_circ5_setf(uint32_t base, uint8_t circ5)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.circ5 = circ5;
}

__STATIC_INLINE void dma_dir5_setf(uint32_t base, uint8_t dir5)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.dir5 = dir5;
}

__STATIC_INLINE void dma_teie5_setf(uint32_t base, uint8_t teie5)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.teie5 = teie5;
}

__STATIC_INLINE void dma_htie5_setf(uint32_t base, uint8_t htie5)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.htie5 = htie5;
}

__STATIC_INLINE void dma_tcie5_setf(uint32_t base, uint8_t tcie5)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.tcie5 = tcie5;
}

__STATIC_INLINE void dma_en5_setf(uint32_t base, uint8_t en5)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.en5 = en5;
}

__STATIC_INLINE void dma_cndtr5_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr5.val = value;
}

__STATIC_INLINE void dma_ndt5_setf(uint32_t base, uint16_t ndt5)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr5.bit_field.ndt5 = ndt5;
}

__STATIC_INLINE void dma_cpar5_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cpar5.val = value;
}

__STATIC_INLINE void dma_pa5_setf(uint32_t base, uint32_t pa5)
{
    ((t_hwp_dma_t*)(base))->dma_cpar5.bit_field.pa5 = pa5;
}

__STATIC_INLINE void dma_cmar5_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cmar5.val = value;
}

__STATIC_INLINE void dma_ma5_setf(uint32_t base, uint32_t ma5)
{
    ((t_hwp_dma_t*)(base))->dma_cmar5.bit_field.ma5 = ma5;
}

__STATIC_INLINE void dma_ccr6_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.val = value;
}

__STATIC_INLINE void dma_mem2mem6_setf(uint32_t base, uint8_t mem2mem6)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.mem2mem6 = mem2mem6;
}

__STATIC_INLINE void dma_pl6_setf(uint32_t base, uint8_t pl6)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.pl6 = pl6;
}

__STATIC_INLINE void dma_msize6_setf(uint32_t base, uint8_t msize6)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.msize6 = msize6;
}

__STATIC_INLINE void dma_psize6_setf(uint32_t base, uint8_t psize6)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.psize6 = psize6;
}

__STATIC_INLINE void dma_minc6_setf(uint32_t base, uint8_t minc6)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.minc6 = minc6;
}

__STATIC_INLINE void dma_pinc6_setf(uint32_t base, uint8_t pinc6)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.pinc6 = pinc6;
}

__STATIC_INLINE void dma_circ6_setf(uint32_t base, uint8_t circ6)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.circ6 = circ6;
}

__STATIC_INLINE void dma_dir6_setf(uint32_t base, uint8_t dir6)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.dir6 = dir6;
}

__STATIC_INLINE void dma_teie6_setf(uint32_t base, uint8_t teie6)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.teie6 = teie6;
}

__STATIC_INLINE void dma_htie6_setf(uint32_t base, uint8_t htie6)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.htie6 = htie6;
}

__STATIC_INLINE void dma_tcie6_setf(uint32_t base, uint8_t tcie6)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.tcie6 = tcie6;
}

__STATIC_INLINE void dma_en6_setf(uint32_t base, uint8_t en6)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.en6 = en6;
}

__STATIC_INLINE void dma_cndtr6_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr6.val = value;
}

__STATIC_INLINE void dma_ndt6_setf(uint32_t base, uint16_t ndt6)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr6.bit_field.ndt6 = ndt6;
}

__STATIC_INLINE void dma_cpar6_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cpar6.val = value;
}

__STATIC_INLINE void dma_pa6_setf(uint32_t base, uint32_t pa6)
{
    ((t_hwp_dma_t*)(base))->dma_cpar6.bit_field.pa6 = pa6;
}

__STATIC_INLINE void dma_cmar6_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cmar6.val = value;
}

__STATIC_INLINE void dma_ma6_setf(uint32_t base, uint32_t ma6)
{
    ((t_hwp_dma_t*)(base))->dma_cmar6.bit_field.ma6 = ma6;
}

__STATIC_INLINE void dma_ccr7_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.val = value;
}

__STATIC_INLINE void dma_mem2mem7_setf(uint32_t base, uint8_t mem2mem7)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.mem2mem7 = mem2mem7;
}

__STATIC_INLINE void dma_pl7_setf(uint32_t base, uint8_t pl7)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.pl7 = pl7;
}

__STATIC_INLINE void dma_msize7_setf(uint32_t base, uint8_t msize7)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.msize7 = msize7;
}

__STATIC_INLINE void dma_psize7_setf(uint32_t base, uint8_t psize7)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.psize7 = psize7;
}

__STATIC_INLINE void dma_minc7_setf(uint32_t base, uint8_t minc7)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.minc7 = minc7;
}

__STATIC_INLINE void dma_pinc7_setf(uint32_t base, uint8_t pinc7)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.pinc7 = pinc7;
}

__STATIC_INLINE void dma_circ7_setf(uint32_t base, uint8_t circ7)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.circ7 = circ7;
}

__STATIC_INLINE void dma_dir7_setf(uint32_t base, uint8_t dir7)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.dir7 = dir7;
}

__STATIC_INLINE void dma_teie7_setf(uint32_t base, uint8_t teie7)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.teie7 = teie7;
}

__STATIC_INLINE void dma_htie7_setf(uint32_t base, uint8_t htie7)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.htie7 = htie7;
}

__STATIC_INLINE void dma_tcie7_setf(uint32_t base, uint8_t tcie7)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.tcie7 = tcie7;
}

__STATIC_INLINE void dma_en7_setf(uint32_t base, uint8_t en7)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.en7 = en7;
}

__STATIC_INLINE void dma_cndtr7_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr7.val = value;
}

__STATIC_INLINE void dma_ndt7_setf(uint32_t base, uint16_t ndt7)
{
    ((t_hwp_dma_t*)(base))->dma_cndtr7.bit_field.ndt7 = ndt7;
}

__STATIC_INLINE void dma_cpar7_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cpar7.val = value;
}

__STATIC_INLINE void dma_pa7_setf(uint32_t base, uint32_t pa7)
{
    ((t_hwp_dma_t*)(base))->dma_cpar7.bit_field.pa7 = pa7;
}

__STATIC_INLINE void dma_cmar7_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_t*)(base))->dma_cmar7.val = value;
}

__STATIC_INLINE void dma_ma7_setf(uint32_t base, uint32_t ma7)
{
    ((t_hwp_dma_t*)(base))->dma_cmar7.bit_field.ma7 = ma7;
}


__STATIC_INLINE void dma_ccr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_ccr.val = value;
}

__STATIC_INLINE void dma_mem2mem_setf(uint32_t base, uint8_t mem2mem)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.mem2mem = mem2mem;
}

__STATIC_INLINE void dma_pl_setf(uint32_t base, uint8_t pl)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.pl = pl;
}

__STATIC_INLINE void dma_msize_setf(uint32_t base, uint8_t msize)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.msize = msize;
}

__STATIC_INLINE void dma_psize_setf(uint32_t base, uint8_t psize)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.psize = psize;
}

__STATIC_INLINE void dma_minc_setf(uint32_t base, uint8_t minc)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.minc = minc;
}

__STATIC_INLINE void dma_pinc_setf(uint32_t base, uint8_t pinc)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.pinc = pinc;
}

__STATIC_INLINE void dma_circ_setf(uint32_t base, uint8_t circ)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.circ = circ;
}

__STATIC_INLINE void dma_dir_setf(uint32_t base, uint8_t dir)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.dir = dir;
}

__STATIC_INLINE void dma_teie_setf(uint32_t base, uint8_t teie)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.teie = teie;
}

__STATIC_INLINE void dma_htie_setf(uint32_t base, uint8_t htie)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.htie = htie;
}

__STATIC_INLINE void dma_tcie_setf(uint32_t base, uint8_t tcie)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.tcie = tcie;
}

__STATIC_INLINE void dma_en_setf(uint32_t base, uint8_t en)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.en = en;
}

__STATIC_INLINE void dma_cndtr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_cndtr.val = value;
}

__STATIC_INLINE void dma_ndt_setf(uint32_t base, uint16_t ndt)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_cndtr.bit_field.ndt = ndt;
}

__STATIC_INLINE void dma_cpar_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_cpar.val = value;
}

__STATIC_INLINE void dma_pa_setf(uint32_t base, uint32_t pa)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_cpar.bit_field.pa = pa;
}

__STATIC_INLINE void dma_cmar_set(uint32_t base, uint32_t value)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_cmar.val = value;
}

__STATIC_INLINE void dma_ma_setf(uint32_t base, uint32_t ma)
{
    ((t_hwp_dma_tmp_t*)(base))->dma_cmar.bit_field.ma = ma;
}
//--------------------------------------------------------------------------------

__STATIC_INLINE uint32_t dma_isr_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.val;
}

__STATIC_INLINE uint8_t dma_teif7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.teif7;
}

__STATIC_INLINE uint8_t dma_htif7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.htif7;
}

__STATIC_INLINE uint8_t dma_tcif7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.tcif7;
}

__STATIC_INLINE uint8_t dma_gif7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.gif7;
}

__STATIC_INLINE uint8_t dma_teif6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.teif6;
}

__STATIC_INLINE uint8_t dma_htif6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.htif6;
}

__STATIC_INLINE uint8_t dma_tcif6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.tcif6;
}

__STATIC_INLINE uint8_t dma_gif6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.gif6;
}

__STATIC_INLINE uint8_t dma_teif5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.teif5;
}

__STATIC_INLINE uint8_t dma_htif5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.htif5;
}

__STATIC_INLINE uint8_t dma_tcif5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.tcif5;
}

__STATIC_INLINE uint8_t dma_gif5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.gif5;
}

__STATIC_INLINE uint8_t dma_teif4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.teif4;
}

__STATIC_INLINE uint8_t dma_htif4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.htif4;
}

__STATIC_INLINE uint8_t dma_tcif4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.tcif4;
}

__STATIC_INLINE uint8_t dma_gif4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.gif4;
}

__STATIC_INLINE uint8_t dma_teif3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.teif3;
}

__STATIC_INLINE uint8_t dma_htif3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.htif3;
}

__STATIC_INLINE uint8_t dma_tcif3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.tcif3;
}

__STATIC_INLINE uint8_t dma_gif3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.gif3;
}

__STATIC_INLINE uint8_t dma_teif2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.teif2;
}

__STATIC_INLINE uint8_t dma_htif2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.htif2;
}

__STATIC_INLINE uint8_t dma_tcif2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.tcif2;
}

__STATIC_INLINE uint8_t dma_gif2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.gif2;
}

__STATIC_INLINE uint8_t dma_teif1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.teif1;
}

__STATIC_INLINE uint8_t dma_htif1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.htif1;
}

__STATIC_INLINE uint8_t dma_tcif1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.tcif1;
}

__STATIC_INLINE uint8_t dma_gif1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_isr.bit_field.gif1;
}

__STATIC_INLINE uint32_t dma_ifcr_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.val;
}

__STATIC_INLINE uint8_t dma_cteif7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif7;
}

__STATIC_INLINE uint8_t dma_chtif7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif7;
}

__STATIC_INLINE uint8_t dma_ctcif7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif7;
}

__STATIC_INLINE uint8_t dma_cgif7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif7;
}

__STATIC_INLINE uint8_t dma_cteif6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif6;
}

__STATIC_INLINE uint8_t dma_chtif6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif6;
}

__STATIC_INLINE uint8_t dma_ctcif6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif6;
}

__STATIC_INLINE uint8_t dma_cgif6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif6;
}

__STATIC_INLINE uint8_t dma_cteif5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif5;
}

__STATIC_INLINE uint8_t dma_chtif5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif5;
}

__STATIC_INLINE uint8_t dma_ctcif5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif5;
}

__STATIC_INLINE uint8_t dma_cgif5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif5;
}

__STATIC_INLINE uint8_t dma_cteif4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif4;
}

__STATIC_INLINE uint8_t dma_chtif4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif4;
}

__STATIC_INLINE uint8_t dma_ctcif4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif4;
}

__STATIC_INLINE uint8_t dma_cgif4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif4;
}

__STATIC_INLINE uint8_t dma_cteif3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif3;
}

__STATIC_INLINE uint8_t dma_chtif3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif3;
}

__STATIC_INLINE uint8_t dma_ctcif3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif3;
}

__STATIC_INLINE uint8_t dma_cgif3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif3;
}

__STATIC_INLINE uint8_t dma_cteif2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif2;
}

__STATIC_INLINE uint8_t dma_chtif2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif2;
}

__STATIC_INLINE uint8_t dma_ctcif2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif2;
}

__STATIC_INLINE uint8_t dma_cgif2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif2;
}

__STATIC_INLINE uint8_t dma_cteif1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cteif1;
}

__STATIC_INLINE uint8_t dma_chtif1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.chtif1;
}

__STATIC_INLINE uint8_t dma_ctcif1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.ctcif1;
}

__STATIC_INLINE uint8_t dma_cgif1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ifcr.bit_field.cgif1;
}

__STATIC_INLINE uint32_t dma_ccr1_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr1.val;
}

__STATIC_INLINE uint8_t dma_mem2mem1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.mem2mem1;
}

__STATIC_INLINE uint8_t dma_pl1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.pl1;
}

__STATIC_INLINE uint8_t dma_msize1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.msize1;
}

__STATIC_INLINE uint8_t dma_psize1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.psize1;
}

__STATIC_INLINE uint8_t dma_minc1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.minc1;
}

__STATIC_INLINE uint8_t dma_pinc1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.pinc1;
}

__STATIC_INLINE uint8_t dma_circ1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.circ1;
}

__STATIC_INLINE uint8_t dma_dir1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.dir1;
}

__STATIC_INLINE uint8_t dma_teie1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.teie1;
}

__STATIC_INLINE uint8_t dma_htie1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.htie1;
}

__STATIC_INLINE uint8_t dma_tcie1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.tcie1;
}

__STATIC_INLINE uint8_t dma_en1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr1.bit_field.en1;
}

__STATIC_INLINE uint32_t dma_cndtr1_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr1.val;
}

__STATIC_INLINE uint16_t dma_ndt1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr1.bit_field.ndt1;
}

__STATIC_INLINE uint32_t dma_cpar1_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar1.val;
}

__STATIC_INLINE uint32_t dma_pa1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar1.bit_field.pa1;
}

__STATIC_INLINE uint32_t dma_cmar1_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar1.val;
}

__STATIC_INLINE uint32_t dma_ma1_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar1.bit_field.ma1;
}

__STATIC_INLINE uint32_t dma_ccr2_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr2.val;
}

__STATIC_INLINE uint8_t dma_mem2mem2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.mem2mem2;
}

__STATIC_INLINE uint8_t dma_pl2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.pl2;
}

__STATIC_INLINE uint8_t dma_msize2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.msize2;
}

__STATIC_INLINE uint8_t dma_psize2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.psize2;
}

__STATIC_INLINE uint8_t dma_minc2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.minc2;
}

__STATIC_INLINE uint8_t dma_pinc2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.pinc2;
}

__STATIC_INLINE uint8_t dma_circ2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.circ2;
}

__STATIC_INLINE uint8_t dma_dir2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.dir2;
}

__STATIC_INLINE uint8_t dma_teie2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.teie2;
}

__STATIC_INLINE uint8_t dma_htie2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.htie2;
}

__STATIC_INLINE uint8_t dma_tcie2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.tcie2;
}

__STATIC_INLINE uint8_t dma_en2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr2.bit_field.en2;
}

__STATIC_INLINE uint32_t dma_cndtr2_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr2.val;
}

__STATIC_INLINE uint16_t dma_ndt2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr2.bit_field.ndt2;
}

__STATIC_INLINE uint32_t dma_cpar2_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar2.val;
}

__STATIC_INLINE uint32_t dma_pa2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar2.bit_field.pa2;
}

__STATIC_INLINE uint32_t dma_cmar2_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar2.val;
}

__STATIC_INLINE uint32_t dma_ma2_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar2.bit_field.ma2;
}

__STATIC_INLINE uint32_t dma_ccr3_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr3.val;
}

__STATIC_INLINE uint8_t dma_mem2mem3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.mem2mem3;
}

__STATIC_INLINE uint8_t dma_pl3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.pl3;
}

__STATIC_INLINE uint8_t dma_msize3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.msize3;
}

__STATIC_INLINE uint8_t dma_psize3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.psize3;
}

__STATIC_INLINE uint8_t dma_minc3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.minc3;
}

__STATIC_INLINE uint8_t dma_pinc3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.pinc3;
}

__STATIC_INLINE uint8_t dma_circ3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.circ3;
}

__STATIC_INLINE uint8_t dma_dir3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.dir3;
}

__STATIC_INLINE uint8_t dma_teie3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.teie3;
}

__STATIC_INLINE uint8_t dma_htie3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.htie3;
}

__STATIC_INLINE uint8_t dma_tcie3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.tcie3;
}

__STATIC_INLINE uint8_t dma_en3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr3.bit_field.en3;
}

__STATIC_INLINE uint32_t dma_cndtr3_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr3.val;
}

__STATIC_INLINE uint16_t dma_ndt3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr3.bit_field.ndt3;
}

__STATIC_INLINE uint32_t dma_cpar3_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar3.val;
}

__STATIC_INLINE uint32_t dma_pa3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar3.bit_field.pa3;
}

__STATIC_INLINE uint32_t dma_cmar3_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar3.val;
}

__STATIC_INLINE uint32_t dma_ma3_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar3.bit_field.ma3;
}

__STATIC_INLINE uint32_t dma_ccr4_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr4.val;
}

__STATIC_INLINE uint8_t dma_mem2mem4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.mem2mem4;
}

__STATIC_INLINE uint8_t dma_pl4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.pl4;
}

__STATIC_INLINE uint8_t dma_msize4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.msize4;
}

__STATIC_INLINE uint8_t dma_psize4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.psize4;
}

__STATIC_INLINE uint8_t dma_minc4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.minc4;
}

__STATIC_INLINE uint8_t dma_pinc4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.pinc4;
}

__STATIC_INLINE uint8_t dma_circ4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.circ4;
}

__STATIC_INLINE uint8_t dma_dir4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.dir4;
}

__STATIC_INLINE uint8_t dma_teie4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.teie4;
}

__STATIC_INLINE uint8_t dma_htie4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.htie4;
}

__STATIC_INLINE uint8_t dma_tcie4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.tcie4;
}

__STATIC_INLINE uint8_t dma_en4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr4.bit_field.en4;
}

__STATIC_INLINE uint32_t dma_cndtr4_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr4.val;
}

__STATIC_INLINE uint16_t dma_ndt4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr4.bit_field.ndt4;
}

__STATIC_INLINE uint32_t dma_cpar4_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar4.val;
}

__STATIC_INLINE uint32_t dma_pa4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar4.bit_field.pa4;
}

__STATIC_INLINE uint32_t dma_cmar4_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar4.val;
}

__STATIC_INLINE uint32_t dma_ma4_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar4.bit_field.ma4;
}

__STATIC_INLINE uint32_t dma_ccr5_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr5.val;
}

__STATIC_INLINE uint8_t dma_mem2mem5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.mem2mem5;
}

__STATIC_INLINE uint8_t dma_pl5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.pl5;
}

__STATIC_INLINE uint8_t dma_msize5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.msize5;
}

__STATIC_INLINE uint8_t dma_psize5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.psize5;
}

__STATIC_INLINE uint8_t dma_minc5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.minc5;
}

__STATIC_INLINE uint8_t dma_pinc5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.pinc5;
}

__STATIC_INLINE uint8_t dma_circ5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.circ5;
}

__STATIC_INLINE uint8_t dma_dir5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.dir5;
}

__STATIC_INLINE uint8_t dma_teie5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.teie5;
}

__STATIC_INLINE uint8_t dma_htie5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.htie5;
}

__STATIC_INLINE uint8_t dma_tcie5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.tcie5;
}

__STATIC_INLINE uint8_t dma_en5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr5.bit_field.en5;
}

__STATIC_INLINE uint32_t dma_cndtr5_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr5.val;
}

__STATIC_INLINE uint16_t dma_ndt5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr5.bit_field.ndt5;
}

__STATIC_INLINE uint32_t dma_cpar5_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar5.val;
}

__STATIC_INLINE uint32_t dma_pa5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar5.bit_field.pa5;
}

__STATIC_INLINE uint32_t dma_cmar5_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar5.val;
}

__STATIC_INLINE uint32_t dma_ma5_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar5.bit_field.ma5;
}

__STATIC_INLINE uint32_t dma_ccr6_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr6.val;
}

__STATIC_INLINE uint8_t dma_mem2mem6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.mem2mem6;
}

__STATIC_INLINE uint8_t dma_pl6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.pl6;
}

__STATIC_INLINE uint8_t dma_msize6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.msize6;
}

__STATIC_INLINE uint8_t dma_psize6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.psize6;
}

__STATIC_INLINE uint8_t dma_minc6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.minc6;
}

__STATIC_INLINE uint8_t dma_pinc6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.pinc6;
}

__STATIC_INLINE uint8_t dma_circ6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.circ6;
}

__STATIC_INLINE uint8_t dma_dir6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.dir6;
}

__STATIC_INLINE uint8_t dma_teie6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.teie6;
}

__STATIC_INLINE uint8_t dma_htie6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.htie6;
}

__STATIC_INLINE uint8_t dma_tcie6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.tcie6;
}

__STATIC_INLINE uint8_t dma_en6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr6.bit_field.en6;
}

__STATIC_INLINE uint32_t dma_cndtr6_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr6.val;
}

__STATIC_INLINE uint16_t dma_ndt6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr6.bit_field.ndt6;
}

__STATIC_INLINE uint32_t dma_cpar6_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar6.val;
}

__STATIC_INLINE uint32_t dma_pa6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar6.bit_field.pa6;
}

__STATIC_INLINE uint32_t dma_cmar6_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar6.val;
}

__STATIC_INLINE uint32_t dma_ma6_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar6.bit_field.ma6;
}

__STATIC_INLINE uint32_t dma_ccr7_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr7.val;
}

__STATIC_INLINE uint8_t dma_mem2mem7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.mem2mem7;
}

__STATIC_INLINE uint8_t dma_pl7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.pl7;
}

__STATIC_INLINE uint8_t dma_msize7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.msize7;
}

__STATIC_INLINE uint8_t dma_psize7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.psize7;
}

__STATIC_INLINE uint8_t dma_minc7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.minc7;
}

__STATIC_INLINE uint8_t dma_pinc7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.pinc7;
}

__STATIC_INLINE uint8_t dma_circ7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.circ7;
}

__STATIC_INLINE uint8_t dma_dir7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.dir7;
}

__STATIC_INLINE uint8_t dma_teie7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.teie7;
}

__STATIC_INLINE uint8_t dma_htie7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.htie7;
}

__STATIC_INLINE uint8_t dma_tcie7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.tcie7;
}

__STATIC_INLINE uint8_t dma_en7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_ccr7.bit_field.en7;
}

__STATIC_INLINE uint32_t dma_cndtr7_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr7.val;
}

__STATIC_INLINE uint16_t dma_ndt7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cndtr7.bit_field.ndt7;
}

__STATIC_INLINE uint32_t dma_cpar7_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar7.val;
}

__STATIC_INLINE uint32_t dma_pa7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cpar7.bit_field.pa7;
}

__STATIC_INLINE uint32_t dma_cmar7_get(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar7.val;
}

__STATIC_INLINE uint32_t dma_ma7_getf(uint32_t base)
{
    return ((t_hwp_dma_t*)(base))->dma_cmar7.bit_field.ma7;
}




__STATIC_INLINE uint32_t dma_ccr_get(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_ccr.val;
}

__STATIC_INLINE uint8_t dma_mem2mem_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.mem2mem;
}

__STATIC_INLINE uint8_t dma_pl_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.pl;
}

__STATIC_INLINE uint8_t dma_msize_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.msize;
}

__STATIC_INLINE uint8_t dma_psize_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.psize;
}

__STATIC_INLINE uint8_t dma_minc_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.minc;
}

__STATIC_INLINE uint8_t dma_pinc_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.pinc;
}

__STATIC_INLINE uint8_t dma_circ_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.circ;
}

__STATIC_INLINE uint8_t dma_dir_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.dir;
}

__STATIC_INLINE uint8_t dma_teie_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.teie;
}

__STATIC_INLINE uint8_t dma_htie_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.htie;
}

__STATIC_INLINE uint8_t dma_tcie_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.tcie;
}

__STATIC_INLINE uint8_t dma_en_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_ccr.bit_field.en;
}

__STATIC_INLINE uint32_t dma_cndtr_get(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_cndtr.val;
}

__STATIC_INLINE uint16_t dma_ndt_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_cndtr.bit_field.ndt;
}

__STATIC_INLINE uint32_t dma_cpar_get(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_cpar.val;
}

__STATIC_INLINE uint32_t dma_pa_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_cpar.bit_field.pa;
}

__STATIC_INLINE uint32_t dma_cmar_get(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_cmar.val;
}

__STATIC_INLINE uint32_t dma_ma_getf(uint32_t base)
{
    return ((t_hwp_dma_tmp_t*)(base))->dma_cmar.bit_field.ma;
}

//--------------------------------------------------------------------------------

__STATIC_INLINE void dma_ifcr_pack(uint32_t base, uint8_t cteif7, uint8_t chtif7, uint8_t ctcif7, uint8_t cgif7, uint8_t cteif6, uint8_t chtif6, uint8_t ctcif6, uint8_t cgif6, uint8_t cteif5, uint8_t chtif5, uint8_t ctcif5, uint8_t cgif5, uint8_t cteif4, uint8_t chtif4, uint8_t ctcif4, uint8_t cgif4, uint8_t cteif3, uint8_t chtif3, uint8_t ctcif3, uint8_t cgif3, uint8_t cteif2, uint8_t chtif2, uint8_t ctcif2, uint8_t cgif2, uint8_t cteif1, uint8_t chtif1, uint8_t ctcif1, uint8_t cgif1)
{
    ((t_hwp_dma_t*)(base))->dma_ifcr.val = ( \
                                    ((uint32_t )cteif7            << 27) \
                                  | ((uint32_t )chtif7            << 26) \
                                  | ((uint32_t )ctcif7            << 25) \
                                  | ((uint32_t )cgif7             << 24) \
                                  | ((uint32_t )cteif6            << 23) \
                                  | ((uint32_t )chtif6            << 22) \
                                  | ((uint32_t )ctcif6            << 21) \
                                  | ((uint32_t )cgif6             << 20) \
                                  | ((uint32_t )cteif5            << 19) \
                                  | ((uint32_t )chtif5            << 18) \
                                  | ((uint32_t )ctcif5            << 17) \
                                  | ((uint32_t )cgif5             << 16) \
                                  | ((uint32_t )cteif4            << 15) \
                                  | ((uint32_t )chtif4            << 14) \
                                  | ((uint32_t )ctcif4            << 13) \
                                  | ((uint32_t )cgif4             << 12) \
                                  | ((uint32_t )cteif3            << 11) \
                                  | ((uint32_t )chtif3            << 10) \
                                  | ((uint32_t )ctcif3            << 9) \
                                  | ((uint32_t )cgif3             << 8) \
                                  | ((uint32_t )cteif2            << 7) \
                                  | ((uint32_t )chtif2            << 6) \
                                  | ((uint32_t )ctcif2            << 5) \
                                  | ((uint32_t )cgif2             << 4) \
                                  | ((uint32_t )cteif1            << 3) \
                                  | ((uint32_t )chtif1            << 2) \
                                  | ((uint32_t )ctcif1            << 1) \
                                  | ((uint32_t )cgif1) \
                                  );
}

__STATIC_INLINE void dma_ccr1_pack(uint32_t base, uint8_t mem2mem1, uint8_t pl1, uint8_t msize1, uint8_t psize1, uint8_t minc1, uint8_t pinc1, uint8_t circ1, uint8_t dir1, uint8_t teie1, uint8_t htie1, uint8_t tcie1, uint8_t en1)
{
    ((t_hwp_dma_t*)(base))->dma_ccr1.val = ( \
                                    ((uint32_t )mem2mem1          << 14) \
                                  | ((uint32_t )pl1               << 12) \
                                  | ((uint32_t )msize1            << 10) \
                                  | ((uint32_t )psize1            << 8) \
                                  | ((uint32_t )minc1             << 7) \
                                  | ((uint32_t )pinc1             << 6) \
                                  | ((uint32_t )circ1             << 5) \
                                  | ((uint32_t )dir1              << 4) \
                                  | ((uint32_t )teie1             << 3) \
                                  | ((uint32_t )htie1             << 2) \
                                  | ((uint32_t )tcie1             << 1) \
                                  | ((uint32_t )en1) \
                                  );
}

__STATIC_INLINE void dma_ccr2_pack(uint32_t base, uint8_t mem2mem2, uint8_t pl2, uint8_t msize2, uint8_t psize2, uint8_t minc2, uint8_t pinc2, uint8_t circ2, uint8_t dir2, uint8_t teie2, uint8_t htie2, uint8_t tcie2, uint8_t en2)
{
    ((t_hwp_dma_t*)(base))->dma_ccr2.val = ( \
                                    ((uint32_t )mem2mem2          << 14) \
                                  | ((uint32_t )pl2               << 12) \
                                  | ((uint32_t )msize2            << 10) \
                                  | ((uint32_t )psize2            << 8) \
                                  | ((uint32_t )minc2             << 7) \
                                  | ((uint32_t )pinc2             << 6) \
                                  | ((uint32_t )circ2             << 5) \
                                  | ((uint32_t )dir2              << 4) \
                                  | ((uint32_t )teie2             << 3) \
                                  | ((uint32_t )htie2             << 2) \
                                  | ((uint32_t )tcie2             << 1) \
                                  | ((uint32_t )en2) \
                                  );
}

__STATIC_INLINE void dma_ccr3_pack(uint32_t base, uint8_t mem2mem3, uint8_t pl3, uint8_t msize3, uint8_t psize3, uint8_t minc3, uint8_t pinc3, uint8_t circ3, uint8_t dir3, uint8_t teie3, uint8_t htie3, uint8_t tcie3, uint8_t en3)
{
    ((t_hwp_dma_t*)(base))->dma_ccr3.val = ( \
                                    ((uint32_t )mem2mem3          << 14) \
                                  | ((uint32_t )pl3               << 12) \
                                  | ((uint32_t )msize3            << 10) \
                                  | ((uint32_t )psize3            << 8) \
                                  | ((uint32_t )minc3             << 7) \
                                  | ((uint32_t )pinc3             << 6) \
                                  | ((uint32_t )circ3             << 5) \
                                  | ((uint32_t )dir3              << 4) \
                                  | ((uint32_t )teie3             << 3) \
                                  | ((uint32_t )htie3             << 2) \
                                  | ((uint32_t )tcie3             << 1) \
                                  | ((uint32_t )en3) \
                                  );
}

__STATIC_INLINE void dma_ccr4_pack(uint32_t base, uint8_t mem2mem4, uint8_t pl4, uint8_t msize4, uint8_t psize4, uint8_t minc4, uint8_t pinc4, uint8_t circ4, uint8_t dir4, uint8_t teie4, uint8_t htie4, uint8_t tcie4, uint8_t en4)
{
    ((t_hwp_dma_t*)(base))->dma_ccr4.val = ( \
                                    ((uint32_t )mem2mem4          << 14) \
                                  | ((uint32_t )pl4               << 12) \
                                  | ((uint32_t )msize4            << 10) \
                                  | ((uint32_t )psize4            << 8) \
                                  | ((uint32_t )minc4             << 7) \
                                  | ((uint32_t )pinc4             << 6) \
                                  | ((uint32_t )circ4             << 5) \
                                  | ((uint32_t )dir4              << 4) \
                                  | ((uint32_t )teie4             << 3) \
                                  | ((uint32_t )htie4             << 2) \
                                  | ((uint32_t )tcie4             << 1) \
                                  | ((uint32_t )en4) \
                                  );
}

__STATIC_INLINE void dma_ccr5_pack(uint32_t base, uint8_t mem2mem5, uint8_t pl5, uint8_t msize5, uint8_t psize5, uint8_t minc5, uint8_t pinc5, uint8_t circ5, uint8_t dir5, uint8_t teie5, uint8_t htie5, uint8_t tcie5, uint8_t en5)
{
    ((t_hwp_dma_t*)(base))->dma_ccr5.val = ( \
                                    ((uint32_t )mem2mem5          << 14) \
                                  | ((uint32_t )pl5               << 12) \
                                  | ((uint32_t )msize5            << 10) \
                                  | ((uint32_t )psize5            << 8) \
                                  | ((uint32_t )minc5             << 7) \
                                  | ((uint32_t )pinc5             << 6) \
                                  | ((uint32_t )circ5             << 5) \
                                  | ((uint32_t )dir5              << 4) \
                                  | ((uint32_t )teie5             << 3) \
                                  | ((uint32_t )htie5             << 2) \
                                  | ((uint32_t )tcie5             << 1) \
                                  | ((uint32_t )en5) \
                                  );
}

__STATIC_INLINE void dma_ccr6_pack(uint32_t base, uint8_t mem2mem6, uint8_t pl6, uint8_t msize6, uint8_t psize6, uint8_t minc6, uint8_t pinc6, uint8_t circ6, uint8_t dir6, uint8_t teie6, uint8_t htie6, uint8_t tcie6, uint8_t en6)
{
    ((t_hwp_dma_t*)(base))->dma_ccr6.val = ( \
                                    ((uint32_t )mem2mem6          << 14) \
                                  | ((uint32_t )pl6               << 12) \
                                  | ((uint32_t )msize6            << 10) \
                                  | ((uint32_t )psize6            << 8) \
                                  | ((uint32_t )minc6             << 7) \
                                  | ((uint32_t )pinc6             << 6) \
                                  | ((uint32_t )circ6             << 5) \
                                  | ((uint32_t )dir6              << 4) \
                                  | ((uint32_t )teie6             << 3) \
                                  | ((uint32_t )htie6             << 2) \
                                  | ((uint32_t )tcie6             << 1) \
                                  | ((uint32_t )en6) \
                                  );
}

__STATIC_INLINE void dma_ccr7_pack(uint32_t base, uint8_t mem2mem7, uint8_t pl7, uint8_t msize7, uint8_t psize7, uint8_t minc7, uint8_t pinc7, uint8_t circ7, uint8_t dir7, uint8_t teie7, uint8_t htie7, uint8_t tcie7, uint8_t en7)
{
    ((t_hwp_dma_t*)(base))->dma_ccr7.val = ( \
                                    ((uint32_t )mem2mem7          << 14) \
                                  | ((uint32_t )pl7               << 12) \
                                  | ((uint32_t )msize7            << 10) \
                                  | ((uint32_t )psize7            << 8) \
                                  | ((uint32_t )minc7             << 7) \
                                  | ((uint32_t )pinc7             << 6) \
                                  | ((uint32_t )circ7             << 5) \
                                  | ((uint32_t )dir7              << 4) \
                                  | ((uint32_t )teie7             << 3) \
                                  | ((uint32_t )htie7             << 2) \
                                  | ((uint32_t )tcie7             << 1) \
                                  | ((uint32_t )en7) \
                                  );
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void dma_isr_unpack(uint32_t base, uint8_t * teif7, uint8_t * htif7, uint8_t * tcif7, uint8_t * gif7, uint8_t * teif6, uint8_t * htif6, uint8_t * tcif6, uint8_t * gif6, uint8_t * teif5, uint8_t * htif5, uint8_t * tcif5, uint8_t * gif5, uint8_t * teif4, uint8_t * htif4, uint8_t * tcif4, uint8_t * gif4, uint8_t * teif3, uint8_t * htif3, uint8_t * tcif3, uint8_t * gif3, uint8_t * teif2, uint8_t * htif2, uint8_t * tcif2, uint8_t * gif2, uint8_t * teif1, uint8_t * htif1, uint8_t * tcif1, uint8_t * gif1)
{
    t_dma_isr local_val = ((t_hwp_dma_t*)(base))->dma_isr;

    *teif7              = local_val.bit_field.teif7;
    *htif7              = local_val.bit_field.htif7;
    *tcif7              = local_val.bit_field.tcif7;
    *gif7               = local_val.bit_field.gif7;
    *teif6              = local_val.bit_field.teif6;
    *htif6              = local_val.bit_field.htif6;
    *tcif6              = local_val.bit_field.tcif6;
    *gif6               = local_val.bit_field.gif6;
    *teif5              = local_val.bit_field.teif5;
    *htif5              = local_val.bit_field.htif5;
    *tcif5              = local_val.bit_field.tcif5;
    *gif5               = local_val.bit_field.gif5;
    *teif4              = local_val.bit_field.teif4;
    *htif4              = local_val.bit_field.htif4;
    *tcif4              = local_val.bit_field.tcif4;
    *gif4               = local_val.bit_field.gif4;
    *teif3              = local_val.bit_field.teif3;
    *htif3              = local_val.bit_field.htif3;
    *tcif3              = local_val.bit_field.tcif3;
    *gif3               = local_val.bit_field.gif3;
    *teif2              = local_val.bit_field.teif2;
    *htif2              = local_val.bit_field.htif2;
    *tcif2              = local_val.bit_field.tcif2;
    *gif2               = local_val.bit_field.gif2;
    *teif1              = local_val.bit_field.teif1;
    *htif1              = local_val.bit_field.htif1;
    *tcif1              = local_val.bit_field.tcif1;
    *gif1               = local_val.bit_field.gif1;
}

__STATIC_INLINE void dma_ifcr_unpack(uint32_t base, uint8_t * cteif7, uint8_t * chtif7, uint8_t * ctcif7, uint8_t * cgif7, uint8_t * cteif6, uint8_t * chtif6, uint8_t * ctcif6, uint8_t * cgif6, uint8_t * cteif5, uint8_t * chtif5, uint8_t * ctcif5, uint8_t * cgif5, uint8_t * cteif4, uint8_t * chtif4, uint8_t * ctcif4, uint8_t * cgif4, uint8_t * cteif3, uint8_t * chtif3, uint8_t * ctcif3, uint8_t * cgif3, uint8_t * cteif2, uint8_t * chtif2, uint8_t * ctcif2, uint8_t * cgif2, uint8_t * cteif1, uint8_t * chtif1, uint8_t * ctcif1, uint8_t * cgif1)
{
    t_dma_ifcr local_val = ((t_hwp_dma_t*)(base))->dma_ifcr;

    *cteif7             = local_val.bit_field.cteif7;
    *chtif7             = local_val.bit_field.chtif7;
    *ctcif7             = local_val.bit_field.ctcif7;
    *cgif7              = local_val.bit_field.cgif7;
    *cteif6             = local_val.bit_field.cteif6;
    *chtif6             = local_val.bit_field.chtif6;
    *ctcif6             = local_val.bit_field.ctcif6;
    *cgif6              = local_val.bit_field.cgif6;
    *cteif5             = local_val.bit_field.cteif5;
    *chtif5             = local_val.bit_field.chtif5;
    *ctcif5             = local_val.bit_field.ctcif5;
    *cgif5              = local_val.bit_field.cgif5;
    *cteif4             = local_val.bit_field.cteif4;
    *chtif4             = local_val.bit_field.chtif4;
    *ctcif4             = local_val.bit_field.ctcif4;
    *cgif4              = local_val.bit_field.cgif4;
    *cteif3             = local_val.bit_field.cteif3;
    *chtif3             = local_val.bit_field.chtif3;
    *ctcif3             = local_val.bit_field.ctcif3;
    *cgif3              = local_val.bit_field.cgif3;
    *cteif2             = local_val.bit_field.cteif2;
    *chtif2             = local_val.bit_field.chtif2;
    *ctcif2             = local_val.bit_field.ctcif2;
    *cgif2              = local_val.bit_field.cgif2;
    *cteif1             = local_val.bit_field.cteif1;
    *chtif1             = local_val.bit_field.chtif1;
    *ctcif1             = local_val.bit_field.ctcif1;
    *cgif1              = local_val.bit_field.cgif1;
}

__STATIC_INLINE void dma_ccr1_unpack(uint32_t base, uint8_t * mem2mem1, uint8_t * pl1, uint8_t * msize1, uint8_t * psize1, uint8_t * minc1, uint8_t * pinc1, uint8_t * circ1, uint8_t * dir1, uint8_t * teie1, uint8_t * htie1, uint8_t * tcie1, uint8_t * en1)
{
    t_dma_ccr1 local_val = ((t_hwp_dma_t*)(base))->dma_ccr1;

    *mem2mem1           = local_val.bit_field.mem2mem1;
    *pl1                = local_val.bit_field.pl1;
    *msize1             = local_val.bit_field.msize1;
    *psize1             = local_val.bit_field.psize1;
    *minc1              = local_val.bit_field.minc1;
    *pinc1              = local_val.bit_field.pinc1;
    *circ1              = local_val.bit_field.circ1;
    *dir1               = local_val.bit_field.dir1;
    *teie1              = local_val.bit_field.teie1;
    *htie1              = local_val.bit_field.htie1;
    *tcie1              = local_val.bit_field.tcie1;
    *en1                = local_val.bit_field.en1;
}

__STATIC_INLINE void dma_ccr2_unpack(uint32_t base, uint8_t * mem2mem2, uint8_t * pl2, uint8_t * msize2, uint8_t * psize2, uint8_t * minc2, uint8_t * pinc2, uint8_t * circ2, uint8_t * dir2, uint8_t * teie2, uint8_t * htie2, uint8_t * tcie2, uint8_t * en2)
{
    t_dma_ccr2 local_val = ((t_hwp_dma_t*)(base))->dma_ccr2;

    *mem2mem2           = local_val.bit_field.mem2mem2;
    *pl2                = local_val.bit_field.pl2;
    *msize2             = local_val.bit_field.msize2;
    *psize2             = local_val.bit_field.psize2;
    *minc2              = local_val.bit_field.minc2;
    *pinc2              = local_val.bit_field.pinc2;
    *circ2              = local_val.bit_field.circ2;
    *dir2               = local_val.bit_field.dir2;
    *teie2              = local_val.bit_field.teie2;
    *htie2              = local_val.bit_field.htie2;
    *tcie2              = local_val.bit_field.tcie2;
    *en2                = local_val.bit_field.en2;
}

__STATIC_INLINE void dma_ccr3_unpack(uint32_t base, uint8_t * mem2mem3, uint8_t * pl3, uint8_t * msize3, uint8_t * psize3, uint8_t * minc3, uint8_t * pinc3, uint8_t * circ3, uint8_t * dir3, uint8_t * teie3, uint8_t * htie3, uint8_t * tcie3, uint8_t * en3)
{
    t_dma_ccr3 local_val = ((t_hwp_dma_t*)(base))->dma_ccr3;

    *mem2mem3           = local_val.bit_field.mem2mem3;
    *pl3                = local_val.bit_field.pl3;
    *msize3             = local_val.bit_field.msize3;
    *psize3             = local_val.bit_field.psize3;
    *minc3              = local_val.bit_field.minc3;
    *pinc3              = local_val.bit_field.pinc3;
    *circ3              = local_val.bit_field.circ3;
    *dir3               = local_val.bit_field.dir3;
    *teie3              = local_val.bit_field.teie3;
    *htie3              = local_val.bit_field.htie3;
    *tcie3              = local_val.bit_field.tcie3;
    *en3                = local_val.bit_field.en3;
}

__STATIC_INLINE void dma_ccr4_unpack(uint32_t base, uint8_t * mem2mem4, uint8_t * pl4, uint8_t * msize4, uint8_t * psize4, uint8_t * minc4, uint8_t * pinc4, uint8_t * circ4, uint8_t * dir4, uint8_t * teie4, uint8_t * htie4, uint8_t * tcie4, uint8_t * en4)
{
    t_dma_ccr4 local_val = ((t_hwp_dma_t*)(base))->dma_ccr4;

    *mem2mem4           = local_val.bit_field.mem2mem4;
    *pl4                = local_val.bit_field.pl4;
    *msize4             = local_val.bit_field.msize4;
    *psize4             = local_val.bit_field.psize4;
    *minc4              = local_val.bit_field.minc4;
    *pinc4              = local_val.bit_field.pinc4;
    *circ4              = local_val.bit_field.circ4;
    *dir4               = local_val.bit_field.dir4;
    *teie4              = local_val.bit_field.teie4;
    *htie4              = local_val.bit_field.htie4;
    *tcie4              = local_val.bit_field.tcie4;
    *en4                = local_val.bit_field.en4;
}

__STATIC_INLINE void dma_ccr5_unpack(uint32_t base, uint8_t * mem2mem5, uint8_t * pl5, uint8_t * msize5, uint8_t * psize5, uint8_t * minc5, uint8_t * pinc5, uint8_t * circ5, uint8_t * dir5, uint8_t * teie5, uint8_t * htie5, uint8_t * tcie5, uint8_t * en5)
{
    t_dma_ccr5 local_val = ((t_hwp_dma_t*)(base))->dma_ccr5;

    *mem2mem5           = local_val.bit_field.mem2mem5;
    *pl5                = local_val.bit_field.pl5;
    *msize5             = local_val.bit_field.msize5;
    *psize5             = local_val.bit_field.psize5;
    *minc5              = local_val.bit_field.minc5;
    *pinc5              = local_val.bit_field.pinc5;
    *circ5              = local_val.bit_field.circ5;
    *dir5               = local_val.bit_field.dir5;
    *teie5              = local_val.bit_field.teie5;
    *htie5              = local_val.bit_field.htie5;
    *tcie5              = local_val.bit_field.tcie5;
    *en5                = local_val.bit_field.en5;
}

__STATIC_INLINE void dma_ccr6_unpack(uint32_t base, uint8_t * mem2mem6, uint8_t * pl6, uint8_t * msize6, uint8_t * psize6, uint8_t * minc6, uint8_t * pinc6, uint8_t * circ6, uint8_t * dir6, uint8_t * teie6, uint8_t * htie6, uint8_t * tcie6, uint8_t * en6)
{
    t_dma_ccr6 local_val = ((t_hwp_dma_t*)(base))->dma_ccr6;

    *mem2mem6           = local_val.bit_field.mem2mem6;
    *pl6                = local_val.bit_field.pl6;
    *msize6             = local_val.bit_field.msize6;
    *psize6             = local_val.bit_field.psize6;
    *minc6              = local_val.bit_field.minc6;
    *pinc6              = local_val.bit_field.pinc6;
    *circ6              = local_val.bit_field.circ6;
    *dir6               = local_val.bit_field.dir6;
    *teie6              = local_val.bit_field.teie6;
    *htie6              = local_val.bit_field.htie6;
    *tcie6              = local_val.bit_field.tcie6;
    *en6                = local_val.bit_field.en6;
}

__STATIC_INLINE void dma_ccr7_unpack(uint32_t base, uint8_t * mem2mem7, uint8_t * pl7, uint8_t * msize7, uint8_t * psize7, uint8_t * minc7, uint8_t * pinc7, uint8_t * circ7, uint8_t * dir7, uint8_t * teie7, uint8_t * htie7, uint8_t * tcie7, uint8_t * en7)
{
    t_dma_ccr7 local_val = ((t_hwp_dma_t*)(base))->dma_ccr7;

    *mem2mem7           = local_val.bit_field.mem2mem7;
    *pl7                = local_val.bit_field.pl7;
    *msize7             = local_val.bit_field.msize7;
    *psize7             = local_val.bit_field.psize7;
    *minc7              = local_val.bit_field.minc7;
    *pinc7              = local_val.bit_field.pinc7;
    *circ7              = local_val.bit_field.circ7;
    *dir7               = local_val.bit_field.dir7;
    *teie7              = local_val.bit_field.teie7;
    *htie7              = local_val.bit_field.htie7;
    *tcie7              = local_val.bit_field.tcie7;
    *en7                = local_val.bit_field.en7;
}

#endif

