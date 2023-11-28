#ifndef __REG_LN_UART_H__
#define __REG_LN_UART_H__

#include "ln882h.h"

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ue                  : 1  ; // 0  : 0
        uint32_t    uesm                : 1  ; // 1  : 1
        uint32_t    re                  : 1  ; // 2  : 2
        uint32_t    te                  : 1  ; // 3  : 3
        uint32_t    idleie              : 1  ; // 4  : 4
        uint32_t    rxneie              : 1  ; // 5  : 5
        uint32_t    tcie                : 1  ; // 6  : 6
        uint32_t    txeie               : 1  ; // 7  : 7
        uint32_t    peie                : 1  ; // 8  : 8
        uint32_t    ps                  : 1  ; // 9  : 9
        uint32_t    pce                 : 1  ; // 10 : 10
        uint32_t    wake                : 1  ; // 11 : 11
        uint32_t    rxafie              : 1  ; // 12 : 12
        uint32_t    mme                 : 1  ; // 13 : 13
        uint32_t    cmie                : 1  ; // 14 : 14
        uint32_t    over8               : 1  ; // 15 : 15
        uint32_t    tout_set            : 4  ; // 19 : 16
        uint32_t    tout_en             : 1  ; // 20 : 20
        uint32_t    reserved_1          : 7  ; // 27 : 21
        uint32_t    m                   : 1  ; // 28 : 28
        uint32_t    reserved_0          : 3  ; // 31 : 29
    } bit_field;
} t_ln_uart_uart_cr1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    reserved_3          : 4  ; // 3  : 0
        uint32_t    addm7               : 1  ; // 4  : 4
        uint32_t    reserved_2          : 7  ; // 11 : 5
        uint32_t    stop                : 2  ; // 13 : 12
        uint32_t    reserved_1          : 5  ; // 18 : 14
        uint32_t    msbfirst            : 1  ; // 19 : 19
        uint32_t    reserved_0          : 4  ; // 23 : 20
        uint32_t    add                 : 8  ; // 31 : 24
    } bit_field;
} t_ln_uart_uart_cr2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    eie                 : 1  ; // 0  : 0
        uint32_t    iren                : 1  ; // 1  : 1
        uint32_t    reserved_2          : 4  ; // 5  : 2
        uint32_t    dmar                : 1  ; // 6  : 6
        uint32_t    dmat                : 1  ; // 7  : 7
        uint32_t    rtse                : 1  ; // 8  : 8
        uint32_t    ctse                : 1  ; // 9  : 9
        uint32_t    ctsie               : 1  ; // 10 : 10
        uint32_t    onebit              : 1  ; // 11 : 11
        uint32_t    ovrdis              : 1  ; // 12 : 12
        uint32_t    ddre                : 1  ; // 13 : 13
        uint32_t    reserved_1          : 6  ; // 19 : 14
        uint32_t    wus                 : 2  ; // 21 : 20
        uint32_t    wufie               : 1  ; // 22 : 22
        uint32_t    reserved_0          : 9  ; // 31 : 23
    } bit_field;
} t_ln_uart_uart_cr3;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    brr                 : 19 ; // 18 : 0
        uint32_t    reserved_0          : 13 ; // 31 : 19
    } bit_field;
} t_ln_uart_uart_brr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pe                  : 1  ; // 0  : 0
        uint32_t    fe                  : 1  ; // 1  : 1
        uint32_t    n                   : 1  ; // 2  : 2
        uint32_t    ore                 : 1  ; // 3  : 3
        uint32_t    idle                : 1  ; // 4  : 4
        uint32_t    rxne                : 1  ; // 5  : 5
        uint32_t    tc                  : 1  ; // 6  : 6
        uint32_t    txef                : 1  ; // 7  : 7
        uint32_t    reserved_4          : 1  ; // 8  : 8
        uint32_t    ctsif               : 1  ; // 9  : 9
        uint32_t    cts                 : 1  ; // 10 : 10
        uint32_t    reserved_3          : 5  ; // 15 : 11
        uint32_t    busy                : 1  ; // 16 : 16
        uint32_t    cmf                 : 1  ; // 17 : 17
        uint32_t    reserved_2          : 1  ; // 18 : 18
        uint32_t    rwu                 : 1  ; // 19 : 19
        uint32_t    wuf                 : 1  ; // 20 : 20
        uint32_t    rx_afull            : 1  ; // 21 : 21
        uint32_t    reserved_1          : 2  ; // 23 : 22
        uint32_t    tx_full             : 1  ; // 24 : 24
        uint32_t    reserved_0          : 7  ; // 31 : 25
    } bit_field;
} t_ln_uart_uart_isr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pecf                : 1  ; // 0  : 0
        uint32_t    fecf                : 1  ; // 1  : 1
        uint32_t    ncf                 : 1  ; // 2  : 2
        uint32_t    reserved_5          : 1  ; // 3  : 3
        uint32_t    idlecf              : 1  ; // 4  : 4
        uint32_t    reserved_4          : 1  ; // 5  : 5
        uint32_t    tccf                : 1  ; // 6  : 6
        uint32_t    reserved_3          : 2  ; // 8  : 7
        uint32_t    ctscf               : 1  ; // 9  : 9
        uint32_t    reserved_2          : 7  ; // 16 : 10
        uint32_t    cmcf                : 1  ; // 17 : 17
        uint32_t    reserved_1          : 2  ; // 19 : 18
        uint32_t    wucf                : 1  ; // 20 : 20
        uint32_t    reserved_0          : 11 ; // 31 : 21
    } bit_field;
} t_ln_uart_uart_icr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    rxdat               : 9  ; // 8  : 0
        uint32_t    reserved_0          : 23 ; // 31 : 9
    } bit_field;
} t_ln_uart_uart_rdr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    tdr                 : 9  ; // 8  : 0
        uint32_t    reserved_0          : 23 ; // 31 : 9
    } bit_field;
} t_ln_uart_uart_tdr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    rx_byte             : 4  ; // 3  : 0
        uint32_t    reserved_2          : 4  ; // 7  : 4
        uint32_t    rx_fifo_level       : 5  ; // 12 : 8
        uint32_t    reserved_1          : 3  ; // 15 : 13
        uint32_t    tx_fifo_level       : 5  ; // 20 : 16
        uint32_t    reserved_0          : 11 ; // 31 : 21
    } bit_field;
} t_ln_uart_uart_fifo;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    rx_under            : 1  ; // 0  : 0
        uint32_t    tx_under            : 1  ; // 1  : 1
        uint32_t    tx_over             : 1  ; // 2  : 2
        uint32_t    rx_full             : 1  ; // 3  : 3
        uint32_t    reserved_0          : 28 ; // 31 : 4
    } bit_field;
} t_ln_uart_uart_misc;


