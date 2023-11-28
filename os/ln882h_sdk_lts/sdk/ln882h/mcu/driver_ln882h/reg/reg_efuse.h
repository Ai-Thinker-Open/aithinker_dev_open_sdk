#ifndef __REG_EFUSE_H__
#define __REG_EFUSE_H__

#include "ln882h.h"

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    avdd_hold           : 3  ; // 2  : 0
        uint32_t    aen_hold            : 3  ; // 5  : 3
        uint32_t    addr_hold           : 2  ; // 7  : 6
        uint32_t    burn_time           : 6  ; // 13 : 8
        uint32_t    addr_setup          : 2  ; // 15 : 14
        uint32_t    pgen_setup          : 2  ; // 17 : 16
        uint32_t    avdd_setup          : 3  ; // 20 : 18
        uint32_t    reserved_0          : 11 ; // 31 : 21
    } bit_field;
} t_efuse_wr_ctrl;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    rden_hold           : 2  ; // 1  : 0
        uint32_t    rden_setup          : 2  ; // 3  : 2
        uint32_t    rd_aen_hold         : 2  ; // 5  : 4
        uint32_t    rd_aen_setup        : 2  ; // 7  : 6
        uint32_t    reserved_0          : 24 ; // 31 : 8
    } bit_field;
} t_efuse_rd_ctrl;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    rden                : 1  ; // 0  : 0
        uint32_t    avdd                : 1  ; // 1  : 1
        uint32_t    reserved_0          : 30 ; // 31 : 2
    } bit_field;
} t_efuse_pwr_ctrl0;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ef_av_set           : 3  ; // 2  : 0
        uint32_t    ef_sr               : 3  ; // 5  : 3
        uint32_t    reserved_0          : 26 ; // 31 : 6
    } bit_field;
} t_efuse_pwr_ctrl1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    key1                : 32 ; // 31 : 0
    } bit_field;
} t_efuse_key1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    key2                : 32 ; // 31 : 0
    } bit_field;
} t_efuse_key2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ef_rd               : 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_efuse_rd_trig;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    efuse_busy          : 1  ; // 0  : 0
        uint32_t    write_enable        : 1  ; // 1  : 1
        uint32_t    reserved_0          : 30 ; // 31 : 2
    } bit_field;
} t_efuse_status;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    efuse_reg0          : 32 ; // 31 : 0
    } bit_field;
} t_efuse_r0;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    efuse_reg1          : 32 ; // 31 : 0
    } bit_field;
} t_efuse_r1;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    efuse_reg2          : 32 ; // 31 : 0
    } bit_field;
} t_efuse_r2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    efuse_reg3          : 32 ; // 31 : 0
    } bit_field;
} t_efuse_r3;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    efuse_reg4          : 32 ; // 31 : 0
    } bit_field;
} t_efuse_r4;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    efuse_reg5          : 32 ; // 31 : 0
    } bit_field;
} t_efuse_r5;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    efuse_reg6          : 32 ; // 31 : 0
    } bit_field;
} t_efuse_r6;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    efuse_reg7          : 32 ; // 31 : 0
    } bit_field;
} t_efuse_r7;


//--------------------------------------------------------------------------------

typedef struct
{
    volatile        t_efuse_wr_ctrl                          efuse_wr_ctrl                          ; // 0x0
    volatile        t_efuse_rd_ctrl                          efuse_rd_ctrl                          ; // 0x4
    volatile        t_efuse_pwr_ctrl0                        efuse_pwr_ctrl0                        ; // 0x8
    volatile        t_efuse_pwr_ctrl1                        efuse_pwr_ctrl1                        ; // 0xc
    volatile        t_efuse_key1                             efuse_key1                             ; // 0x10
    volatile        t_efuse_key2                             efuse_key2                             ; // 0x14
    volatile        t_efuse_rd_trig                          efuse_rd_trig                          ; // 0x18
    volatile        t_efuse_status                           efuse_status                           ; // 0x1c
    volatile        t_efuse_r0                               efuse_r0                               ; // 0x20
    volatile        t_efuse_r1                               efuse_r1                               ; // 0x24
    volatile        t_efuse_r2                               efuse_r2                               ; // 0x28
    volatile        t_efuse_r3                               efuse_r3                               ; // 0x2c
    volatile        t_efuse_r4                               efuse_r4                               ; // 0x30
    volatile        t_efuse_r5                               efuse_r5                               ; // 0x34
    volatile        t_efuse_r6                               efuse_r6                               ; // 0x38
    volatile        t_efuse_r7                               efuse_r7                               ; // 0x3c
} t_hwp_efuse_t;

