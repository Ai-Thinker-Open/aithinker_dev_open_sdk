#ifndef __REG_ADC_H__
#define __REG_ADC_H__

#include "ln882h.h"

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    eoc_0               : 1  ; // 0  : 0 
        uint32_t    eoc_1               : 1  ; // 1  : 1
        uint32_t    eoc_2               : 1  ; // 2  : 2
        uint32_t    eoc_3               : 1  ; // 3  : 3
        uint32_t    eoc_4               : 1  ; // 4  : 4
        uint32_t    eoc_5               : 1  ; // 5  : 5
        uint32_t    eoc_6               : 1  ; // 6  : 6
        uint32_t    eoc_7               : 1  ; // 7  : 7
        uint32_t    eos                 : 1  ; // 8  : 8
        uint32_t    ovr                 : 1  ; // 9  : 9
        uint32_t    awd                 : 1  ; // 10 : 10
        uint32_t    reserved_0          : 21 ; // 31 : 11
    } bit_field;
} t_adc_isr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    eocie               : 8  ; // 7  : 0
        uint32_t    eosie               : 1  ; // 8  : 8
        uint32_t    ovrie               : 1  ; // 9  : 9
        uint32_t    awdie               : 1  ; // 10 : 10
        uint32_t    reserved_0          : 21 ; // 31 : 11
    } bit_field;
} t_adc_ier;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    aden                : 1  ; // 0  : 0
        uint32_t    adstart             : 1  ; // 1  : 1
        uint32_t    adstp               : 1  ; // 2  : 2
        uint32_t    reserved_0          : 29 ; // 31 : 3
    } bit_field;
} t_adc_cr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    extsel              : 2  ; // 1  : 0
        uint32_t    exten               : 2  ; // 3  : 2
        uint32_t    spec_extsel         : 2  ; // 5  : 4
        uint32_t    spec_exten          : 2  ; // 7  : 6
        uint32_t    align               : 1  ; // 8  : 8
        uint32_t    reserved_1          : 4  ; // 12 : 9
        uint32_t    cont                : 1  ; // 13 : 13
        uint32_t    waitm               : 1  ; // 14 : 14
        uint32_t    autoff              : 1  ; // 15 : 15
        uint32_t    awdsgl              : 1  ; // 16 : 16
        uint32_t    awden               : 1  ; // 17 : 17
        uint32_t    awdch               : 3  ; // 20 : 18
        uint32_t    dma_en              : 1  ; // 21
        uint32_t    reserved_0          : 10 ; // 31 : 22
    } bit_field;
} t_adc_cfgr1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ovse                : 8  ; // 7  : 0
        uint32_t    ovsr                : 3  ; // 10 : 8
        uint32_t    mode_sel            : 1  ; // 11 : 11
        uint32_t    vref_sel            : 1  ; // 12 : 12
        uint32_t    reserved_1          : 3  ; // 15 : 13
        uint32_t    stab                : 4  ; // 19 : 16
        uint32_t    cal_en              : 1  ; // 20 : 20
        uint32_t    vset                : 2  ; // 22 : 21
        uint32_t    ibuf_b              : 1  ; // 23 : 23
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_adc_cfgr2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    lt                  : 12 ; // 11 : 0
        uint32_t    reserved_1          : 4  ; // 15 : 12
        uint32_t    ht                  : 12 ; // 27 : 16
        uint32_t    reserved_0          : 4  ; // 31 : 28
    } bit_field;
} t_adc_tr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    chselx              : 8  ; // 7  : 0
        uint32_t    spec_ch             : 3  ; // 10 : 8
        uint32_t    spec_sw_start       : 1  ; // 11 : 11
        uint32_t    reserved_0          : 20 ; // 31 : 12
    } bit_field;
} t_adc_chselr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    presc               : 8  ; // 7  : 0
        uint32_t    auxadc_ckinv        : 1  ; // 8  : 8
        uint32_t    reserved_0          : 23 ; // 31 : 9
    } bit_field;
} t_adc_ccr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    data                : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_adc_dr0;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    data1               : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_adc_dr1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    data2               : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_adc_dr2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    data3               : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_adc_dr3;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    data4               : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_adc_dr4;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    data5               : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_adc_dr5;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    data6               : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_adc_dr6;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    data7               : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_adc_dr7;


