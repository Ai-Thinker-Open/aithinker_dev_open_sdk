#ifndef __REG_I2C_H__
#define __REG_I2C_H__

#include "ln882h.h"

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pe                  : 1  ; // 0  : 0
        uint32_t    smbus               : 1  ; // 1  : 1
        uint32_t    reserved_2          : 1  ; // 2  : 2
        uint32_t    smbtype             : 1  ; // 3  : 3
        uint32_t    enarp               : 1  ; // 4  : 4
        uint32_t    enpec               : 1  ; // 5  : 5
        uint32_t    engc                : 1  ; // 6  : 6
        uint32_t    nostretch           : 1  ; // 7  : 7
        uint32_t    start               : 1  ; // 8  : 8
        uint32_t    stop                : 1  ; // 9  : 9
        uint32_t    ack                 : 1  ; // 10 : 10
        uint32_t    pos                 : 1  ; // 11 : 11
        uint32_t    pec                 : 1  ; // 12 : 12
        uint32_t    alert               : 1  ; // 13 : 13
        uint32_t    reserved_1          : 1  ; // 14 : 14
        uint32_t    swrst               : 1  ; // 15 : 15
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_i2c_cr1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    freq                : 6  ; // 5  : 0
        uint32_t    reserved_1          : 2  ; // 7  : 6
        uint32_t    iterren             : 1  ; // 8  : 8
        uint32_t    itevten             : 1  ; // 9  : 9
        uint32_t    itbufen             : 1  ; // 10 : 10
        uint32_t    dmaen               : 1  ; // 11 : 11
        uint32_t    last                : 1  ; // 12 : 12
        uint32_t    reserved_0          : 19 ; // 31 : 13
    } bit_field;
} t_i2c_cr2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    add                 : 10 ; // 9  : 0
        uint32_t    reserved_1          : 5  ; // 14 : 10
        uint32_t    addmode             : 1  ; // 15 : 15
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_i2c_oar1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    endual              : 1  ; // 0  : 0
        uint32_t    add2                : 7  ; // 7  : 1
        uint32_t    reserved_0          : 24 ; // 31 : 8
    } bit_field;
} t_i2c_oar2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    dr                  : 8  ; // 7  : 0
        uint32_t    reserved_0          : 24 ; // 31 : 8
    } bit_field;
} t_i2c_dr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    sb                  : 1  ; // 0  : 0
        uint32_t    addr                : 1  ; // 1  : 1
        uint32_t    btf                 : 1  ; // 2  : 2
        uint32_t    add10               : 1  ; // 3  : 3
        uint32_t    stopf               : 1  ; // 4  : 4
        uint32_t    reserved_2          : 1  ; // 5  : 5
        uint32_t    rxne                : 1  ; // 6  : 6
        uint32_t    txe                 : 1  ; // 7  : 7
        uint32_t    berr                : 1  ; // 8  : 8
        uint32_t    arlo                : 1  ; // 9  : 9
        uint32_t    af                  : 1  ; // 10 : 10
        uint32_t    ovr                 : 1  ; // 11 : 11
        uint32_t    pecerr              : 1  ; // 12 : 12
        uint32_t    reserved_1          : 1  ; // 13 : 13
        uint32_t    timeout             : 1  ; // 14 : 14
        uint32_t    smbalert            : 1  ; // 15 : 15
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_i2c_sr1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    msl                 : 1  ; // 0  : 0
        uint32_t    busy                : 1  ; // 1  : 1
        uint32_t    tra                 : 1  ; // 2  : 2
        uint32_t    reserved_1          : 1  ; // 3  : 3
        uint32_t    gencall             : 1  ; // 4  : 4
        uint32_t    smbdefault          : 1  ; // 5  : 5
        uint32_t    smbhost             : 1  ; // 6  : 6
        uint32_t    dualf               : 1  ; // 7  : 7
        uint32_t    pec1                : 8  ; // 15 : 8
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_i2c_sr2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ccr                 : 12 ; // 11 : 0
        uint32_t    reserved_1          : 2  ; // 13 : 12
        uint32_t    duty                : 1  ; // 14 : 14
        uint32_t    fs                  : 1  ; // 15 : 15
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_i2c_ccr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    trise               : 6  ; // 5  : 0
        uint32_t    reserved_0          : 26 ; // 31 : 6
    } bit_field;
} t_i2c_trise;