#define hwp_efuse ((t_hwp_efuse_t*)EFUSE_BASE)


//--------------------------------------------------------------------------------

__STATIC_INLINE void efuse_wr_ctrl_set(uint32_t value)
{
    hwp_efuse->efuse_wr_ctrl.val = value;
}

__STATIC_INLINE void efuse_avdd_setup_setf(uint8_t avdd_setup)
{
    hwp_efuse->efuse_wr_ctrl.bit_field.avdd_setup = avdd_setup;
}

__STATIC_INLINE void efuse_pgen_setup_setf(uint8_t pgen_setup)
{
    hwp_efuse->efuse_wr_ctrl.bit_field.pgen_setup = pgen_setup;
}

__STATIC_INLINE void efuse_addr_setup_setf(uint8_t addr_setup)
{
    hwp_efuse->efuse_wr_ctrl.bit_field.addr_setup = addr_setup;
}

__STATIC_INLINE void efuse_burn_time_setf(uint8_t burn_time)
{
    hwp_efuse->efuse_wr_ctrl.bit_field.burn_time = burn_time;
}

__STATIC_INLINE void efuse_addr_hold_setf(uint8_t addr_hold)
{
    hwp_efuse->efuse_wr_ctrl.bit_field.addr_hold = addr_hold;
}

__STATIC_INLINE void efuse_aen_hold_setf(uint8_t aen_hold)
{
    hwp_efuse->efuse_wr_ctrl.bit_field.aen_hold = aen_hold;
}

__STATIC_INLINE void efuse_avdd_hold_setf(uint8_t avdd_hold)
{
    hwp_efuse->efuse_wr_ctrl.bit_field.avdd_hold = avdd_hold;
}

__STATIC_INLINE void efuse_rd_ctrl_set(uint32_t value)
{
    hwp_efuse->efuse_rd_ctrl.val = value;
}

__STATIC_INLINE void efuse_rd_aen_setup_setf(uint8_t rd_aen_setup)
{
    hwp_efuse->efuse_rd_ctrl.bit_field.rd_aen_setup = rd_aen_setup;
}

__STATIC_INLINE void efuse_rd_aen_hold_setf(uint8_t rd_aen_hold)
{
    hwp_efuse->efuse_rd_ctrl.bit_field.rd_aen_hold = rd_aen_hold;
}

__STATIC_INLINE void efuse_rden_setup_setf(uint8_t rden_setup)
{
    hwp_efuse->efuse_rd_ctrl.bit_field.rden_setup = rden_setup;
}

__STATIC_INLINE void efuse_rden_hold_setf(uint8_t rden_hold)
{
    hwp_efuse->efuse_rd_ctrl.bit_field.rden_hold = rden_hold;
}

__STATIC_INLINE void efuse_pwr_ctrl0_set(uint32_t value)
{
    hwp_efuse->efuse_pwr_ctrl0.val = value;
}

__STATIC_INLINE void efuse_avdd_setf(uint8_t avdd)
{
    hwp_efuse->efuse_pwr_ctrl0.bit_field.avdd = avdd;
}

__STATIC_INLINE void efuse_rden_setf(uint8_t rden)
{
    hwp_efuse->efuse_pwr_ctrl0.bit_field.rden = rden;
}

__STATIC_INLINE void efuse_pwr_ctrl1_set(uint32_t value)
{
    hwp_efuse->efuse_pwr_ctrl1.val = value;
}

__STATIC_INLINE void efuse_ef_sr_setf(uint8_t ef_sr)
{
    hwp_efuse->efuse_pwr_ctrl1.bit_field.ef_sr = ef_sr;
}

__STATIC_INLINE void efuse_ef_av_set_setf(uint8_t ef_av_set)
{
    hwp_efuse->efuse_pwr_ctrl1.bit_field.ef_av_set = ef_av_set;
}

__STATIC_INLINE void efuse_key1_set(uint32_t value)
{
    hwp_efuse->efuse_key1.val = value;
}

__STATIC_INLINE void efuse_key1_setf(uint32_t key1)
{
    hwp_efuse->efuse_key1.bit_field.key1 = key1;
}

__STATIC_INLINE void efuse_key2_set(uint32_t value)
{
    hwp_efuse->efuse_key2.val = value;
}

__STATIC_INLINE void efuse_key2_setf(uint32_t key2)
{
    hwp_efuse->efuse_key2.bit_field.key2 = key2;
}