//--------------------------------------------------------------------------------

typedef struct
{
    volatile        t_adc_isr                   adc_isr            ; // 0x0
    volatile        t_adc_ier                   adc_ier            ; // 0x4
    volatile        t_adc_cr                    adc_cr             ; // 0x8
    volatile        t_adc_cfgr1                 adc_cfgr1          ; // 0xc
    volatile        t_adc_cfgr2                 adc_cfgr2          ; // 0x10
    volatile        t_adc_tr                    adc_tr             ; // 0x14
    volatile        t_adc_chselr                adc_chselr         ; // 0x18
    volatile        t_adc_ccr                   adc_ccr            ; // 0x1c
    volatile        t_adc_dr0                   adc_dr0            ; // 0x20
    volatile        t_adc_dr1                   adc_dr1            ; // 0x24
    volatile        t_adc_dr2                   adc_dr2            ; // 0x28
    volatile        t_adc_dr3                   adc_dr3            ; // 0x2c
    volatile        t_adc_dr4                   adc_dr4            ; // 0x30
    volatile        t_adc_dr5                   adc_dr5            ; // 0x34
    volatile        t_adc_dr6                   adc_dr6            ; // 0x38
    volatile        t_adc_dr7                   adc_dr7            ; // 0x3c
} t_hwp_adc_t;


//--------------------------------------------------------------------------------

__STATIC_INLINE void adc_isr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_adc_t*)(base))->adc_isr.val = value;
}

__STATIC_INLINE void adc_awd_setf(uint32_t base, uint8_t awd)
{
    ((t_hwp_adc_t*)(base))->adc_isr.bit_field.awd = awd;
}

__STATIC_INLINE void adc_ovr_setf(uint32_t base, uint8_t ovr)
{
    ((t_hwp_adc_t*)(base))->adc_isr.bit_field.ovr = ovr;
}

__STATIC_INLINE void adc_eos_setf(uint32_t base, uint8_t eos)
{
    ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eos = eos;
}

__STATIC_INLINE void adc_eoc_7_setf(uint32_t base, uint8_t eoc_7)
{
    ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_7 = eoc_7;
}

__STATIC_INLINE void adc_eoc_6_setf(uint32_t base, uint8_t eoc_6)
{
    ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_6 = eoc_6;
}

__STATIC_INLINE void adc_eoc_5_setf(uint32_t base, uint8_t eoc_5)
{
    ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_5 = eoc_5;
}

__STATIC_INLINE void adc_eoc_4_setf(uint32_t base, uint8_t eoc_4)
{
    ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_4 = eoc_4;
}

__STATIC_INLINE void adc_eoc_3_setf(uint32_t base, uint8_t eoc_3)
{
    ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_3 = eoc_3;
}

__STATIC_INLINE void adc_eoc_2_setf(uint32_t base, uint8_t eoc_2)
{
    ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_2 = eoc_2;
}

__STATIC_INLINE void adc_eoc_1_setf(uint32_t base, uint8_t eoc_1)
{
    ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_1 = eoc_1;
}

__STATIC_INLINE void adc_eoc_0_setf(uint32_t base, uint8_t eoc_0)
{
    ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_0 = eoc_0;
}

__STATIC_INLINE void adc_ier_set(uint32_t base, uint32_t value)
{
    ((t_hwp_adc_t*)(base))->adc_ier.val = value;
}

__STATIC_INLINE void adc_awdie_setf(uint32_t base, uint8_t awdie)
{
    ((t_hwp_adc_t*)(base))->adc_ier.bit_field.awdie = awdie;
}

__STATIC_INLINE void adc_ovrie_setf(uint32_t base, uint8_t ovrie)
{
    ((t_hwp_adc_t*)(base))->adc_ier.bit_field.ovrie = ovrie;
}