//--------------------------------------------------------------------------------

typedef struct
{
    volatile        t_ln_uart_uart_cr1                       uart_cr1                               ; // 0x0
    volatile        t_ln_uart_uart_cr2                       uart_cr2                               ; // 0x4
    volatile        t_ln_uart_uart_cr3                       uart_cr3                               ; // 0x8
    volatile        t_ln_uart_uart_brr                       uart_brr                               ; // 0xc
    volatile        uint32_t                                 reserved_1[3]                          ; // 0x10
    volatile        t_ln_uart_uart_isr                       uart_isr                               ; // 0x1c
    volatile        t_ln_uart_uart_icr                       uart_icr                               ; // 0x20
    volatile        t_ln_uart_uart_rdr                       uart_rdr                               ; // 0x24
    volatile        t_ln_uart_uart_tdr                       uart_tdr                               ; // 0x28
    volatile        t_ln_uart_uart_fifo                      uart_fifo                              ; // 0x2c
    volatile        t_ln_uart_uart_misc                      uart_misc                              ; // 0x30
} t_hwp_ln_uart_t;


//--------------------------------------------------------------------------------

__STATIC_INLINE void ln_uart_uart_cr1_set(uint32_t base, uint32_t value)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.val = value;
}

__STATIC_INLINE void ln_uart_m_setf(uint32_t base, uint8_t m)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.m = m;
}

__STATIC_INLINE void ln_uart_tout_en_setf(uint32_t base, uint8_t tout_en)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.tout_en = tout_en;
}

