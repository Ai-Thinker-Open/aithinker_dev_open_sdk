#ifndef __REG_SPI_H__
#define __REG_SPI_H__

#include "ln882h.h"

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    cpha                : 1  ; // 0  : 0
        uint32_t    cpol                : 1  ; // 1  : 1
        uint32_t    mstr                : 1  ; // 2  : 2
        uint32_t    br                  : 3  ; // 5  : 3
        uint32_t    spe                 : 1  ; // 6  : 6
        uint32_t    lsbfirst            : 1  ; // 7  : 7
        uint32_t    ssi                 : 1  ; // 8  : 8
        uint32_t    ssm                 : 1  ; // 9  : 9
        uint32_t    rxonly              : 1  ; // 10 : 10
        uint32_t    dff                 : 1  ; // 11 : 11
        uint32_t    crcnext             : 1  ; // 12 : 12
        uint32_t    crcen               : 1  ; // 13 : 13
        uint32_t    bidioe              : 1  ; // 14 : 14
        uint32_t    bidimode            : 1  ; // 15 : 15
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_spi_cr1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    rxdmaen             : 1  ; // 0  : 0
        uint32_t    txdmaen             : 1  ; // 1  : 1
        uint32_t    ssoe                : 1  ; // 2  : 2
        uint32_t    reserved_1          : 2  ; // 4  : 3
        uint32_t    errie               : 1  ; // 5  : 5
        uint32_t    rxneie              : 1  ; // 6  : 6
        uint32_t    txeie               : 1  ; // 7  : 7
        uint32_t    reserved_0          : 24 ; // 31 : 8
    } bit_field;
} t_spi_cr2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    rxne                : 1  ; // 0  : 0
        uint32_t    txe                 : 1  ; // 1  : 1
        uint32_t    reserved_2          : 1  ; // 2  : 2
        uint32_t    udr                 : 1  ; // 3  : 3
        uint32_t    crcerr              : 1  ; // 4  : 4
        uint32_t    reserved_1          : 1  ; // 5  : 5
        uint32_t    ovr                 : 1  ; // 6  : 6
        uint32_t    bsy                 : 1  ; // 7  : 7
        uint32_t    reserved_0          : 24 ; // 31 : 8
    } bit_field;
} t_spi_sr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    dr                  : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_spi_dr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    crcpoly             : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_spi_crcpr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    rxcrc               : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_spi_rxcrcr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    txcrc               : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_spi_txcrcr;


//--------------------------------------------------------------------------------

typedef struct
{
    volatile        t_spi_cr1                                cr1                                    ; // 0x0
    volatile        t_spi_cr2                                cr2                                    ; // 0x4
    volatile        t_spi_sr                                 sr                                     ; // 0x8
    volatile        t_spi_dr                                 dr                                     ; // 0xc
    volatile        t_spi_crcpr                              crcpr                                  ; // 0x10
    volatile        t_spi_rxcrcr                             rxcrcr                                 ; // 0x14
    volatile        t_spi_txcrcr                             txcrcr                                 ; // 0x18
} t_hwp_spi_t;


//--------------------------------------------------------------------------------

__STATIC_INLINE void spi_cr1_set(uint32_t base, uint32_t value)
{
    ((t_hwp_spi_t*)(base))->cr1.val = value;
}

__STATIC_INLINE void spi_bidimode_setf(uint32_t base, uint8_t bidimode)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.bidimode = bidimode;
}

__STATIC_INLINE void spi_bidioe_setf(uint32_t base, uint8_t bidioe)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.bidioe = bidioe;
}

__STATIC_INLINE void spi_crcen_setf(uint32_t base, uint8_t crcen)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.crcen = crcen;
}

__STATIC_INLINE void spi_crcnext_setf(uint32_t base, uint8_t crcnext)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.crcnext = crcnext;
}

__STATIC_INLINE void spi_dff_setf(uint32_t base, uint8_t dff)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.dff = dff;
}

__STATIC_INLINE void spi_rxonly_setf(uint32_t base, uint8_t rxonly)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.rxonly = rxonly;
}

__STATIC_INLINE void spi_ssm_setf(uint32_t base, uint8_t ssm)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.ssm = ssm;
}

__STATIC_INLINE void spi_ssi_setf(uint32_t base, uint8_t ssi)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.ssi = ssi;
}

__STATIC_INLINE void spi_lsbfirst_setf(uint32_t base, uint8_t lsbfirst)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.lsbfirst = lsbfirst;
}

__STATIC_INLINE void spi_spe_setf(uint32_t base, uint8_t spe)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.spe = spe;
}

__STATIC_INLINE void spi_br_setf(uint32_t base, uint8_t br)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.br = br;
}

__STATIC_INLINE void spi_mstr_setf(uint32_t base, uint8_t mstr)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.mstr = mstr;
}

__STATIC_INLINE void spi_cpol_setf(uint32_t base, uint8_t cpol)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.cpol = cpol;
}

__STATIC_INLINE void spi_cpha_setf(uint32_t base, uint8_t cpha)
{
    ((t_hwp_spi_t*)(base))->cr1.bit_field.cpha = cpha;
}