__STATIC_INLINE void adc_eosie_setf(uint32_t base, uint8_t eosie)
{
    ((t_hwp_adc_t*)(base))->adc_ier.bit_field.eosie = eosie;
}

__STATIC_INLINE void adc_eocie_setf(uint32_t base, uint8_t eocie)
{
    ((t_hwp_adc_t*)(base))->adc_ier.bit_field.eocie = eocie;
}

__STATIC_INLINE void adc_cr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_adc_t*)(base))->adc_cr.val = value;
}

__STATIC_INLINE void adc_adstp_setf(uint32_t base, uint8_t adstp)
{
    ((t_hwp_adc_t*)(base))->adc_cr.bit_field.adstp = adstp;
}

__STATIC_INLINE void adc_adstart_setf(uint32_t base, uint8_t adstart)
{
    ((t_hwp_adc_t*)(base))->adc_cr.bit_field.adstart = adstart;
}

__STATIC_INLINE void adc_aden_setf(uint32_t base, uint8_t aden)
{
    ((t_hwp_adc_t*)(base))->adc_cr.bit_field.aden = aden;
}

__STATIC_INLINE void adc_cfgr1_set(uint32_t base, uint32_t value)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.val = value;
}

__STATIC_INLINE void adc_dma_en_setf(uint32_t base, uint8_t dma_en)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.dma_en = dma_en;
}

__STATIC_INLINE uint8_t adc_dma_en_getf(uint32_t base)
{
    return (uint8_t) ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.dma_en;
}

__STATIC_INLINE void adc_awdch_setf(uint32_t base, uint8_t awdch)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.awdch = awdch;
}

__STATIC_INLINE void adc_awden_setf(uint32_t base, uint8_t awden)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.awden = awden;
}

__STATIC_INLINE void adc_awdsgl_setf(uint32_t base, uint8_t awdsgl)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.awdsgl = awdsgl;
}

__STATIC_INLINE void adc_autoff_setf(uint32_t base, uint8_t autoff)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.autoff = autoff;
}

__STATIC_INLINE void adc_waitm_setf(uint32_t base, uint8_t waitm)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.waitm = waitm;
}

__STATIC_INLINE void adc_cont_setf(uint32_t base, uint8_t cont)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.cont = cont;
}

__STATIC_INLINE void adc_align_setf(uint32_t base, uint8_t align)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.align = align;
}

__STATIC_INLINE void adc_spec_exten_setf(uint32_t base, uint8_t spec_exten)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.spec_exten = spec_exten;
}

__STATIC_INLINE void adc_spec_extsel_setf(uint32_t base, uint8_t spec_extsel)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.spec_extsel = spec_extsel;
}

__STATIC_INLINE void adc_exten_setf(uint32_t base, uint8_t exten)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.exten = exten;
}

__STATIC_INLINE void adc_extsel_setf(uint32_t base, uint8_t extsel)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.extsel = extsel;
}

__STATIC_INLINE void adc_cfgr2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr2.val = value;
}

__STATIC_INLINE void adc_ibuf_b_setf(uint32_t base, uint8_t ibuf_b)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.ibuf_b = ibuf_b;
}

__STATIC_INLINE void adc_vset_setf(uint32_t base, uint8_t vset)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.vset = vset;
}

__STATIC_INLINE void adc_cal_en_setf(uint32_t base, uint8_t cal_en)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.cal_en = cal_en;
}

__STATIC_INLINE void adc_stab_setf(uint32_t base, uint8_t stab)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.stab = stab;
}

__STATIC_INLINE void adc_vref_sel_setf(uint32_t base, uint8_t vref_sel)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.vref_sel = vref_sel;
}

__STATIC_INLINE void adc_mode_sel_setf(uint32_t base, uint8_t mode_sel)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.mode_sel = mode_sel;
}

__STATIC_INLINE void adc_ovsr_setf(uint32_t base, uint8_t ovsr)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.ovsr = ovsr;
}

__STATIC_INLINE void adc_ovse_setf(uint32_t base, uint8_t ovse)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.ovse = ovse;
}

__STATIC_INLINE void adc_tr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_adc_t*)(base))->adc_tr.val = value;
}