//--------------------------------------------------------------------------------

typedef struct
{
    volatile        t_i2c_cr1                                i2c_cr1                                ; // 0x0
    volatile        t_i2c_cr2                                i2c_cr2                                ; // 0x4
    volatile        t_i2c_oar1                               i2c_oar1                               ; // 0x8
    volatile        t_i2c_oar2                               i2c_oar2                               ; // 0xc
    volatile        t_i2c_dr                                 i2c_dr                                 ; // 0x10
    volatile        t_i2c_sr1                                i2c_sr1                                ; // 0x14
    volatile        t_i2c_sr2                                i2c_sr2                                ; // 0x18
    volatile        t_i2c_ccr                                i2c_ccr                                ; // 0x1c
    volatile        t_i2c_trise                              i2c_trise                              ; // 0x20
} t_hwp_i2c_t;


//--------------------------------------------------------------------------------

__STATIC_INLINE void i2c_cr1_set(uint32_t base, uint32_t value)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.val = value;
}

__STATIC_INLINE void i2c_swrst_setf(uint32_t base, uint8_t swrst)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.swrst = swrst;
}

__STATIC_INLINE void i2c_alert_setf(uint32_t base, uint8_t alert)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.alert = alert;
}

__STATIC_INLINE void i2c_pec_setf(uint32_t base, uint8_t pec)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.pec = pec;
}

__STATIC_INLINE void i2c_pos_setf(uint32_t base, uint8_t pos)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.pos = pos;
}

__STATIC_INLINE void i2c_ack_setf(uint32_t base, uint8_t ack)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.ack = ack;
}

__STATIC_INLINE void i2c_stop_setf(uint32_t base, uint8_t stop)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.stop = stop;
}

__STATIC_INLINE void i2c_start_setf(uint32_t base, uint8_t start)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.start = start;
}

__STATIC_INLINE void i2c_nostretch_setf(uint32_t base, uint8_t nostretch)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.nostretch = nostretch;
}

__STATIC_INLINE void i2c_engc_setf(uint32_t base, uint8_t engc)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.engc = engc;
}

__STATIC_INLINE void i2c_enpec_setf(uint32_t base, uint8_t enpec)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.enpec = enpec;
}

__STATIC_INLINE void i2c_enarp_setf(uint32_t base, uint8_t enarp)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.enarp = enarp;
}

__STATIC_INLINE void i2c_smbtype_setf(uint32_t base, uint8_t smbtype)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.smbtype = smbtype;
}

__STATIC_INLINE void i2c_smbus_setf(uint32_t base, uint8_t smbus)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.smbus = smbus;
}

__STATIC_INLINE void i2c_pe_setf(uint32_t base, uint8_t pe)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.pe = pe;
}

__STATIC_INLINE void i2c_cr2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr2.val = value;
}

__STATIC_INLINE void i2c_last_setf(uint32_t base, uint8_t last)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr2.bit_field.last = last;
}

__STATIC_INLINE void i2c_dmaen_setf(uint32_t base, uint8_t dmaen)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr2.bit_field.dmaen = dmaen;
}

__STATIC_INLINE void i2c_itbufen_setf(uint32_t base, uint8_t itbufen)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr2.bit_field.itbufen = itbufen;
}

__STATIC_INLINE void i2c_itevten_setf(uint32_t base, uint8_t itevten)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr2.bit_field.itevten = itevten;
}

__STATIC_INLINE void i2c_iterren_setf(uint32_t base, uint8_t iterren)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr2.bit_field.iterren = iterren;
}

__STATIC_INLINE void i2c_freq_setf(uint32_t base, uint8_t freq)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr2.bit_field.freq = freq;
}

__STATIC_INLINE void i2c_oar1_set(uint32_t base, uint32_t value)
{
    ((t_hwp_i2c_t*)(base))->i2c_oar1.val = value;
}

__STATIC_INLINE void i2c_addmode_setf(uint32_t base, uint8_t addmode)
{
    ((t_hwp_i2c_t*)(base))->i2c_oar1.bit_field.addmode = addmode;
}

__STATIC_INLINE void i2c_add_setf(uint32_t base, uint16_t add)
{
    ((t_hwp_i2c_t*)(base))->i2c_oar1.bit_field.add = add;
}