__STATIC_INLINE void ln_uart_tout_set_setf(uint32_t base, uint8_t tout_set)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.tout_set = tout_set;
}

__STATIC_INLINE void ln_uart_over8_setf(uint32_t base, uint8_t over8)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.over8 = over8;
}

__STATIC_INLINE void ln_uart_cmie_setf(uint32_t base, uint8_t cmie)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.cmie = cmie;
}

__STATIC_INLINE void ln_uart_mme_setf(uint32_t base, uint8_t mme)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.mme = mme;
}

__STATIC_INLINE void ln_uart_rxafie_setf(uint32_t base, uint8_t rxafie)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.rxafie = rxafie;
}

__STATIC_INLINE void ln_uart_wake_setf(uint32_t base, uint8_t wake)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.wake = wake;
}

__STATIC_INLINE void ln_uart_pce_setf(uint32_t base, uint8_t pce)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.pce = pce;
}

__STATIC_INLINE void ln_uart_ps_setf(uint32_t base, uint8_t ps)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.ps = ps;
}

__STATIC_INLINE void ln_uart_peie_setf(uint32_t base, uint8_t peie)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.peie = peie;
}

__STATIC_INLINE void ln_uart_txeie_setf(uint32_t base, uint8_t txeie)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.txeie = txeie;
}

__STATIC_INLINE void ln_uart_tcie_setf(uint32_t base, uint8_t tcie)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.tcie = tcie;
}

__STATIC_INLINE void ln_uart_rxneie_setf(uint32_t base, uint8_t rxneie)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.rxneie = rxneie;
}

__STATIC_INLINE void ln_uart_idleie_setf(uint32_t base, uint8_t idleie)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.idleie = idleie;
}

__STATIC_INLINE void ln_uart_te_setf(uint32_t base, uint8_t te)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.te = te;
}

__STATIC_INLINE void ln_uart_re_setf(uint32_t base, uint8_t re)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.re = re;
}

__STATIC_INLINE void ln_uart_uesm_setf(uint32_t base, uint8_t uesm)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.uesm = uesm;
}

__STATIC_INLINE void ln_uart_ue_setf(uint32_t base, uint8_t ue)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.ue = ue;
}

__STATIC_INLINE void ln_uart_uart_cr2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr2.val = value;
}

__STATIC_INLINE void ln_uart_add_setf(uint32_t base, uint8_t add)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr2.bit_field.add = add;
}

__STATIC_INLINE void ln_uart_msbfirst_setf(uint32_t base, uint8_t msbfirst)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr2.bit_field.msbfirst = msbfirst;
}

__STATIC_INLINE void ln_uart_stop_setf(uint32_t base, uint8_t stop)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr2.bit_field.stop = stop;
}

__STATIC_INLINE void ln_uart_addm7_setf(uint32_t base, uint8_t addm7)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr2.bit_field.addm7 = addm7;
}

__STATIC_INLINE void ln_uart_uart_cr3_set(uint32_t base, uint32_t value)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.val = value;
}

__STATIC_INLINE void ln_uart_wufie_setf(uint32_t base, uint8_t wufie)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.wufie = wufie;
}

__STATIC_INLINE void ln_uart_wus_setf(uint32_t base, uint8_t wus)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.wus = wus;
}

__STATIC_INLINE void ln_uart_ddre_setf(uint32_t base, uint8_t ddre)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.ddre = ddre;
}

__STATIC_INLINE void ln_uart_ovrdis_setf(uint32_t base, uint8_t ovrdis)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.ovrdis = ovrdis;
}

__STATIC_INLINE void ln_uart_onebit_setf(uint32_t base, uint8_t onebit)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.onebit = onebit;
}

__STATIC_INLINE void ln_uart_ctsie_setf(uint32_t base, uint8_t ctsie)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.ctsie = ctsie;
}

__STATIC_INLINE void ln_uart_ctse_setf(uint32_t base, uint8_t ctse)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.ctse = ctse;
}

__STATIC_INLINE void ln_uart_rtse_setf(uint32_t base, uint8_t rtse)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.rtse = rtse;
}

__STATIC_INLINE void ln_uart_dmat_setf(uint32_t base, uint8_t dmat)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.dmat = dmat;
}

