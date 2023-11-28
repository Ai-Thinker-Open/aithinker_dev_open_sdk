#include "ln_compiler.h"
#include "ln_types.h"
#include "ln_utils.h"
#include "ln882h.h"

#include "utils/power_mgmt/ln_pm.h"

#define STOP_OS_TICK_MISSED_NS      (220*1000)
#define PM_OBJ_MAX_COUNT            (4)

typedef struct pm_obj_st {
    const char *name;

    int (*veto_check) (void);
    int (*pre_proc)   (void);
    int (*post_proc)  (void);
} pm_obj_t;

typedef struct pm_ctrl_st {
    uint8_t         obj_count;
    uint8_t         flag; /*TODO:*/
    sleep_mode_t    slp_mode;
    uint32_t        clk_gate_cfg;
    uint32_t        sleep_ms;
    pm_obj_t        obj[PM_OBJ_MAX_COUNT];
} pm_ctl_t;

static pm_ctl_t sg_pm_ctrl = {0,};


/*****************************************************************************/
/*                power save management related hardware                     */
/*****************************************************************************/
#include "reg_sysc_awo.h"
#include "reg_sysc_cmp.h"

__STATIC_INLINE void soc_sleep_timer_config(uint64_t time)
{
    sysc_awo_o_cpu_sleep_time_l_setf((uint32_t)time);
    sysc_awo_o_cpu_sleep_time_h_setf((time >> 32U) & 0xFFFF);
    sysc_awo_o_cpu_sleep_inten_setf(1);
    NVIC_EnableIRQ(RFSLP_IRQn);
}

__STATIC_INLINE uint64_t soc_sleep_timer_real_time_get(void)
{
    uint64_t temp = sysc_awo_i_real_sleep_time_h_getf();
    temp = (temp << 32U) + sysc_awo_i_real_sleep_time_l_getf();
    return temp;
}

__STATIC_INLINE void soc_io_pmu_ret_config(uint8_t ret, uint8_t unret)
{
    sysc_awo_r_pmu_ret_en_setf(ret);      // gpio retention
    sysc_awo_r_pmu_unret_en_setf(unret);  // gpio unretention
}

__STATIC_INLINE void soc_cortex_m4_select_deepsleep(void)
{
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
}

__STATIC_INLINE void soc_cortex_m4_deselect_deepsleep(void)
{
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
}

#define SOC_AWO_PMU_CFG_SET(pdcmp_po, cpucore_retreg_po,  ldo18_po_en, ldo18_pd_en,  digldo_norvdd, digldo_retvdd,   \
                        sram_po_en, sram_pd_en,  phy_po_en, phy_pd_en,  digpll_po, xtal_pd_en)                       \
    do {                                                                                                             \
        uint32_t temp = 0;                                                                                           \
        temp |= ((pdcmp_po     << 0U)  | (cpucore_retreg_po << 1U)  | (ldo18_po_en << 2U)  | (ldo18_pd_en << 5U)     \
              | (digldo_norvdd << 8U)  | (digldo_retvdd     << 12U) | (sram_po_en  << 16U) | (sram_pd_en  << 20U)    \
              | (phy_po_en     << 24U) | (phy_pd_en         << 28U) | (digpll_po   << 29U) | (xtal_pd_en  << 30U));  \
        sysc_awo_pmu_cfg_set(temp);                                                                                  \
    } while(0);

__STATIC_INLINE void soc_start_lightsleep(void)
{
    uint8_t i = 0;
    /* trigger o_sw_po_start=bit0 o_sw_pd_start=bit1 */
    sysc_awo_sw_pwr_ctrl_set(0x02);
    while (i < 2) {
        if (sysc_awo_pmu_fsm_getf() == 0) {
            i++;
        }
    }
}

__STATIC_INLINE void soc_stop_lightsleep(void)
{
    uint8_t i = 0;
    /* trigger o_sw_po_start=bit0 o_sw_pd_start=bit1 */
    sysc_awo_sw_pwr_ctrl_set(0x01);
    while (i < 2) {
        if (sysc_awo_pmu_fsm_getf() == 0) {
            i++;
        }
    }
}