__STATIC_INLINE void i2c_oar2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_i2c_t*)(base))->i2c_oar2.val = value;
}

__STATIC_INLINE void i2c_add2_setf(uint32_t base, uint8_t add2)
{
    ((t_hwp_i2c_t*)(base))->i2c_oar2.bit_field.add2 = add2;
}

__STATIC_INLINE void i2c_endual_setf(uint32_t base, uint8_t endual)
{
    ((t_hwp_i2c_t*)(base))->i2c_oar2.bit_field.endual = endual;
}

__STATIC_INLINE void i2c_dr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_i2c_t*)(base))->i2c_dr.val = value;
}

__STATIC_INLINE void i2c_dr_setf(uint32_t base, uint8_t dr)
{
    ((t_hwp_i2c_t*)(base))->i2c_dr.bit_field.dr = dr;
}

__STATIC_INLINE void i2c_sr1_set(uint32_t base, uint32_t value)
{
    ((t_hwp_i2c_t*)(base))->i2c_sr1.val = value;
}

__STATIC_INLINE void i2c_smbalert_setf(uint32_t base, uint8_t smbalert)
{
    ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.smbalert = smbalert;
}

__STATIC_INLINE void i2c_timeout_setf(uint32_t base, uint8_t timeout)
{
    ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.timeout = timeout;
}

__STATIC_INLINE void i2c_pecerr_setf(uint32_t base, uint8_t pecerr)
{
    ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.pecerr = pecerr;
}

__STATIC_INLINE void i2c_ovr_setf(uint32_t base, uint8_t ovr)
{
    ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.ovr = ovr;
}

__STATIC_INLINE void i2c_af_setf(uint32_t base, uint8_t af)
{
    ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.af = af;
}

__STATIC_INLINE void i2c_arlo_setf(uint32_t base, uint8_t arlo)
{
    ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.arlo = arlo;
}

__STATIC_INLINE void i2c_berr_setf(uint32_t base, uint8_t berr)
{
    ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.berr = berr;
}

__STATIC_INLINE void i2c_stopf_setf(uint32_t base, uint8_t stopf)
{
    ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.stopf = stopf;
}

__STATIC_INLINE void i2c_ccr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_i2c_t*)(base))->i2c_ccr.val = value;
}

__STATIC_INLINE void i2c_fs_setf(uint32_t base, uint8_t fs)
{
    ((t_hwp_i2c_t*)(base))->i2c_ccr.bit_field.fs = fs;
}

__STATIC_INLINE void i2c_duty_setf(uint32_t base, uint8_t duty)
{
    ((t_hwp_i2c_t*)(base))->i2c_ccr.bit_field.duty = duty;
}

__STATIC_INLINE void i2c_ccr_setf(uint32_t base, uint16_t ccr)
{
    ((t_hwp_i2c_t*)(base))->i2c_ccr.bit_field.ccr = ccr;
}

__STATIC_INLINE void i2c_trise_set(uint32_t base, uint32_t value)
{
    ((t_hwp_i2c_t*)(base))->i2c_trise.val = value;
}

__STATIC_INLINE void i2c_trise_setf(uint32_t base, uint8_t trise)
{
    ((t_hwp_i2c_t*)(base))->i2c_trise.bit_field.trise = trise;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE uint32_t i2c_cr1_get(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.val;
}

__STATIC_INLINE uint8_t i2c_swrst_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.swrst;
}

__STATIC_INLINE uint8_t i2c_alert_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.alert;
}

__STATIC_INLINE uint8_t i2c_pec_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.pec;
}

__STATIC_INLINE uint8_t i2c_pos_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.pos;
}

__STATIC_INLINE uint8_t i2c_ack_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.ack;
}

__STATIC_INLINE uint8_t i2c_stop_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.stop;
}

__STATIC_INLINE uint8_t i2c_start_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.start;
}

__STATIC_INLINE uint8_t i2c_nostretch_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.nostretch;
}

__STATIC_INLINE uint8_t i2c_engc_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.engc;
}

__STATIC_INLINE uint8_t i2c_enpec_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.enpec;
}

__STATIC_INLINE uint8_t i2c_enarp_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.enarp;
}

__STATIC_INLINE uint8_t i2c_smbtype_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.smbtype;
}

__STATIC_INLINE uint8_t i2c_smbus_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.smbus;
}