__STATIC_INLINE void adc_ht_setf(uint32_t base, uint16_t ht)
{
    ((t_hwp_adc_t*)(base))->adc_tr.bit_field.ht = ht;
}

__STATIC_INLINE void adc_lt_setf(uint32_t base, uint16_t lt)
{
    ((t_hwp_adc_t*)(base))->adc_tr.bit_field.lt = lt;
}

__STATIC_INLINE void adc_chselr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_adc_t*)(base))->adc_chselr.val = value;
}

__STATIC_INLINE void adc_spec_sw_start_setf(uint32_t base, uint8_t spec_sw_start)
{
    ((t_hwp_adc_t*)(base))->adc_chselr.bit_field.spec_sw_start = spec_sw_start;
}

__STATIC_INLINE void adc_spec_ch_setf(uint32_t base, uint8_t spec_ch)
{
    ((t_hwp_adc_t*)(base))->adc_chselr.bit_field.spec_ch = spec_ch;
}

__STATIC_INLINE void adc_chselx_setf(uint32_t base, uint8_t chselx)
{
    ((t_hwp_adc_t*)(base))->adc_chselr.bit_field.chselx = chselx;
}

__STATIC_INLINE void adc_ccr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_adc_t*)(base))->adc_ccr.val = value;
}

__STATIC_INLINE void adc_auxadc_ckinv_setf(uint32_t base, uint8_t auxadc_ckinv)
{
    ((t_hwp_adc_t*)(base))->adc_ccr.bit_field.auxadc_ckinv = auxadc_ckinv;
}

__STATIC_INLINE void adc_presc_setf(uint32_t base, uint8_t presc)
{
    ((t_hwp_adc_t*)(base))->adc_ccr.bit_field.presc = presc;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE uint32_t adc_isr_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_isr.val;
}

__STATIC_INLINE uint8_t adc_awd_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_isr.bit_field.awd;
}

__STATIC_INLINE uint8_t adc_ovr_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_isr.bit_field.ovr;
}

__STATIC_INLINE uint8_t adc_eos_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eos;
}

__STATIC_INLINE uint8_t adc_eoc_7_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_7;
}

__STATIC_INLINE uint8_t adc_eoc_6_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_6;
}

__STATIC_INLINE uint8_t adc_eoc_5_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_5;
}

__STATIC_INLINE uint8_t adc_eoc_4_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_4;
}

__STATIC_INLINE uint8_t adc_eoc_3_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_3;
}

__STATIC_INLINE uint8_t adc_eoc_2_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_2;
}

__STATIC_INLINE uint8_t adc_eoc_1_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_1;
}

__STATIC_INLINE uint8_t adc_eoc_0_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_isr.bit_field.eoc_0;
}


__STATIC_INLINE uint32_t adc_ier_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_ier.val;
}

__STATIC_INLINE uint8_t adc_awdie_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_ier.bit_field.awdie;
}

__STATIC_INLINE uint8_t adc_ovrie_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_ier.bit_field.ovrie;
}

__STATIC_INLINE uint8_t adc_eosie_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_ier.bit_field.eosie;
}

__STATIC_INLINE uint8_t adc_eocie_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_ier.bit_field.eocie;
}

__STATIC_INLINE uint32_t adc_cr_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cr.val;
}

__STATIC_INLINE uint8_t adc_adstp_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cr.bit_field.adstp;
}

__STATIC_INLINE uint8_t adc_adstart_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cr.bit_field.adstart;
}

__STATIC_INLINE uint8_t adc_aden_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cr.bit_field.aden;
}

__STATIC_INLINE uint32_t adc_cfgr1_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr1.val;
}

__STATIC_INLINE uint8_t adc_awdch_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.awdch;
}

__STATIC_INLINE uint8_t adc_awden_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.awden;
}

__STATIC_INLINE uint8_t adc_awdsgl_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.awdsgl;
}

__STATIC_INLINE uint8_t adc_autoff_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.autoff;
}

__STATIC_INLINE uint8_t adc_waitm_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.waitm;
}