void soc_module_clk_gate_disable(uint32_t gate_cfg)
{
    uint32_t gate = sysc_cmp_sw_clkg_get();
    gate &= ~gate_cfg;
    sysc_cmp_sw_clkg_set(gate);
}

void soc_module_clk_gate_enable(uint32_t gate_cfg)
{
    uint32_t gate = sysc_cmp_sw_clkg_get();
    gate |= gate_cfg;
    sysc_cmp_sw_clkg_set(gate);
}

__STATIC_INLINE void soc_rom_phase_reset_flag(void)
{
    sysc_awo_idle_reg_set(0x52);
}

__STATIC_INLINE void soc_pll2xtal(void)
{
    sysc_awo_clk_src_sel_setf(0);       //0 = xtal 1 = pll
    sysc_cmp_pclk0_div_para_m1_setf(0); //0 = 1div
    sysc_cmp_pclk0_div_para_up_setf(1);
    sysc_awo_syspll_en_setf(0);         //pll disable
}

__STATIC_INLINE void soc_xtal2pll(void)
{
    sysc_awo_syspll_en_setf(1);         //pll enable
    sysc_cmp_pclk0_div_para_m1_setf(3); //3 = 4div
    sysc_cmp_pclk0_div_para_up_setf(1);
    sysc_awo_clk_src_sel_setf(1);       //0 = xtal 1 = pll
}

/*****************************************************************************/
/*                        power save management                              */
/*****************************************************************************/

#include "hal/hal_timer.h"
#define portNVIC_SYSTICK_CTRL_REG			( * ( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG			( * ( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG	( * ( ( volatile uint32_t * ) 0xe000e018 ) )

#define portNVIC_SYSTICK_INT_BIT			( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT			( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT		( 1UL << 16UL )

#define TIM_BASE                            (TIMER3_BASE)

#if defined ( __CC_ARM )
  #define LIGHT_SLEEP_MISSED_US               (10)
#elif defined ( __GNUC__ )
  #define LIGHT_SLEEP_MISSED_US               (10)
#else
  #error "Unsupported compiler"
#endif

static uint32_t last_us = 0;
extern void wlib_hwtimer_init_v2(void * timer_cb);

static void tim_callback (void)
{
    hal_tim_en(TIM_BASE, HAL_DISABLE);
}

__STATIC_INLINE void pmu_pre_sleep_update(pm_ctl_t * ctrl)
{
    switch (ctrl->slp_mode)
    {
        case LIGHT_SLEEP:
            SOC_AWO_PMU_CFG_SET(1, 1, 7, 7, 4, 4, 0, 0, 1, 1, 1, 1); /* 0x600044FF */
            soc_io_pmu_ret_config(0, 0);                             /* no pd cmp, not need retention/unret */
            soc_module_clk_gate_disable(ctrl->clk_gate_cfg);
            soc_pll2xtal();
            soc_start_lightsleep();                                  /* trigger sleep at last */
            break;

        case DEEP_SLEEP:
            SOC_AWO_PMU_CFG_SET(1, 1, 7, 7, 4, 4, 0, 0, 1, 1, 1, 0); /* 0x310044FF */
            soc_io_pmu_ret_config(0, 0);                             /* no pd cmp, not need retention/unret */
            soc_cortex_m4_select_deepsleep();                        /* trigger sleep at last */                
            break;

        case FROZEN_SLEEP:
            SOC_AWO_PMU_CFG_SET(0, 0, 7, 7, 4, 4, 7, 7, 1, 1, 1, 1); /* 0x717744FC */
            soc_io_pmu_ret_config(1, 1);                             /* pd cmp, need retention/unret */
            soc_rom_phase_reset_flag();                              /* ROM boot phase reset all digital module */
            soc_cortex_m4_select_deepsleep();                        /* trigger sleep at last */
            break;
        
        default:
            break;
    }
}

