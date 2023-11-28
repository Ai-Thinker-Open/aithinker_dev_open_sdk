#ifndef __REG_SYSC_AWO_H__
#define __REG_SYSC_AWO_H__

#include "ln882h.h"

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    hclk_div_para_up    : 1  ; // 0  : 0
        uint32_t    pclk1_div_para_up   : 1  ; // 1  : 1
        uint32_t    reserved_0          : 30 ; // 31 : 2
    } bit_field;
} t_sysc_awo_div_update;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    hclk_div_para_m1    : 4  ; // 3  : 0
        uint32_t    pclk1_div_para_m1   : 4  ; // 7  : 4
        uint32_t    reserved_0          : 24 ; // 31 : 8
    } bit_field;
} t_sysc_awo_div_para;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    swclkg_rtc          : 1  ; // 0  : 0
        uint32_t    swclkg_wic          : 1  ; // 1  : 1
        uint32_t    swclkg_phy          : 1  ; // 2  : 2
        uint32_t    swclkg_mac_20m      : 1  ; // 3  : 3
        uint32_t    swclkg_mac_40m      : 1  ; // 4  : 4
        uint32_t    swclkg_mac_80m      : 1  ; // 5  : 5
        uint32_t    reserved_0          : 26 ; // 31 : 6
    } bit_field;
} t_sysc_awo_sw_clkg;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    srstn_pmu           : 1  ; // 0  : 0
        uint32_t    srstn_wic           : 1  ; // 1  : 1
        uint32_t    srstn_rtc           : 1  ; // 2  : 2
        uint32_t    srstn_phy           : 1  ; // 3  : 3
        uint32_t    srstn_all           : 1  ; // 4  : 4
        uint32_t    reserved_0          : 27 ; // 31 : 5
    } bit_field;
} t_sysc_awo_sw_rstn;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    clk_src_sel         : 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_sysc_awo_clk_sel;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    reserved_3          : 8  ; // 7  : 0
        uint32_t    r_vtrim             : 2  ; // 9  : 8
        uint32_t    en_bod              : 1  ; // 10 : 10
        uint32_t    bod_vth             : 2  ; // 12 : 11
        uint32_t    reserved_2          : 1  ; // 13 : 13
        uint32_t    r_tm1               : 1  ; // 14 : 14
        uint32_t    r_tm2               : 1  ; // 15 : 15
        uint32_t    r_xtal40m_ldo_vbit  : 3  ; // 18 : 16
        uint32_t    reserved_1          : 1  ; // 19 : 19
        uint32_t    r_xtal40m_ibsel     : 2  ; // 21 : 20
        uint32_t    reserved_0          : 2  ; // 23 : 22
        uint32_t    r_xtal40m_cap_bit   : 7  ; // 30 : 24
        uint32_t    r_xtal40m_syspll_fref_en: 1  ; // 31 : 31
    } bit_field;
} t_sysc_awo_pmu_reg;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    o_pdcmp_po          : 1  ; // 0  : 0
        uint32_t    o_cpucore_retreg_po : 1  ; // 1  : 1
        uint32_t    ldo18_po_en         : 3  ; // 4  : 2
        uint32_t    ldo18_pd_en         : 3  ; // 7  : 5
        uint32_t    o_digldo_norvdd     : 3  ; // 10 : 8
        uint32_t    reserved_5          : 1  ; // 11 : 11
        uint32_t    o_digldo_retvdd     : 3  ; // 14 : 12
        uint32_t    reserved_4          : 1  ; // 15 : 15
        uint32_t    o_sram_po_en        : 3  ; // 18 : 16
        uint32_t    reserved_3          : 1  ; // 19 : 19
        uint32_t    o_sram_pd_en        : 3  ; // 22 : 20
        uint32_t    reserved_2          : 1  ; // 23 : 23
        uint32_t    o_phy_po_en         : 1  ; // 24 : 24
        uint32_t    reserved_1          : 3  ; // 27 : 25
        uint32_t    o_phy_pd_en         : 1  ; // 28 : 28
        uint32_t    o_digpll_po         : 1  ; // 29 : 29
        uint32_t    xtal_pd_en          : 1  ; // 30 : 30
        uint32_t    reserved_0          : 1  ; // 31 : 31
    } bit_field;
} t_sysc_awo_pmu_cfg;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    o_sw_po_start       : 1  ; // 0  : 0
        uint32_t    o_sw_pd_start       : 1  ; // 1  : 1
        uint32_t    reserved_0          : 30 ; // 31 : 2
    } bit_field;
} t_sysc_awo_sw_pwr_ctrl;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ext_inrp_sense_0    : 2  ; // 1  : 0
        uint32_t    ext_inrp_sense_1    : 2  ; // 3  : 2
        uint32_t    ext_inrp_sense_2    : 2  ; // 5  : 4
        uint32_t    ext_inrp_sense_3    : 2  ; // 7  : 6
        uint32_t    ext_inrp_sense_4    : 2  ; // 9  : 8
        uint32_t    ext_inrp_sense_5    : 2  ; // 11 : 10
        uint32_t    ext_inrp_sense_6    : 2  ; // 13 : 12
        uint32_t    ext_inrp_sense_7    : 2  ; // 15 : 14
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_sysc_awo_ext_intr_sence;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ext_inrp_en         : 8  ; // 7  : 0
        uint32_t    reserved_1          : 8  ; // 15 : 8
        uint32_t    pin_is_inrp_src     : 8  ; // 23 : 16
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_sysc_awo_ext_intr_en;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ext_inrp_value_r    : 8  ; // 7  : 0
        uint32_t    reserved_1          : 8  ; // 15 : 8
        uint32_t    ext_rawinrp_value   : 8  ; // 23 : 16
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_sysc_awo_intr_stat;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ext_inrp_clr_r      : 8  ; // 7  : 0
        uint32_t    reserved_0          : 24 ; // 31 : 8
    } bit_field;
} t_sysc_awo_ext_inrp_clr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pmu_enable          : 1  ; // 0  : 0
        uint32_t    o_xtal40_stb_dly    : 7  ; // 7  : 1
        uint32_t    o_pll_stb_dly       : 4  ; // 11 : 8
        uint32_t    reserved_0          : 4  ; // 15 : 12
        uint32_t    o_clk_swc_dly       : 4  ; // 19 : 16
        uint32_t    o_vdd11_h2l_dly     : 4  ; // 23 : 20
        uint32_t    o_vdd11_l2h_dly     : 4  ; // 27 : 24
        uint32_t    pmu_seq_time        : 4  ; // 31 : 28
    } bit_field;
} t_sysc_awo_swc_time;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    o_flash_mode        : 1  ; // 0  : 0
        uint32_t    o_pad_rst_mask      : 1  ; // 1  : 1
        uint32_t    o_cpu_rstreq_mask   : 1  ; // 2  : 2
        uint32_t    o_sdio_rstreq_mask  : 1  ; // 3  : 3
        uint32_t    o_pd_switch_intv    : 7  ; // 10 : 4
        uint32_t    o_digldo_debug_mode : 1  ; // 11 : 11
        uint32_t    o_wdt_rst_mask      : 1  ; // 12 : 12
        uint32_t    o_cpu_lockup_rst_mask: 1  ; // 13 : 13
        uint32_t    r_pmu_ret_en        : 1  ; // 14 : 14
        uint32_t    r_pmu_unret_en      : 1  ; // 15 : 15
        uint32_t    r_io_retention      : 1  ; // 16 : 16
        uint32_t    lp_mode_awo         : 1  ; // 17 : 17
        uint32_t    reserved_0          : 14 ; // 31 : 18
    } bit_field;
} t_sysc_awo_misc;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    r_idle_reg          : 32 ; // 31 : 0
    } bit_field;
} t_sysc_awo_idle_reg;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    i_boot_mode         : 3  ; // 2  : 0
        uint32_t    reserved_1          : 1  ; // 3  : 3
        uint32_t    pmu_fsm             : 5  ; // 8  : 4
        uint32_t    reserved_0          : 23 ; // 31 : 9
    } bit_field;
} t_sysc_awo_boot_mode_pmu_fsm;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    r_rco32k_ibit       : 4  ; // 3  : 0
        uint32_t    r_rco32k_rbit       : 3  ; // 6  : 4
        uint32_t    r_rco32k_bitsel     : 1  ; // 7  : 7
        uint32_t    r_rco32k_cbit       : 5  ; // 12 : 8
        uint32_t    reserved_0          : 19 ; // 31 : 13
    } bit_field;
} t_sysc_awo_rc32k_ctrl;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    syspll_div          : 5  ; // 4  : 0
        uint32_t    syspll_cpaux_ibit   : 3  ; // 7  : 5
        uint32_t    syspll_vreg_bit     : 4  ; // 11 : 8
        uint32_t    syspll_lf_ibit      : 3  ; // 14 : 12
        uint32_t    syspll_en           : 1  ; // 15 : 15
        uint32_t    syspll_cp_ibit      : 3  ; // 18 : 16
        uint32_t    syspll_rst          : 1  ; // 19 : 19
        uint32_t    syspll_sdm_clk_sel  : 1  ; // 20 : 20
        uint32_t    syspll_rvco_l_test  : 1  ; // 21 : 21
        uint32_t    syspll_rvco_h_test  : 1  ; // 22 : 22
        uint32_t    syspll_sdmclk_test_en: 1  ; // 23 : 23
        uint32_t    syspll_test_en      : 1  ; // 24 : 24
        uint32_t    syspll_ref2x_en     : 1  ; // 25 : 25
        uint32_t    reserved_0          : 6  ; // 31 : 26
    } bit_field;
} t_sysc_awo_syspll_ctrl;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    r_syspll_lock_deb_dly: 8  ; // 7  : 0
        uint32_t    r_syspll_lock_polarity: 1  ; // 8  : 8
        uint32_t    r_syspll_lock       : 1  ; // 9  : 9
        uint32_t    reserved_0          : 22 ; // 31 : 10
    } bit_field;
} t_sysc_awo_syspll_lock;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    o_cpu_sleep_time_l  : 32 ; // 31 : 0
    } bit_field;
} t_sysc_awo_slp_cnt_cfg_low;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    o_cpu_sleep_time_h  : 15 ; // 14 : 0
        uint32_t    o_cpu_sleep_counter_bp: 1  ; // 15 : 15
        uint32_t    o_cpu_sleep_inten   : 1  ; // 16 : 16
        uint32_t    reserved_0          : 15 ; // 31 : 17
    } bit_field;
} t_sysc_awo_slp_cnt_cfg_hi;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    i_rco32k_period_ns  : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_sysc_awo_cal_32k_result;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    i_real_sleep_time_l : 32 ; // 31 : 0
    } bit_field;
} t_sysc_awo_real_slp_time_l;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    i_real_sleep_time_h : 15 ; // 14 : 0
        uint32_t    reserved_0          : 17 ; // 31 : 15
    } bit_field;
} t_sysc_awo_real_slp_time_h;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    swd_en              : 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_sysc_awo_swd_en0;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    dbg_host_en         : 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_sysc_awo_dbg_hst_en;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ldo18_1_vset        : 3  ; // 2  : 0
        uint32_t    ldo18_2_vset        : 3  ; // 5  : 3
        uint32_t    ldo18_3_vset        : 3  ; // 8  : 6
        uint32_t    ldo18_4_vset        : 3  ; // 11 : 9
        uint32_t    ldo15_1_vset        : 3  ; // 14 : 12
        uint32_t    ldo18_phy_vset      : 2  ; // 16 : 15
        uint32_t    vlimit_vset_awo     : 2  ; // 18 : 17
        uint32_t    reserved_0          : 13 ; // 31 : 19
    } bit_field;
} t_sysc_awo_vset;


//--------------------------------------------------------------------------------