__STATIC_INLINE void ln_uart_dmar_setf(uint32_t base, uint8_t dmar)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.dmar = dmar;
}

__STATIC_INLINE void ln_uart_iren_setf(uint32_t base, uint8_t iren)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.iren = iren;
}

__STATIC_INLINE void ln_uart_eie_setf(uint32_t base, uint8_t eie)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.eie = eie;
}

__STATIC_INLINE void ln_uart_uart_brr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_ln_uart_t*)(base))->uart_brr.val = value;
}

__STATIC_INLINE void ln_uart_brr_setf(uint32_t base, uint32_t brr)
{
    ((t_hwp_ln_uart_t*)(base))->uart_brr.bit_field.brr = brr;
}

__STATIC_INLINE void ln_uart_uart_isr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_ln_uart_t*)(base))->uart_isr.val = value;
}

__STATIC_INLINE void ln_uart_rwu_setf(uint32_t base, uint8_t rwu)
{
    ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.rwu = rwu;
}

__STATIC_INLINE void ln_uart_uart_icr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_ln_uart_t*)(base))->uart_icr.val = value;
}

__STATIC_INLINE void ln_uart_wucf_setf(uint32_t base, uint8_t wucf)
{
    ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.wucf = wucf;
}

__STATIC_INLINE void ln_uart_cmcf_setf(uint32_t base, uint8_t cmcf)
{
    ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.cmcf = cmcf;
}

__STATIC_INLINE void ln_uart_ctscf_setf(uint32_t base, uint8_t ctscf)
{
    ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.ctscf = ctscf;
}

__STATIC_INLINE void ln_uart_tccf_setf(uint32_t base, uint8_t tccf)
{
    ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.tccf = tccf;
}

__STATIC_INLINE void ln_uart_idlecf_setf(uint32_t base, uint8_t idlecf)
{
    ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.idlecf = idlecf;
}

__STATIC_INLINE void ln_uart_ncf_setf(uint32_t base, uint8_t ncf)
{
    ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.ncf = ncf;
}

__STATIC_INLINE void ln_uart_fecf_setf(uint32_t base, uint8_t fecf)
{
    ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.fecf = fecf;
}

__STATIC_INLINE void ln_uart_pecf_setf(uint32_t base, uint8_t pecf)
{
    ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.pecf = pecf;
}

__STATIC_INLINE void ln_uart_uart_tdr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_ln_uart_t*)(base))->uart_tdr.val = value;
}

__STATIC_INLINE void ln_uart_tdr_setf(uint32_t base, uint16_t tdr)
{
    ((t_hwp_ln_uart_t*)(base))->uart_tdr.bit_field.tdr = tdr;
}

__STATIC_INLINE void ln_uart_uart_fifo_set(uint32_t base, uint32_t value)
{
    ((t_hwp_ln_uart_t*)(base))->uart_fifo.val = value;
}

__STATIC_INLINE void ln_uart_rx_byte_setf(uint32_t base, uint8_t rx_byte)
{
    ((t_hwp_ln_uart_t*)(base))->uart_fifo.bit_field.rx_byte = rx_byte;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE uint32_t ln_uart_uart_cr1_get(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.val;
}

__STATIC_INLINE uint8_t ln_uart_m_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.m;
}

__STATIC_INLINE uint8_t ln_uart_tout_en_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.tout_en;
}

__STATIC_INLINE uint8_t ln_uart_tout_set_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.tout_set;
}

__STATIC_INLINE uint8_t ln_uart_over8_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.over8;
}

__STATIC_INLINE uint8_t ln_uart_cmie_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.cmie;
}

__STATIC_INLINE uint8_t ln_uart_mme_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.mme;
}

__STATIC_INLINE uint8_t ln_uart_rxafie_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.rxafie;
}

__STATIC_INLINE uint8_t ln_uart_wake_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.wake;
}

__STATIC_INLINE uint8_t ln_uart_pce_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.pce;
}

__STATIC_INLINE uint8_t ln_uart_ps_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.ps;
}

__STATIC_INLINE uint8_t ln_uart_peie_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.peie;
}

__STATIC_INLINE uint8_t ln_uart_txeie_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.txeie;
}

__STATIC_INLINE uint8_t ln_uart_tcie_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.tcie;
}