__STATIC_INLINE void pmu_post_sleep_update(pm_ctl_t * ctrl)
{
    switch (ctrl->slp_mode)
    {
        case LIGHT_SLEEP:
            soc_stop_lightsleep();
            soc_xtal2pll();
            soc_module_clk_gate_enable(ctrl->clk_gate_cfg);
            break;

        case DEEP_SLEEP:
            soc_cortex_m4_deselect_deepsleep();
            break;

        case FROZEN_SLEEP:
            /* Reset to ROM, nothing to do! */
            break;

        default:
            break;
    }
}

__STATIC_INLINE int obj_sleep_vote(pm_ctl_t *ctrl)
{
    uint8_t i = ctrl->obj_count;

    while ((i--) > 0)
    {
        if (ctrl->obj[i].veto_check) {
            if (!ctrl->obj[i].veto_check()) {
                return LN_FALSE;
            }
        }
    }

    return LN_TRUE;
}

__STATIC_INLINE void obj_prepare_sleep_proc(pm_ctl_t *ctrl)
{
    uint8_t i = ctrl->obj_count;

    while ((i--) > 0)
    {
        if (ctrl->obj[i].pre_proc) {
            ctrl->obj[i].pre_proc();
        }
    }
}

__STATIC_INLINE void obj_post_sleep_proc(pm_ctl_t *ctrl)
{
    uint8_t i = ctrl->obj_count;

    while ((i--) > 0)
    {
        if (ctrl->obj[i].post_proc) {
            ctrl->obj[i].post_proc();
        }
    }
}

extern uint32_t __curr_value;
extern uint32_t ulReloadValue;

//#include "utils/debug/log.h"
void ln_pm_rtos_pre_sleep_proc(uint32_t *expect_ms)
{
    uint64_t u64val = 0;
    pm_ctl_t *pm_ctrl = &sg_pm_ctrl;

    if (pm_ctrl->slp_mode == ACTIVE || pm_ctrl->flag) {
        return;
    }

    if (!obj_sleep_vote(pm_ctrl)) {
        return;
    }
    
    pm_ctrl->flag     = LN_TRUE;
    pm_ctrl->sleep_ms = *expect_ms;

    static uint8_t _init_flag = 0;
    if (_init_flag == 0) {
        _init_flag = 1;
        wlib_hwtimer_init_v2(tim_callback);
    }

    if (pm_ctrl->slp_mode == LIGHT_SLEEP) {
        int32_t load_val = ((*expect_ms*1000) - 1 - last_us);
        if (load_val < 0) {
            //LOG(LOG_LVL_ERROR, "pre: load_val=%d\r\n", load_val);
            return;
        }

        hal_tim_set_load_value(TIM_BASE, load_val);
        hal_tim_en(TIM_BASE, HAL_ENABLE);
        portNVIC_SYSTICK_CTRL_REG &= ~portNVIC_SYSTICK_ENABLE_BIT;
        if (__curr_value > (ulReloadValue - portNVIC_SYSTICK_CURRENT_VALUE_REG)){
            __curr_value -= (ulReloadValue - portNVIC_SYSTICK_CURRENT_VALUE_REG);
        }else{
            __curr_value = 1;
        }
    }

    if(pm_ctrl->slp_mode == DEEP_SLEEP) /* DEEP_SLEEP */
    {
        if(((uint64_t)*expect_ms * 1000000) > STOP_OS_TICK_MISSED_NS) {
            u64val = (((uint64_t)*expect_ms * 1000000) - STOP_OS_TICK_MISSED_NS);
        } else {
            u64val = 1000000; //1ms
        }
        
        soc_sleep_timer_config(u64val);
    }

//    LOG(LOG_LVL_ERROR, "pre\r\n");
    obj_prepare_sleep_proc(pm_ctrl);
    pmu_pre_sleep_update(pm_ctrl);
}