__STATIC_INLINE void efuse_rd_trig_set(uint32_t value)
{
    hwp_efuse->efuse_rd_trig.val = value;
}

__STATIC_INLINE void efuse_ef_rd_setf(uint8_t ef_rd)
{
    hwp_efuse->efuse_rd_trig.bit_field.ef_rd = ef_rd;
}

__STATIC_INLINE void efuse_r0_set(uint32_t value)
{
    hwp_efuse->efuse_r0.val = value;
}

__STATIC_INLINE void efuse_reg0_setf(uint32_t efuse_reg0)
{
    hwp_efuse->efuse_r0.bit_field.efuse_reg0 = efuse_reg0;
}

__STATIC_INLINE void efuse_r1_set(uint32_t value)
{
    hwp_efuse->efuse_r1.val = value;
}

__STATIC_INLINE void efuse_reg1_setf(uint32_t efuse_reg1)
{
    hwp_efuse->efuse_r1.bit_field.efuse_reg1 = efuse_reg1;
}

__STATIC_INLINE void efuse_r2_set(uint32_t value)
{
    hwp_efuse->efuse_r2.val = value;
}

__STATIC_INLINE void efuse_reg2_setf(uint32_t efuse_reg2)
{
    hwp_efuse->efuse_r2.bit_field.efuse_reg2 = efuse_reg2;
}

__STATIC_INLINE void efuse_r3_set(uint32_t value)
{
    hwp_efuse->efuse_r3.val = value;
}

__STATIC_INLINE void efuse_reg3_setf(uint32_t efuse_reg3)
{
    hwp_efuse->efuse_r3.bit_field.efuse_reg3 = efuse_reg3;
}

__STATIC_INLINE void efuse_r4_set(uint32_t value)
{
    hwp_efuse->efuse_r4.val = value;
}

__STATIC_INLINE void efuse_reg4_setf(uint32_t efuse_reg4)
{
    hwp_efuse->efuse_r4.bit_field.efuse_reg4 = efuse_reg4;
}

__STATIC_INLINE void efuse_r5_set(uint32_t value)
{
    hwp_efuse->efuse_r5.val = value;
}

__STATIC_INLINE void efuse_reg5_setf(uint32_t efuse_reg5)
{
    hwp_efuse->efuse_r5.bit_field.efuse_reg5 = efuse_reg5;
}

__STATIC_INLINE void efuse_r6_set(uint32_t value)
{
    hwp_efuse->efuse_r6.val = value;
}

__STATIC_INLINE void efuse_reg6_setf(uint32_t efuse_reg6)
{
    hwp_efuse->efuse_r6.bit_field.efuse_reg6 = efuse_reg6;
}

__STATIC_INLINE void efuse_r7_set(uint32_t value)
{
    hwp_efuse->efuse_r7.val = value;
}