__STATIC_INLINE uint8_t i2c_pe_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr1.bit_field.pe;
}

__STATIC_INLINE uint32_t i2c_cr2_get(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr2.val;
}

__STATIC_INLINE uint8_t i2c_last_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr2.bit_field.last;
}

__STATIC_INLINE uint8_t i2c_dmaen_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr2.bit_field.dmaen;
}

__STATIC_INLINE uint8_t i2c_itbufen_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr2.bit_field.itbufen;
}

__STATIC_INLINE uint8_t i2c_itevten_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr2.bit_field.itevten;
}

__STATIC_INLINE uint8_t i2c_iterren_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr2.bit_field.iterren;
}

__STATIC_INLINE uint8_t i2c_freq_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_cr2.bit_field.freq;
}

__STATIC_INLINE uint32_t i2c_oar1_get(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_oar1.val;
}

__STATIC_INLINE uint8_t i2c_addmode_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_oar1.bit_field.addmode;
}

__STATIC_INLINE uint16_t i2c_add_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_oar1.bit_field.add;
}

__STATIC_INLINE uint32_t i2c_oar2_get(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_oar2.val;
}

__STATIC_INLINE uint8_t i2c_add2_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_oar2.bit_field.add2;
}

__STATIC_INLINE uint8_t i2c_endual_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_oar2.bit_field.endual;
}

__STATIC_INLINE uint32_t i2c_dr_get(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_dr.val;
}

__STATIC_INLINE uint8_t i2c_dr_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_dr.bit_field.dr;
}

__STATIC_INLINE uint32_t i2c_sr1_get(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.val;
}

__STATIC_INLINE uint8_t i2c_smbalert_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.smbalert;
}

__STATIC_INLINE uint8_t i2c_timeout_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.timeout;
}

__STATIC_INLINE uint8_t i2c_pecerr_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.pecerr;
}

__STATIC_INLINE uint8_t i2c_ovr_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.ovr;
}

__STATIC_INLINE uint8_t i2c_af_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.af;
}

__STATIC_INLINE uint8_t i2c_arlo_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.arlo;
}

__STATIC_INLINE uint8_t i2c_berr_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.berr;
}

__STATIC_INLINE uint8_t i2c_txe_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.txe;
}

__STATIC_INLINE uint8_t i2c_rxne_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.rxne;
}

__STATIC_INLINE uint8_t i2c_stopf_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.stopf;
}

__STATIC_INLINE uint8_t i2c_add10_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.add10;
}

__STATIC_INLINE uint8_t i2c_btf_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.btf;
}

__STATIC_INLINE uint8_t i2c_addr_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.addr;
}

__STATIC_INLINE uint8_t i2c_sb_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr1.bit_field.sb;
}

__STATIC_INLINE uint32_t i2c_sr2_get(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr2.val;
}

__STATIC_INLINE uint8_t i2c_pec1_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr2.bit_field.pec1;
}

__STATIC_INLINE uint8_t i2c_dualf_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr2.bit_field.dualf;
}

__STATIC_INLINE uint8_t i2c_smbhost_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr2.bit_field.smbhost;
}

__STATIC_INLINE uint8_t i2c_smbdefault_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr2.bit_field.smbdefault;
}

__STATIC_INLINE uint8_t i2c_gencall_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr2.bit_field.gencall;
}

__STATIC_INLINE uint8_t i2c_tra_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr2.bit_field.tra;
}

__STATIC_INLINE uint8_t i2c_busy_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr2.bit_field.busy;
}

__STATIC_INLINE uint8_t i2c_msl_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_sr2.bit_field.msl;
}

__STATIC_INLINE uint32_t i2c_ccr_get(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_ccr.val;
}

__STATIC_INLINE uint8_t i2c_fs_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_ccr.bit_field.fs;
}

__STATIC_INLINE uint8_t i2c_duty_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_ccr.bit_field.duty;
}

__STATIC_INLINE uint16_t i2c_ccr_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_ccr.bit_field.ccr;
}

__STATIC_INLINE uint32_t i2c_trise_get(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_trise.val;
}