typedef struct
{
    volatile        t_sysc_awo_div_update                    div_update                             ; // 0x0
    volatile        t_sysc_awo_div_para                      div_para                               ; // 0x4
    volatile        t_sysc_awo_sw_clkg                       sw_clkg                                ; // 0x8
    volatile        t_sysc_awo_sw_rstn                       sw_rstn                                ; // 0xc
    volatile        t_sysc_awo_clk_sel                       clk_sel                                ; // 0x10
    volatile        t_sysc_awo_pmu_reg                       pmu_reg                                ; // 0x14
    volatile        t_sysc_awo_pmu_cfg                       pmu_cfg                                ; // 0x18
    volatile        t_sysc_awo_sw_pwr_ctrl                   sw_pwr_ctrl                            ; // 0x1c
    volatile        t_sysc_awo_ext_intr_sence                ext_intr_sence                         ; // 0x20
    volatile        t_sysc_awo_ext_intr_en                   ext_intr_en                            ; // 0x24
    volatile        t_sysc_awo_intr_stat                     intr_stat                              ; // 0x28
    volatile        t_sysc_awo_ext_inrp_clr                  ext_inrp_clr                           ; // 0x2c
    volatile        t_sysc_awo_swc_time                      swc_time                               ; // 0x30
    volatile        t_sysc_awo_misc                          misc                                   ; // 0x34
    volatile        t_sysc_awo_idle_reg                      idle_reg                               ; // 0x38
    volatile        uint32_t                                 reserved_1[1]                          ; // 0x3c
    volatile        t_sysc_awo_boot_mode_pmu_fsm             boot_mode_pmu_fsm                      ; // 0x40
    volatile        uint32_t                                 reserved_2[10]                         ; // 0x44
    volatile        t_sysc_awo_rc32k_ctrl                    rc32k_ctrl                             ; // 0x6c
    volatile        t_sysc_awo_syspll_ctrl                   syspll_ctrl                            ; // 0x70
    volatile        t_sysc_awo_syspll_lock                   syspll_lock                            ; // 0x74
    volatile        t_sysc_awo_slp_cnt_cfg_low               slp_cnt_cfg_low                        ; // 0x78
    volatile        t_sysc_awo_slp_cnt_cfg_hi                slp_cnt_cfg_hi                         ; // 0x7c
    volatile        t_sysc_awo_cal_32k_result                cal_32k_result                         ; // 0x80
    volatile        t_sysc_awo_real_slp_time_l               real_slp_time_l                        ; // 0x84
    volatile        t_sysc_awo_real_slp_time_h               real_slp_time_h                        ; // 0x88
    volatile        uint32_t                                 reserved_3[2]                          ; // 0x8c
    volatile        t_sysc_awo_swd_en0                       swd_en0                                ; // 0x94
    volatile        t_sysc_awo_dbg_hst_en                    dbg_hst_en                             ; // 0x98
    volatile        t_sysc_awo_vset                          vset                                   ; // 0x9c
} t_hwp_sysc_awo_t;

#define hwp_sysc_awo ((t_hwp_sysc_awo_t*)SYSC_AWO_BASE)


//--------------------------------------------------------------------------------

__STATIC_INLINE void sysc_awo_div_update_set(uint32_t value)
{
    hwp_sysc_awo->div_update.val = value;
}

__STATIC_INLINE void sysc_awo_pclk1_div_para_up_setf(uint8_t pclk1_div_para_up)
{
    hwp_sysc_awo->div_update.bit_field.pclk1_div_para_up = pclk1_div_para_up;
}

__STATIC_INLINE void sysc_awo_hclk_div_para_up_setf(uint8_t hclk_div_para_up)
{
    hwp_sysc_awo->div_update.bit_field.hclk_div_para_up = hclk_div_para_up;
}

__STATIC_INLINE void sysc_awo_div_para_set(uint32_t value)
{
    hwp_sysc_awo->div_para.val = value;
}

__STATIC_INLINE void sysc_awo_pclk1_div_para_m1_setf(uint8_t pclk1_div_para_m1)
{
    hwp_sysc_awo->div_para.bit_field.pclk1_div_para_m1 = pclk1_div_para_m1;
}

__STATIC_INLINE void sysc_awo_hclk_div_para_m1_setf(uint8_t hclk_div_para_m1)
{
    hwp_sysc_awo->div_para.bit_field.hclk_div_para_m1 = hclk_div_para_m1;
}

__STATIC_INLINE void sysc_awo_sw_clkg_set(uint32_t value)
{
    hwp_sysc_awo->sw_clkg.val = value;
}

__STATIC_INLINE void sysc_awo_swclkg_mac_80m_setf(uint8_t swclkg_mac_80m)
{
    hwp_sysc_awo->sw_clkg.bit_field.swclkg_mac_80m = swclkg_mac_80m;
}

__STATIC_INLINE void sysc_awo_swclkg_mac_40m_setf(uint8_t swclkg_mac_40m)
{
    hwp_sysc_awo->sw_clkg.bit_field.swclkg_mac_40m = swclkg_mac_40m;
}

__STATIC_INLINE void sysc_awo_swclkg_mac_20m_setf(uint8_t swclkg_mac_20m)
{
    hwp_sysc_awo->sw_clkg.bit_field.swclkg_mac_20m = swclkg_mac_20m;
}

__STATIC_INLINE void sysc_awo_swclkg_phy_setf(uint8_t swclkg_phy)
{
    hwp_sysc_awo->sw_clkg.bit_field.swclkg_phy = swclkg_phy;
}

__STATIC_INLINE void sysc_awo_swclkg_wic_setf(uint8_t swclkg_wic)
{
    hwp_sysc_awo->sw_clkg.bit_field.swclkg_wic = swclkg_wic;
}

__STATIC_INLINE void sysc_awo_swclkg_rtc_setf(uint8_t swclkg_rtc)
{
    hwp_sysc_awo->sw_clkg.bit_field.swclkg_rtc = swclkg_rtc;
}

__STATIC_INLINE void sysc_awo_sw_rstn_set(uint32_t value)
{
    hwp_sysc_awo->sw_rstn.val = value;
}

__STATIC_INLINE void sysc_awo_srstn_all_setf(uint8_t srstn_all)
{
    hwp_sysc_awo->sw_rstn.bit_field.srstn_all = srstn_all;
}

__STATIC_INLINE void sysc_awo_srstn_phy_setf(uint8_t srstn_phy)
{
    hwp_sysc_awo->sw_rstn.bit_field.srstn_phy = srstn_phy;
}

__STATIC_INLINE void sysc_awo_srstn_rtc_setf(uint8_t srstn_rtc)
{
    hwp_sysc_awo->sw_rstn.bit_field.srstn_rtc = srstn_rtc;
}

__STATIC_INLINE void sysc_awo_srstn_wic_setf(uint8_t srstn_wic)
{
    hwp_sysc_awo->sw_rstn.bit_field.srstn_wic = srstn_wic;
}

__STATIC_INLINE void sysc_awo_srstn_pmu_setf(uint8_t srstn_pmu)
{
    hwp_sysc_awo->sw_rstn.bit_field.srstn_pmu = srstn_pmu;
}

__STATIC_INLINE void sysc_awo_clk_sel_set(uint32_t value)
{
    hwp_sysc_awo->clk_sel.val = value;
}

__STATIC_INLINE void sysc_awo_clk_src_sel_setf(uint8_t clk_src_sel)
{
    hwp_sysc_awo->clk_sel.bit_field.clk_src_sel = clk_src_sel;
}

__STATIC_INLINE void sysc_awo_pmu_reg_set(uint32_t value)
{
    hwp_sysc_awo->pmu_reg.val = value;
}

__STATIC_INLINE void sysc_awo_r_xtal40m_syspll_fref_en_setf(uint8_t r_xtal40m_syspll_fref_en)
{
    hwp_sysc_awo->pmu_reg.bit_field.r_xtal40m_syspll_fref_en = r_xtal40m_syspll_fref_en;
}

__STATIC_INLINE void sysc_awo_r_xtal40m_cap_bit_setf(uint8_t r_xtal40m_cap_bit)
{
    hwp_sysc_awo->pmu_reg.bit_field.r_xtal40m_cap_bit = r_xtal40m_cap_bit;
}

__STATIC_INLINE void sysc_awo_r_xtal40m_ibsel_setf(uint8_t r_xtal40m_ibsel)
{
    hwp_sysc_awo->pmu_reg.bit_field.r_xtal40m_ibsel = r_xtal40m_ibsel;
}

__STATIC_INLINE void sysc_awo_r_xtal40m_ldo_vbit_setf(uint8_t r_xtal40m_ldo_vbit)
{
    hwp_sysc_awo->pmu_reg.bit_field.r_xtal40m_ldo_vbit = r_xtal40m_ldo_vbit;
}

__STATIC_INLINE void sysc_awo_r_tm2_setf(uint8_t r_tm2)
{
    hwp_sysc_awo->pmu_reg.bit_field.r_tm2 = r_tm2;
}

__STATIC_INLINE void sysc_awo_r_tm1_setf(uint8_t r_tm1)
{
    hwp_sysc_awo->pmu_reg.bit_field.r_tm1 = r_tm1;
}

__STATIC_INLINE void sysc_awo_bod_vth_setf(uint8_t bod_vth)
{
    hwp_sysc_awo->pmu_reg.bit_field.bod_vth = bod_vth;
}

__STATIC_INLINE void sysc_awo_en_bod_setf(uint8_t en_bod)
{
    hwp_sysc_awo->pmu_reg.bit_field.en_bod = en_bod;
}

__STATIC_INLINE void sysc_awo_r_vtrim_setf(uint8_t r_vtrim)
{
    hwp_sysc_awo->pmu_reg.bit_field.r_vtrim = r_vtrim;
}

__STATIC_INLINE void sysc_awo_pmu_cfg_set(uint32_t value)
{
    hwp_sysc_awo->pmu_cfg.val = value;
}

__STATIC_INLINE void sysc_awo_xtal_pd_en_setf(uint8_t xtal_pd_en)
{
    hwp_sysc_awo->pmu_cfg.bit_field.xtal_pd_en = xtal_pd_en;
}

__STATIC_INLINE void sysc_awo_o_digpll_po_setf(uint8_t o_digpll_po)
{
    hwp_sysc_awo->pmu_cfg.bit_field.o_digpll_po = o_digpll_po;
}

__STATIC_INLINE void sysc_awo_o_phy_pd_en_setf(uint8_t o_phy_pd_en)
{
    hwp_sysc_awo->pmu_cfg.bit_field.o_phy_pd_en = o_phy_pd_en;
}

__STATIC_INLINE void sysc_awo_o_phy_po_en_setf(uint8_t o_phy_po_en)
{
    hwp_sysc_awo->pmu_cfg.bit_field.o_phy_po_en = o_phy_po_en;
}

__STATIC_INLINE void sysc_awo_o_sram_pd_en_setf(uint8_t o_sram_pd_en)
{
    hwp_sysc_awo->pmu_cfg.bit_field.o_sram_pd_en = o_sram_pd_en;
}

__STATIC_INLINE void sysc_awo_o_sram_po_en_setf(uint8_t o_sram_po_en)
{
    hwp_sysc_awo->pmu_cfg.bit_field.o_sram_po_en = o_sram_po_en;
}

__STATIC_INLINE void sysc_awo_o_digldo_retvdd_setf(uint8_t o_digldo_retvdd)
{
    hwp_sysc_awo->pmu_cfg.bit_field.o_digldo_retvdd = o_digldo_retvdd;
}

__STATIC_INLINE void sysc_awo_o_digldo_norvdd_setf(uint8_t o_digldo_norvdd)
{
    hwp_sysc_awo->pmu_cfg.bit_field.o_digldo_norvdd = o_digldo_norvdd;
}