__STATIC_INLINE void spi_cr2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_spi_t*)(base))->cr2.val = value;
}

__STATIC_INLINE void spi_txeie_setf(uint32_t base, uint8_t txeie)
{
    ((t_hwp_spi_t*)(base))->cr2.bit_field.txeie = txeie;
}

__STATIC_INLINE void spi_rxneie_setf(uint32_t base, uint8_t rxneie)
{
    ((t_hwp_spi_t*)(base))->cr2.bit_field.rxneie = rxneie;
}

__STATIC_INLINE void spi_errie_setf(uint32_t base, uint8_t errie)
{
    ((t_hwp_spi_t*)(base))->cr2.bit_field.errie = errie;
}

__STATIC_INLINE void spi_ssoe_setf(uint32_t base, uint8_t ssoe)
{
    ((t_hwp_spi_t*)(base))->cr2.bit_field.ssoe = ssoe;
}

__STATIC_INLINE void spi_txdmaen_setf(uint32_t base, uint8_t txdmaen)
{
    ((t_hwp_spi_t*)(base))->cr2.bit_field.txdmaen = txdmaen;
}

__STATIC_INLINE void spi_rxdmaen_setf(uint32_t base, uint8_t rxdmaen)
{
    ((t_hwp_spi_t*)(base))->cr2.bit_field.rxdmaen = rxdmaen;
}

__STATIC_INLINE void spi_sr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_spi_t*)(base))->sr.val = value;
}

__STATIC_INLINE void spi_crcerr_setf(uint32_t base, uint8_t crcerr)
{
    ((t_hwp_spi_t*)(base))->sr.bit_field.crcerr = crcerr;
}

__STATIC_INLINE void spi_dr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_spi_t*)(base))->dr.val = value;
}

__STATIC_INLINE void spi_dr_setf(uint32_t base, uint16_t dr)
{
    ((t_hwp_spi_t*)(base))->dr.bit_field.dr = dr;
}

__STATIC_INLINE void spi_crcpr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_spi_t*)(base))->crcpr.val = value;
}

__STATIC_INLINE void spi_crcpoly_setf(uint32_t base, uint16_t crcpoly)
{
    ((t_hwp_spi_t*)(base))->crcpr.bit_field.crcpoly = crcpoly;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE uint32_t spi_cr1_get(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.val;
}

__STATIC_INLINE uint8_t spi_bidimode_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.bidimode;
}

__STATIC_INLINE uint8_t spi_bidioe_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.bidioe;
}

__STATIC_INLINE uint8_t spi_crcen_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.crcen;
}

__STATIC_INLINE uint8_t spi_crcnext_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.crcnext;
}

__STATIC_INLINE uint8_t spi_dff_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.dff;
}

__STATIC_INLINE uint8_t spi_rxonly_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.rxonly;
}

__STATIC_INLINE uint8_t spi_ssm_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.ssm;
}

__STATIC_INLINE uint8_t spi_ssi_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.ssi;
}

__STATIC_INLINE uint8_t spi_lsbfirst_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.lsbfirst;
}

__STATIC_INLINE uint8_t spi_spe_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.spe;
}

__STATIC_INLINE uint8_t spi_br_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.br;
}

__STATIC_INLINE uint8_t spi_mstr_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.mstr;
}

__STATIC_INLINE uint8_t spi_cpol_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.cpol;
}

__STATIC_INLINE uint8_t spi_cpha_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr1.bit_field.cpha;
}

__STATIC_INLINE uint32_t spi_cr2_get(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr2.val;
}

__STATIC_INLINE uint8_t spi_txeie_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr2.bit_field.txeie;
}

__STATIC_INLINE uint8_t spi_rxneie_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr2.bit_field.rxneie;
}

__STATIC_INLINE uint8_t spi_errie_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr2.bit_field.errie;
}

__STATIC_INLINE uint8_t spi_ssoe_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr2.bit_field.ssoe;
}

__STATIC_INLINE uint8_t spi_txdmaen_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr2.bit_field.txdmaen;
}

__STATIC_INLINE uint8_t spi_rxdmaen_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->cr2.bit_field.rxdmaen;
}

__STATIC_INLINE uint32_t spi_sr_get(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->sr.val;
}

__STATIC_INLINE uint8_t spi_bsy_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->sr.bit_field.bsy;
}

__STATIC_INLINE uint8_t spi_ovr_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->sr.bit_field.ovr;
}

__STATIC_INLINE uint8_t spi_crcerr_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->sr.bit_field.crcerr;
}

__STATIC_INLINE uint8_t spi_udr_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->sr.bit_field.udr;
}

__STATIC_INLINE uint8_t spi_txe_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->sr.bit_field.txe;
}

__STATIC_INLINE uint8_t spi_rxne_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->sr.bit_field.rxne;
}

__STATIC_INLINE uint32_t spi_dr_get(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->dr.val;
}

__STATIC_INLINE uint16_t spi_dr_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->dr.bit_field.dr;
}

__STATIC_INLINE uint32_t spi_crcpr_get(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->crcpr.val;
}