__STATIC_INLINE uint8_t i2c_trise_getf(uint32_t base)
{
    return ((t_hwp_i2c_t*)(base))->i2c_trise.bit_field.trise;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void i2c_cr1_pack(uint32_t base, uint8_t swrst, uint8_t alert, uint8_t pec, uint8_t pos, uint8_t ack, uint8_t stop, uint8_t start, uint8_t nostretch, uint8_t engc, uint8_t enpec, uint8_t enarp, uint8_t smbtype, uint8_t smbus, uint8_t pe)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr1.val = ( \
                                    ((uint32_t )swrst             << 15) \
                                  | ((uint32_t )alert             << 13) \
                                  | ((uint32_t )pec               << 12) \
                                  | ((uint32_t )pos               << 11) \
                                  | ((uint32_t )ack               << 10) \
                                  | ((uint32_t )stop              << 9) \
                                  | ((uint32_t )start             << 8) \
                                  | ((uint32_t )nostretch         << 7) \
                                  | ((uint32_t )engc              << 6) \
                                  | ((uint32_t )enpec             << 5) \
                                  | ((uint32_t )enarp             << 4) \
                                  | ((uint32_t )smbtype           << 3) \
                                  | ((uint32_t )smbus             << 1) \
                                  | ((uint32_t )pe) \
                                  );
}

__STATIC_INLINE void i2c_cr2_pack(uint32_t base, uint8_t last, uint8_t dmaen, uint8_t itbufen, uint8_t itevten, uint8_t iterren, uint8_t freq)
{
    ((t_hwp_i2c_t*)(base))->i2c_cr2.val = ( \
                                    ((uint32_t )last              << 12) \
                                  | ((uint32_t )dmaen             << 11) \
                                  | ((uint32_t )itbufen           << 10) \
                                  | ((uint32_t )itevten           << 9) \
                                  | ((uint32_t )iterren           << 8) \
                                  | ((uint32_t )freq) \
                                  );
}

__STATIC_INLINE void i2c_oar1_pack(uint32_t base, uint8_t addmode, uint16_t add)
{
    ((t_hwp_i2c_t*)(base))->i2c_oar1.val = ( \
                                    ((uint32_t )addmode           << 15) \
                                  | ((uint32_t )add) \
                                  );
}

__STATIC_INLINE void i2c_oar2_pack(uint32_t base, uint8_t add2, uint8_t endual)
{
    ((t_hwp_i2c_t*)(base))->i2c_oar2.val = ( \
                                    ((uint32_t )add2              << 1) \
                                  | ((uint32_t )endual) \
                                  );
}

__STATIC_INLINE void i2c_sr1_pack(uint32_t base, uint8_t smbalert, uint8_t timeout, uint8_t pecerr, uint8_t ovr, uint8_t af, uint8_t arlo, uint8_t berr, uint8_t stopf)
{
    ((t_hwp_i2c_t*)(base))->i2c_sr1.val = ( \
                                    ((uint32_t )smbalert          << 15) \
                                  | ((uint32_t )timeout           << 14) \
                                  | ((uint32_t )pecerr            << 12) \
                                  | ((uint32_t )ovr               << 11) \
                                  | ((uint32_t )af                << 10) \
                                  | ((uint32_t )arlo              << 9) \
                                  | ((uint32_t )berr              << 8) \
                                  | ((uint32_t )stopf             << 4) \
                                  );
}