__STATIC_INLINE void sysc_awo_ldo18_pd_en_setf(uint8_t ldo18_pd_en)
{
    hwp_sysc_awo->pmu_cfg.bit_field.ldo18_pd_en = ldo18_pd_en;
}

__STATIC_INLINE void sysc_awo_ldo18_po_en_setf(uint8_t ldo18_po_en)
{
    hwp_sysc_awo->pmu_cfg.bit_field.ldo18_po_en = ldo18_po_en;
}

__STATIC_INLINE void sysc_awo_o_cpucore_retreg_po_setf(uint8_t o_cpucore_retreg_po)
{
    hwp_sysc_awo->pmu_cfg.bit_field.o_cpucore_retreg_po = o_cpucore_retreg_po;
}

__STATIC_INLINE void sysc_awo_o_pdcmp_po_setf(uint8_t o_pdcmp_po)
{
    hwp_sysc_awo->pmu_cfg.bit_field.o_pdcmp_po = o_pdcmp_po;
}

__STATIC_INLINE void sysc_awo_sw_pwr_ctrl_set(uint32_t value)
{
    hwp_sysc_awo->sw_pwr_ctrl.val = value;
}

__STATIC_INLINE void sysc_awo_o_sw_pd_start_setf(uint8_t o_sw_pd_start)
{
    hwp_sysc_awo->sw_pwr_ctrl.bit_field.o_sw_pd_start = o_sw_pd_start;
}

__STATIC_INLINE void sysc_awo_o_sw_po_start_setf(uint8_t o_sw_po_start)
{
    hwp_sysc_awo->sw_pwr_ctrl.bit_field.o_sw_po_start = o_sw_po_start;
}

__STATIC_INLINE void sysc_awo_ext_intr_sence_set(uint32_t value)
{
    hwp_sysc_awo->ext_intr_sence.val = value;
}

__STATIC_INLINE void sysc_awo_ext_inrp_sense_7_setf(uint8_t ext_inrp_sense_7)
{
    hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_7 = ext_inrp_sense_7;
}

__STATIC_INLINE void sysc_awo_ext_inrp_sense_6_setf(uint8_t ext_inrp_sense_6)
{
    hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_6 = ext_inrp_sense_6;
}

__STATIC_INLINE void sysc_awo_ext_inrp_sense_5_setf(uint8_t ext_inrp_sense_5)
{
    hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_5 = ext_inrp_sense_5;
}

__STATIC_INLINE void sysc_awo_ext_inrp_sense_4_setf(uint8_t ext_inrp_sense_4)
{
    hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_4 = ext_inrp_sense_4;
}

__STATIC_INLINE void sysc_awo_ext_inrp_sense_3_setf(uint8_t ext_inrp_sense_3)
{
    hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_3 = ext_inrp_sense_3;
}

__STATIC_INLINE void sysc_awo_ext_inrp_sense_2_setf(uint8_t ext_inrp_sense_2)
{
    hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_2 = ext_inrp_sense_2;
}

__STATIC_INLINE void sysc_awo_ext_inrp_sense_1_setf(uint8_t ext_inrp_sense_1)
{
    hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_1 = ext_inrp_sense_1;
}

__STATIC_INLINE void sysc_awo_ext_inrp_sense_0_setf(uint8_t ext_inrp_sense_0)
{
    hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_0 = ext_inrp_sense_0;
}

__STATIC_INLINE void sysc_awo_ext_intr_en_set(uint32_t value)
{
    hwp_sysc_awo->ext_intr_en.val = value;
}

__STATIC_INLINE void sysc_awo_pin_is_inrp_src_setf(uint8_t pin_is_inrp_src)
{
    hwp_sysc_awo->ext_intr_en.bit_field.pin_is_inrp_src = pin_is_inrp_src;
}

__STATIC_INLINE void sysc_awo_ext_inrp_en_setf(uint8_t ext_inrp_en)
{
    hwp_sysc_awo->ext_intr_en.bit_field.ext_inrp_en = ext_inrp_en;
}

__STATIC_INLINE void sysc_awo_ext_inrp_clr_set(uint32_t value)
{
    hwp_sysc_awo->ext_inrp_clr.val = value;
}

__STATIC_INLINE void sysc_awo_ext_inrp_clr_r_setf(uint8_t ext_inrp_clr_r)
{
    hwp_sysc_awo->ext_inrp_clr.bit_field.ext_inrp_clr_r = ext_inrp_clr_r;
}

__STATIC_INLINE void sysc_awo_swc_time_set(uint32_t value)
{
    hwp_sysc_awo->swc_time.val = value;
}

__STATIC_INLINE void sysc_awo_pmu_seq_time_setf(uint8_t pmu_seq_time)
{
    hwp_sysc_awo->swc_time.bit_field.pmu_seq_time = pmu_seq_time;
}

__STATIC_INLINE void sysc_awo_o_vdd11_l2h_dly_setf(uint8_t o_vdd11_l2h_dly)
{
    hwp_sysc_awo->swc_time.bit_field.o_vdd11_l2h_dly = o_vdd11_l2h_dly;
}

__STATIC_INLINE void sysc_awo_o_vdd11_h2l_dly_setf(uint8_t o_vdd11_h2l_dly)
{
    hwp_sysc_awo->swc_time.bit_field.o_vdd11_h2l_dly = o_vdd11_h2l_dly;
}

__STATIC_INLINE void sysc_awo_o_clk_swc_dly_setf(uint8_t o_clk_swc_dly)
{
    hwp_sysc_awo->swc_time.bit_field.o_clk_swc_dly = o_clk_swc_dly;
}

__STATIC_INLINE void sysc_awo_o_pll_stb_dly_setf(uint8_t o_pll_stb_dly)
{
    hwp_sysc_awo->swc_time.bit_field.o_pll_stb_dly = o_pll_stb_dly;
}

__STATIC_INLINE void sysc_awo_o_xtal40_stb_dly_setf(uint8_t o_xtal40_stb_dly)
{
    hwp_sysc_awo->swc_time.bit_field.o_xtal40_stb_dly = o_xtal40_stb_dly;
}

__STATIC_INLINE void sysc_awo_pmu_enable_setf(uint8_t pmu_enable)
{
    hwp_sysc_awo->swc_time.bit_field.pmu_enable = pmu_enable;
}

__STATIC_INLINE void sysc_awo_misc_set(uint32_t value)
{
    hwp_sysc_awo->misc.val = value;
}

__STATIC_INLINE void sysc_awo_lp_mode_awo_setf(uint8_t lp_mode_awo)
{
    hwp_sysc_awo->misc.bit_field.lp_mode_awo = lp_mode_awo;
}

__STATIC_INLINE void sysc_awo_r_io_retention_setf(uint8_t r_io_retention)
{
    hwp_sysc_awo->misc.bit_field.r_io_retention = r_io_retention;
}

__STATIC_INLINE void sysc_awo_r_pmu_unret_en_setf(uint8_t r_pmu_unret_en)
{
    hwp_sysc_awo->misc.bit_field.r_pmu_unret_en = r_pmu_unret_en;
}

__STATIC_INLINE void sysc_awo_r_pmu_ret_en_setf(uint8_t r_pmu_ret_en)
{
    hwp_sysc_awo->misc.bit_field.r_pmu_ret_en = r_pmu_ret_en;
}

__STATIC_INLINE void sysc_awo_o_cpu_lockup_rst_mask_setf(uint8_t o_cpu_lockup_rst_mask)
{
    hwp_sysc_awo->misc.bit_field.o_cpu_lockup_rst_mask = o_cpu_lockup_rst_mask;
}

__STATIC_INLINE void sysc_awo_o_wdt_rst_mask_setf(uint8_t o_wdt_rst_mask)
{
    hwp_sysc_awo->misc.bit_field.o_wdt_rst_mask = o_wdt_rst_mask;
}

__STATIC_INLINE void sysc_awo_o_digldo_debug_mode_setf(uint8_t o_digldo_debug_mode)
{
    hwp_sysc_awo->misc.bit_field.o_digldo_debug_mode = o_digldo_debug_mode;
}

__STATIC_INLINE void sysc_awo_o_pd_switch_intv_setf(uint8_t o_pd_switch_intv)
{
    hwp_sysc_awo->misc.bit_field.o_pd_switch_intv = o_pd_switch_intv;
}

__STATIC_INLINE void sysc_awo_o_sdio_rstreq_mask_setf(uint8_t o_sdio_rstreq_mask)
{
    hwp_sysc_awo->misc.bit_field.o_sdio_rstreq_mask = o_sdio_rstreq_mask;
}

__STATIC_INLINE void sysc_awo_o_cpu_rstreq_mask_setf(uint8_t o_cpu_rstreq_mask)
{
    hwp_sysc_awo->misc.bit_field.o_cpu_rstreq_mask = o_cpu_rstreq_mask;
}

__STATIC_INLINE void sysc_awo_o_pad_rst_mask_setf(uint8_t o_pad_rst_mask)
{
    hwp_sysc_awo->misc.bit_field.o_pad_rst_mask = o_pad_rst_mask;
}

__STATIC_INLINE void sysc_awo_o_flash_mode_setf(uint8_t o_flash_mode)
{
    hwp_sysc_awo->misc.bit_field.o_flash_mode = o_flash_mode;
}

__STATIC_INLINE void sysc_awo_idle_reg_set(uint32_t value)
{
    hwp_sysc_awo->idle_reg.val = value;
}

__STATIC_INLINE void sysc_awo_r_idle_reg_setf(uint32_t r_idle_reg)
{
    hwp_sysc_awo->idle_reg.bit_field.r_idle_reg = r_idle_reg;
}

__STATIC_INLINE void sysc_awo_rc32k_ctrl_set(uint32_t value)
{
    hwp_sysc_awo->rc32k_ctrl.val = value;
}

__STATIC_INLINE void sysc_awo_r_rco32k_cbit_setf(uint8_t r_rco32k_cbit)
{
    hwp_sysc_awo->rc32k_ctrl.bit_field.r_rco32k_cbit = r_rco32k_cbit;
}

__STATIC_INLINE void sysc_awo_r_rco32k_bitsel_setf(uint8_t r_rco32k_bitsel)
{
    hwp_sysc_awo->rc32k_ctrl.bit_field.r_rco32k_bitsel = r_rco32k_bitsel;
}

__STATIC_INLINE void sysc_awo_r_rco32k_rbit_setf(uint8_t r_rco32k_rbit)
{
    hwp_sysc_awo->rc32k_ctrl.bit_field.r_rco32k_rbit = r_rco32k_rbit;
}

__STATIC_INLINE void sysc_awo_r_rco32k_ibit_setf(uint8_t r_rco32k_ibit)
{
    hwp_sysc_awo->rc32k_ctrl.bit_field.r_rco32k_ibit = r_rco32k_ibit;
}

__STATIC_INLINE void sysc_awo_syspll_ctrl_set(uint32_t value)
{
    hwp_sysc_awo->syspll_ctrl.val = value;
}

__STATIC_INLINE void sysc_awo_syspll_ref2x_en_setf(uint8_t syspll_ref2x_en)
{
    hwp_sysc_awo->syspll_ctrl.bit_field.syspll_ref2x_en = syspll_ref2x_en;
}

__STATIC_INLINE void sysc_awo_syspll_test_en_setf(uint8_t syspll_test_en)
{
    hwp_sysc_awo->syspll_ctrl.bit_field.syspll_test_en = syspll_test_en;
}

__STATIC_INLINE void sysc_awo_syspll_sdmclk_test_en_setf(uint8_t syspll_sdmclk_test_en)
{
    hwp_sysc_awo->syspll_ctrl.bit_field.syspll_sdmclk_test_en = syspll_sdmclk_test_en;
}