__STATIC_INLINE void efuse_reg7_setf(uint32_t efuse_reg7)
{
    hwp_efuse->efuse_r7.bit_field.efuse_reg7 = efuse_reg7;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE uint32_t efuse_wr_ctrl_get(void)
{
    return hwp_efuse->efuse_wr_ctrl.val;
}

__STATIC_INLINE uint8_t efuse_avdd_setup_getf(void)
{
    return hwp_efuse->efuse_wr_ctrl.bit_field.avdd_setup;
}

__STATIC_INLINE uint8_t efuse_pgen_setup_getf(void)
{
    return hwp_efuse->efuse_wr_ctrl.bit_field.pgen_setup;
}

__STATIC_INLINE uint8_t efuse_addr_setup_getf(void)
{
    return hwp_efuse->efuse_wr_ctrl.bit_field.addr_setup;
}

__STATIC_INLINE uint8_t efuse_burn_time_getf(void)
{
    return hwp_efuse->efuse_wr_ctrl.bit_field.burn_time;
}

__STATIC_INLINE uint8_t efuse_addr_hold_getf(void)
{
    return hwp_efuse->efuse_wr_ctrl.bit_field.addr_hold;
}

__STATIC_INLINE uint8_t efuse_aen_hold_getf(void)
{
    return hwp_efuse->efuse_wr_ctrl.bit_field.aen_hold;
}

__STATIC_INLINE uint8_t efuse_avdd_hold_getf(void)
{
    return hwp_efuse->efuse_wr_ctrl.bit_field.avdd_hold;
}

__STATIC_INLINE uint32_t efuse_rd_ctrl_get(void)
{
    return hwp_efuse->efuse_rd_ctrl.val;
}

__STATIC_INLINE uint8_t efuse_rd_aen_setup_getf(void)
{
    return hwp_efuse->efuse_rd_ctrl.bit_field.rd_aen_setup;
}

__STATIC_INLINE uint8_t efuse_rd_aen_hold_getf(void)
{
    return hwp_efuse->efuse_rd_ctrl.bit_field.rd_aen_hold;
}

__STATIC_INLINE uint8_t efuse_rden_setup_getf(void)
{
    return hwp_efuse->efuse_rd_ctrl.bit_field.rden_setup;
}

__STATIC_INLINE uint8_t efuse_rden_hold_getf(void)
{
    return hwp_efuse->efuse_rd_ctrl.bit_field.rden_hold;
}

__STATIC_INLINE uint32_t efuse_pwr_ctrl0_get(void)
{
    return hwp_efuse->efuse_pwr_ctrl0.val;
}

__STATIC_INLINE uint8_t efuse_avdd_getf(void)
{
    return hwp_efuse->efuse_pwr_ctrl0.bit_field.avdd;
}

__STATIC_INLINE uint8_t efuse_rden_getf(void)
{
    return hwp_efuse->efuse_pwr_ctrl0.bit_field.rden;
}

__STATIC_INLINE uint32_t efuse_pwr_ctrl1_get(void)
{
    return hwp_efuse->efuse_pwr_ctrl1.val;
}

__STATIC_INLINE uint8_t efuse_ef_sr_getf(void)
{
    return hwp_efuse->efuse_pwr_ctrl1.bit_field.ef_sr;
}

__STATIC_INLINE uint8_t efuse_ef_av_set_getf(void)
{
    return hwp_efuse->efuse_pwr_ctrl1.bit_field.ef_av_set;
}

__STATIC_INLINE uint32_t efuse_status_get(void)
{
    return hwp_efuse->efuse_status.val;
}

__STATIC_INLINE uint8_t efuse_write_enable_getf(void)
{
    return hwp_efuse->efuse_status.bit_field.write_enable;
}

__STATIC_INLINE uint8_t efuse_busy_getf(void)
{
    return hwp_efuse->efuse_status.bit_field.efuse_busy;
}

__STATIC_INLINE uint32_t efuse_r0_get(void)
{
    return hwp_efuse->efuse_r0.val;
}

__STATIC_INLINE uint32_t efuse_reg0_getf(void)
{
    return hwp_efuse->efuse_r0.bit_field.efuse_reg0;
}

__STATIC_INLINE uint32_t efuse_r1_get(void)
{
    return hwp_efuse->efuse_r1.val;
}

__STATIC_INLINE uint32_t efuse_reg1_getf(void)
{
    return hwp_efuse->efuse_r1.bit_field.efuse_reg1;
}

__STATIC_INLINE uint32_t efuse_r2_get(void)
{
    return hwp_efuse->efuse_r2.val;
}

__STATIC_INLINE uint32_t efuse_reg2_getf(void)
{
    return hwp_efuse->efuse_r2.bit_field.efuse_reg2;
}

__STATIC_INLINE uint32_t efuse_r3_get(void)
{
    return hwp_efuse->efuse_r3.val;
}

__STATIC_INLINE uint32_t efuse_reg3_getf(void)
{
    return hwp_efuse->efuse_r3.bit_field.efuse_reg3;
}

__STATIC_INLINE uint32_t efuse_r4_get(void)
{
    return hwp_efuse->efuse_r4.val;
}

__STATIC_INLINE uint32_t efuse_reg4_getf(void)
{
    return hwp_efuse->efuse_r4.bit_field.efuse_reg4;
}

__STATIC_INLINE uint32_t efuse_r5_get(void)
{
    return hwp_efuse->efuse_r5.val;
}

__STATIC_INLINE uint32_t efuse_reg5_getf(void)
{
    return hwp_efuse->efuse_r5.bit_field.efuse_reg5;
}

__STATIC_INLINE uint32_t efuse_r6_get(void)
{
    return hwp_efuse->efuse_r6.val;
}

__STATIC_INLINE uint32_t efuse_reg6_getf(void)
{
    return hwp_efuse->efuse_r6.bit_field.efuse_reg6;
}

__STATIC_INLINE uint32_t efuse_r7_get(void)
{
    return hwp_efuse->efuse_r7.val;
}

__STATIC_INLINE uint32_t efuse_reg7_getf(void)
{
    return hwp_efuse->efuse_r7.bit_field.efuse_reg7;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void efuse_wr_ctrl_pack(uint8_t avdd_setup, uint8_t pgen_setup, uint8_t addr_setup, uint8_t burn_time, uint8_t addr_hold, uint8_t aen_hold, uint8_t avdd_hold)
{
    hwp_efuse->efuse_wr_ctrl.val = ( \
                                    ((uint32_t )avdd_setup        << 18) \
                                  | ((uint32_t )pgen_setup        << 16) \
                                  | ((uint32_t )addr_setup        << 14) \
                                  | ((uint32_t )burn_time         << 8) \
                                  | ((uint32_t )addr_hold         << 6) \
                                  | ((uint32_t )aen_hold          << 3) \
                                  | ((uint32_t )avdd_hold) \
                                  );
}

__STATIC_INLINE void efuse_rd_ctrl_pack(uint8_t rd_aen_setup, uint8_t rd_aen_hold, uint8_t rden_setup, uint8_t rden_hold)
{
    hwp_efuse->efuse_rd_ctrl.val = ( \
                                    ((uint32_t )rd_aen_setup      << 6) \
                                  | ((uint32_t )rd_aen_hold       << 4) \
                                  | ((uint32_t )rden_setup        << 2) \
                                  | ((uint32_t )rden_hold) \
                                  );
}

__STATIC_INLINE void efuse_pwr_ctrl0_pack(uint8_t avdd, uint8_t rden)
{
    hwp_efuse->efuse_pwr_ctrl0.val = ( \
                                    ((uint32_t )avdd              << 1) \
                                  | ((uint32_t )rden) \
                                  );
}

__STATIC_INLINE void efuse_pwr_ctrl1_pack(uint8_t ef_sr, uint8_t ef_av_set)
{
    hwp_efuse->efuse_pwr_ctrl1.val = ( \
                                    ((uint32_t )ef_sr             << 3) \
                                  | ((uint32_t )ef_av_set) \
                                  );
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void efuse_wr_ctrl_unpack(uint8_t * avdd_setup, uint8_t * pgen_setup, uint8_t * addr_setup, uint8_t * burn_time, uint8_t * addr_hold, uint8_t * aen_hold, uint8_t * avdd_hold)
{
    t_efuse_wr_ctrl local_val = hwp_efuse->efuse_wr_ctrl;

    *avdd_setup         = local_val.bit_field.avdd_setup;
    *pgen_setup         = local_val.bit_field.pgen_setup;
    *addr_setup         = local_val.bit_field.addr_setup;
    *burn_time          = local_val.bit_field.burn_time;
    *addr_hold          = local_val.bit_field.addr_hold;
    *aen_hold           = local_val.bit_field.aen_hold;
    *avdd_hold          = local_val.bit_field.avdd_hold;
}

__STATIC_INLINE void efuse_rd_ctrl_unpack(uint8_t * rd_aen_setup, uint8_t * rd_aen_hold, uint8_t * rden_setup, uint8_t * rden_hold)
{
    t_efuse_rd_ctrl local_val = hwp_efuse->efuse_rd_ctrl;

    *rd_aen_setup       = local_val.bit_field.rd_aen_setup;
    *rd_aen_hold        = local_val.bit_field.rd_aen_hold;
    *rden_setup         = local_val.bit_field.rden_setup;
    *rden_hold          = local_val.bit_field.rden_hold;
}

__STATIC_INLINE void efuse_pwr_ctrl0_unpack(uint8_t * avdd, uint8_t * rden)
{
    t_efuse_pwr_ctrl0 local_val = hwp_efuse->efuse_pwr_ctrl0;

    *avdd               = local_val.bit_field.avdd;
    *rden               = local_val.bit_field.rden;
}

__STATIC_INLINE void efuse_pwr_ctrl1_unpack(uint8_t * ef_sr, uint8_t * ef_av_set)
{
    t_efuse_pwr_ctrl1 local_val = hwp_efuse->efuse_pwr_ctrl1;

    *ef_sr              = local_val.bit_field.ef_sr;
    *ef_av_set          = local_val.bit_field.ef_av_set;
}

__STATIC_INLINE void efuse_status_unpack(uint8_t * write_enable, uint8_t * efuse_busy)
{
    t_efuse_status local_val = hwp_efuse->efuse_status;

    *write_enable       = local_val.bit_field.write_enable;
    *efuse_busy         = local_val.bit_field.efuse_busy;
}

#endif