__STATIC_INLINE uint8_t ln_uart_rxneie_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.rxneie;
}

__STATIC_INLINE uint8_t ln_uart_idleie_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.idleie;
}

__STATIC_INLINE uint8_t ln_uart_te_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.te;
}

__STATIC_INLINE uint8_t ln_uart_re_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.re;
}

__STATIC_INLINE uint8_t ln_uart_uesm_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.uesm;
}

__STATIC_INLINE uint8_t ln_uart_ue_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr1.bit_field.ue;
}

__STATIC_INLINE uint32_t ln_uart_uart_cr2_get(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr2.val;
}

__STATIC_INLINE uint8_t ln_uart_add_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr2.bit_field.add;
}

__STATIC_INLINE uint8_t ln_uart_msbfirst_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr2.bit_field.msbfirst;
}

__STATIC_INLINE uint8_t ln_uart_stop_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr2.bit_field.stop;
}

__STATIC_INLINE uint8_t ln_uart_addm7_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr2.bit_field.addm7;
}

__STATIC_INLINE uint32_t ln_uart_uart_cr3_get(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr3.val;
}

__STATIC_INLINE uint8_t ln_uart_wufie_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.wufie;
}

__STATIC_INLINE uint8_t ln_uart_wus_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.wus;
}

__STATIC_INLINE uint8_t ln_uart_ddre_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.ddre;
}

__STATIC_INLINE uint8_t ln_uart_ovrdis_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.ovrdis;
}

__STATIC_INLINE uint8_t ln_uart_onebit_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.onebit;
}

__STATIC_INLINE uint8_t ln_uart_ctsie_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.ctsie;
}

__STATIC_INLINE uint8_t ln_uart_ctse_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.ctse;
}

__STATIC_INLINE uint8_t ln_uart_rtse_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.rtse;
}

__STATIC_INLINE uint8_t ln_uart_dmat_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.dmat;
}

__STATIC_INLINE uint8_t ln_uart_dmar_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.dmar;
}

__STATIC_INLINE uint8_t ln_uart_iren_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.iren;
}

__STATIC_INLINE uint8_t ln_uart_eie_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_cr3.bit_field.eie;
}

__STATIC_INLINE uint32_t ln_uart_uart_brr_get(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_brr.val;
}

__STATIC_INLINE uint32_t ln_uart_brr_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_brr.bit_field.brr;
}

__STATIC_INLINE uint32_t ln_uart_uart_isr_get(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.val;
}

__STATIC_INLINE uint8_t ln_uart_tx_full_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.tx_full;
}

__STATIC_INLINE uint8_t ln_uart_rx_afull_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.rx_afull;
}

__STATIC_INLINE uint8_t ln_uart_wuf_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.wuf;
}

__STATIC_INLINE uint8_t ln_uart_rwu_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.rwu;
}

__STATIC_INLINE uint8_t ln_uart_cmf_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.cmf;
}

__STATIC_INLINE uint8_t ln_uart_busy_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.busy;
}

__STATIC_INLINE uint8_t ln_uart_cts_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.cts;
}

__STATIC_INLINE uint8_t ln_uart_ctsif_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.ctsif;
}

__STATIC_INLINE uint8_t ln_uart_txef_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.txef;
}

__STATIC_INLINE uint8_t ln_uart_tc_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.tc;
}

__STATIC_INLINE uint8_t ln_uart_rxne_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.rxne;
}

__STATIC_INLINE uint8_t ln_uart_idle_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.idle;
}

__STATIC_INLINE uint8_t ln_uart_ore_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.ore;
}

__STATIC_INLINE uint8_t ln_uart_n_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.n;
}

__STATIC_INLINE uint8_t ln_uart_fe_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.fe;
}

__STATIC_INLINE uint8_t ln_uart_pe_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_isr.bit_field.pe;
}

__STATIC_INLINE uint32_t ln_uart_uart_icr_get(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_icr.val;
}

__STATIC_INLINE uint8_t ln_uart_wucf_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.wucf;
}

__STATIC_INLINE uint8_t ln_uart_cmcf_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.cmcf;
}

__STATIC_INLINE uint8_t ln_uart_ctscf_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.ctscf;
}