__STATIC_INLINE uint8_t adc_cont_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.cont;
}

__STATIC_INLINE uint8_t adc_align_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.align;
}

__STATIC_INLINE uint8_t adc_spec_exten_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.spec_exten;
}

__STATIC_INLINE uint8_t adc_spec_extsel_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.spec_extsel;
}

__STATIC_INLINE uint8_t adc_exten_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.exten;
}

__STATIC_INLINE uint8_t adc_extsel_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr1.bit_field.extsel;
}

__STATIC_INLINE uint32_t adc_cfgr2_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr2.val;
}

__STATIC_INLINE uint8_t adc_ibuf_b_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.ibuf_b;
}

__STATIC_INLINE uint8_t adc_vset_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.vset;
}

__STATIC_INLINE uint8_t adc_cal_en_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.cal_en;
}

__STATIC_INLINE uint8_t adc_stab_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.stab;
}

__STATIC_INLINE uint8_t adc_vref_sel_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.vref_sel;
}

__STATIC_INLINE uint8_t adc_mode_sel_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.mode_sel;
}

__STATIC_INLINE uint8_t adc_ovsr_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.ovsr;
}

__STATIC_INLINE uint8_t adc_ovse_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_cfgr2.bit_field.ovse;
}

__STATIC_INLINE uint32_t adc_tr_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_tr.val;
}

__STATIC_INLINE uint16_t adc_ht_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_tr.bit_field.ht;
}

__STATIC_INLINE uint16_t adc_lt_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_tr.bit_field.lt;
}

__STATIC_INLINE uint32_t adc_chselr_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_chselr.val;
}

__STATIC_INLINE uint8_t adc_spec_sw_start_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_chselr.bit_field.spec_sw_start;
}

__STATIC_INLINE uint8_t adc_spec_ch_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_chselr.bit_field.spec_ch;
}

__STATIC_INLINE uint8_t adc_chselx_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_chselr.bit_field.chselx;
}

__STATIC_INLINE uint32_t adc_ccr_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_ccr.val;
}

__STATIC_INLINE uint8_t adc_auxadc_ckinv_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_ccr.bit_field.auxadc_ckinv;
}

__STATIC_INLINE uint8_t adc_presc_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_ccr.bit_field.presc;
}

__STATIC_INLINE uint32_t adc_dr0_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr0.val;
}

__STATIC_INLINE uint32_t *adc_dr0_addr_get(uint32_t base)
{
    return (uint32_t *)&((t_hwp_adc_t*)(base))->adc_dr0;
}

__STATIC_INLINE uint16_t adc_data_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr0.bit_field.data;
}

__STATIC_INLINE uint32_t adc_dr1_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr1.val;
}

__STATIC_INLINE uint16_t adc_data1_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr1.bit_field.data1;
}

__STATIC_INLINE uint32_t adc_dr2_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr2.val;
}

__STATIC_INLINE uint16_t adc_data2_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr2.bit_field.data2;
}

__STATIC_INLINE uint32_t adc_dr3_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr3.val;
}

__STATIC_INLINE uint16_t adc_data3_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr3.bit_field.data3;
}

__STATIC_INLINE uint32_t adc_dr4_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr4.val;
}

__STATIC_INLINE uint16_t adc_data4_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr4.bit_field.data4;
}

__STATIC_INLINE uint32_t adc_dr5_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr5.val;
}

__STATIC_INLINE uint16_t adc_data5_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr5.bit_field.data5;
}

__STATIC_INLINE uint32_t adc_dr6_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr6.val;
}

__STATIC_INLINE uint16_t adc_data6_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr6.bit_field.data6;
}

__STATIC_INLINE uint32_t adc_dr7_get(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr7.val;
}

