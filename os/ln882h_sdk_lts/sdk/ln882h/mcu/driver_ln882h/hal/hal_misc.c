/**
 * @file     hal_misc.c
 * @author   BSP Team 
 * @brief    This file contains the read and write operations of syscon and syscmp registers.
 * @version  0.0.0.1
 * @date     2021-08-17
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "hal/hal_misc.h"

//AWO DIV_UPDATE
void hal_misc_awo_set_pclk1_div_para_up(uint8_t value)
{
    sysc_awo_pclk1_div_para_up_setf(value);
}

void hal_misc_awo_set_hclk_div_para_up(uint8_t value)
{
    sysc_awo_hclk_div_para_up_setf(value);
}

//AWO DIV_PARA
void hal_misc_awo_set_pclk1_div_para_m1(uint8_t value)
{
    sysc_awo_pclk1_div_para_m1_setf(value);
}

void hal_misc_awo_set_hclk_div_para_m1(uint8_t value)
{
    sysc_awo_hclk_div_para_m1_setf(value);
}

uint8_t hal_misc_awo_get_pclk1_div_para_m1(void)
{
    return sysc_awo_pclk1_div_para_m1_getf();
}

uint8_t hal_misc_awo_get_hclk_div_para_m1(void)
{
    return sysc_awo_hclk_div_para_m1_getf();
}

//AWO SW_CLKG
void hal_misc_awo_set_swclkg_mac_80m(uint8_t value)
{
    sysc_awo_swclkg_mac_80m_setf(value);
}

void hal_misc_awo_set_swclkg_mac_40m(uint8_t value)
{
    sysc_awo_swclkg_mac_40m_setf(value);
}

void hal_misc_awo_set_swclkg_mac_20m(uint8_t value)
{
    sysc_awo_swclkg_mac_20m_setf(value);
}

void hal_misc_awo_set_swclkg_phy(uint8_t value)
{
    sysc_awo_swclkg_phy_setf(value);
}

void hal_misc_awo_set_swclkg_wic(uint8_t value)
{
    sysc_awo_swclkg_wic_setf(value);
}

void hal_misc_awo_set_swclkg_rtc(uint8_t value)
{
    sysc_awo_swclkg_rtc_setf(value);
}

uint8_t hal_misc_awo_get_swclkg_mac_80m(void)
{
    return sysc_awo_swclkg_mac_80m_getf();
}

uint8_t hal_misc_awo_get_swclkg_mac_40m(void)
{
    return sysc_awo_swclkg_mac_40m_getf();
}

uint8_t hal_misc_awo_get_swclkg_mac_20m(void)
{
    return sysc_awo_swclkg_mac_20m_getf();
}

uint8_t hal_misc_awo_get_swclkg_phy(void)
{
    return sysc_awo_swclkg_phy_getf();
}

uint8_t hal_misc_awo_get_swclkg_wic(void)
{
    return sysc_awo_swclkg_wic_getf();
}

uint8_t hal_misc_awo_get_swclkg_rtc(void)
{
    return sysc_awo_swclkg_rtc_getf();
}


//AWO SW_RSTn
void hal_misc_awo_set_srstn_all(uint8_t value)
{
    sysc_awo_srstn_all_setf(value);
}

void hal_misc_awo_set_srstn_phy(uint8_t value)
{
    sysc_awo_srstn_phy_setf(value);
}

void hal_misc_awo_set_srstn_rtc(uint8_t value)
{
    sysc_awo_srstn_rtc_setf(value);
}

void hal_misc_awo_set_srstn_wic(uint8_t value)
{
    sysc_awo_srstn_wic_setf(value);
}

void hal_misc_awo_set_srstn_pmu(uint8_t value)
{
    sysc_awo_srstn_pmu_setf(value);
}

uint8_t hal_misc_awo_get_srstn_all(void)
{
    return sysc_awo_srstn_all_getf();
}

uint8_t hal_misc_awo_get_srstn_phy(void)
{
    return sysc_awo_srstn_phy_getf();
}

uint8_t hal_misc_awo_get_srstn_rtc(void)
{
    return sysc_awo_srstn_rtc_getf();
}

uint8_t hal_misc_awo_get_srstn_wic(void)
{
    return sysc_awo_srstn_wic_getf();
}

uint8_t hal_misc_awo_get_srstn_pmu(void)
{
    return sysc_awo_srstn_pmu_getf();
}


//AWO CLK_SEL
void hal_misc_awo_set_clk_src_sel(uint8_t value)
{
    sysc_awo_clk_src_sel_setf(value);
}

uint8_t hal_misc_awo_get_clk_src_sel(void)
{
    return sysc_awo_clk_src_sel_getf();
}


//AWO PMU_REG
void hal_misc_awo_set_r_xtal40m_syspll_fref_en(uint8_t value)
{
    sysc_awo_r_xtal40m_syspll_fref_en_setf(value);
}

void hal_misc_awo_set_r_xtal40m_cap_bit(uint8_t value)
{
    sysc_awo_r_xtal40m_cap_bit_setf(value);
}

void hal_misc_awo_set_r_xtal40m_ibsel(uint8_t value)
{
    sysc_awo_r_xtal40m_ibsel_setf(value);
}

void hal_misc_awo_set_r_xtal40m_ldo_vbit(uint8_t value)
{
    sysc_awo_r_xtal40m_ldo_vbit_setf(value);
}

void hal_misc_awo_set_r_tm2(uint8_t value)
{
    sysc_awo_r_tm2_setf(value);
}

void hal_misc_awo_set_r_tm1(uint8_t value)
{
    sysc_awo_r_tm1_setf(value);
}

void hal_misc_awo_set_bod_vth(uint8_t value)
{
    sysc_awo_bod_vth_setf(value);
}

void hal_misc_awo_set_en_bod(uint8_t value)
{
    sysc_awo_en_bod_setf(value);
}

void hal_misc_awo_set_r_vtrim(uint8_t value)
{
    sysc_awo_r_vtrim_setf(value);
}

uint8_t hal_misc_awo_get_r_xtal40m_syspll_fref_en(void)
{
    return sysc_awo_r_xtal40m_syspll_fref_en_getf();
}

uint8_t hal_misc_awo_get_r_xtal40m_cap_bit(void)
{
    return sysc_awo_r_xtal40m_cap_bit_getf();
}

uint8_t hal_misc_awo_get_r_xtal40m_ibsel(void)
{
    return sysc_awo_r_xtal40m_ibsel_getf();
}

uint8_t hal_misc_awo_get_r_xtal40m_ldo_vbit(void)
{
    return sysc_awo_r_xtal40m_ldo_vbit_getf();
}

uint8_t hal_misc_awo_get_r_tm2(void)
{
    return sysc_awo_r_tm2_getf();
}

uint8_t hal_misc_awo_get_r_tm1(void)
{
    return sysc_awo_r_tm1_getf();
}

uint8_t hal_misc_awo_get_bod_vth(void)
{
    return sysc_awo_bod_vth_getf();
}

uint8_t hal_misc_awo_get_en_bod(void)
{
    return sysc_awo_en_bod_getf();
}

uint8_t hal_misc_awo_get_r_vtrim(void)
{
    return sysc_awo_r_vtrim_getf();
}

//AWO PMU_CFG
void hal_misc_awo_set_o_digpll_po(uint8_t value)
{
    sysc_awo_o_digpll_po_setf(value);
}

void hal_misc_awo_set_o_phy_pd_en(uint8_t value)
{
    sysc_awo_o_phy_pd_en_setf(value);
}

void hal_misc_awo_set_o_phy_po_en(uint8_t value)
{
    sysc_awo_o_phy_po_en_setf(value);
}

void hal_misc_awo_set_o_sram_pd_en(uint8_t value)
{
    sysc_awo_o_sram_pd_en_setf(value);
}

void hal_misc_awo_set_o_sram_po_en(uint8_t value)
{
    sysc_awo_o_sram_po_en_setf(value);
}

void hal_misc_awo_set_o_digldo_retvdd(uint8_t value)
{
    sysc_awo_o_digldo_retvdd_setf(value);
}

void hal_misc_awo_set_o_digldo_norvdd(uint8_t value)
{
    sysc_awo_o_digldo_norvdd_setf(value);
}

void hal_misc_awo_set_ldo18_pd_en(uint8_t value)
{
    sysc_awo_ldo18_pd_en_setf(value);
}

void hal_misc_awo_set_ldo18_po_en(uint8_t value)
{
    sysc_awo_ldo18_po_en_setf(value);
}

void hal_misc_awo_set_o_cpucore_retreg_po(uint8_t value)
{
    sysc_awo_o_cpucore_retreg_po_setf(value);
}

void hal_misc_awo_set_o_pdcmp_po(uint8_t value)
{
    sysc_awo_o_pdcmp_po_setf(value);
}

uint8_t hal_misc_awo_get_o_digpll_po(void)
{
    return sysc_awo_o_digpll_po_getf();
}

uint8_t hal_misc_awo_get_o_phy_pd_en(void)
{
    return sysc_awo_o_phy_pd_en_getf();
}

uint8_t hal_misc_awo_get_o_phy_po_en(void)
{
    return sysc_awo_o_phy_po_en_getf();
}

uint8_t hal_misc_awo_get_o_sram_pd_en(void)
{
    return sysc_awo_o_sram_pd_en_getf();
}

uint8_t hal_misc_awo_get_o_sram_po_en(void)
{
    return sysc_awo_o_sram_po_en_getf();
}

uint8_t hal_misc_awo_get_o_digldo_retvdd(void)
{
    return sysc_awo_o_digldo_retvdd_getf();
}

uint8_t hal_misc_awo_get_o_digldo_norvdd(void)
{
    return sysc_awo_o_digldo_norvdd_getf();
}

uint8_t hal_misc_awo_get_ldo18_pd_en(void)
{
    return sysc_awo_ldo18_pd_en_getf();
}

uint8_t hal_misc_awo_get_ldo18_po_en(void)
{
    return sysc_awo_ldo18_po_en_getf();
}

uint8_t hal_misc_awo_get_o_cpucore_retreg_po(void)
{
    return sysc_awo_o_cpucore_retreg_po_getf();
}

uint8_t hal_misc_awo_get_o_pdcmp_po(void)
{
    return sysc_awo_o_pdcmp_po_getf();
}


//AWO SW_PWR_CTRL
void hal_misc_awo_set_o_sw_pd_start(uint8_t value)
{
    sysc_awo_o_sw_pd_start_setf(value);
}

void hal_misc_awo_set_o_sw_po_start(uint8_t value)
{
    sysc_awo_o_sw_po_start_setf(value);
}

//AWO EXT_INTR_SENCE
void hal_misc_awo_set_ext_inrp_sense_7(uint8_t value)
{
    sysc_awo_ext_inrp_sense_7_setf(value);
}

void hal_misc_awo_set_ext_inrp_sense_6(uint8_t value)
{
    sysc_awo_ext_inrp_sense_6_setf(value);
}

void hal_misc_awo_set_ext_inrp_sense_5(uint8_t value)
{
    sysc_awo_ext_inrp_sense_5_setf(value);
}

void hal_misc_awo_set_ext_inrp_sense_4(uint8_t value)
{
    sysc_awo_ext_inrp_sense_4_setf(value);
}

void hal_misc_awo_set_ext_inrp_sense_3(uint8_t value)
{
    sysc_awo_ext_inrp_sense_3_setf(value);
}

void hal_misc_awo_set_ext_inrp_sense_2(uint8_t value)
{
    sysc_awo_ext_inrp_sense_2_setf(value);
}

void hal_misc_awo_set_ext_inrp_sense_1(uint8_t value)
{
    sysc_awo_ext_inrp_sense_1_setf(value);
}

void hal_misc_awo_set_ext_inrp_sense_0(uint8_t value)
{
    sysc_awo_ext_inrp_sense_0_setf(value);
}

uint8_t hal_misc_awo_get_ext_inrp_sense_7(void)
{
    return sysc_awo_ext_inrp_sense_7_getf();
}

uint8_t hal_misc_awo_get_ext_inrp_sense_6(void)
{
    return sysc_awo_ext_inrp_sense_6_getf();
}

uint8_t hal_misc_awo_get_ext_inrp_sense_5(void)
{
    return sysc_awo_ext_inrp_sense_5_getf();
}

uint8_t hal_misc_awo_get_ext_inrp_sense_4(void)
{
    return sysc_awo_ext_inrp_sense_4_getf();
}

uint8_t hal_misc_awo_get_ext_inrp_sense_3(void)
{
    return sysc_awo_ext_inrp_sense_3_getf();
}

uint8_t hal_misc_awo_get_ext_inrp_sense_2(void)
{
    return sysc_awo_ext_inrp_sense_2_getf();
}

uint8_t hal_misc_awo_get_ext_inrp_sense_1(void)
{
    return sysc_awo_ext_inrp_sense_1_getf();
}

uint8_t hal_misc_awo_get_ext_inrp_sense_0(void)
{
    return sysc_awo_ext_inrp_sense_0_getf();
}

//AWO EXT_INTR_EN
void hal_misc_awo_set_pin_is_inrp_src(uint8_t value)
{
    sysc_awo_pin_is_inrp_src_setf(value);
}

void hal_misc_awo_set_ext_inrp_en(uint8_t value)
{
    sysc_awo_ext_inrp_en_setf(value);
}

uint8_t hal_misc_awo_get_pin_is_inrp_src(void)
{
    return sysc_awo_pin_is_inrp_src_getf();
}

uint8_t hal_misc_awo_get_ext_inrp_en(void)
{
    return sysc_awo_ext_inrp_en_getf();
}

//AWO INTR_STAT
uint8_t hal_misc_awo_get_ext_rawinrp_value(void)
{
    return sysc_awo_ext_rawinrp_value_getf();
}

uint8_t hal_misc_awo_get_ext_inrp_value_r(void)
{
    return sysc_awo_ext_inrp_value_r_getf();
}

//AWO EXT_INRP_CLR
void hal_misc_awo_set_ext_inrp_clr_r(uint8_t value)
{
    sysc_awo_ext_inrp_clr_r_setf(value);
}

//AWO SWC_TIME
void hal_misc_awo_set_pmu_seq_time(uint8_t value)
{
    sysc_awo_pmu_seq_time_setf(value);
}

void hal_misc_awo_set_o_vdd11_l2h_dly(uint8_t value)
{
    sysc_awo_o_vdd11_l2h_dly_setf(value);
}

void hal_misc_awo_set_o_vdd11_h2l_dly(uint8_t value)
{
    sysc_awo_o_vdd11_h2l_dly_setf(value);
}

void hal_misc_awo_set_o_clk_swc_dly(uint8_t value)
{
    sysc_awo_o_clk_swc_dly_setf(value);
}

void hal_misc_awo_set_o_pll_stb_dly(uint8_t value)
{
    sysc_awo_o_pll_stb_dly_setf(value);
}

void hal_misc_awo_set_o_xtal40_stb_dly(uint8_t value)
{
    sysc_awo_o_xtal40_stb_dly_setf(value);
}

void hal_misc_awo_set_pmu_enable(uint8_t value)
{
    sysc_awo_pmu_enable_setf(value);
}

uint8_t hal_misc_awo_get_pmu_seq_time(void)
{
    return sysc_awo_pmu_seq_time_getf();
}

uint8_t hal_misc_awo_get_o_vdd11_l2h_dly(void)
{
    return sysc_awo_o_vdd11_l2h_dly_getf();
}

uint8_t hal_misc_awo_get_o_vdd11_h2l_dly(void)
{
    return sysc_awo_o_vdd11_h2l_dly_getf();
}

uint8_t hal_misc_awo_get_o_clk_swc_dly(void)
{
    return sysc_awo_o_clk_swc_dly_getf();
}

uint8_t hal_misc_awo_get_o_pll_stb_dly(void)
{
    return sysc_awo_o_pll_stb_dly_getf();
}

uint8_t hal_misc_awo_get_o_xtal40_stb_dly(void)
{
    return sysc_awo_o_xtal40_stb_dly_getf();
}

uint8_t hal_misc_awo_get_pmu_enable(void)
{
    return sysc_awo_pmu_enable_getf();
}

//AWO MISC
void hal_misc_awo_set_lp_mode_awo(uint8_t value)
{
    sysc_awo_lp_mode_awo_setf(value);
}

void hal_misc_awo_set_r_io_retention(uint8_t value)
{
    sysc_awo_r_io_retention_setf(value);
}

void hal_misc_awo_set_r_pmu_unret_en(uint8_t value)
{
    sysc_awo_r_pmu_unret_en_setf(value);
}

void hal_misc_awo_set_r_pmu_ret_en(uint8_t value)
{
    sysc_awo_r_pmu_ret_en_setf(value);
}

void hal_misc_awo_set_o_cpu_lockup_rst_mask(uint8_t value)
{
    sysc_awo_o_cpu_lockup_rst_mask_setf(value);
}

void hal_misc_awo_set_o_wdt_rst_mask(uint8_t value)
{
    sysc_awo_o_wdt_rst_mask_setf(value);
}

void hal_misc_awo_set_o_digldo_debug_mode(uint8_t value)
{
    sysc_awo_o_digldo_debug_mode_setf(value);
}

void hal_misc_awo_set_o_pd_switch_intv(uint8_t value)
{
    sysc_awo_o_pd_switch_intv_setf(value);
}

void hal_misc_awo_set_o_sdio_rstreq_mask(uint8_t value)
{
    sysc_awo_o_sdio_rstreq_mask_setf(value);
}

void hal_misc_awo_set_o_cpu_rstreq_mask(uint8_t value)
{
    sysc_awo_o_cpu_rstreq_mask_setf(value);
}

void hal_misc_awo_set_o_pad_rst_mask(uint8_t value)
{
    sysc_awo_o_pad_rst_mask_setf(value);
}

void hal_misc_awo_set_o_flash_mode(uint8_t value)
{
    sysc_awo_o_flash_mode_setf(value);
}

uint8_t hal_misc_awo_get_lp_mode_awo(void)
{
    return sysc_awo_lp_mode_awo_getf();
}

uint8_t hal_misc_awo_get_r_io_retention(void)
{
    return sysc_awo_r_io_retention_getf();
}

uint8_t hal_misc_awo_get_r_pmu_unret_en(void)
{
    return sysc_awo_r_pmu_unret_en_getf();
}

uint8_t hal_misc_awo_get_r_pmu_ret_en(void)
{
    return sysc_awo_r_pmu_ret_en_getf();
}

uint8_t hal_misc_awo_get_o_cpu_lockup_rst_mask(void)
{
    return sysc_awo_o_cpu_lockup_rst_mask_getf();
}

uint8_t hal_misc_awo_get_o_wdt_rst_mask(void)
{
    return sysc_awo_o_wdt_rst_mask_getf();
}

uint8_t hal_misc_awo_get_o_digldo_debug_mode(void)
{
    return sysc_awo_o_digldo_debug_mode_getf();
}

uint8_t hal_misc_awo_get_o_pd_switch_intv(void)
{
    return sysc_awo_o_pd_switch_intv_getf();
}

uint8_t hal_misc_awo_get_o_sdio_rstreq_mask(void)
{
    return sysc_awo_o_sdio_rstreq_mask_getf();
}

uint8_t hal_misc_awo_get_o_cpu_rstreq_mask(void)
{
    return sysc_awo_o_cpu_rstreq_mask_getf();
}

uint8_t hal_misc_awo_get_o_pad_rst_mask(void)
{
    return sysc_awo_o_pad_rst_mask_getf();
}

uint8_t hal_misc_awo_get_o_flash_mode(void)
{
    return sysc_awo_o_flash_mode_getf();
}

//AWO IDLE_REG
void hal_misc_awo_set_r_idle_reg(uint32_t value)
{
    sysc_awo_r_idle_reg_setf(value);
}

uint32_t hal_misc_awo_get_r_idle_reg(void)
{
    return sysc_awo_r_idle_reg_getf();
}

//AWO BOOT_MODE_PMU_FSM
uint8_t hal_misc_awo_get_pmu_fsm(void)
{
    return sysc_awo_pmu_fsm_getf();
}

uint8_t hal_misc_awo_get_i_boot_mode(void)
{
    return sysc_awo_i_boot_mode_getf();
}

//AWO RC32K_CTRL
void hal_misc_awo_set_r_rco32k_cbit(uint8_t value)
{
    sysc_awo_r_rco32k_cbit_setf(value);
}

void hal_misc_awo_set_r_rco32k_bitsel(uint8_t value)
{
    sysc_awo_r_rco32k_bitsel_setf(value);
}

void hal_misc_awo_set_r_rco32k_rbit(uint8_t value)
{
    sysc_awo_r_rco32k_rbit_setf(value);
}

void hal_misc_awo_set_r_rco32k_ibit(uint8_t value)
{
    sysc_awo_r_rco32k_ibit_setf(value);
}

uint8_t hal_misc_awo_get_r_rco32k_cbit(void)
{
    return sysc_awo_r_rco32k_cbit_getf();
}

uint8_t hal_misc_awo_get_r_rco32k_bitsel(void)
{
    return sysc_awo_r_rco32k_bitsel_getf();
}

uint8_t hal_misc_awo_get_r_rco32k_rbit(void)
{
    return sysc_awo_r_rco32k_rbit_getf();
}

uint8_t hal_misc_awo_get_r_rco32k_ibit(void)
{
    return sysc_awo_r_rco32k_ibit_getf();
}

//AWO SYSPLL_CTRL
void hal_misc_awo_set_syspll_ref2x_en(uint8_t value)
{
    sysc_awo_syspll_ref2x_en_setf(value);
}

void hal_misc_awo_set_syspll_test_en(uint8_t value)
{
    sysc_awo_syspll_test_en_setf(value);
}

void hal_misc_awo_set_syspll_sdmclk_test_en(uint8_t value)
{
    sysc_awo_syspll_sdmclk_test_en_setf(value);
}

void hal_misc_awo_set_syspll_rvco_h_test(uint8_t value)
{
    sysc_awo_syspll_rvco_h_test_setf(value);
}

void hal_misc_awo_set_syspll_rvco_l_test(uint8_t value)
{
    sysc_awo_syspll_rvco_l_test_setf(value);
}

void hal_misc_awo_set_syspll_sdm_clk_sel(uint8_t value)
{
    sysc_awo_syspll_sdm_clk_sel_setf(value);
}

void hal_misc_awo_set_syspll_rst(uint8_t value)
{
    sysc_awo_syspll_rst_setf(value);
}

void hal_misc_awo_set_syspll_cp_ibit(uint8_t value)
{
    sysc_awo_syspll_cp_ibit_setf(value);
}

void hal_misc_awo_set_syspll_en(uint8_t value)
{
    sysc_awo_syspll_en_setf(value);
}

void hal_misc_awo_set_syspll_lf_ibit(uint8_t value)
{
    sysc_awo_syspll_lf_ibit_setf(value);
}

void hal_misc_awo_set_syspll_vreg_bit(uint8_t value)
{
    sysc_awo_syspll_vreg_bit_setf(value);
}

void hal_misc_awo_set_syspll_cpaux_ibit(uint8_t value)
{
    sysc_awo_syspll_cpaux_ibit_setf(value);
}

void hal_misc_awo_set_syspll_div(uint8_t value)
{
    sysc_awo_syspll_div_setf(value);
}

uint8_t hal_misc_awo_get_syspll_ref2x_en(void)
{
    return sysc_awo_syspll_ref2x_en_getf();
}

uint8_t hal_misc_awo_get_syspll_test_en(void)
{
    return sysc_awo_syspll_test_en_getf();
}

uint8_t hal_misc_awo_get_syspll_sdmclk_test_en(void)
{
    return sysc_awo_syspll_sdmclk_test_en_getf();
}

uint8_t hal_misc_awo_get_syspll_rvco_h_test(void)
{
    return sysc_awo_syspll_rvco_h_test_getf();
}

uint8_t hal_misc_awo_get_syspll_rvco_l_test(void)
{
    return sysc_awo_syspll_rvco_l_test_getf();
}

uint8_t hal_misc_awo_get_syspll_sdm_clk_sel(void)
{
    return sysc_awo_syspll_sdm_clk_sel_getf();
}

uint8_t hal_misc_awo_get_syspll_rst(void)
{
    return sysc_awo_syspll_rst_getf();
}

uint8_t hal_misc_awo_get_syspll_cp_ibit(void)
{
    return sysc_awo_syspll_cp_ibit_getf();
}

uint8_t hal_misc_awo_get_syspll_en(void)
{
    return sysc_awo_syspll_en_getf();
}

uint8_t hal_misc_awo_get_syspll_lf_ibit(void)
{
    return sysc_awo_syspll_lf_ibit_getf();
}

uint8_t hal_misc_awo_get_syspll_vreg_bit(void)
{
    return sysc_awo_syspll_vreg_bit_getf();
}

uint8_t hal_misc_awo_get_syspll_cpaux_ibit(void)
{
    return sysc_awo_syspll_cpaux_ibit_getf();
}

uint8_t hal_misc_awo_get_syspll_div(void)
{
    return sysc_awo_syspll_div_getf();
}

//AWO SYSPLL_LOCK
void hal_misc_awo_set_r_syspll_lock_polarity(uint8_t value)
{
    sysc_awo_r_syspll_lock_polarity_setf(value);
}

void hal_misc_awo_set_r_syspll_lock_deb_dly(uint8_t value)
{
    sysc_awo_r_syspll_lock_deb_dly_setf(value);
}

uint8_t hal_misc_awo_get_r_syspll_lock(void)
{
    return sysc_awo_r_syspll_lock_getf();
}

uint8_t hal_misc_awo_get_r_syspll_lock_polarity(void)
{
    return sysc_awo_r_syspll_lock_polarity_getf();
}

uint8_t hal_misc_awo_get_r_syspll_lock_deb_dly(void)
{
    return sysc_awo_r_syspll_lock_deb_dly_getf();
}

//AWO SLP_CNT_CFG_LOW
void hal_misc_awo_set_o_cpu_sleep_time_l(uint32_t value)
{
    sysc_awo_o_cpu_sleep_time_l_setf(value);
}

uint32_t hal_misc_awo_get_o_cpu_sleep_time_l(void)
{
    return sysc_awo_o_cpu_sleep_time_l_getf();
}


//AWO SLP_CNT_CFG_HI
void hal_misc_awo_set_o_cpu_sleep_inten(uint8_t value)
{
    sysc_awo_o_cpu_sleep_inten_setf(value);
}

void hal_misc_awo_set_o_cpu_sleep_counter_bp(uint8_t value)
{
    sysc_awo_o_cpu_sleep_counter_bp_setf(value);
}

void hal_misc_awo_set_o_cpu_sleep_time_h(uint16_t value)
{
    sysc_awo_o_cpu_sleep_time_h_setf(value);
}

uint8_t hal_misc_awo_get_o_cpu_sleep_inten(void)
{
    return sysc_awo_o_cpu_sleep_inten_getf();
}

uint8_t hal_misc_awo_get_o_cpu_sleep_counter_bp(void)
{
    return sysc_awo_o_cpu_sleep_counter_bp_getf();
}

uint16_t hal_misc_awo_get_o_cpu_sleep_time_h(void)
{
    return sysc_awo_o_cpu_sleep_time_h_getf();
}


//AWO CAL_32K_RESULT
uint16_t hal_misc_awo_get_i_rco32k_period_ns(void)
{
    return sysc_awo_i_rco32k_period_ns_getf();
}

//AWO REAL_SLP_TIME_L
uint32_t hal_misc_awo_get_i_real_sleep_time_l(void)
{
    return sysc_awo_i_real_sleep_time_l_getf();
}

//AWO REAL_SLP_TIME_H
uint16_t hal_misc_awo_get_i_real_sleep_time_h(void)
{
    return sysc_awo_i_real_sleep_time_h_getf();
}

//AWO SWD_EN0
void hal_misc_awo_set_swd_en(uint8_t value)
{
    sysc_awo_swd_en_setf(value);
}

uint8_t hal_misc_awo_get_swd_en(void)
{
    return sysc_awo_swd_en_getf();
}

//AWO DBG_HST_EN
void hal_misc_awo_set_dbg_host_en(uint8_t value)
{
    sysc_awo_dbg_host_en_setf(value);
}

uint8_t hal_misc_awo_get_dbg_host_en(void)
{
    return sysc_awo_dbg_host_en_getf();
}


//AWO VSET
void hal_misc_awo_set_vlimit_vset_awo(uint8_t value)
{
    sysc_awo_vlimit_vset_awo_setf(value);
}

void hal_misc_awo_set_ldo18_phy_vset(uint8_t value)
{
    sysc_awo_ldo18_phy_vset_setf(value);
}

void hal_misc_awo_set_ldo15_1_vset(uint8_t value)
{
    sysc_awo_ldo15_1_vset_setf(value);
}

void hal_misc_awo_set_ldo18_4_vset(uint8_t value)
{
    sysc_awo_ldo18_4_vset_setf(value);
}

void hal_misc_awo_set_ldo18_3_vset(uint8_t value)
{
    sysc_awo_ldo18_3_vset_setf(value);
}

void hal_misc_awo_set_ldo18_2_vset(uint8_t value)
{
    sysc_awo_ldo18_2_vset_setf(value);
}

void hal_misc_awo_set_ldo18_1_vset(uint8_t value)
{
    sysc_awo_ldo18_1_vset_setf(value);
}

uint8_t hal_misc_awo_get_vlimit_vset_awo(void)
{
    return sysc_awo_vlimit_vset_awo_getf();
}

uint8_t hal_misc_awo_get_ldo18_phy_vset(void)
{
    return sysc_awo_ldo18_phy_vset_getf();
}

uint8_t hal_misc_awo_get_ldo15_1_vset(void)
{
    return sysc_awo_ldo15_1_vset_getf();
}

uint8_t hal_misc_awo_get_ldo18_4_vset(void)
{
    return sysc_awo_ldo18_4_vset_getf();
}

uint8_t hal_misc_awo_get_ldo18_3_vset(void)
{
    return sysc_awo_ldo18_3_vset_getf();
}

uint8_t hal_misc_awo_get_ldo18_2_vset(void)
{
    return sysc_awo_ldo18_2_vset_getf();
}

uint8_t hal_misc_awo_get_ldo18_1_vset(void)
{
    return sysc_awo_ldo18_1_vset_getf();
}

//CMP DIV_UPDATE
void hal_misc_cmp_set_ble_div_para_up(uint8_t value)
{
    sysc_cmp_ble_div_para_up_setf(value);
}

void hal_misc_cmp_set_rfintf_div_para_up(uint8_t value)
{
    sysc_cmp_rfintf_div_para_up_setf(value);
}

void hal_misc_cmp_set_timer4_div_para_up(uint8_t value)
{
    sysc_cmp_timer4_div_para_up_setf(value);
}

void hal_misc_cmp_set_timer3_div_para_up(uint8_t value)
{
    sysc_cmp_timer3_div_para_up_setf(value);
}

void hal_misc_cmp_set_timer2_div_para_up(uint8_t value)
{
    sysc_cmp_timer2_div_para_up_setf(value);
}

void hal_misc_cmp_set_timer1_div_para_up(uint8_t value)
{
    sysc_cmp_timer1_div_para_up_setf(value);
}

void hal_misc_cmp_set_pclk0_div_para_up(uint8_t value)
{
    sysc_cmp_pclk0_div_para_up_setf(value);
}

//CMP DIV_PARA
void hal_misc_cmp_set_ble_div_para_m1(uint8_t value)
{
    sysc_cmp_ble_div_para_m1_setf(value);
}

void hal_misc_cmp_set_rfintf_div_para_m1(uint8_t value)
{
    sysc_cmp_rfintf_div_para_m1_setf(value);
}

void hal_misc_cmp_set_pclk0_div_para_m1(uint8_t value)
{
    sysc_cmp_pclk0_div_para_m1_setf(value);
}

uint8_t hal_misc_cmp_get_ble_div_para_m1(void)
{
    return sysc_cmp_ble_div_para_m1_getf();
}

uint8_t hal_misc_cmp_get_rfintf_div_para_m1(void)
{
    return sysc_cmp_rfintf_div_para_m1_getf();
}

uint8_t hal_misc_cmp_get_pclk0_div_para_m1(void)
{
    return sysc_cmp_pclk0_div_para_m1_getf();
}

//CMP DIV_PARA_TIMER_0
void hal_misc_cmp_set_timer4_div_para_m1(uint8_t value)
{
    sysc_cmp_timer4_div_para_m1_setf(value);
}

void hal_misc_cmp_set_timer3_div_para_m1(uint8_t value)
{
    sysc_cmp_timer3_div_para_m1_setf(value);
}

void hal_misc_cmp_set_timer2_div_para_m1(uint8_t value)
{
    sysc_cmp_timer2_div_para_m1_setf(value);
}

void hal_misc_cmp_set_timer1_div_para_m1(uint8_t value)
{
    sysc_cmp_timer1_div_para_m1_setf(value);
}

uint8_t hal_misc_cmp_get_timer4_div_para_m1(void)
{
    return sysc_cmp_timer4_div_para_m1_getf();
}

uint8_t hal_misc_cmp_get_timer3_div_para_m1(void)
{
    return sysc_cmp_timer3_div_para_m1_getf();
}

uint8_t hal_misc_cmp_get_timer2_div_para_m1(void)
{
    return sysc_cmp_timer2_div_para_m1_getf();
}

uint8_t hal_misc_cmp_get_timer1_div_para_m1(void)
{
    return sysc_cmp_timer1_div_para_m1_getf();
}

//CMP SW_CLKG
void hal_misc_cmp_set_aes_gate_en(uint8_t value)
{
    sysc_cmp_aes_gate_en_setf(value);
}

void hal_misc_cmp_set_trng_gate_en(uint8_t value)
{
    sysc_cmp_trng_gate_en_setf(value);
}

void hal_misc_cmp_set_ef_gate_en(uint8_t value)
{
    sysc_cmp_ef_gate_en_setf(value);
}

void hal_misc_cmp_set_pwm_gate_en(uint8_t value)
{
    sysc_cmp_pwm_gate_en_setf(value);
}

void hal_misc_cmp_set_rfreg_gate_en(uint8_t value)
{
    sysc_cmp_rfreg_gate_en_setf(value);
}

void hal_misc_cmp_set_dma_gate_en(uint8_t value)
{
    sysc_cmp_dma_gate_en_setf(value);
}

void hal_misc_cmp_set_cache_gate_en(uint8_t value)
{
    sysc_cmp_cache_gate_en_setf(value);
}

void hal_misc_cmp_set_mac_gate_en(uint8_t value)
{
    sysc_cmp_mac_gate_en_setf(value);
}

void hal_misc_cmp_set_sdio_gate_en(uint8_t value)
{
    sysc_cmp_sdio_gate_en_setf(value);
}

void hal_misc_cmp_set_dbgh_gate_en(uint8_t value)
{
    sysc_cmp_dbgh_gate_en_setf(value);
}

void hal_misc_cmp_set_timer4_gate_en(uint8_t value)
{
    sysc_cmp_timer4_gate_en_setf(value);
}

void hal_misc_cmp_set_timer3_gate_en(uint8_t value)
{
    sysc_cmp_timer3_gate_en_setf(value);
}

void hal_misc_cmp_set_timer2_gate_en(uint8_t value)
{
    sysc_cmp_timer2_gate_en_setf(value);
}

void hal_misc_cmp_set_timer1_gate_en(uint8_t value)
{
    sysc_cmp_timer1_gate_en_setf(value);
}

void hal_misc_cmp_set_timer_gate_en(uint8_t value)
{
    sysc_cmp_timer_gate_en_setf(value);
}

void hal_misc_cmp_set_wdt_gate_en(uint8_t value)
{
    sysc_cmp_wdt_gate_en_setf(value);
}

void hal_misc_cmp_set_uart2_gate_en(uint8_t value)
{
    sysc_cmp_uart2_gate_en_setf(value);
}

void hal_misc_cmp_set_uart1_gate_en(uint8_t value)
{
    sysc_cmp_uart1_gate_en_setf(value);
}

void hal_misc_cmp_set_uart0_gate_en(uint8_t value)
{
    sysc_cmp_uart0_gate_en_setf(value);
}

void hal_misc_cmp_set_i2c0_gate_en(uint8_t value)
{
    sysc_cmp_i2c0_gate_en_setf(value);
}

void hal_misc_cmp_set_ws2811_gate_en(uint8_t value)
{
    sysc_cmp_ws2811_gate_en_setf(value);
}

void hal_misc_cmp_set_spi1_gate_en(uint8_t value)
{
    sysc_cmp_spi1_gate_en_setf(value);
}

void hal_misc_cmp_set_spi0_gate_en(uint8_t value)
{
    sysc_cmp_spi0_gate_en_setf(value);
}

void hal_misc_cmp_set_gpiob_gate_en(uint8_t value)
{
    sysc_cmp_gpiob_gate_en_setf(value);
}

void hal_misc_cmp_set_gpioa_gate_en(uint8_t value)
{
    sysc_cmp_gpioa_gate_en_setf(value);
}

void hal_misc_cmp_set_i2s_gate_en(uint8_t value)
{
    sysc_cmp_i2s_gate_en_setf(value);
}

void hal_misc_cmp_set_adcc_gate_en(uint8_t value)
{
    sysc_cmp_adcc_gate_en_setf(value);
}

void hal_misc_cmp_set_qspi_gate_en(uint8_t value)
{
    sysc_cmp_qspi_gate_en_setf(value);
}

uint8_t hal_misc_cmp_get_aes_gate_en(void)
{
    return sysc_cmp_aes_gate_en_getf();
}

uint8_t hal_misc_cmp_get_trng_gate_en(void)
{
    return sysc_cmp_trng_gate_en_getf();
}

uint8_t hal_misc_cmp_get_ef_gate_en(void)
{
    return sysc_cmp_ef_gate_en_getf();
}

uint8_t hal_misc_cmp_get_pwm_gate_en(void)
{
    return sysc_cmp_pwm_gate_en_getf();
}

uint8_t hal_misc_cmp_get_rfreg_gate_en(void)
{
    return sysc_cmp_rfreg_gate_en_getf();
}

uint8_t hal_misc_cmp_get_dma_gate_en(void)
{
    return sysc_cmp_dma_gate_en_getf();
}

uint8_t hal_misc_cmp_get_cache_gate_en(void)
{
    return sysc_cmp_cache_gate_en_getf();
}

uint8_t hal_misc_cmp_get_mac_gate_en(void)
{
    return sysc_cmp_mac_gate_en_getf();
}

uint8_t hal_misc_cmp_get_sdio_gate_en(void)
{
    return sysc_cmp_sdio_gate_en_getf();
}

uint8_t hal_misc_cmp_get_dbgh_gate_en(void)
{
    return sysc_cmp_dbgh_gate_en_getf();
}

uint8_t hal_misc_cmp_get_timer4_gate_en(void)
{
    return sysc_cmp_timer4_gate_en_getf();
}

uint8_t hal_misc_cmp_get_timer3_gate_en(void)
{
    return sysc_cmp_timer3_gate_en_getf();
}

uint8_t hal_misc_cmp_get_timer2_gate_en(void)
{
    return sysc_cmp_timer2_gate_en_getf();
}

uint8_t hal_misc_cmp_get_timer1_gate_en(void)
{
    return sysc_cmp_timer1_gate_en_getf();
}

uint8_t hal_misc_cmp_get_timer_gate_en(void)
{
    return sysc_cmp_timer_gate_en_getf();
}

uint8_t hal_misc_cmp_get_wdt_gate_en(void)
{
    return sysc_cmp_wdt_gate_en_getf();
}

uint8_t hal_misc_cmp_get_uart2_gate_en(void)
{
    return sysc_cmp_uart2_gate_en_getf();
}

uint8_t hal_misc_cmp_get_uart1_gate_en(void)
{
    return sysc_cmp_uart1_gate_en_getf();
}

uint8_t hal_misc_cmp_get_uart0_gate_en(void)
{
    return sysc_cmp_uart0_gate_en_getf();
}

uint8_t hal_misc_cmp_get_i2c0_gate_en(void)
{
    return sysc_cmp_i2c0_gate_en_getf();
}

uint8_t hal_misc_cmp_get_ws2811_gate_en(void)
{
    return sysc_cmp_ws2811_gate_en_getf();
}

uint8_t hal_misc_cmp_get_spi1_gate_en(void)
{
    return sysc_cmp_spi1_gate_en_getf();
}

uint8_t hal_misc_cmp_get_spi0_gate_en(void)
{
    return sysc_cmp_spi0_gate_en_getf();
}

uint8_t hal_misc_cmp_get_gpiob_gate_en(void)
{
    return sysc_cmp_gpiob_gate_en_getf();
}

uint8_t hal_misc_cmp_get_gpioa_gate_en(void)
{
    return sysc_cmp_gpioa_gate_en_getf();
}

uint8_t hal_misc_cmp_get_i2s_gate_en(void)
{
    return sysc_cmp_i2s_gate_en_getf();
}

uint8_t hal_misc_cmp_get_adcc_gate_en(void)
{
    return sysc_cmp_adcc_gate_en_getf();
}

uint8_t hal_misc_cmp_get_qspi_gate_en(void)
{
    return sysc_cmp_qspi_gate_en_getf();
}

uint8_t hal_misc_cmp_get_ble_gate_en(void)
{
    return sysc_cmp_ble_gate_en_getf();
}


//CMP SW_RSTn
void hal_misc_cmp_set_srstn_aes(uint8_t value)
{
    sysc_cmp_srstn_aes_setf(value);
}

void hal_misc_cmp_set_srstn_trng(uint8_t value)
{
    sysc_cmp_srstn_trng_setf(value);
}

void hal_misc_cmp_set_srstn_ef(uint8_t value)
{
    sysc_cmp_srstn_ef_setf(value);
}

void hal_misc_cmp_set_srstn_pwm(uint8_t value)
{
    sysc_cmp_srstn_pwm_setf(value);
}

void hal_misc_cmp_set_srstn_rfreg(uint8_t value)
{
    sysc_cmp_srstn_rfreg_setf(value);
}

void hal_misc_cmp_set_srstn_dma(uint8_t value)
{
    sysc_cmp_srstn_dma_setf(value);
}

void hal_misc_cmp_set_srstn_cache(uint8_t value)
{
    sysc_cmp_srstn_cache_setf(value);
}

void hal_misc_cmp_set_srstn_mac(uint8_t value)
{
    sysc_cmp_srstn_mac_setf(value);
}

void hal_misc_cmp_set_srstn_sdio(uint8_t value)
{
    sysc_cmp_srstn_sdio_setf(value);
}

void hal_misc_cmp_set_srstn_dbgh(uint8_t value)
{
    sysc_cmp_srstn_dbgh_setf(value);
}

void hal_misc_cmp_set_srstn_timer(uint8_t value)
{
    sysc_cmp_srstn_timer_setf(value);
}

void hal_misc_cmp_set_srstn_wdt(uint8_t value)
{
    sysc_cmp_srstn_wdt_setf(value);
}

void hal_misc_cmp_set_srstn_uart2(uint8_t value)
{
    sysc_cmp_srstn_uart2_setf(value);
}

void hal_misc_cmp_set_srstn_uart1(uint8_t value)
{
    sysc_cmp_srstn_uart1_setf(value);
}

void hal_misc_cmp_set_srstn_uart0(uint8_t value)
{
    sysc_cmp_srstn_uart0_setf(value);
}

void hal_misc_cmp_set_srstn_i2c0(uint8_t value)
{
    sysc_cmp_srstn_i2c0_setf(value);
}

void hal_misc_cmp_set_srstn_ws2811(uint8_t value)
{
    sysc_cmp_srstn_ws2811_setf(value);
}

void hal_misc_cmp_set_srstn_spi1(uint8_t value)
{
    sysc_cmp_srstn_spi1_setf(value);
}

void hal_misc_cmp_set_srstn_spi0(uint8_t value)
{
    sysc_cmp_srstn_spi0_setf(value);
}

void hal_misc_cmp_set_srstn_gpiob(uint8_t value)
{
    sysc_cmp_srstn_gpiob_setf(value);
}

void hal_misc_cmp_set_srstn_gpioa(uint8_t value)
{
    sysc_cmp_srstn_gpioa_setf(value);
}

void hal_misc_cmp_set_srstn_i2s(uint8_t value)
{
    sysc_cmp_srstn_i2s_setf(value);
}

void hal_misc_cmp_set_srstn_adcc(uint8_t value)
{
    sysc_cmp_srstn_adcc_setf(value);
}

void hal_misc_cmp_set_srstn_qspi(uint8_t value)
{
    sysc_cmp_srstn_qspi_setf(value);
}

uint8_t hal_misc_cmp_get_srstn_aes(void)
{
    return sysc_cmp_srstn_aes_getf();
}

uint8_t hal_misc_cmp_get_srstn_trng(void)
{
    return sysc_cmp_srstn_trng_getf();
}

uint8_t hal_misc_cmp_get_srstn_ef(void)
{
    return sysc_cmp_srstn_ef_getf();
}

uint8_t hal_misc_cmp_get_srstn_pwm(void)
{
    return sysc_cmp_srstn_pwm_getf();
}

uint8_t hal_misc_cmp_get_srstn_rfreg(void)
{
    return sysc_cmp_srstn_rfreg_getf();
}

uint8_t hal_misc_cmp_get_srstn_dma(void)
{
    return sysc_cmp_srstn_dma_getf();
}

uint8_t hal_misc_cmp_get_srstn_cache(void)
{
    return sysc_cmp_srstn_cache_getf();
}

uint8_t hal_misc_cmp_get_srstn_mac(void)
{
    return sysc_cmp_srstn_mac_getf();
}

uint8_t hal_misc_cmp_get_srstn_sdio(void)
{
    return sysc_cmp_srstn_sdio_getf();
}

uint8_t hal_misc_cmp_get_srstn_dbgh(void)
{
    return sysc_cmp_srstn_dbgh_getf();
}

uint8_t hal_misc_cmp_get_srstn_timer(void)
{
    return sysc_cmp_srstn_timer_getf();
}

uint8_t hal_misc_cmp_get_srstn_wdt(void)
{
    return sysc_cmp_srstn_wdt_getf();
}

uint8_t hal_misc_cmp_get_srstn_uart2(void)
{
    return sysc_cmp_srstn_uart2_getf();
}

uint8_t hal_misc_cmp_get_srstn_uart1(void)
{
    return sysc_cmp_srstn_uart1_getf();
}

uint8_t hal_misc_cmp_get_srstn_uart0(void)
{
    return sysc_cmp_srstn_uart0_getf();
}

uint8_t hal_misc_cmp_get_srstn_i2c0(void)
{
    return sysc_cmp_srstn_i2c0_getf();
}

uint8_t hal_misc_cmp_get_srstn_ws2811(void)
{
    return sysc_cmp_srstn_ws2811_getf();
}

uint8_t hal_misc_cmp_get_srstn_spi1(void)
{
    return sysc_cmp_srstn_spi1_getf();
}

uint8_t hal_misc_cmp_get_srstn_spi0(void)
{
    return sysc_cmp_srstn_spi0_getf();
}

uint8_t hal_misc_cmp_get_srstn_gpiob(void)
{
    return sysc_cmp_srstn_gpiob_getf();
}

uint8_t hal_misc_cmp_get_srstn_gpioa(void)
{
    return sysc_cmp_srstn_gpioa_getf();
}

uint8_t hal_misc_cmp_get_srstn_i2s(void)
{
    return sysc_cmp_srstn_i2s_getf();
}

uint8_t hal_misc_cmp_get_srstn_adcc(void)
{
    return sysc_cmp_srstn_adcc_getf();
}

uint8_t hal_misc_cmp_get_srstn_qspi(void)
{
    return sysc_cmp_srstn_qspi_getf();
}

uint8_t hal_misc_cmp_get_srstn_ble(void)
{
    return sysc_cmp_srstn_ble_getf();
}

//CMP IO_EN
void hal_misc_cmp_set_i2s_io_en1(uint8_t value)
{
    sysc_cmp_i2s_io_en1_setf(value);
}

void hal_misc_cmp_set_i2s_io_en0(uint8_t value)
{
    sysc_cmp_i2s_io_en0_setf(value);
}

void hal_misc_cmp_set_spif_io_en(uint8_t value)
{
    sysc_cmp_spif_io_en_setf(value);
}

void hal_misc_cmp_set_sdio_io_en(uint8_t value)
{
    sysc_cmp_sdio_io_en_setf(value);
}

uint8_t hal_misc_cmp_get_i2s_io_en1(void)
{
    return sysc_cmp_i2s_io_en1_getf();
}

uint8_t hal_misc_cmp_get_i2s_io_en0(void)
{
    return sysc_cmp_i2s_io_en0_getf();
}

uint8_t hal_misc_cmp_get_spif_io_en(void)
{
    return sysc_cmp_spif_io_en_getf();
}

uint8_t hal_misc_cmp_get_sdio_io_en(void)
{
    return sysc_cmp_sdio_io_en_getf();
}

//CMP MCU_STCALIB
void hal_misc_cmp_set_mcu_stcalib(uint32_t value)
{
    sysc_cmp_mcu_stcalib_setf(value);
}

uint32_t hal_misc_cmp_get_mcu_stcalib(void)
{
    return sysc_cmp_mcu_stcalib_getf();
}

//CMP IDLE
void hal_misc_cmp_set_idle_reg(uint32_t value)
{
    sysc_cmp_idle_reg_setf(value);
}

uint32_t hal_misc_cmp_get_idle_reg(void)
{
    return sysc_cmp_idle_reg_getf();
}

//CMP MAC_ABORT
void hal_misc_cmp_set_mac_txabort_reqn(uint8_t value)
{
    sysc_cmp_mac_txabort_reqn_setf(value);
}

uint8_t hal_misc_cmp_get_mac_txabort_reqn(void)
{
    return sysc_cmp_mac_txabort_reqn_getf();
}

//CMP MEM_EMA_0
void hal_misc_cmp_set_ram32_ema(uint8_t value)
{
    sysc_cmp_ram32_ema_setf(value);
}

void hal_misc_cmp_set_ram31_ema(uint8_t value)
{
    sysc_cmp_ram31_ema_setf(value);
}

void hal_misc_cmp_set_ram30_ema(uint8_t value)
{
    sysc_cmp_ram30_ema_setf(value);
}

void hal_misc_cmp_set_ram2_ema(uint8_t value)
{
    sysc_cmp_ram2_ema_setf(value);
}

void hal_misc_cmp_set_ram1_ema(uint8_t value)
{
    sysc_cmp_ram1_ema_setf(value);
}

void hal_misc_cmp_set_ram0_ema(uint8_t value)
{
    sysc_cmp_ram0_ema_setf(value);
}

void hal_misc_cmp_set_rom_ema(uint8_t value)
{
    sysc_cmp_rom_ema_setf(value);
}

uint8_t hal_misc_cmp_getram32_ema(void)
{
    return sysc_cmp_ram32_ema_getf();
}

uint8_t hal_misc_cmp_getram31_ema(void)
{
    return sysc_cmp_ram31_ema_getf();
}

uint8_t hal_misc_cmp_getram30_ema(void)
{
    return sysc_cmp_ram30_ema_getf();
}

uint8_t hal_misc_cmp_getram2_ema(void)
{
    return sysc_cmp_ram2_ema_getf();
}

uint8_t hal_misc_cmp_getram1_ema(void)
{
    return sysc_cmp_ram1_ema_getf();
}

uint8_t hal_misc_cmp_getram0_ema(void)
{
    return sysc_cmp_ram0_ema_getf();
}

uint8_t hal_misc_cmp_getrom_ema(void)
{
    return sysc_cmp_rom_ema_getf();
}

//CMP MEM_EMA_1
void hal_misc_cmp_set_ram35_emaw(uint8_t value)
{
    sysc_cmp_ram35_emaw_setf(value);
}

void hal_misc_cmp_set_ram34_emaw(uint8_t value)
{
    sysc_cmp_ram34_emaw_setf(value);
}

void hal_misc_cmp_set_ram33_emaw(uint8_t value)
{
    sysc_cmp_ram33_emaw_setf(value);
}

void hal_misc_cmp_set_ram32_emaw(uint8_t value)
{
    sysc_cmp_ram32_emaw_setf(value);
}

void hal_misc_cmp_set_ram31_emaw(uint8_t value)
{
    sysc_cmp_ram31_emaw_setf(value);
}

void hal_misc_cmp_set_ram30_emaw(uint8_t value)
{
    sysc_cmp_ram30_emaw_setf(value);
}

void hal_misc_cmp_set_ram2_emaw(uint8_t value)
{
    sysc_cmp_ram2_emaw_setf(value);
}

void hal_misc_cmp_set_ram1_emaw(uint8_t value)
{
    sysc_cmp_ram1_emaw_setf(value);
}

void hal_misc_cmp_set_ram0_emaw(uint8_t value)
{
    sysc_cmp_ram0_emaw_setf(value);
}

void hal_misc_cmp_set_ram35_ema(uint8_t value)
{
    sysc_cmp_ram35_ema_setf(value);
}

void hal_misc_cmp_set_ram34_ema(uint8_t value)
{
    sysc_cmp_ram34_ema_setf(value);
}

void hal_misc_cmp_set_ram33_ema(uint8_t value)
{
    sysc_cmp_ram33_ema_setf(value);
}

uint8_t hal_misc_cmp_get_ram35_emaw(void)
{
    return sysc_cmp_ram35_emaw_getf();
}

uint8_t hal_misc_cmp_get_ram34_emaw(void)
{
    return sysc_cmp_ram34_emaw_getf();
}

uint8_t hal_misc_cmp_get_ram33_emaw(void)
{
    return sysc_cmp_ram33_emaw_getf();
}

uint8_t hal_misc_cmp_get_ram32_emaw(void)
{
    return sysc_cmp_ram32_emaw_getf();
}

uint8_t hal_misc_cmp_get_ram31_emaw(void)
{
    return sysc_cmp_ram31_emaw_getf();
}

uint8_t hal_misc_cmp_get_ram30_emaw(void)
{
    return sysc_cmp_ram30_emaw_getf();
}

uint8_t hal_misc_cmp_get_ram2_emaw(void)
{
    return sysc_cmp_ram2_emaw_getf();
}

uint8_t hal_misc_cmp_get_ram1_emaw(void)
{
    return sysc_cmp_ram1_emaw_getf();
}

uint8_t hal_misc_cmp_get_ram0_emaw(void)
{
    return sysc_cmp_ram0_emaw_getf();
}

uint8_t hal_misc_cmp_get_ram35_ema(void)
{
    return sysc_cmp_ram35_ema_getf();
}

uint8_t hal_misc_cmp_get_ram34_ema(void)
{
    return sysc_cmp_ram34_ema_getf();
}


//CMP CLK_TEST
void hal_misc_cmp_set_clk_test_sel1(uint8_t value)
{
    sysc_cmp_clk_test_sel1_setf(value);
}

void hal_misc_cmp_set_clk_test_en1(uint8_t value)
{
    sysc_cmp_clk_test_en1_setf(value);
}

void hal_misc_cmp_set_clk_test_sel0(uint8_t value)
{
    sysc_cmp_clk_test_sel0_setf(value);
}

void hal_misc_cmp_set_clk_test_en0(uint8_t value)
{
    sysc_cmp_clk_test_en0_setf(value);
}

uint8_t hal_misc_cmp_get_clk_test_sel1(void)
{
    return sysc_cmp_clk_test_sel1_getf();
}

uint8_t hal_misc_cmp_get_clk_test_en1(void)
{
    return sysc_cmp_clk_test_en1_getf();
}

uint8_t hal_misc_cmp_get_clk_test_sel0(void)
{
    return sysc_cmp_clk_test_sel0_getf();
}

uint8_t hal_misc_cmp_get_clk_test_en0(void)
{
    return sysc_cmp_clk_test_en0_getf();
}

//CMP RFREG_CFG
void hal_misc_cmp_set_rfreg_base_addr(uint16_t value)
{
    sysc_cmp_rfreg_base_addr_setf(value);
}

void hal_misc_cmp_set_rfreg_len(uint16_t value)
{
    sysc_cmp_rfreg_len_setf(value);
}

uint16_t hal_misc_cmp_get_rfreg_base_addr(void)
{
    return sysc_cmp_rfreg_base_addr_getf();
}

uint16_t hal_misc_cmp_get_rfreg_len(void)
{
    return sysc_cmp_rfreg_len_getf();
}

//CMP RFREG_ACTION
void hal_misc_cmp_set_rfreg_restore(uint8_t value)
{
    sysc_cmp_rfreg_restore_setf(value);
}

void hal_misc_cmp_set_rfreg_save(uint8_t value)
{
    sysc_cmp_rfreg_save_setf(value);
}

//CMP RFREG_ACTION_STATUS
uint8_t hal_misc_cmp_get_restore_done(void)
{
    return sysc_cmp_restore_done_getf();
}

uint8_t hal_misc_cmp_get_save_done(void)
{
    return sysc_cmp_save_done_getf();
}

//CMP DEBUG_IO_EN
void hal_misc_cmp_set_debug_io_en(uint8_t value)
{
    sysc_cmp_debug_io_en_setf(value);
}

uint8_t hal_misc_cmp_get_debug_io_en(void)
{
    return sysc_cmp_debug_io_en_getf();
}


//CMP SW_CLKG
void hal_misc_cmp_set_mac_debug_port_sel(uint8_t value)
{
    sysc_cmp_mac_debug_port_sel_setf(value);
}

void hal_misc_cmp_set_debug_port_sel(uint8_t value)
{
    sysc_cmp_debug_port_sel_setf(value);
}

void hal_misc_cmp_set_debug_sw(uint16_t value)
{
    sysc_cmp_debug_sw_setf(value);
}

uint8_t hal_misc_cmp_get_mac_debug_port_sel(void)
{
    return sysc_cmp_mac_debug_port_sel_getf();
}

uint8_t hal_misc_cmp_get_debug_port_sel(void)
{
    return sysc_cmp_debug_port_sel_getf();
}

uint16_t hal_misc_cmp_get_debug_sw(void)
{
    return sysc_cmp_debug_sw_getf();
}

//CMP PA_OVD_IE
void hal_misc_cmp_set_pa_ovd_ie(uint8_t value)
{
    sysc_cmp_pa_ovd_ie_setf(value);
}

uint8_t hal_misc_cmp_get_pa_ovd_ie(void)
{
    return sysc_cmp_pa_ovd_ie_getf();
}

//CMP PA_OVD
void hal_misc_cmp_set_pa_ovd(uint8_t value)
{
    sysc_cmp_pa_ovd_setf(value);
}


void hal_misc_reset_all(void)
{
    sysc_awo_srstn_all_setf(0);
    sysc_awo_srstn_all_setf(1);
}

void hal_misc_reset_phy(void)
{
    sysc_awo_srstn_phy_setf(0);
    sysc_awo_srstn_phy_setf(1);
}


void hal_misc_reset_rtc(void)
{
    sysc_awo_srstn_rtc_setf(0);
    sysc_awo_srstn_rtc_setf(1);
}

void hal_misc_reset_wic(void)
{
    sysc_awo_srstn_wic_setf(0);
    sysc_awo_srstn_wic_setf(1);
}

void hal_misc_reset_pmu(void)
{
    sysc_awo_srstn_pmu_setf(0);
    sysc_awo_srstn_pmu_setf(1);
}

void hal_misc_reset_ble(void)
{
    sysc_cmp_srstn_ble_setf(0);
    sysc_cmp_srstn_ble_setf(1);
}

void hal_misc_reset_aes(void)
{
    sysc_cmp_srstn_aes_setf(0);
    sysc_cmp_srstn_aes_setf(1);
}

void hal_misc_reset_trng(void)
{
    sysc_cmp_srstn_trng_setf(0);
    sysc_cmp_srstn_trng_setf(1);
}

void hal_misc_reset_ef(void)
{
    sysc_cmp_srstn_ef_setf(0);
    sysc_cmp_srstn_ef_setf(1);
}


void hal_misc_reset_pwm(void)
{
    sysc_cmp_srstn_pwm_setf(0);
    sysc_cmp_srstn_pwm_setf(1);
}


void hal_misc_reset_rfreg(void)
{
    sysc_cmp_srstn_rfreg_setf(0);
    sysc_cmp_srstn_rfreg_setf(1);
}

void hal_misc_reset_dma(void)
{
    sysc_cmp_srstn_dma_setf(0);
    sysc_cmp_srstn_dma_setf(1);
}

void hal_misc_reset_cache(void)
{
    sysc_cmp_srstn_cache_setf(0);
    sysc_cmp_srstn_cache_setf(1);
}

void hal_misc_reset_mac(void)
{
    sysc_cmp_srstn_mac_setf(0);
    sysc_cmp_srstn_mac_setf(1);
}

void hal_misc_reset_sdio(void)
{
    sysc_cmp_srstn_sdio_setf(0);
    sysc_cmp_srstn_sdio_setf(1);
}

void hal_misc_reset_dbgh(void)
{
    sysc_cmp_srstn_dbgh_setf(0);
    sysc_cmp_srstn_dbgh_setf(1);
}

void hal_misc_reset_timer(void)
{
    sysc_cmp_srstn_timer_setf(0);
    sysc_cmp_srstn_timer_setf(1);
}

void hal_misc_reset_wdt(void)
{
    sysc_cmp_srstn_wdt_setf(0);
    sysc_cmp_srstn_wdt_setf(1);
}

void hal_misc_reset_uart2(void)
{
    sysc_cmp_srstn_uart2_setf(0);
    sysc_cmp_srstn_uart2_setf(1);
}

void hal_misc_reset_uart1(void)
{
    sysc_cmp_srstn_uart1_setf(0);
    sysc_cmp_srstn_uart1_setf(1);
}

void hal_misc_reset_uart0(void)
{
    sysc_cmp_srstn_uart0_setf(0);
    sysc_cmp_srstn_uart0_setf(1);
}


void hal_misc_reset_i2c0(void)
{
    sysc_cmp_srstn_i2c0_setf(0);
    sysc_cmp_srstn_i2c0_setf(1);
}

void hal_misc_reset_ws2811(void)
{
    sysc_cmp_srstn_ws2811_setf(0);
    sysc_cmp_srstn_ws2811_setf(1);
}

void hal_misc_reset_spi1(void)
{
    sysc_cmp_srstn_spi1_setf(0);
    sysc_cmp_srstn_spi1_setf(1);
}

void hal_misc_reset_spi0(void)
{
    sysc_cmp_srstn_spi0_setf(0);
    sysc_cmp_srstn_spi0_setf(1);
}

void hal_misc_reset_gpiob(void)
{
    sysc_cmp_srstn_gpiob_setf(0);
    sysc_cmp_srstn_gpiob_setf(1);
}

void hal_misc_reset_gpioa(void)
{
    sysc_cmp_srstn_gpioa_setf(0);
    sysc_cmp_srstn_gpioa_setf(1);
}   

void hal_misc_reset_i2s(void)
{
    sysc_cmp_srstn_i2s_setf(0);
    sysc_cmp_srstn_i2s_setf(1);
}

void hal_misc_reset_adcc(void)
{
    sysc_cmp_srstn_adcc_setf(0);
    sysc_cmp_srstn_adcc_setf(1);
}

void hal_misc_reset_qspi(void)
{
    sysc_cmp_srstn_qspi_setf(0);
    sysc_cmp_srstn_qspi_setf(1);
}


void hal_misc_set_aes_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_aes_gate_en_setf(1);
    else
        sysc_cmp_aes_gate_en_setf(0);
}

void hal_misc_set_trng_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_trng_gate_en_setf(1);
    else
        sysc_cmp_trng_gate_en_setf(0);
}

void hal_misc_set_ef_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_ef_gate_en_setf(1);
    else
        sysc_cmp_ef_gate_en_setf(0);
}

void hal_misc_set_pwm_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_pwm_gate_en_setf(1);
    else
        sysc_cmp_pwm_gate_en_setf(0);
}

void hal_misc_set_rfreg_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_rfreg_gate_en_setf(1);
    else
        sysc_cmp_rfreg_gate_en_setf(0);
}

void hal_misc_set_dma_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_dma_gate_en_setf(1);
    else
        sysc_cmp_dma_gate_en_setf(0);
}

void hal_misc_set_cache_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_cache_gate_en_setf(1);
    else
        sysc_cmp_cache_gate_en_setf(0);
}

void hal_misc_set_mac_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_mac_gate_en_setf(1);
    else
        sysc_cmp_mac_gate_en_setf(0);
}

void hal_misc_set_sdio_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_sdio_gate_en_setf(1);
    else
        sysc_cmp_sdio_gate_en_setf(0);
}

void hal_misc_set_dbgh_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_dbgh_gate_en_setf(1);
    else
        sysc_cmp_dbgh_gate_en_setf(0);
}

void hal_misc_set_timer4_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_timer4_gate_en_setf(1);
    else
        sysc_cmp_timer4_gate_en_setf(0);
}

void hal_misc_set_timer3_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_timer3_gate_en_setf(1);
    else
        sysc_cmp_timer3_gate_en_setf(0);
}

void hal_misc_set_timer2_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_timer2_gate_en_setf(1);
    else
        sysc_cmp_timer2_gate_en_setf(0);
}

void hal_misc_set_timer1_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_timer1_gate_en_setf(1);
    else
        sysc_cmp_timer1_gate_en_setf(0);
}

void hal_misc_set_timer_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_timer_gate_en_setf(1);
    else
        sysc_cmp_timer_gate_en_setf(0);
}

void hal_misc_set_wdt_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_wdt_gate_en_setf(1);
    else
        sysc_cmp_wdt_gate_en_setf(0);
}

void hal_misc_set_uart2_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_uart2_gate_en_setf(1);
    else
        sysc_cmp_uart2_gate_en_setf(0);
}

void hal_misc_set_uart1_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_uart1_gate_en_setf(1);
    else
        sysc_cmp_uart1_gate_en_setf(0);
}

void hal_misc_set_uart0_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_uart0_gate_en_setf(1);
    else
        sysc_cmp_uart0_gate_en_setf(0);
}

void hal_misc_set_i2c0_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_i2c0_gate_en_setf(1);
    else
        sysc_cmp_i2c0_gate_en_setf(0);
}

void hal_misc_set_ws2811_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_ws2811_gate_en_setf(1);
    else
        sysc_cmp_ws2811_gate_en_setf(0);
}

void hal_misc_set_spi1_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_spi1_gate_en_setf(1);
    else
        sysc_cmp_spi1_gate_en_setf(0);
}

void hal_misc_set_spi0_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_spi0_gate_en_setf(1);
    else
        sysc_cmp_spi0_gate_en_setf(0);
}

void hal_misc_set_gpiob_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_gpiob_gate_en_setf(1);
    else
        sysc_cmp_gpiob_gate_en_setf(0);
}

void hal_misc_set_gpioa_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_gpioa_gate_en_setf(1);
    else
        sysc_cmp_gpioa_gate_en_setf(0);
}

void hal_misc_set_i2s_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_i2s_gate_en_setf(1);
    else
        sysc_cmp_i2s_gate_en_setf(0);
}

void hal_misc_set_adcc_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_adcc_gate_en_setf(1);
    else
        sysc_cmp_adcc_gate_en_setf(0);
}

void hal_misc_set_qspi_clk_en(hal_en_t en)
{
    if(en == HAL_ENABLE)
        sysc_cmp_qspi_gate_en_setf(1);
    else
        sysc_cmp_qspi_gate_en_setf(0);
}

hal_en_t hal_misc_get_aes_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_aes_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_trng_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_trng_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_ef_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_ef_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_pwm_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_pwm_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_rfreg_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_rfreg_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_dma_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_dma_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_cache_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_cache_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_mac_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_mac_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_sdio_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_sdio_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_dbgh_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_dbgh_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_timer4_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_timer4_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_timer3_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_timer3_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_timer2_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_timer2_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_timer1_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_timer1_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_timer_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_timer_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_wdt_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_wdt_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_uart2_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_uart2_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_uart1_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_uart1_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_uart0_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_uart0_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_i2c0_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_i2c0_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_ws2811_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_ws2811_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_spi1_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_spi1_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_spi0_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_spi0_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_gpiob_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_gpiob_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_gpioa_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_gpioa_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_i2s_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_i2s_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_adcc_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_adcc_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}

hal_en_t hal_misc_get_qspi_clk_en(void)
{
    hal_en_t ret = HAL_ENABLE;
    if(sysc_cmp_qspi_gate_en_getf() == 1)
        ret = HAL_ENABLE;
    else
        ret = HAL_DISABLE;
    return ret;
}   