__STATIC_INLINE uint8_t ln_uart_tccf_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.tccf;
}

__STATIC_INLINE uint8_t ln_uart_idlecf_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.idlecf;
}

__STATIC_INLINE uint8_t ln_uart_ncf_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.ncf;
}

__STATIC_INLINE uint8_t ln_uart_fecf_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.fecf;
}

__STATIC_INLINE uint8_t ln_uart_pecf_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_icr.bit_field.pecf;
}

__STATIC_INLINE uint32_t ln_uart_uart_rdr_get(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_rdr.val;
}

__STATIC_INLINE uint16_t ln_uart_rxdat_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_rdr.bit_field.rxdat;
}

__STATIC_INLINE uint32_t ln_uart_uart_tdr_get(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_tdr.val;
}

__STATIC_INLINE uint16_t ln_uart_tdr_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_tdr.bit_field.tdr;
}

__STATIC_INLINE uint32_t ln_uart_uart_fifo_get(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_fifo.val;
}

__STATIC_INLINE uint8_t ln_uart_tx_fifo_level_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_fifo.bit_field.tx_fifo_level;
}

__STATIC_INLINE uint8_t ln_uart_rx_fifo_level_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_fifo.bit_field.rx_fifo_level;
}

__STATIC_INLINE uint8_t ln_uart_rx_byte_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_fifo.bit_field.rx_byte;
}

__STATIC_INLINE uint32_t ln_uart_uart_misc_get(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_misc.val;
}

__STATIC_INLINE uint8_t ln_uart_rx_full_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_misc.bit_field.rx_full;
}

__STATIC_INLINE uint8_t ln_uart_tx_over_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_misc.bit_field.tx_over;
}

__STATIC_INLINE uint8_t ln_uart_tx_under_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_misc.bit_field.tx_under;
}

__STATIC_INLINE uint8_t ln_uart_rx_under_getf(uint32_t base)
{
    return ((t_hwp_ln_uart_t*)(base))->uart_misc.bit_field.rx_under;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void ln_uart_uart_cr1_pack(uint32_t base, uint8_t m, uint8_t tout_en, uint8_t tout_set, uint8_t over8, uint8_t cmie, uint8_t mme, uint8_t rxafie, uint8_t wake, uint8_t pce, uint8_t ps, uint8_t peie, uint8_t txeie, uint8_t tcie, uint8_t rxneie, uint8_t idleie, uint8_t te, uint8_t re, uint8_t uesm, uint8_t ue)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr1.val = ( \
                                    ((uint32_t )m                 << 28) \
                                  | ((uint32_t )tout_en           << 20) \
                                  | ((uint32_t )tout_set          << 16) \
                                  | ((uint32_t )over8             << 15) \
                                  | ((uint32_t )cmie              << 14) \
                                  | ((uint32_t )mme               << 13) \
                                  | ((uint32_t )rxafie            << 12) \
                                  | ((uint32_t )wake              << 11) \
                                  | ((uint32_t )pce               << 10) \
                                  | ((uint32_t )ps                << 9) \
                                  | ((uint32_t )peie              << 8) \
                                  | ((uint32_t )txeie             << 7) \
                                  | ((uint32_t )tcie              << 6) \
                                  | ((uint32_t )rxneie            << 5) \
                                  | ((uint32_t )idleie            << 4) \
                                  | ((uint32_t )te                << 3) \
                                  | ((uint32_t )re                << 2) \
                                  | ((uint32_t )uesm              << 1) \
                                  | ((uint32_t )ue) \
                                  );
}

__STATIC_INLINE void ln_uart_uart_cr2_pack(uint32_t base, uint8_t add, uint8_t msbfirst, uint8_t stop, uint8_t addm7)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr2.val = ( \
                                    ((uint32_t )add               << 24) \
                                  | ((uint32_t )msbfirst          << 19) \
                                  | ((uint32_t )stop              << 12) \
                                  | ((uint32_t )addm7             << 4) \
                                  );
}