__STATIC_INLINE uint16_t adc_data7_getf(uint32_t base)
{
    return ((t_hwp_adc_t*)(base))->adc_dr7.bit_field.data7;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void adc_isr_pack(uint32_t base, uint8_t awd, uint8_t ovr, uint8_t eos, uint8_t eoc)
{
    ((t_hwp_adc_t*)(base))->adc_isr.val = ( \
                                    ((uint32_t )awd               << 10) \
                                  | ((uint32_t )ovr               << 9) \
                                  | ((uint32_t )eos               << 8) \
                                  | ((uint32_t )eoc) \
                                  );
}

__STATIC_INLINE void adc_ier_pack(uint32_t base, uint8_t awdie, uint8_t ovrie, uint8_t eosie, uint8_t eocie)
{
    ((t_hwp_adc_t*)(base))->adc_ier.val = ( \
                                    ((uint32_t )awdie             << 10) \
                                  | ((uint32_t )ovrie             << 9) \
                                  | ((uint32_t )eosie             << 8) \
                                  | ((uint32_t )eocie) \
                                  );
}

__STATIC_INLINE void adc_cr_pack(uint32_t base, uint8_t adstp, uint8_t adstart, uint8_t aden)
{
    ((t_hwp_adc_t*)(base))->adc_cr.val = ( \
                                    ((uint32_t )adstp             << 2) \
                                  | ((uint32_t )adstart           << 1) \
                                  | ((uint32_t )aden) \
                                  );
}

__STATIC_INLINE void adc_cfgr1_pack(uint32_t base, uint8_t awdch, uint8_t awden, uint8_t awdsgl, uint8_t autoff, uint8_t waitm, uint8_t cont, uint8_t align, uint8_t spec_exten, uint8_t spec_extsel, uint8_t exten, uint8_t extsel)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr1.val = ( \
                                    ((uint32_t )awdch             << 18) \
                                  | ((uint32_t )awden             << 17) \
                                  | ((uint32_t )awdsgl            << 16) \
                                  | ((uint32_t )autoff            << 15) \
                                  | ((uint32_t )waitm             << 14) \
                                  | ((uint32_t )cont              << 13) \
                                  | ((uint32_t )align             << 8) \
                                  | ((uint32_t )spec_exten        << 6) \
                                  | ((uint32_t )spec_extsel       << 4) \
                                  | ((uint32_t )exten             << 2) \
                                  | ((uint32_t )extsel) \
                                  );
}

__STATIC_INLINE void adc_cfgr2_pack(uint32_t base, uint8_t ibuf_b, uint8_t vset, uint8_t cal_en, uint8_t stab, uint8_t vref_sel, uint8_t mode_sel, uint8_t ovsr, uint8_t ovse)
{
    ((t_hwp_adc_t*)(base))->adc_cfgr2.val = ( \
                                    ((uint32_t )ibuf_b            << 23) \
                                  | ((uint32_t )vset              << 21) \
                                  | ((uint32_t )cal_en            << 20) \
                                  | ((uint32_t )stab              << 16) \
                                  | ((uint32_t )vref_sel          << 12) \
                                  | ((uint32_t )mode_sel          << 11) \
                                  | ((uint32_t )ovsr              << 8) \
                                  | ((uint32_t )ovse) \
                                  );
}

__STATIC_INLINE void adc_tr_pack(uint32_t base, uint16_t ht, uint16_t lt)
{
    ((t_hwp_adc_t*)(base))->adc_tr.val = ( \
                                    ((uint32_t )ht                << 16) \
                                  | ((uint32_t )lt) \
                                  );
}

__STATIC_INLINE void adc_chselr_pack(uint32_t base, uint8_t spec_sw_start, uint8_t spec_ch, uint8_t chselx)
{
    ((t_hwp_adc_t*)(base))->adc_chselr.val = ( \
                                    ((uint32_t )spec_sw_start     << 11) \
                                  | ((uint32_t )spec_ch           << 8) \
                                  | ((uint32_t )chselx) \
                                  );
}

__STATIC_INLINE void adc_ccr_pack(uint32_t base, uint8_t auxadc_ckinv, uint8_t presc)
{
    ((t_hwp_adc_t*)(base))->adc_ccr.val = ( \
                                    ((uint32_t )auxadc_ckinv      << 8) \
                                  | ((uint32_t )presc) \
                                  );
}


//--------------------------------------------------------------------------------
#endif