__STATIC_INLINE uint16_t spi_crcpoly_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->crcpr.bit_field.crcpoly;
}

__STATIC_INLINE uint32_t spi_rxcrcr_get(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->rxcrcr.val;
}

__STATIC_INLINE uint16_t spi_rxcrc_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->rxcrcr.bit_field.rxcrc;
}

__STATIC_INLINE uint32_t spi_txcrcr_get(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->txcrcr.val;
}

__STATIC_INLINE uint16_t spi_txcrc_getf(uint32_t base)
{
    return ((t_hwp_spi_t*)(base))->txcrcr.bit_field.txcrc;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void spi_cr1_pack(uint32_t base, uint8_t bidimode, uint8_t bidioe, uint8_t crcen, uint8_t crcnext, uint8_t dff, uint8_t rxonly, uint8_t ssm, uint8_t ssi, uint8_t lsbfirst, uint8_t spe, uint8_t br, uint8_t mstr, uint8_t cpol, uint8_t cpha)
{
    ((t_hwp_spi_t*)(base))->cr1.val = ( \
                                    ((uint32_t )bidimode          << 15) \
                                  | ((uint32_t )bidioe            << 14) \
                                  | ((uint32_t )crcen             << 13) \
                                  | ((uint32_t )crcnext           << 12) \
                                  | ((uint32_t )dff               << 11) \
                                  | ((uint32_t )rxonly            << 10) \
                                  | ((uint32_t )ssm               << 9) \
                                  | ((uint32_t )ssi               << 8) \
                                  | ((uint32_t )lsbfirst          << 7) \
                                  | ((uint32_t )spe               << 6) \
                                  | ((uint32_t )br                << 3) \
                                  | ((uint32_t )mstr              << 2) \
                                  | ((uint32_t )cpol              << 1) \
                                  | ((uint32_t )cpha) \
                                  );
}

__STATIC_INLINE void spi_cr2_pack(uint32_t base, uint8_t txeie, uint8_t rxneie, uint8_t errie, uint8_t ssoe, uint8_t txdmaen, uint8_t rxdmaen)
{
    ((t_hwp_spi_t*)(base))->cr2.val = ( \
                                    ((uint32_t )txeie             << 7) \
                                  | ((uint32_t )rxneie            << 6) \
                                  | ((uint32_t )errie             << 5) \
                                  | ((uint32_t )ssoe              << 2) \
                                  | ((uint32_t )txdmaen           << 1) \
                                  | ((uint32_t )rxdmaen) \
                                  );
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void spi_cr1_unpack(uint32_t base, uint8_t * bidimode, uint8_t * bidioe, uint8_t * crcen, uint8_t * crcnext, uint8_t * dff, uint8_t * rxonly, uint8_t * ssm, uint8_t * ssi, uint8_t * lsbfirst, uint8_t * spe, uint8_t * br, uint8_t * mstr, uint8_t * cpol, uint8_t * cpha)
{
    t_spi_cr1 local_val = ((t_hwp_spi_t*)(base))->cr1;

    *bidimode           = local_val.bit_field.bidimode;
    *bidioe             = local_val.bit_field.bidioe;
    *crcen              = local_val.bit_field.crcen;
    *crcnext            = local_val.bit_field.crcnext;
    *dff                = local_val.bit_field.dff;
    *rxonly             = local_val.bit_field.rxonly;
    *ssm                = local_val.bit_field.ssm;
    *ssi                = local_val.bit_field.ssi;
    *lsbfirst           = local_val.bit_field.lsbfirst;
    *spe                = local_val.bit_field.spe;
    *br                 = local_val.bit_field.br;
    *mstr               = local_val.bit_field.mstr;
    *cpol               = local_val.bit_field.cpol;
    *cpha               = local_val.bit_field.cpha;
}

__STATIC_INLINE void spi_cr2_unpack(uint32_t base, uint8_t * txeie, uint8_t * rxneie, uint8_t * errie, uint8_t * ssoe, uint8_t * txdmaen, uint8_t * rxdmaen)
{
    t_spi_cr2 local_val = ((t_hwp_spi_t*)(base))->cr2;

    *txeie              = local_val.bit_field.txeie;
    *rxneie             = local_val.bit_field.rxneie;
    *errie              = local_val.bit_field.errie;
    *ssoe               = local_val.bit_field.ssoe;
    *txdmaen            = local_val.bit_field.txdmaen;
    *rxdmaen            = local_val.bit_field.rxdmaen;
}

__STATIC_INLINE void spi_sr_unpack(uint32_t base, uint8_t * bsy, uint8_t * ovr, uint8_t * crcerr, uint8_t * udr, uint8_t * txe, uint8_t * rxne)
{
    t_spi_sr local_val = ((t_hwp_spi_t*)(base))->sr;

    *bsy                = local_val.bit_field.bsy;
    *ovr                = local_val.bit_field.ovr;
    *crcerr             = local_val.bit_field.crcerr;
    *udr                = local_val.bit_field.udr;
    *txe                = local_val.bit_field.txe;
    *rxne               = local_val.bit_field.rxne;
}

#endif