__STATIC_INLINE void ln_uart_uart_cr3_pack(uint32_t base, uint8_t wufie, uint8_t wus, uint8_t ddre, uint8_t ovrdis, uint8_t onebit, uint8_t ctsie, uint8_t ctse, uint8_t rtse, uint8_t dmat, uint8_t dmar, uint8_t iren, uint8_t eie)
{
    ((t_hwp_ln_uart_t*)(base))->uart_cr3.val = ( \
                                    ((uint32_t )wufie             << 22) \
                                  | ((uint32_t )wus               << 20) \
                                  | ((uint32_t )ddre              << 13) \
                                  | ((uint32_t )ovrdis            << 12) \
                                  | ((uint32_t )onebit            << 11) \
                                  | ((uint32_t )ctsie             << 10) \
                                  | ((uint32_t )ctse              << 9) \
                                  | ((uint32_t )rtse              << 8) \
                                  | ((uint32_t )dmat              << 7) \
                                  | ((uint32_t )dmar              << 6) \
                                  | ((uint32_t )iren              << 1) \
                                  | ((uint32_t )eie) \
                                  );
}

__STATIC_INLINE void ln_uart_uart_icr_pack(uint32_t base, uint8_t wucf, uint8_t cmcf, uint8_t ctscf, uint8_t tccf, uint8_t idlecf, uint8_t ncf, uint8_t fecf, uint8_t pecf)
{
    ((t_hwp_ln_uart_t*)(base))->uart_icr.val = ( \
                                    ((uint32_t )wucf              << 20) \
                                  | ((uint32_t )cmcf              << 17) \
                                  | ((uint32_t )ctscf             << 9) \
                                  | ((uint32_t )tccf              << 6) \
                                  | ((uint32_t )idlecf            << 4) \
                                  | ((uint32_t )ncf               << 2) \
                                  | ((uint32_t )fecf              << 1) \
                                  | ((uint32_t )pecf) \
                                  );
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void ln_uart_uart_cr1_unpack(uint32_t base, uint8_t * m, uint8_t * tout_en, uint8_t * tout_set, uint8_t * over8, uint8_t * cmie, uint8_t * mme, uint8_t * rxafie, uint8_t * wake, uint8_t * pce, uint8_t * ps, uint8_t * peie, uint8_t * txeie, uint8_t * tcie, uint8_t * rxneie, uint8_t * idleie, uint8_t * te, uint8_t * re, uint8_t * uesm, uint8_t * ue)
{
    t_ln_uart_uart_cr1 local_val = ((t_hwp_ln_uart_t*)(base))->uart_cr1;

    *m                  = local_val.bit_field.m;
    *tout_en            = local_val.bit_field.tout_en;
    *tout_set           = local_val.bit_field.tout_set;
    *over8              = local_val.bit_field.over8;
    *cmie               = local_val.bit_field.cmie;
    *mme                = local_val.bit_field.mme;
    *rxafie             = local_val.bit_field.rxafie;
    *wake               = local_val.bit_field.wake;
    *pce                = local_val.bit_field.pce;
    *ps                 = local_val.bit_field.ps;
    *peie               = local_val.bit_field.peie;
    *txeie              = local_val.bit_field.txeie;
    *tcie               = local_val.bit_field.tcie;
    *rxneie             = local_val.bit_field.rxneie;
    *idleie             = local_val.bit_field.idleie;
    *te                 = local_val.bit_field.te;
    *re                 = local_val.bit_field.re;
    *uesm               = local_val.bit_field.uesm;
    *ue                 = local_val.bit_field.ue;
}

__STATIC_INLINE void ln_uart_uart_cr2_unpack(uint32_t base, uint8_t * add, uint8_t * msbfirst, uint8_t * stop, uint8_t * addm7)
{
    t_ln_uart_uart_cr2 local_val = ((t_hwp_ln_uart_t*)(base))->uart_cr2;

    *add                = local_val.bit_field.add;
    *msbfirst           = local_val.bit_field.msbfirst;
    *stop               = local_val.bit_field.stop;
    *addm7              = local_val.bit_field.addm7;
}

__STATIC_INLINE void ln_uart_uart_cr3_unpack(uint32_t base, uint8_t * wufie, uint8_t * wus, uint8_t * ddre, uint8_t * ovrdis, uint8_t * onebit, uint8_t * ctsie, uint8_t * ctse, uint8_t * rtse, uint8_t * dmat, uint8_t * dmar, uint8_t * iren, uint8_t * eie)
{
    t_ln_uart_uart_cr3 local_val = ((t_hwp_ln_uart_t*)(base))->uart_cr3;

    *wufie              = local_val.bit_field.wufie;
    *wus                = local_val.bit_field.wus;
    *ddre               = local_val.bit_field.ddre;
    *ovrdis             = local_val.bit_field.ovrdis;
    *onebit             = local_val.bit_field.onebit;
    *ctsie              = local_val.bit_field.ctsie;
    *ctse               = local_val.bit_field.ctse;
    *rtse               = local_val.bit_field.rtse;
    *dmat               = local_val.bit_field.dmat;
    *dmar               = local_val.bit_field.dmar;
    *iren               = local_val.bit_field.iren;
    *eie                = local_val.bit_field.eie;
}

__STATIC_INLINE void ln_uart_uart_isr_unpack(uint32_t base, uint8_t * tx_full, uint8_t * rx_afull, uint8_t * wuf, uint8_t * rwu, uint8_t * cmf, uint8_t * busy, uint8_t * cts, uint8_t * ctsif, uint8_t * txef, uint8_t * tc, uint8_t * rxne, uint8_t * idle, uint8_t * ore, uint8_t * n, uint8_t * fe, uint8_t * pe)
{
    t_ln_uart_uart_isr local_val = ((t_hwp_ln_uart_t*)(base))->uart_isr;

    *tx_full            = local_val.bit_field.tx_full;
    *rx_afull           = local_val.bit_field.rx_afull;
    *wuf                = local_val.bit_field.wuf;
    *rwu                = local_val.bit_field.rwu;
    *cmf                = local_val.bit_field.cmf;
    *busy               = local_val.bit_field.busy;
    *cts                = local_val.bit_field.cts;
    *ctsif              = local_val.bit_field.ctsif;
    *txef               = local_val.bit_field.txef;
    *tc                 = local_val.bit_field.tc;
    *rxne               = local_val.bit_field.rxne;
    *idle               = local_val.bit_field.idle;
    *ore                = local_val.bit_field.ore;
    *n                  = local_val.bit_field.n;
    *fe                 = local_val.bit_field.fe;
    *pe                 = local_val.bit_field.pe;
}

__STATIC_INLINE void ln_uart_uart_icr_unpack(uint32_t base, uint8_t * wucf, uint8_t * cmcf, uint8_t * ctscf, uint8_t * tccf, uint8_t * idlecf, uint8_t * ncf, uint8_t * fecf, uint8_t * pecf)
{
    t_ln_uart_uart_icr local_val = ((t_hwp_ln_uart_t*)(base))->uart_icr;

    *wucf               = local_val.bit_field.wucf;
    *cmcf               = local_val.bit_field.cmcf;
    *ctscf              = local_val.bit_field.ctscf;
    *tccf               = local_val.bit_field.tccf;
    *idlecf             = local_val.bit_field.idlecf;
    *ncf                = local_val.bit_field.ncf;
    *fecf               = local_val.bit_field.fecf;
    *pecf               = local_val.bit_field.pecf;
}

__STATIC_INLINE void ln_uart_uart_fifo_unpack(uint32_t base, uint8_t * tx_fifo_level, uint8_t * rx_fifo_level, uint8_t * rx_byte)
{
    t_ln_uart_uart_fifo local_val = ((t_hwp_ln_uart_t*)(base))->uart_fifo;

    *tx_fifo_level      = local_val.bit_field.tx_fifo_level;
    *rx_fifo_level      = local_val.bit_field.rx_fifo_level;
    *rx_byte            = local_val.bit_field.rx_byte;
}

__STATIC_INLINE void ln_uart_uart_misc_unpack(uint32_t base, uint8_t * rx_full, uint8_t * tx_over, uint8_t * tx_under, uint8_t * rx_under)
{
    t_ln_uart_uart_misc local_val = ((t_hwp_ln_uart_t*)(base))->uart_misc;

    *rx_full            = local_val.bit_field.rx_full;
    *tx_over            = local_val.bit_field.tx_over;
    *tx_under           = local_val.bit_field.tx_under;
    *rx_under           = local_val.bit_field.rx_under;
}

#endif