__STATIC_INLINE void sysc_awo_syspll_rvco_h_test_setf(uint8_t syspll_rvco_h_test)
{
    hwp_sysc_awo->syspll_ctrl.bit_field.syspll_rvco_h_test = syspll_rvco_h_test;
}

__STATIC_INLINE void sysc_awo_syspll_rvco_l_test_setf(uint8_t syspll_rvco_l_test)
{
    hwp_sysc_awo->syspll_ctrl.bit_field.syspll_rvco_l_test = syspll_rvco_l_test;
}

__STATIC_INLINE void sysc_awo_syspll_sdm_clk_sel_setf(uint8_t syspll_sdm_clk_sel)
{
    hwp_sysc_awo->syspll_ctrl.bit_field.syspll_sdm_clk_sel = syspll_sdm_clk_sel;
}

__STATIC_INLINE void sysc_awo_syspll_rst_setf(uint8_t syspll_rst)
{
    hwp_sysc_awo->syspll_ctrl.bit_field.syspll_rst = syspll_rst;
}

__STATIC_INLINE void sysc_awo_syspll_cp_ibit_setf(uint8_t syspll_cp_ibit)
{
    hwp_sysc_awo->syspll_ctrl.bit_field.syspll_cp_ibit = syspll_cp_ibit;
}

__STATIC_INLINE void sysc_awo_syspll_en_setf(uint8_t syspll_en)
{
    hwp_sysc_awo->syspll_ctrl.bit_field.syspll_en = syspll_en;
}

__STATIC_INLINE void sysc_awo_syspll_lf_ibit_setf(uint8_t syspll_lf_ibit)
{
    hwp_sysc_awo->syspll_ctrl.bit_field.syspll_lf_ibit = syspll_lf_ibit;
}

__STATIC_INLINE void sysc_awo_syspll_vreg_bit_setf(uint8_t syspll_vreg_bit)
{
    hwp_sysc_awo->syspll_ctrl.bit_field.syspll_vreg_bit = syspll_vreg_bit;
}

__STATIC_INLINE void sysc_awo_syspll_cpaux_ibit_setf(uint8_t syspll_cpaux_ibit)
{
    hwp_sysc_awo->syspll_ctrl.bit_field.syspll_cpaux_ibit = syspll_cpaux_ibit;
}

__STATIC_INLINE void sysc_awo_syspll_div_setf(uint8_t syspll_div)
{
    hwp_sysc_awo->syspll_ctrl.bit_field.syspll_div = syspll_div;
}

__STATIC_INLINE void sysc_awo_syspll_lock_set(uint32_t value)
{
    hwp_sysc_awo->syspll_lock.val = value;
}

__STATIC_INLINE void sysc_awo_r_syspll_lock_polarity_setf(uint8_t r_syspll_lock_polarity)
{
    hwp_sysc_awo->syspll_lock.bit_field.r_syspll_lock_polarity = r_syspll_lock_polarity;
}

__STATIC_INLINE void sysc_awo_r_syspll_lock_deb_dly_setf(uint8_t r_syspll_lock_deb_dly)
{
    hwp_sysc_awo->syspll_lock.bit_field.r_syspll_lock_deb_dly = r_syspll_lock_deb_dly;
}

__STATIC_INLINE void sysc_awo_slp_cnt_cfg_low_set(uint32_t value)
{
    hwp_sysc_awo->slp_cnt_cfg_low.val = value;
}

__STATIC_INLINE void sysc_awo_o_cpu_sleep_time_l_setf(uint32_t o_cpu_sleep_time_l)
{
    hwp_sysc_awo->slp_cnt_cfg_low.bit_field.o_cpu_sleep_time_l = o_cpu_sleep_time_l;
}

__STATIC_INLINE void sysc_awo_slp_cnt_cfg_hi_set(uint32_t value)
{
    hwp_sysc_awo->slp_cnt_cfg_hi.val = value;
}

__STATIC_INLINE void sysc_awo_o_cpu_sleep_inten_setf(uint8_t o_cpu_sleep_inten)
{
    hwp_sysc_awo->slp_cnt_cfg_hi.bit_field.o_cpu_sleep_inten = o_cpu_sleep_inten;
}

__STATIC_INLINE void sysc_awo_o_cpu_sleep_counter_bp_setf(uint8_t o_cpu_sleep_counter_bp)
{
    hwp_sysc_awo->slp_cnt_cfg_hi.bit_field.o_cpu_sleep_counter_bp = o_cpu_sleep_counter_bp;
}

__STATIC_INLINE void sysc_awo_o_cpu_sleep_time_h_setf(uint16_t o_cpu_sleep_time_h)
{
    hwp_sysc_awo->slp_cnt_cfg_hi.bit_field.o_cpu_sleep_time_h = o_cpu_sleep_time_h;
}

__STATIC_INLINE void sysc_awo_swd_en0_set(uint32_t value)
{
    hwp_sysc_awo->swd_en0.val = value;
}

__STATIC_INLINE void sysc_awo_swd_en_setf(uint8_t swd_en)
{
    hwp_sysc_awo->swd_en0.bit_field.swd_en = swd_en;
}

__STATIC_INLINE void sysc_awo_dbg_hst_en_set(uint32_t value)
{
    hwp_sysc_awo->dbg_hst_en.val = value;
}

__STATIC_INLINE void sysc_awo_dbg_host_en_setf(uint8_t dbg_host_en)
{
    hwp_sysc_awo->dbg_hst_en.bit_field.dbg_host_en = dbg_host_en;
}

__STATIC_INLINE void sysc_awo_vset_set(uint32_t value)
{
    hwp_sysc_awo->vset.val = value;
}

__STATIC_INLINE void sysc_awo_vlimit_vset_awo_setf(uint8_t vlimit_vset_awo)
{
    hwp_sysc_awo->vset.bit_field.vlimit_vset_awo = vlimit_vset_awo;
}

__STATIC_INLINE void sysc_awo_ldo18_phy_vset_setf(uint8_t ldo18_phy_vset)
{
    hwp_sysc_awo->vset.bit_field.ldo18_phy_vset = ldo18_phy_vset;
}

__STATIC_INLINE void sysc_awo_ldo15_1_vset_setf(uint8_t ldo15_1_vset)
{
    hwp_sysc_awo->vset.bit_field.ldo15_1_vset = ldo15_1_vset;
}

__STATIC_INLINE void sysc_awo_ldo18_4_vset_setf(uint8_t ldo18_4_vset)
{
    hwp_sysc_awo->vset.bit_field.ldo18_4_vset = ldo18_4_vset;
}

__STATIC_INLINE void sysc_awo_ldo18_3_vset_setf(uint8_t ldo18_3_vset)
{
    hwp_sysc_awo->vset.bit_field.ldo18_3_vset = ldo18_3_vset;
}

__STATIC_INLINE void sysc_awo_ldo18_2_vset_setf(uint8_t ldo18_2_vset)
{
    hwp_sysc_awo->vset.bit_field.ldo18_2_vset = ldo18_2_vset;
}