__STATIC_INLINE void i2c_ccr_pack(uint32_t base, uint8_t fs, uint8_t duty, uint16_t ccr)
{
    ((t_hwp_i2c_t*)(base))->i2c_ccr.val = ( \
                                    ((uint32_t )fs                << 15) \
                                  | ((uint32_t )duty              << 14) \
                                  | ((uint32_t )ccr) \
                                  );
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void i2c_cr1_unpack(uint32_t base, uint8_t * swrst, uint8_t * alert, uint8_t * pec, uint8_t * pos, uint8_t * ack, uint8_t * stop, uint8_t * start, uint8_t * nostretch, uint8_t * engc, uint8_t * enpec, uint8_t * enarp, uint8_t * smbtype, uint8_t * smbus, uint8_t * pe)
{
    t_i2c_cr1 local_val = ((t_hwp_i2c_t*)(base))->i2c_cr1;

    *swrst              = local_val.bit_field.swrst;
    *alert              = local_val.bit_field.alert;
    *pec                = local_val.bit_field.pec;
    *pos                = local_val.bit_field.pos;
    *ack                = local_val.bit_field.ack;
    *stop               = local_val.bit_field.stop;
    *start              = local_val.bit_field.start;
    *nostretch          = local_val.bit_field.nostretch;
    *engc               = local_val.bit_field.engc;
    *enpec              = local_val.bit_field.enpec;
    *enarp              = local_val.bit_field.enarp;
    *smbtype            = local_val.bit_field.smbtype;
    *smbus              = local_val.bit_field.smbus;
    *pe                 = local_val.bit_field.pe;
}

__STATIC_INLINE void i2c_cr2_unpack(uint32_t base, uint8_t * last, uint8_t * dmaen, uint8_t * itbufen, uint8_t * itevten, uint8_t * iterren, uint8_t * freq)
{
    t_i2c_cr2 local_val = ((t_hwp_i2c_t*)(base))->i2c_cr2;

    *last               = local_val.bit_field.last;
    *dmaen              = local_val.bit_field.dmaen;
    *itbufen            = local_val.bit_field.itbufen;
    *itevten            = local_val.bit_field.itevten;
    *iterren            = local_val.bit_field.iterren;
    *freq               = local_val.bit_field.freq;
}

__STATIC_INLINE void i2c_oar1_unpack(uint32_t base, uint8_t * addmode, uint16_t * add)
{
    t_i2c_oar1 local_val = ((t_hwp_i2c_t*)(base))->i2c_oar1;

    *addmode            = local_val.bit_field.addmode;
    *add                = local_val.bit_field.add;
}

__STATIC_INLINE void i2c_oar2_unpack(uint32_t base, uint8_t * add2, uint8_t * endual)
{
    t_i2c_oar2 local_val = ((t_hwp_i2c_t*)(base))->i2c_oar2;

    *add2               = local_val.bit_field.add2;
    *endual             = local_val.bit_field.endual;
}

__STATIC_INLINE void i2c_sr1_unpack(uint32_t base, uint8_t * smbalert, uint8_t * timeout, uint8_t * pecerr, uint8_t * ovr, uint8_t * af, uint8_t * arlo, uint8_t * berr, uint8_t * txe, uint8_t * rxne, uint8_t * stopf, uint8_t * add10, uint8_t * btf, uint8_t * addr, uint8_t * sb)
{
    t_i2c_sr1 local_val = ((t_hwp_i2c_t*)(base))->i2c_sr1;

    *smbalert           = local_val.bit_field.smbalert;
    *timeout            = local_val.bit_field.timeout;
    *pecerr             = local_val.bit_field.pecerr;
    *ovr                = local_val.bit_field.ovr;
    *af                 = local_val.bit_field.af;
    *arlo               = local_val.bit_field.arlo;
    *berr               = local_val.bit_field.berr;
    *txe                = local_val.bit_field.txe;
    *rxne               = local_val.bit_field.rxne;
    *stopf              = local_val.bit_field.stopf;
    *add10              = local_val.bit_field.add10;
    *btf                = local_val.bit_field.btf;
    *addr               = local_val.bit_field.addr;
    *sb                 = local_val.bit_field.sb;
}

__STATIC_INLINE void i2c_sr2_unpack(uint32_t base, uint8_t * pec1, uint8_t * dualf, uint8_t * smbhost, uint8_t * smbdefault, uint8_t * gencall, uint8_t * tra, uint8_t * busy, uint8_t * msl)
{
    t_i2c_sr2 local_val = ((t_hwp_i2c_t*)(base))->i2c_sr2;

    *pec1               = local_val.bit_field.pec1;
    *dualf              = local_val.bit_field.dualf;
    *smbhost            = local_val.bit_field.smbhost;
    *smbdefault         = local_val.bit_field.smbdefault;
    *gencall            = local_val.bit_field.gencall;
    *tra                = local_val.bit_field.tra;
    *busy               = local_val.bit_field.busy;
    *msl                = local_val.bit_field.msl;
}

__STATIC_INLINE void i2c_ccr_unpack(uint32_t base, uint8_t * fs, uint8_t * duty, uint16_t * ccr)
{
    t_i2c_ccr local_val = ((t_hwp_i2c_t*)(base))->i2c_ccr;

    *fs                 = local_val.bit_field.fs;
    *duty               = local_val.bit_field.duty;
    *ccr                = local_val.bit_field.ccr;
}

#endif