int ln_pm_rtos_post_sleep_proc(uint32_t *expect_ms)
{
    int ret = -1;
    uint32_t real = 0;
    pm_ctl_t *pm_ctrl = &sg_pm_ctrl;

    if (pm_ctrl->slp_mode == ACTIVE || !pm_ctrl->flag) {
        return -1;
    }
    
    pmu_post_sleep_update(pm_ctrl);
    obj_post_sleep_proc(pm_ctrl);
//    LOG(LOG_LVL_ERROR, "post\r\n");
    
    if (pm_ctrl->slp_mode == LIGHT_SLEEP) {

        if (hal_tim_get_it_flag(TIM_BASE, TIM_IT_FLAG_ACTIVE) == 1) {
            hal_tim_clr_it_flag(TIM_BASE, TIM_IT_FLAG_ACTIVE);
            *expect_ms = pm_ctrl->sleep_ms;
            last_us = (hal_tim_get_load_value(TIM_BASE) - hal_tim_get_current_cnt_value(TIM_BASE));
            last_us += (LIGHT_SLEEP_MISSED_US);
        } else {
            uint32_t tmp_real_us = hal_tim_get_load_value(TIM_BASE) - hal_tim_get_current_cnt_value(TIM_BASE) + last_us;
            real    = tmp_real_us / 1000;
            last_us = tmp_real_us % 1000;

            if (real > pm_ctrl->sleep_ms) {
                //LOG(LOG_LVL_ERROR, "post: real=%d sleep=%d\r\n", real, pm_ctrl->sleep_ms);
                *expect_ms = pm_ctrl->sleep_ms;
            } else {
                *expect_ms = real;
            }
            last_us += (LIGHT_SLEEP_MISSED_US + 1);
        }

        hal_tim_en(TIM_BASE, HAL_DISABLE);
        ret = 0;
    }

    if(pm_ctrl->slp_mode >= DEEP_SLEEP) /* DEEP_SLEEP, FROZEN_SLEEP */
    {        
        real = ((soc_sleep_timer_real_time_get()/1000000) + (STOP_OS_TICK_MISSED_NS/1000000)) ;
        *expect_ms = MIN(pm_ctrl->sleep_ms, real);
        ret = 0;
    }

    pm_ctrl->flag = LN_FALSE;
    return ret;
}

void ln_pm_sleep_mode_set(sleep_mode_t mode)
{
    GLOBAL_INT_DISABLE();
    sg_pm_ctrl.slp_mode = mode;
    GLOBAL_INT_RESTORE();
}

sleep_mode_t ln_pm_sleep_mode_get(void)
{
    return sg_pm_ctrl.slp_mode;
}

void ln_pm_always_clk_disable_select(uint32_t gate_cfg)
{
    GLOBAL_INT_DISABLE();
    soc_module_clk_gate_disable(gate_cfg);
    GLOBAL_INT_RESTORE();
}

void ln_pm_lightsleep_clk_disable_select(uint32_t gate_cfg)
{
    GLOBAL_INT_DISABLE();
    sg_pm_ctrl.clk_gate_cfg = gate_cfg;
    GLOBAL_INT_RESTORE();
}

__STATIC_INLINE int search_obj_idx(const char *name, uint8_t *idx)
{
    pm_ctl_t *pm_ctrl = &sg_pm_ctrl;
    
    for (uint8_t index = 0; index < pm_ctrl->obj_count; index++) {
        if (!strcmp(pm_ctrl->obj[index].name, name)) {
            *idx = index;
            return 0;
        }
    }
    
    return -1;
}

int ln_pm_obj_register(const char *name, int(*veto)(void), int(*pre_proc)(void), int(*post_proc)(void))
{
    int8_t  ret   = 0;
    uint8_t index = 0;
    pm_ctl_t *pm_ctrl = &sg_pm_ctrl;
    
    GLOBAL_INT_DISABLE();
    if (pm_ctrl->obj_count > PM_OBJ_MAX_COUNT) {
        ret = -1;
    } else {
        
        if (search_obj_idx(name, &index) < 0) {
            /* search failed, add the obj */
            index = pm_ctrl->obj_count++;
        }

        pm_ctrl->obj[index].name       = name;
        pm_ctrl->obj[index].veto_check = veto;
        pm_ctrl->obj[index].pre_proc   = pre_proc;
        pm_ctrl->obj[index].post_proc  = post_proc;
        ret = 0;
    }
    GLOBAL_INT_RESTORE();

    return ret;
}

void ln_pm_sleep_frozen(void)
{
    ln_pm_sleep_mode_set(FROZEN_SLEEP);
    pmu_pre_sleep_update(&sg_pm_ctrl);
}