__STATIC_INLINE void sysc_awo_ldo18_1_vset_setf(uint8_t ldo18_1_vset)
{
    hwp_sysc_awo->vset.bit_field.ldo18_1_vset = ldo18_1_vset;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE uint32_t sysc_awo_div_para_get(void)
{
    return hwp_sysc_awo->div_para.val;
}

__STATIC_INLINE uint8_t sysc_awo_pclk1_div_para_m1_getf(void)
{
    return hwp_sysc_awo->div_para.bit_field.pclk1_div_para_m1;
}

__STATIC_INLINE uint8_t sysc_awo_hclk_div_para_m1_getf(void)
{
    return hwp_sysc_awo->div_para.bit_field.hclk_div_para_m1;
}

__STATIC_INLINE uint32_t sysc_awo_sw_clkg_get(void)
{
    return hwp_sysc_awo->sw_clkg.val;
}

__STATIC_INLINE uint8_t sysc_awo_swclkg_mac_80m_getf(void)
{
    return hwp_sysc_awo->sw_clkg.bit_field.swclkg_mac_80m;
}

__STATIC_INLINE uint8_t sysc_awo_swclkg_mac_40m_getf(void)
{
    return hwp_sysc_awo->sw_clkg.bit_field.swclkg_mac_40m;
}

__STATIC_INLINE uint8_t sysc_awo_swclkg_mac_20m_getf(void)
{
    return hwp_sysc_awo->sw_clkg.bit_field.swclkg_mac_20m;
}

__STATIC_INLINE uint8_t sysc_awo_swclkg_phy_getf(void)
{
    return hwp_sysc_awo->sw_clkg.bit_field.swclkg_phy;
}

__STATIC_INLINE uint8_t sysc_awo_swclkg_wic_getf(void)
{
    return hwp_sysc_awo->sw_clkg.bit_field.swclkg_wic;
}

__STATIC_INLINE uint8_t sysc_awo_swclkg_rtc_getf(void)
{
    return hwp_sysc_awo->sw_clkg.bit_field.swclkg_rtc;
}

__STATIC_INLINE uint32_t sysc_awo_sw_rstn_get(void)
{
    return hwp_sysc_awo->sw_rstn.val;
}

__STATIC_INLINE uint8_t sysc_awo_srstn_all_getf(void)
{
    return hwp_sysc_awo->sw_rstn.bit_field.srstn_all;
}

__STATIC_INLINE uint8_t sysc_awo_srstn_phy_getf(void)
{
    return hwp_sysc_awo->sw_rstn.bit_field.srstn_phy;
}

__STATIC_INLINE uint8_t sysc_awo_srstn_rtc_getf(void)
{
    return hwp_sysc_awo->sw_rstn.bit_field.srstn_rtc;
}

__STATIC_INLINE uint8_t sysc_awo_srstn_wic_getf(void)
{
    return hwp_sysc_awo->sw_rstn.bit_field.srstn_wic;
}

__STATIC_INLINE uint8_t sysc_awo_srstn_pmu_getf(void)
{
    return hwp_sysc_awo->sw_rstn.bit_field.srstn_pmu;
}

__STATIC_INLINE uint32_t sysc_awo_clk_sel_get(void)
{
    return hwp_sysc_awo->clk_sel.val;
}

__STATIC_INLINE uint8_t sysc_awo_clk_src_sel_getf(void)
{
    return hwp_sysc_awo->clk_sel.bit_field.clk_src_sel;
}

__STATIC_INLINE uint32_t sysc_awo_pmu_reg_get(void)
{
    return hwp_sysc_awo->pmu_reg.val;
}

__STATIC_INLINE uint8_t sysc_awo_r_xtal40m_syspll_fref_en_getf(void)
{
    return hwp_sysc_awo->pmu_reg.bit_field.r_xtal40m_syspll_fref_en;
}

__STATIC_INLINE uint8_t sysc_awo_r_xtal40m_cap_bit_getf(void)
{
    return hwp_sysc_awo->pmu_reg.bit_field.r_xtal40m_cap_bit;
}

__STATIC_INLINE uint8_t sysc_awo_r_xtal40m_ibsel_getf(void)
{
    return hwp_sysc_awo->pmu_reg.bit_field.r_xtal40m_ibsel;
}

__STATIC_INLINE uint8_t sysc_awo_r_xtal40m_ldo_vbit_getf(void)
{
    return hwp_sysc_awo->pmu_reg.bit_field.r_xtal40m_ldo_vbit;
}

__STATIC_INLINE uint8_t sysc_awo_r_tm2_getf(void)
{
    return hwp_sysc_awo->pmu_reg.bit_field.r_tm2;
}

__STATIC_INLINE uint8_t sysc_awo_r_tm1_getf(void)
{
    return hwp_sysc_awo->pmu_reg.bit_field.r_tm1;
}

__STATIC_INLINE uint8_t sysc_awo_bod_vth_getf(void)
{
    return hwp_sysc_awo->pmu_reg.bit_field.bod_vth;
}

__STATIC_INLINE uint8_t sysc_awo_en_bod_getf(void)
{
    return hwp_sysc_awo->pmu_reg.bit_field.en_bod;
}

__STATIC_INLINE uint8_t sysc_awo_r_vtrim_getf(void)
{
    return hwp_sysc_awo->pmu_reg.bit_field.r_vtrim;
}

__STATIC_INLINE uint32_t sysc_awo_pmu_cfg_get(void)
{
    return hwp_sysc_awo->pmu_cfg.val;
}

__STATIC_INLINE uint8_t sysc_awo_xtal_pd_en_getf(void)
{
    return hwp_sysc_awo->pmu_cfg.bit_field.xtal_pd_en;
}

__STATIC_INLINE uint8_t sysc_awo_o_digpll_po_getf(void)
{
    return hwp_sysc_awo->pmu_cfg.bit_field.o_digpll_po;
}

__STATIC_INLINE uint8_t sysc_awo_o_phy_pd_en_getf(void)
{
    return hwp_sysc_awo->pmu_cfg.bit_field.o_phy_pd_en;
}

__STATIC_INLINE uint8_t sysc_awo_o_phy_po_en_getf(void)
{
    return hwp_sysc_awo->pmu_cfg.bit_field.o_phy_po_en;
}

__STATIC_INLINE uint8_t sysc_awo_o_sram_pd_en_getf(void)
{
    return hwp_sysc_awo->pmu_cfg.bit_field.o_sram_pd_en;
}

__STATIC_INLINE uint8_t sysc_awo_o_sram_po_en_getf(void)
{
    return hwp_sysc_awo->pmu_cfg.bit_field.o_sram_po_en;
}

__STATIC_INLINE uint8_t sysc_awo_o_digldo_retvdd_getf(void)
{
    return hwp_sysc_awo->pmu_cfg.bit_field.o_digldo_retvdd;
}

__STATIC_INLINE uint8_t sysc_awo_o_digldo_norvdd_getf(void)
{
    return hwp_sysc_awo->pmu_cfg.bit_field.o_digldo_norvdd;
}

__STATIC_INLINE uint8_t sysc_awo_ldo18_pd_en_getf(void)
{
    return hwp_sysc_awo->pmu_cfg.bit_field.ldo18_pd_en;
}

__STATIC_INLINE uint8_t sysc_awo_ldo18_po_en_getf(void)
{
    return hwp_sysc_awo->pmu_cfg.bit_field.ldo18_po_en;
}

__STATIC_INLINE uint8_t sysc_awo_o_cpucore_retreg_po_getf(void)
{
    return hwp_sysc_awo->pmu_cfg.bit_field.o_cpucore_retreg_po;
}

__STATIC_INLINE uint8_t sysc_awo_o_pdcmp_po_getf(void)
{
    return hwp_sysc_awo->pmu_cfg.bit_field.o_pdcmp_po;
}

__STATIC_INLINE uint32_t sysc_awo_ext_intr_sence_get(void)
{
    return hwp_sysc_awo->ext_intr_sence.val;
}

__STATIC_INLINE uint8_t sysc_awo_ext_inrp_sense_7_getf(void)
{
    return hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_7;
}

__STATIC_INLINE uint8_t sysc_awo_ext_inrp_sense_6_getf(void)
{
    return hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_6;
}

__STATIC_INLINE uint8_t sysc_awo_ext_inrp_sense_5_getf(void)
{
    return hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_5;
}

__STATIC_INLINE uint8_t sysc_awo_ext_inrp_sense_4_getf(void)
{
    return hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_4;
}

__STATIC_INLINE uint8_t sysc_awo_ext_inrp_sense_3_getf(void)
{
    return hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_3;
}

__STATIC_INLINE uint8_t sysc_awo_ext_inrp_sense_2_getf(void)
{
    return hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_2;
}

__STATIC_INLINE uint8_t sysc_awo_ext_inrp_sense_1_getf(void)
{
    return hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_1;
}

__STATIC_INLINE uint8_t sysc_awo_ext_inrp_sense_0_getf(void)
{
    return hwp_sysc_awo->ext_intr_sence.bit_field.ext_inrp_sense_0;
}

__STATIC_INLINE uint32_t sysc_awo_ext_intr_en_get(void)
{
    return hwp_sysc_awo->ext_intr_en.val;
}

__STATIC_INLINE uint8_t sysc_awo_pin_is_inrp_src_getf(void)
{
    return hwp_sysc_awo->ext_intr_en.bit_field.pin_is_inrp_src;
}

__STATIC_INLINE uint8_t sysc_awo_ext_inrp_en_getf(void)
{
    return hwp_sysc_awo->ext_intr_en.bit_field.ext_inrp_en;
}

__STATIC_INLINE uint32_t sysc_awo_intr_stat_get(void)
{
    return hwp_sysc_awo->intr_stat.val;
}

__STATIC_INLINE uint8_t sysc_awo_ext_rawinrp_value_getf(void)
{
    return hwp_sysc_awo->intr_stat.bit_field.ext_rawinrp_value;
}

__STATIC_INLINE uint8_t sysc_awo_ext_inrp_value_r_getf(void)
{
    return hwp_sysc_awo->intr_stat.bit_field.ext_inrp_value_r;
}

__STATIC_INLINE uint32_t sysc_awo_swc_time_get(void)
{
    return hwp_sysc_awo->swc_time.val;
}

__STATIC_INLINE uint8_t sysc_awo_pmu_seq_time_getf(void)
{
    return hwp_sysc_awo->swc_time.bit_field.pmu_seq_time;
}

__STATIC_INLINE uint8_t sysc_awo_o_vdd11_l2h_dly_getf(void)
{
    return hwp_sysc_awo->swc_time.bit_field.o_vdd11_l2h_dly;
}

__STATIC_INLINE uint8_t sysc_awo_o_vdd11_h2l_dly_getf(void)
{
    return hwp_sysc_awo->swc_time.bit_field.o_vdd11_h2l_dly;
}

__STATIC_INLINE uint8_t sysc_awo_o_clk_swc_dly_getf(void)
{
    return hwp_sysc_awo->swc_time.bit_field.o_clk_swc_dly;
}

__STATIC_INLINE uint8_t sysc_awo_o_pll_stb_dly_getf(void)
{
    return hwp_sysc_awo->swc_time.bit_field.o_pll_stb_dly;
}

__STATIC_INLINE uint8_t sysc_awo_o_xtal40_stb_dly_getf(void)
{
    return hwp_sysc_awo->swc_time.bit_field.o_xtal40_stb_dly;
}

__STATIC_INLINE uint8_t sysc_awo_pmu_enable_getf(void)
{
    return hwp_sysc_awo->swc_time.bit_field.pmu_enable;
}

__STATIC_INLINE uint32_t sysc_awo_misc_get(void)
{
    return hwp_sysc_awo->misc.val;
}

__STATIC_INLINE uint8_t sysc_awo_lp_mode_awo_getf(void)
{
    return hwp_sysc_awo->misc.bit_field.lp_mode_awo;
}

__STATIC_INLINE uint8_t sysc_awo_r_io_retention_getf(void)
{
    return hwp_sysc_awo->misc.bit_field.r_io_retention;
}

__STATIC_INLINE uint8_t sysc_awo_r_pmu_unret_en_getf(void)
{
    return hwp_sysc_awo->misc.bit_field.r_pmu_unret_en;
}

__STATIC_INLINE uint8_t sysc_awo_r_pmu_ret_en_getf(void)
{
    return hwp_sysc_awo->misc.bit_field.r_pmu_ret_en;
}

__STATIC_INLINE uint8_t sysc_awo_o_cpu_lockup_rst_mask_getf(void)
{
    return hwp_sysc_awo->misc.bit_field.o_cpu_lockup_rst_mask;
}

__STATIC_INLINE uint8_t sysc_awo_o_wdt_rst_mask_getf(void)
{
    return hwp_sysc_awo->misc.bit_field.o_wdt_rst_mask;
}

__STATIC_INLINE uint8_t sysc_awo_o_digldo_debug_mode_getf(void)
{
    return hwp_sysc_awo->misc.bit_field.o_digldo_debug_mode;
}

__STATIC_INLINE uint8_t sysc_awo_o_pd_switch_intv_getf(void)
{
    return hwp_sysc_awo->misc.bit_field.o_pd_switch_intv;
}

__STATIC_INLINE uint8_t sysc_awo_o_sdio_rstreq_mask_getf(void)
{
    return hwp_sysc_awo->misc.bit_field.o_sdio_rstreq_mask;
}

__STATIC_INLINE uint8_t sysc_awo_o_cpu_rstreq_mask_getf(void)
{
    return hwp_sysc_awo->misc.bit_field.o_cpu_rstreq_mask;
}

__STATIC_INLINE uint8_t sysc_awo_o_pad_rst_mask_getf(void)
{
    return hwp_sysc_awo->misc.bit_field.o_pad_rst_mask;
}

__STATIC_INLINE uint8_t sysc_awo_o_flash_mode_getf(void)
{
    return hwp_sysc_awo->misc.bit_field.o_flash_mode;
}

__STATIC_INLINE uint32_t sysc_awo_idle_reg_get(void)
{
    return hwp_sysc_awo->idle_reg.val;
}

__STATIC_INLINE uint32_t sysc_awo_r_idle_reg_getf(void)
{
    return hwp_sysc_awo->idle_reg.bit_field.r_idle_reg;
}

__STATIC_INLINE uint32_t sysc_awo_boot_mode_pmu_fsm_get(void)
{
    return hwp_sysc_awo->boot_mode_pmu_fsm.val;
}

__STATIC_INLINE uint8_t sysc_awo_pmu_fsm_getf(void)
{
    return hwp_sysc_awo->boot_mode_pmu_fsm.bit_field.pmu_fsm;
}

__STATIC_INLINE uint8_t sysc_awo_i_boot_mode_getf(void)
{
    return hwp_sysc_awo->boot_mode_pmu_fsm.bit_field.i_boot_mode;
}

__STATIC_INLINE uint32_t sysc_awo_rc32k_ctrl_get(void)
{
    return hwp_sysc_awo->rc32k_ctrl.val;
}

__STATIC_INLINE uint8_t sysc_awo_r_rco32k_cbit_getf(void)
{
    return hwp_sysc_awo->rc32k_ctrl.bit_field.r_rco32k_cbit;
}

__STATIC_INLINE uint8_t sysc_awo_r_rco32k_bitsel_getf(void)
{
    return hwp_sysc_awo->rc32k_ctrl.bit_field.r_rco32k_bitsel;
}

__STATIC_INLINE uint8_t sysc_awo_r_rco32k_rbit_getf(void)
{
    return hwp_sysc_awo->rc32k_ctrl.bit_field.r_rco32k_rbit;
}

__STATIC_INLINE uint8_t sysc_awo_r_rco32k_ibit_getf(void)
{
    return hwp_sysc_awo->rc32k_ctrl.bit_field.r_rco32k_ibit;
}

__STATIC_INLINE uint32_t sysc_awo_syspll_ctrl_get(void)
{
    return hwp_sysc_awo->syspll_ctrl.val;
}

__STATIC_INLINE uint8_t sysc_awo_syspll_ref2x_en_getf(void)
{
    return hwp_sysc_awo->syspll_ctrl.bit_field.syspll_ref2x_en;
}

__STATIC_INLINE uint8_t sysc_awo_syspll_test_en_getf(void)
{
    return hwp_sysc_awo->syspll_ctrl.bit_field.syspll_test_en;
}

__STATIC_INLINE uint8_t sysc_awo_syspll_sdmclk_test_en_getf(void)
{
    return hwp_sysc_awo->syspll_ctrl.bit_field.syspll_sdmclk_test_en;
}

__STATIC_INLINE uint8_t sysc_awo_syspll_rvco_h_test_getf(void)
{
    return hwp_sysc_awo->syspll_ctrl.bit_field.syspll_rvco_h_test;
}

__STATIC_INLINE uint8_t sysc_awo_syspll_rvco_l_test_getf(void)
{
    return hwp_sysc_awo->syspll_ctrl.bit_field.syspll_rvco_l_test;
}

__STATIC_INLINE uint8_t sysc_awo_syspll_sdm_clk_sel_getf(void)
{
    return hwp_sysc_awo->syspll_ctrl.bit_field.syspll_sdm_clk_sel;
}

__STATIC_INLINE uint8_t sysc_awo_syspll_rst_getf(void)
{
    return hwp_sysc_awo->syspll_ctrl.bit_field.syspll_rst;
}

__STATIC_INLINE uint8_t sysc_awo_syspll_cp_ibit_getf(void)
{
    return hwp_sysc_awo->syspll_ctrl.bit_field.syspll_cp_ibit;
}

__STATIC_INLINE uint8_t sysc_awo_syspll_en_getf(void)
{
    return hwp_sysc_awo->syspll_ctrl.bit_field.syspll_en;
}

__STATIC_INLINE uint8_t sysc_awo_syspll_lf_ibit_getf(void)
{
    return hwp_sysc_awo->syspll_ctrl.bit_field.syspll_lf_ibit;
}

__STATIC_INLINE uint8_t sysc_awo_syspll_vreg_bit_getf(void)
{
    return hwp_sysc_awo->syspll_ctrl.bit_field.syspll_vreg_bit;
}

__STATIC_INLINE uint8_t sysc_awo_syspll_cpaux_ibit_getf(void)
{
    return hwp_sysc_awo->syspll_ctrl.bit_field.syspll_cpaux_ibit;
}

__STATIC_INLINE uint8_t sysc_awo_syspll_div_getf(void)
{
    return hwp_sysc_awo->syspll_ctrl.bit_field.syspll_div;
}

__STATIC_INLINE uint32_t sysc_awo_syspll_lock_get(void)
{
    return hwp_sysc_awo->syspll_lock.val;
}

__STATIC_INLINE uint8_t sysc_awo_r_syspll_lock_getf(void)
{
    return hwp_sysc_awo->syspll_lock.bit_field.r_syspll_lock;
}

__STATIC_INLINE uint8_t sysc_awo_r_syspll_lock_polarity_getf(void)
{
    return hwp_sysc_awo->syspll_lock.bit_field.r_syspll_lock_polarity;
}

__STATIC_INLINE uint8_t sysc_awo_r_syspll_lock_deb_dly_getf(void)
{
    return hwp_sysc_awo->syspll_lock.bit_field.r_syspll_lock_deb_dly;
}

__STATIC_INLINE uint32_t sysc_awo_slp_cnt_cfg_low_get(void)
{
    return hwp_sysc_awo->slp_cnt_cfg_low.val;
}

__STATIC_INLINE uint32_t sysc_awo_o_cpu_sleep_time_l_getf(void)
{
    return hwp_sysc_awo->slp_cnt_cfg_low.bit_field.o_cpu_sleep_time_l;
}

__STATIC_INLINE uint32_t sysc_awo_slp_cnt_cfg_hi_get(void)
{
    return hwp_sysc_awo->slp_cnt_cfg_hi.val;
}

__STATIC_INLINE uint8_t sysc_awo_o_cpu_sleep_inten_getf(void)
{
    return hwp_sysc_awo->slp_cnt_cfg_hi.bit_field.o_cpu_sleep_inten;
}

__STATIC_INLINE uint8_t sysc_awo_o_cpu_sleep_counter_bp_getf(void)
{
    return hwp_sysc_awo->slp_cnt_cfg_hi.bit_field.o_cpu_sleep_counter_bp;
}

__STATIC_INLINE uint16_t sysc_awo_o_cpu_sleep_time_h_getf(void)
{
    return hwp_sysc_awo->slp_cnt_cfg_hi.bit_field.o_cpu_sleep_time_h;
}

__STATIC_INLINE uint32_t sysc_awo_cal_32k_result_get(void)
{
    return hwp_sysc_awo->cal_32k_result.val;
}

__STATIC_INLINE uint16_t sysc_awo_i_rco32k_period_ns_getf(void)
{
    return hwp_sysc_awo->cal_32k_result.bit_field.i_rco32k_period_ns;
}

__STATIC_INLINE uint32_t sysc_awo_real_slp_time_l_get(void)
{
    return hwp_sysc_awo->real_slp_time_l.val;
}

__STATIC_INLINE uint32_t sysc_awo_i_real_sleep_time_l_getf(void)
{
    return hwp_sysc_awo->real_slp_time_l.bit_field.i_real_sleep_time_l;
}

__STATIC_INLINE uint32_t sysc_awo_real_slp_time_h_get(void)
{
    return hwp_sysc_awo->real_slp_time_h.val;
}

__STATIC_INLINE uint16_t sysc_awo_i_real_sleep_time_h_getf(void)
{
    return hwp_sysc_awo->real_slp_time_h.bit_field.i_real_sleep_time_h;
}

__STATIC_INLINE uint32_t sysc_awo_swd_en0_get(void)
{
    return hwp_sysc_awo->swd_en0.val;
}

__STATIC_INLINE uint8_t sysc_awo_swd_en_getf(void)
{
    return hwp_sysc_awo->swd_en0.bit_field.swd_en;
}

__STATIC_INLINE uint32_t sysc_awo_dbg_hst_en_get(void)
{
    return hwp_sysc_awo->dbg_hst_en.val;
}

__STATIC_INLINE uint8_t sysc_awo_dbg_host_en_getf(void)
{
    return hwp_sysc_awo->dbg_hst_en.bit_field.dbg_host_en;
}

__STATIC_INLINE uint32_t sysc_awo_vset_get(void)
{
    return hwp_sysc_awo->vset.val;
}

__STATIC_INLINE uint8_t sysc_awo_vlimit_vset_awo_getf(void)
{
    return hwp_sysc_awo->vset.bit_field.vlimit_vset_awo;
}

__STATIC_INLINE uint8_t sysc_awo_ldo18_phy_vset_getf(void)
{
    return hwp_sysc_awo->vset.bit_field.ldo18_phy_vset;
}

__STATIC_INLINE uint8_t sysc_awo_ldo15_1_vset_getf(void)
{
    return hwp_sysc_awo->vset.bit_field.ldo15_1_vset;
}

__STATIC_INLINE uint8_t sysc_awo_ldo18_4_vset_getf(void)
{
    return hwp_sysc_awo->vset.bit_field.ldo18_4_vset;
}

__STATIC_INLINE uint8_t sysc_awo_ldo18_3_vset_getf(void)
{
    return hwp_sysc_awo->vset.bit_field.ldo18_3_vset;
}

__STATIC_INLINE uint8_t sysc_awo_ldo18_2_vset_getf(void)
{
    return hwp_sysc_awo->vset.bit_field.ldo18_2_vset;
}

__STATIC_INLINE uint8_t sysc_awo_ldo18_1_vset_getf(void)
{
    return hwp_sysc_awo->vset.bit_field.ldo18_1_vset;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void sysc_awo_div_update_pack(uint8_t pclk1_div_para_up, uint8_t hclk_div_para_up)
{
    hwp_sysc_awo->div_update.val = ( \
                                    ((uint32_t )pclk1_div_para_up << 1) \
                                  | ((uint32_t )hclk_div_para_up) \
                                  );
}

__STATIC_INLINE void sysc_awo_div_para_pack(uint8_t pclk1_div_para_m1, uint8_t hclk_div_para_m1)
{
    hwp_sysc_awo->div_para.val = ( \
                                    ((uint32_t )pclk1_div_para_m1 << 4) \
                                  | ((uint32_t )hclk_div_para_m1) \
                                  );
}

__STATIC_INLINE void sysc_awo_sw_clkg_pack(uint8_t swclkg_mac_80m, uint8_t swclkg_mac_40m, uint8_t swclkg_mac_20m, uint8_t swclkg_phy, uint8_t swclkg_wic, uint8_t swclkg_rtc)
{
    hwp_sysc_awo->sw_clkg.val = ( \
                                    ((uint32_t )swclkg_mac_80m    << 5) \
                                  | ((uint32_t )swclkg_mac_40m    << 4) \
                                  | ((uint32_t )swclkg_mac_20m    << 3) \
                                  | ((uint32_t )swclkg_phy        << 2) \
                                  | ((uint32_t )swclkg_wic        << 1) \
                                  | ((uint32_t )swclkg_rtc) \
                                  );
}

__STATIC_INLINE void sysc_awo_sw_rstn_pack(uint8_t srstn_all, uint8_t srstn_phy, uint8_t srstn_rtc, uint8_t srstn_wic, uint8_t srstn_pmu)
{
    hwp_sysc_awo->sw_rstn.val = ( \
                                    ((uint32_t )srstn_all         << 4) \
                                  | ((uint32_t )srstn_phy         << 3) \
                                  | ((uint32_t )srstn_rtc         << 2) \
                                  | ((uint32_t )srstn_wic         << 1) \
                                  | ((uint32_t )srstn_pmu) \
                                  );
}

__STATIC_INLINE void sysc_awo_pmu_reg_pack(uint8_t r_xtal40m_syspll_fref_en, uint8_t r_xtal40m_cap_bit, uint8_t r_xtal40m_ibsel, uint8_t r_xtal40m_ldo_vbit, uint8_t r_tm2, uint8_t r_tm1, uint8_t bod_vth, uint8_t en_bod, uint8_t r_vtrim)
{
    hwp_sysc_awo->pmu_reg.val = ( \
                                    ((uint32_t )r_xtal40m_syspll_fref_en<< 31) \
                                  | ((uint32_t )r_xtal40m_cap_bit << 24) \
                                  | ((uint32_t )r_xtal40m_ibsel   << 20) \
                                  | ((uint32_t )r_xtal40m_ldo_vbit<< 16) \
                                  | ((uint32_t )r_tm2             << 15) \
                                  | ((uint32_t )r_tm1             << 14) \
                                  | ((uint32_t )bod_vth           << 11) \
                                  | ((uint32_t )en_bod            << 10) \
                                  | ((uint32_t )r_vtrim           << 8) \
                                  );
}

__STATIC_INLINE void sysc_awo_pmu_cfg_pack(uint8_t xtal_pd_en, uint8_t o_digpll_po, uint8_t o_phy_pd_en, uint8_t o_phy_po_en, uint8_t o_sram_pd_en, uint8_t o_sram_po_en, uint8_t o_digldo_retvdd, uint8_t o_digldo_norvdd, uint8_t ldo18_pd_en, uint8_t ldo18_po_en, uint8_t o_cpucore_retreg_po, uint8_t o_pdcmp_po)
{
    hwp_sysc_awo->pmu_cfg.val = ( \
                                    ((uint32_t )xtal_pd_en        << 30) \
                                  | ((uint32_t )o_digpll_po       << 29) \
                                  | ((uint32_t )o_phy_pd_en       << 28) \
                                  | ((uint32_t )o_phy_po_en       << 24) \
                                  | ((uint32_t )o_sram_pd_en      << 20) \
                                  | ((uint32_t )o_sram_po_en      << 16) \
                                  | ((uint32_t )o_digldo_retvdd   << 12) \
                                  | ((uint32_t )o_digldo_norvdd   << 8) \
                                  | ((uint32_t )ldo18_pd_en       << 5) \
                                  | ((uint32_t )ldo18_po_en       << 2) \
                                  | ((uint32_t )o_cpucore_retreg_po<< 1) \
                                  | ((uint32_t )o_pdcmp_po) \
                                  );
}

__STATIC_INLINE void sysc_awo_sw_pwr_ctrl_pack(uint8_t o_sw_pd_start, uint8_t o_sw_po_start)
{
    hwp_sysc_awo->sw_pwr_ctrl.val = ( \
                                    ((uint32_t )o_sw_pd_start     << 1) \
                                  | ((uint32_t )o_sw_po_start) \
                                  );
}

__STATIC_INLINE void sysc_awo_ext_intr_sence_pack(uint8_t ext_inrp_sense_7, uint8_t ext_inrp_sense_6, uint8_t ext_inrp_sense_5, uint8_t ext_inrp_sense_4, uint8_t ext_inrp_sense_3, uint8_t ext_inrp_sense_2, uint8_t ext_inrp_sense_1, uint8_t ext_inrp_sense_0)
{
    hwp_sysc_awo->ext_intr_sence.val = ( \
                                    ((uint32_t )ext_inrp_sense_7  << 14) \
                                  | ((uint32_t )ext_inrp_sense_6  << 12) \
                                  | ((uint32_t )ext_inrp_sense_5  << 10) \
                                  | ((uint32_t )ext_inrp_sense_4  << 8) \
                                  | ((uint32_t )ext_inrp_sense_3  << 6) \
                                  | ((uint32_t )ext_inrp_sense_2  << 4) \
                                  | ((uint32_t )ext_inrp_sense_1  << 2) \
                                  | ((uint32_t )ext_inrp_sense_0) \
                                  );
}

__STATIC_INLINE void sysc_awo_ext_intr_en_pack(uint8_t pin_is_inrp_src, uint8_t ext_inrp_en)
{
    hwp_sysc_awo->ext_intr_en.val = ( \
                                    ((uint32_t )pin_is_inrp_src   << 16) \
                                  | ((uint32_t )ext_inrp_en) \
                                  );
}

__STATIC_INLINE void sysc_awo_swc_time_pack(uint8_t pmu_seq_time, uint8_t o_vdd11_l2h_dly, uint8_t o_vdd11_h2l_dly, uint8_t o_clk_swc_dly, uint8_t o_pll_stb_dly, uint8_t o_xtal40_stb_dly, uint8_t pmu_enable)
{
    hwp_sysc_awo->swc_time.val = ( \
                                    ((uint32_t )pmu_seq_time      << 28) \
                                  | ((uint32_t )o_vdd11_l2h_dly   << 24) \
                                  | ((uint32_t )o_vdd11_h2l_dly   << 20) \
                                  | ((uint32_t )o_clk_swc_dly     << 16) \
                                  | ((uint32_t )o_pll_stb_dly     << 8) \
                                  | ((uint32_t )o_xtal40_stb_dly  << 1) \
                                  | ((uint32_t )pmu_enable) \
                                  );
}

__STATIC_INLINE void sysc_awo_misc_pack(uint8_t lp_mode_awo, uint8_t r_io_retention, uint8_t r_pmu_unret_en, uint8_t r_pmu_ret_en, uint8_t o_cpu_lockup_rst_mask, uint8_t o_wdt_rst_mask, uint8_t o_digldo_debug_mode, uint8_t o_pd_switch_intv, uint8_t o_sdio_rstreq_mask, uint8_t o_cpu_rstreq_mask, uint8_t o_pad_rst_mask, uint8_t o_flash_mode)
{
    hwp_sysc_awo->misc.val = ( \
                                    ((uint32_t )lp_mode_awo       << 17) \
                                  | ((uint32_t )r_io_retention    << 16) \
                                  | ((uint32_t )r_pmu_unret_en    << 15) \
                                  | ((uint32_t )r_pmu_ret_en      << 14) \
                                  | ((uint32_t )o_cpu_lockup_rst_mask<< 13) \
                                  | ((uint32_t )o_wdt_rst_mask    << 12) \
                                  | ((uint32_t )o_digldo_debug_mode<< 11) \
                                  | ((uint32_t )o_pd_switch_intv  << 4) \
                                  | ((uint32_t )o_sdio_rstreq_mask<< 3) \
                                  | ((uint32_t )o_cpu_rstreq_mask << 2) \
                                  | ((uint32_t )o_pad_rst_mask    << 1) \
                                  | ((uint32_t )o_flash_mode) \
                                  );
}

__STATIC_INLINE void sysc_awo_rc32k_ctrl_pack(uint8_t r_rco32k_cbit, uint8_t r_rco32k_bitsel, uint8_t r_rco32k_rbit, uint8_t r_rco32k_ibit)
{
    hwp_sysc_awo->rc32k_ctrl.val = ( \
                                    ((uint32_t )r_rco32k_cbit     << 8) \
                                  | ((uint32_t )r_rco32k_bitsel   << 7) \
                                  | ((uint32_t )r_rco32k_rbit     << 4) \
                                  | ((uint32_t )r_rco32k_ibit) \
                                  );
}

__STATIC_INLINE void sysc_awo_syspll_ctrl_pack(uint8_t syspll_ref2x_en, uint8_t syspll_test_en, uint8_t syspll_sdmclk_test_en, uint8_t syspll_rvco_h_test, uint8_t syspll_rvco_l_test, uint8_t syspll_sdm_clk_sel, uint8_t syspll_rst, uint8_t syspll_cp_ibit, uint8_t syspll_en, uint8_t syspll_lf_ibit, uint8_t syspll_vreg_bit, uint8_t syspll_cpaux_ibit, uint8_t syspll_div)
{
    hwp_sysc_awo->syspll_ctrl.val = ( \
                                    ((uint32_t )syspll_ref2x_en   << 25) \
                                  | ((uint32_t )syspll_test_en    << 24) \
                                  | ((uint32_t )syspll_sdmclk_test_en<< 23) \
                                  | ((uint32_t )syspll_rvco_h_test<< 22) \
                                  | ((uint32_t )syspll_rvco_l_test<< 21) \
                                  | ((uint32_t )syspll_sdm_clk_sel<< 20) \
                                  | ((uint32_t )syspll_rst        << 19) \
                                  | ((uint32_t )syspll_cp_ibit    << 16) \
                                  | ((uint32_t )syspll_en         << 15) \
                                  | ((uint32_t )syspll_lf_ibit    << 12) \
                                  | ((uint32_t )syspll_vreg_bit   << 8) \
                                  | ((uint32_t )syspll_cpaux_ibit << 5) \
                                  | ((uint32_t )syspll_div) \
                                  );
}

__STATIC_INLINE void sysc_awo_syspll_lock_pack(uint8_t r_syspll_lock_polarity, uint8_t r_syspll_lock_deb_dly)
{
    hwp_sysc_awo->syspll_lock.val = ( \
                                    ((uint32_t )r_syspll_lock_polarity<< 8) \
                                  | ((uint32_t )r_syspll_lock_deb_dly) \
                                  );
}

__STATIC_INLINE void sysc_awo_slp_cnt_cfg_hi_pack(uint8_t o_cpu_sleep_inten, uint8_t o_cpu_sleep_counter_bp, uint16_t o_cpu_sleep_time_h)
{
    hwp_sysc_awo->slp_cnt_cfg_hi.val = ( \
                                    ((uint32_t )o_cpu_sleep_inten << 16) \
                                  | ((uint32_t )o_cpu_sleep_counter_bp<< 15) \
                                  | ((uint32_t )o_cpu_sleep_time_h) \
                                  );
}

__STATIC_INLINE void sysc_awo_vset_pack(uint8_t vlimit_vset_awo, uint8_t ldo18_phy_vset, uint8_t ldo15_1_vset, uint8_t ldo18_4_vset, uint8_t ldo18_3_vset, uint8_t ldo18_2_vset, uint8_t ldo18_1_vset)
{
    hwp_sysc_awo->vset.val = ( \
                                    ((uint32_t )vlimit_vset_awo   << 17) \
                                  | ((uint32_t )ldo18_phy_vset    << 15) \
                                  | ((uint32_t )ldo15_1_vset      << 12) \
                                  | ((uint32_t )ldo18_4_vset      << 9) \
                                  | ((uint32_t )ldo18_3_vset      << 6) \
                                  | ((uint32_t )ldo18_2_vset      << 3) \
                                  | ((uint32_t )ldo18_1_vset) \
                                  );
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void sysc_awo_div_para_unpack(uint8_t * pclk1_div_para_m1, uint8_t * hclk_div_para_m1)
{
    t_sysc_awo_div_para local_val = hwp_sysc_awo->div_para;

    *pclk1_div_para_m1  = local_val.bit_field.pclk1_div_para_m1;
    *hclk_div_para_m1   = local_val.bit_field.hclk_div_para_m1;
}

__STATIC_INLINE void sysc_awo_sw_clkg_unpack(uint8_t * swclkg_mac_80m, uint8_t * swclkg_mac_40m, uint8_t * swclkg_mac_20m, uint8_t * swclkg_phy, uint8_t * swclkg_wic, uint8_t * swclkg_rtc)
{
    t_sysc_awo_sw_clkg local_val = hwp_sysc_awo->sw_clkg;

    *swclkg_mac_80m     = local_val.bit_field.swclkg_mac_80m;
    *swclkg_mac_40m     = local_val.bit_field.swclkg_mac_40m;
    *swclkg_mac_20m     = local_val.bit_field.swclkg_mac_20m;
    *swclkg_phy         = local_val.bit_field.swclkg_phy;
    *swclkg_wic         = local_val.bit_field.swclkg_wic;
    *swclkg_rtc         = local_val.bit_field.swclkg_rtc;
}

__STATIC_INLINE void sysc_awo_sw_rstn_unpack(uint8_t * srstn_all, uint8_t * srstn_phy, uint8_t * srstn_rtc, uint8_t * srstn_wic, uint8_t * srstn_pmu)
{
    t_sysc_awo_sw_rstn local_val = hwp_sysc_awo->sw_rstn;

    *srstn_all          = local_val.bit_field.srstn_all;
    *srstn_phy          = local_val.bit_field.srstn_phy;
    *srstn_rtc          = local_val.bit_field.srstn_rtc;
    *srstn_wic          = local_val.bit_field.srstn_wic;
    *srstn_pmu          = local_val.bit_field.srstn_pmu;
}

__STATIC_INLINE void sysc_awo_pmu_reg_unpack(uint8_t * r_xtal40m_syspll_fref_en, uint8_t * r_xtal40m_cap_bit, uint8_t * r_xtal40m_ibsel, uint8_t * r_xtal40m_ldo_vbit, uint8_t * r_tm2, uint8_t * r_tm1, uint8_t * bod_vth, uint8_t * en_bod, uint8_t * r_vtrim)
{
    t_sysc_awo_pmu_reg local_val = hwp_sysc_awo->pmu_reg;

    *r_xtal40m_syspll_fref_en= local_val.bit_field.r_xtal40m_syspll_fref_en;
    *r_xtal40m_cap_bit  = local_val.bit_field.r_xtal40m_cap_bit;
    *r_xtal40m_ibsel    = local_val.bit_field.r_xtal40m_ibsel;
    *r_xtal40m_ldo_vbit = local_val.bit_field.r_xtal40m_ldo_vbit;
    *r_tm2              = local_val.bit_field.r_tm2;
    *r_tm1              = local_val.bit_field.r_tm1;
    *bod_vth            = local_val.bit_field.bod_vth;
    *en_bod             = local_val.bit_field.en_bod;
    *r_vtrim            = local_val.bit_field.r_vtrim;
}

__STATIC_INLINE void sysc_awo_pmu_cfg_unpack(uint8_t * xtal_pd_en, uint8_t * o_digpll_po, uint8_t * o_phy_pd_en, uint8_t * o_phy_po_en, uint8_t * o_sram_pd_en, uint8_t * o_sram_po_en, uint8_t * o_digldo_retvdd, uint8_t * o_digldo_norvdd, uint8_t * ldo18_pd_en, uint8_t * ldo18_po_en, uint8_t * o_cpucore_retreg_po, uint8_t * o_pdcmp_po)
{
    t_sysc_awo_pmu_cfg local_val = hwp_sysc_awo->pmu_cfg;

    *xtal_pd_en         = local_val.bit_field.xtal_pd_en;
    *o_digpll_po        = local_val.bit_field.o_digpll_po;
    *o_phy_pd_en        = local_val.bit_field.o_phy_pd_en;
    *o_phy_po_en        = local_val.bit_field.o_phy_po_en;
    *o_sram_pd_en       = local_val.bit_field.o_sram_pd_en;
    *o_sram_po_en       = local_val.bit_field.o_sram_po_en;
    *o_digldo_retvdd    = local_val.bit_field.o_digldo_retvdd;
    *o_digldo_norvdd    = local_val.bit_field.o_digldo_norvdd;
    *ldo18_pd_en        = local_val.bit_field.ldo18_pd_en;
    *ldo18_po_en        = local_val.bit_field.ldo18_po_en;
    *o_cpucore_retreg_po= local_val.bit_field.o_cpucore_retreg_po;
    *o_pdcmp_po         = local_val.bit_field.o_pdcmp_po;
}

__STATIC_INLINE void sysc_awo_ext_intr_sence_unpack(uint8_t * ext_inrp_sense_7, uint8_t * ext_inrp_sense_6, uint8_t * ext_inrp_sense_5, uint8_t * ext_inrp_sense_4, uint8_t * ext_inrp_sense_3, uint8_t * ext_inrp_sense_2, uint8_t * ext_inrp_sense_1, uint8_t * ext_inrp_sense_0)
{
    t_sysc_awo_ext_intr_sence local_val = hwp_sysc_awo->ext_intr_sence;

    *ext_inrp_sense_7   = local_val.bit_field.ext_inrp_sense_7;
    *ext_inrp_sense_6   = local_val.bit_field.ext_inrp_sense_6;
    *ext_inrp_sense_5   = local_val.bit_field.ext_inrp_sense_5;
    *ext_inrp_sense_4   = local_val.bit_field.ext_inrp_sense_4;
    *ext_inrp_sense_3   = local_val.bit_field.ext_inrp_sense_3;
    *ext_inrp_sense_2   = local_val.bit_field.ext_inrp_sense_2;
    *ext_inrp_sense_1   = local_val.bit_field.ext_inrp_sense_1;
    *ext_inrp_sense_0   = local_val.bit_field.ext_inrp_sense_0;
}

__STATIC_INLINE void sysc_awo_ext_intr_en_unpack(uint8_t * pin_is_inrp_src, uint8_t * ext_inrp_en)
{
    t_sysc_awo_ext_intr_en local_val = hwp_sysc_awo->ext_intr_en;

    *pin_is_inrp_src    = local_val.bit_field.pin_is_inrp_src;
    *ext_inrp_en        = local_val.bit_field.ext_inrp_en;
}

__STATIC_INLINE void sysc_awo_intr_stat_unpack(uint8_t * ext_rawinrp_value, uint8_t * ext_inrp_value_r)
{
    t_sysc_awo_intr_stat local_val = hwp_sysc_awo->intr_stat;

    *ext_rawinrp_value  = local_val.bit_field.ext_rawinrp_value;
    *ext_inrp_value_r   = local_val.bit_field.ext_inrp_value_r;
}

__STATIC_INLINE void sysc_awo_swc_time_unpack(uint8_t * pmu_seq_time, uint8_t * o_vdd11_l2h_dly, uint8_t * o_vdd11_h2l_dly, uint8_t * o_clk_swc_dly, uint8_t * o_pll_stb_dly, uint8_t * o_xtal40_stb_dly, uint8_t * pmu_enable)
{
    t_sysc_awo_swc_time local_val = hwp_sysc_awo->swc_time;

    *pmu_seq_time       = local_val.bit_field.pmu_seq_time;
    *o_vdd11_l2h_dly    = local_val.bit_field.o_vdd11_l2h_dly;
    *o_vdd11_h2l_dly    = local_val.bit_field.o_vdd11_h2l_dly;
    *o_clk_swc_dly      = local_val.bit_field.o_clk_swc_dly;
    *o_pll_stb_dly      = local_val.bit_field.o_pll_stb_dly;
    *o_xtal40_stb_dly   = local_val.bit_field.o_xtal40_stb_dly;
    *pmu_enable         = local_val.bit_field.pmu_enable;
}

__STATIC_INLINE void sysc_awo_misc_unpack(uint8_t * lp_mode_awo, uint8_t * r_io_retention, uint8_t * r_pmu_unret_en, uint8_t * r_pmu_ret_en, uint8_t * o_cpu_lockup_rst_mask, uint8_t * o_wdt_rst_mask, uint8_t * o_digldo_debug_mode, uint8_t * o_pd_switch_intv, uint8_t * o_sdio_rstreq_mask, uint8_t * o_cpu_rstreq_mask, uint8_t * o_pad_rst_mask, uint8_t * o_flash_mode)
{
    t_sysc_awo_misc local_val = hwp_sysc_awo->misc;

    *lp_mode_awo        = local_val.bit_field.lp_mode_awo;
    *r_io_retention     = local_val.bit_field.r_io_retention;
    *r_pmu_unret_en     = local_val.bit_field.r_pmu_unret_en;
    *r_pmu_ret_en       = local_val.bit_field.r_pmu_ret_en;
    *o_cpu_lockup_rst_mask= local_val.bit_field.o_cpu_lockup_rst_mask;
    *o_wdt_rst_mask     = local_val.bit_field.o_wdt_rst_mask;
    *o_digldo_debug_mode= local_val.bit_field.o_digldo_debug_mode;
    *o_pd_switch_intv   = local_val.bit_field.o_pd_switch_intv;
    *o_sdio_rstreq_mask = local_val.bit_field.o_sdio_rstreq_mask;
    *o_cpu_rstreq_mask  = local_val.bit_field.o_cpu_rstreq_mask;
    *o_pad_rst_mask     = local_val.bit_field.o_pad_rst_mask;
    *o_flash_mode       = local_val.bit_field.o_flash_mode;
}

__STATIC_INLINE void sysc_awo_boot_mode_pmu_fsm_unpack(uint8_t * pmu_fsm, uint8_t * i_boot_mode)
{
    t_sysc_awo_boot_mode_pmu_fsm local_val = hwp_sysc_awo->boot_mode_pmu_fsm;

    *pmu_fsm            = local_val.bit_field.pmu_fsm;
    *i_boot_mode        = local_val.bit_field.i_boot_mode;
}

__STATIC_INLINE void sysc_awo_rc32k_ctrl_unpack(uint8_t * r_rco32k_cbit, uint8_t * r_rco32k_bitsel, uint8_t * r_rco32k_rbit, uint8_t * r_rco32k_ibit)
{
    t_sysc_awo_rc32k_ctrl local_val = hwp_sysc_awo->rc32k_ctrl;

    *r_rco32k_cbit      = local_val.bit_field.r_rco32k_cbit;
    *r_rco32k_bitsel    = local_val.bit_field.r_rco32k_bitsel;
    *r_rco32k_rbit      = local_val.bit_field.r_rco32k_rbit;
    *r_rco32k_ibit      = local_val.bit_field.r_rco32k_ibit;
}

__STATIC_INLINE void sysc_awo_syspll_ctrl_unpack(uint8_t * syspll_ref2x_en, uint8_t * syspll_test_en, uint8_t * syspll_sdmclk_test_en, uint8_t * syspll_rvco_h_test, uint8_t * syspll_rvco_l_test, uint8_t * syspll_sdm_clk_sel, uint8_t * syspll_rst, uint8_t * syspll_cp_ibit, uint8_t * syspll_en, uint8_t * syspll_lf_ibit, uint8_t * syspll_vreg_bit, uint8_t * syspll_cpaux_ibit, uint8_t * syspll_div)
{
    t_sysc_awo_syspll_ctrl local_val = hwp_sysc_awo->syspll_ctrl;

    *syspll_ref2x_en    = local_val.bit_field.syspll_ref2x_en;
    *syspll_test_en     = local_val.bit_field.syspll_test_en;
    *syspll_sdmclk_test_en= local_val.bit_field.syspll_sdmclk_test_en;
    *syspll_rvco_h_test = local_val.bit_field.syspll_rvco_h_test;
    *syspll_rvco_l_test = local_val.bit_field.syspll_rvco_l_test;
    *syspll_sdm_clk_sel = local_val.bit_field.syspll_sdm_clk_sel;
    *syspll_rst         = local_val.bit_field.syspll_rst;
    *syspll_cp_ibit     = local_val.bit_field.syspll_cp_ibit;
    *syspll_en          = local_val.bit_field.syspll_en;
    *syspll_lf_ibit     = local_val.bit_field.syspll_lf_ibit;
    *syspll_vreg_bit    = local_val.bit_field.syspll_vreg_bit;
    *syspll_cpaux_ibit  = local_val.bit_field.syspll_cpaux_ibit;
    *syspll_div         = local_val.bit_field.syspll_div;
}

__STATIC_INLINE void sysc_awo_syspll_lock_unpack(uint8_t * r_syspll_lock, uint8_t * r_syspll_lock_polarity, uint8_t * r_syspll_lock_deb_dly)
{
    t_sysc_awo_syspll_lock local_val = hwp_sysc_awo->syspll_lock;

    *r_syspll_lock      = local_val.bit_field.r_syspll_lock;
    *r_syspll_lock_polarity= local_val.bit_field.r_syspll_lock_polarity;
    *r_syspll_lock_deb_dly= local_val.bit_field.r_syspll_lock_deb_dly;
}

__STATIC_INLINE void sysc_awo_slp_cnt_cfg_hi_unpack(uint8_t * o_cpu_sleep_inten, uint8_t * o_cpu_sleep_counter_bp, uint16_t * o_cpu_sleep_time_h)
{
    t_sysc_awo_slp_cnt_cfg_hi local_val = hwp_sysc_awo->slp_cnt_cfg_hi;

    *o_cpu_sleep_inten  = local_val.bit_field.o_cpu_sleep_inten;
    *o_cpu_sleep_counter_bp= local_val.bit_field.o_cpu_sleep_counter_bp;
    *o_cpu_sleep_time_h = local_val.bit_field.o_cpu_sleep_time_h;
}

__STATIC_INLINE void sysc_awo_vset_unpack(uint8_t * vlimit_vset_awo, uint8_t * ldo18_phy_vset, uint8_t * ldo15_1_vset, uint8_t * ldo18_4_vset, uint8_t * ldo18_3_vset, uint8_t * ldo18_2_vset, uint8_t * ldo18_1_vset)
{
    t_sysc_awo_vset local_val = hwp_sysc_awo->vset;

    *vlimit_vset_awo    = local_val.bit_field.vlimit_vset_awo;
    *ldo18_phy_vset     = local_val.bit_field.ldo18_phy_vset;
    *ldo15_1_vset       = local_val.bit_field.ldo15_1_vset;
    *ldo18_4_vset       = local_val.bit_field.ldo18_4_vset;
    *ldo18_3_vset       = local_val.bit_field.ldo18_3_vset;
    *ldo18_2_vset       = local_val.bit_field.ldo18_2_vset;
    *ldo18_1_vset       = local_val.bit_field.ldo18_1_vset;
}

#endif

