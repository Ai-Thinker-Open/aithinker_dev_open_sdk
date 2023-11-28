#ifndef __REG_WDT_H__
#define __REG_WDT_H__
//Auto-gen by fr

#include "ln882h.h"

//wdt_cr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                         wdt_en :  1; /* 0: 0, When the configuration parameter WDT_ALWAYS_EN = 0, this bit can be set;otherwise, it is read-only. This bit is used to enable and disable the DW_apb_wdt.When disabled, the counter does not decrement. Thus, no interrupts or systemresets are generated.
The DW_apb_wdt is used to prevent system lock-up. To prevent a software bug from disabling the DW_apb_wdt, once this bit has been enabled, it can be cleared only by a system reset. 0 = WDT disabled.
1 = WDT enabled.*/
        uint32_t                           rmod :  1; /* 1: 1, Writes have no effect when the parameter WDT_HC_RMOD = 1, thus this register becomes read-only. Selects the output response generated to a timeout. 0 = Generate a system reset. 1 = First generate an interrupt and if it is not cleared by the time a second timeout occurs then generate a system reset.*/
        uint32_t                            rpl :  3; /* 4: 2, Reset pulse length.
Writes have no effect when the configuration parameter WDT_HC_RPL is 1, making the register bits read-only. This is used to select the number of pclk cycles for which the system reset stays asserted. The range of values available is 2 to 256 pclk cycles.
000 �C 2 pclk cycles
001 �C 4 pclk cycles
010 �C 8 pclk cycles
011 �C 16 pclk cycles
100 �C 32 pclk cycles
101 �C 64 pclk cycles
110 �C 128 pclk cycles
111 �C 256 pclk cycles*/
        uint32_t                          dummy :  1; /* 5: 5, Redundant R/W bit. Included for ping test purposes, as it is the only R/W register bit that is in every configuration of the DW_apb_wdt.*/
        uint32_t                     reserved_0 : 26; /*31: 6,                             NA*/
    } bit_field;
} T_WDT_WDT_CR;

//wdt_torr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                            top :  4; /* 3: 0, Timeout period.
Writes have no effect when the configuration parameter DT_HC_TOP = 1, thus making this register read-only. This field is used to select the timeout period from which the watchdog counter restarts. A change of the timeout period takes effect only after the next counter restart (kick). The range of values is limited by the DT_CNT_WIDTH. If TOP is programmed to select a range that is greater than the counter width, the timeout period is truncated to fit to the counter width. This affects only the non-user specified values as users are limited to these boundaries during configuration. The range of values available for a 32-bit watchdog counter are:
Where i = TOP and t = timeout period
For i = 0 to 15
  if WDT_USE_FIX_TOP==1
    t = 2(16 + i)
  else
    t = WDT_USER_TOP_(i)*/
        uint32_t                       top_init :  4; /* 7: 4, Timeout period for initialization.
Writes to these register bits have no effect when the configuration parameter WDT_HC_TOP = 1 or WDT_ALWAYS_EN = 1. Used to select the timeout period that the watchdog counter restarts from for the first counter restart (kick). This register should be written after reset and before the WDT is enabled. A change of the TOP_INIT is seen only once the WDT has been enabled, and any change after the first kick is not seen as subsequent kicks use the period specified by
the TOP bits. The range of values is limited by the DT_CNT_WIDTH. If TOP_INIT is programmed to select a range that is greater than the counter width, the timeout period is truncated to fit to the counter width. This affects only the non-user specified values as users are limited to these boundaries during configuration. The range of values available for a 32-bit watchdog counter are:
Where i = TOP_INIT and t = timeout period
For i = 0 to 15
  if WDT_USE_FIX_TOP==1
    t = 2(16 + i)
  else
    t = WDT_USER_TOP_INIT_(i)*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_WDT_WDT_TORR;

//wdt_ccvr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t     currentcountervalueregiste : 16; /*15: 0, This register, when read, is the current value of the internal counter. This value is read coherently when ever it is read, which is relevant when the APB_DATA_WIDTH is less than the counter width.*/
        uint32_t                     reserved_0 : 16; /*31:16,                             NA*/
    } bit_field;
} T_WDT_WDT_CCVR;

//wdt_crr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t         counterrestartregister :  8; /* 7: 0, This register is used to restart the WDT counter. As a safety feature to prevent accidental restarts, the value 0x76 must be written. A restart also clears the WDT interrupt. Reading this register returns zero.*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_WDT_WDT_CRR;

//wdt_stat
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t        interruptstatusregister :  1; /* 0: 0, This register shows the interrupt status of the WDT.
1 = Interrupt is active regardless of polarity.
0 = Interrupt is inactive.*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_WDT_WDT_STAT;

//wdt_eoi
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t         interruptclearregister :  1; /* 0: 0, Clears the watchdog interrupt. This can be used to clear the interrupt without restarting the watchdog counter.*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_WDT_WDT_EOI;

//wdt_comp_version_0
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t           wdtcomponentversion0 : 32; /*31: 0, ASCII value for each number in the version, followed by *. For example 32_30_31_2A represents the version 2.01* */
    } bit_field;
} T_WDT_WDT_COMP_VERSION_0;

//wdt_comp_params_5
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t            cp_wdt_user_top_max : 32; /*31: 0, Upper limit of Timeout Period parameters. The value of this
register is derived from the WDT_USER_TOP_* coreConsultant parameters. See Table 4-2 on page 42 for a description of these parameters.*/
    } bit_field;
} T_WDT_WDT_COMP_PARAMS_5;

//wdt_comp_params_4
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t       cp_wdt_user_top_init_max : 32; /*31: 0, Upper limit of Initial Timeout Period parameters. The value of this register is derived from the WDT_USER_TOP_INIT_* coreConsultant parameters. See Table 4-2 on page 42 for a description of these
parameters.*/
    } bit_field;
} T_WDT_WDT_COMP_PARAMS_4;

//wdt_comp_params_3
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                 cd_wdt_top_rst : 32; /*31: 0, The value of this register is derived from the WDT_TOP_RST coreConsultant parameter. See Table 4-2 on page 42 for a escription
of this parameter.*/
    } bit_field;
} T_WDT_WDT_COMP_PARAMS_3;

//wdt_comp_params_2
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                 cp_wdt_cnt_rst : 32; /*31: 0, The value of this register is derived from the WDT_RST_CNT coreConsultant parameter. See Table 4-2 on page 42 for a escription
of this parameter.*/
    } bit_field;
} T_WDT_WDT_COMP_PARAMS_2;

//wdt_comp_params_1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t               cp_wdt_always_en :  1; /* 0: 0, (WDT_ALWAYS_EN == FALSE) =0
(WDT_ALWAYS_EN == TRUE) =1*/
        uint32_t               cp_wdt_dflt_rmod :  1; /* 1: 1, (WDT_DFLT_RMOD == FALSE) =0
(WDT_DFLT_RMOD == TRUE) =1*/
        uint32_t                cp_wdt_dual_top :  1; /* 2: 2, (WDT_DUAL_TOP == FALSE) =0
(WDT_DUAL_TOP == TRUE) =1*/
        uint32_t                 cp_wdt_hc_rmod :  1; /* 3: 3, (WDT_HC_RMOD == FALSE) =0
(WDT_HC_RMOD == TRUE) =1*/
        uint32_t                  cp_wdt_hc_rpl :  1; /* 4: 4, (WDT_HC_RPL == FALSE) =0
(WDT_HC_RPL == TRUE) =1*/
        uint32_t                  cp_wdt_hc_top :  1; /* 5: 5, (WDT_HC_TOP == FALSE) =0
(WDT_HC_TOP == TRUE) =1*/
        uint32_t             cp_wdt_use_fix_top :  1; /* 6: 6, (WDT_USE_FIX_TOP == FALSE) =0
(WDT_USE_FIX_TOP == TRUE) =1*/
        uint32_t                   cp_wdt_pause :  1; /* 7: 7,                             NA*/
        uint32_t          cp_wdt_apb_data_width :  2; /* 9: 8, (APB_DATA_WIDTH == 8) = 0
(APB_DATA_WIDTH == 16) = 1
(APB_DATA_WIDTH == 32) = 2
reserved = 3*/
        uint32_t                cp_wdt_dflt_rpl :  3; /*12:10,                   WDT_DFLT_RPL*/
        uint32_t                     reserved_1 :  3; /*15:13,                             NA*/
        uint32_t                cp_wdt_dflt_top :  4; /*19:16,                   WDT_DFLT_TOP*/
        uint32_t           cp_wdt_dflt_top_init :  4; /*23:20,              WDT_DFLT_TOP_INIT*/
        uint32_t               cp_wdt_cnt_width :  5; /*28:24,             WDT_CNT_WIDTH - 16*/
        uint32_t                     reserved_0 :  3; /*31:29,                             NA*/
    } bit_field;
} T_WDT_WDT_COMP_PARAMS_1;

//wdt_comp_version_1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t           wdtcomponentversion1 : 32; /*31: 0, ASCII value for each number in the version, followed by *. For example 32_30_31_2A represents the version 2.01* */
    } bit_field;
} T_WDT_WDT_COMP_VERSION_1;

//wdt_comp_type
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t          componenttyperegister : 32; /*31: 0, Designware Component Type number = 0x44_57_01_20. This assigned unique hex value is constant, and is derived from the two ASCII letters ��DW�� followed by a 16-bit unsigned number.*/
    } bit_field;
} T_WDT_WDT_COMP_TYPE;

//Registers Mapping to the same address

typedef struct
{
    volatile                    T_WDT_WDT_CR                         wdt_cr; /*  0x0,    RW, 0x00000002,               Control Register*/
    volatile                  T_WDT_WDT_TORR                       wdt_torr; /*  0x4,    RW, 0x00000007,         Timeout Range Register*/
    volatile                  T_WDT_WDT_CCVR                       wdt_ccvr; /*  0x8,    RO, 0x00007FFF, Current Counter Value Register*/
    volatile                   T_WDT_WDT_CRR                        wdt_crr; /*  0xc,    RW, 0x00000000,       Counter Restart Register*/
    volatile                  T_WDT_WDT_STAT                       wdt_stat; /* 0x10,    RO, 0x00000000,      Interrupt Status Register*/
    volatile                   T_WDT_WDT_EOI                        wdt_eoi; /* 0x14,    RO, 0x00000000,       Interrupt Clear Register*/
    volatile        T_WDT_WDT_COMP_VERSION_0             wdt_comp_version_0; /* 0x18,    RO, 0x3130392A,     Component Version Register*/
    volatile                        uint32_t                 reserved_0[50];
    volatile         T_WDT_WDT_COMP_PARAMS_5              wdt_comp_params_5; /* 0xe4,    RO, 0x007FFFFF, Component Parameters Register 5*/
    volatile         T_WDT_WDT_COMP_PARAMS_4              wdt_comp_params_4; /* 0xe8,    RO, 0x00000000, Component Parameters Register 4*/
    volatile         T_WDT_WDT_COMP_PARAMS_3              wdt_comp_params_3; /* 0xec,    RO, 0x00000007, Component Parameters Register 3*/
    volatile         T_WDT_WDT_COMP_PARAMS_2              wdt_comp_params_2; /* 0xf0,    RO, 0x00007FFF, Component Parameters Register 2*/
    volatile         T_WDT_WDT_COMP_PARAMS_1              wdt_comp_params_1; /* 0xf4,    RO, 0x08070202, Component Parameters Register 1*/
    volatile        T_WDT_WDT_COMP_VERSION_1             wdt_comp_version_1; /* 0xf8,    RO, 0x3130392A,     Component Version Register*/
    volatile             T_WDT_WDT_COMP_TYPE                  wdt_comp_type; /* 0xfc,    RO, 0x44570120,                             NA*/
} T_HWP_WDT_T;



__STATIC_INLINE uint32_t wdt_wdt_cr_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_cr.val;
}

__STATIC_INLINE void wdt_wdt_cr_set(uint32_t base, uint32_t value)
{
    ((T_HWP_WDT_T*)(base))->wdt_cr.val = value;
}

__STATIC_INLINE void wdt_wdt_cr_pack(uint32_t base, uint8_t dummy, uint8_t rpl, uint8_t rmod, uint8_t wdt_en)
{
    ((T_HWP_WDT_T*)(base))->wdt_cr.val = (((uint32_t)dummy << 5) | ((uint32_t)rpl << 2) | ((uint32_t)rmod << 1) | ((uint32_t)wdt_en << 0));
}

__STATIC_INLINE void wdt_wdt_cr_unpack(uint32_t base, uint8_t* dummy, uint8_t* rpl, uint8_t* rmod, uint8_t* wdt_en)
{
    T_WDT_WDT_CR localVal = ((T_HWP_WDT_T*)(base))->wdt_cr;

    *dummy = localVal.bit_field.dummy;
    *rpl = localVal.bit_field.rpl;
    *rmod = localVal.bit_field.rmod;
    *wdt_en = localVal.bit_field.wdt_en;
}

__STATIC_INLINE uint8_t wdt_dummy_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_cr.bit_field.dummy;
}

__STATIC_INLINE void wdt_dummy_setf(uint32_t base, uint8_t dummy)
{
    ((T_HWP_WDT_T*)(base))->wdt_cr.bit_field.dummy = dummy;
}

__STATIC_INLINE uint8_t wdt_rpl_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_cr.bit_field.rpl;
}

__STATIC_INLINE void wdt_rpl_setf(uint32_t base, uint8_t rpl)
{
    ((T_HWP_WDT_T*)(base))->wdt_cr.bit_field.rpl = rpl;
}

__STATIC_INLINE uint8_t wdt_rmod_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_cr.bit_field.rmod;
}

__STATIC_INLINE void wdt_rmod_setf(uint32_t base, uint8_t rmod)
{
    ((T_HWP_WDT_T*)(base))->wdt_cr.bit_field.rmod = rmod;
}

__STATIC_INLINE uint8_t wdt_wdt_en_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_cr.bit_field.wdt_en;
}

__STATIC_INLINE void wdt_wdt_en_setf(uint32_t base, uint8_t wdt_en)
{
    ((T_HWP_WDT_T*)(base))->wdt_cr.bit_field.wdt_en = wdt_en;
}

__STATIC_INLINE uint32_t wdt_wdt_torr_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_torr.val;
}

__STATIC_INLINE void wdt_wdt_torr_set(uint32_t base, uint32_t value)
{
    ((T_HWP_WDT_T*)(base))->wdt_torr.val = value;
}

__STATIC_INLINE void wdt_wdt_torr_pack(uint32_t base, uint8_t top)
{
    ((T_HWP_WDT_T*)(base))->wdt_torr.val = (((uint32_t)top << 0));
}

__STATIC_INLINE void wdt_wdt_torr_unpack(uint32_t base, uint8_t* top_init, uint8_t* top)
{
    T_WDT_WDT_TORR localVal = ((T_HWP_WDT_T*)(base))->wdt_torr;

    *top_init = localVal.bit_field.top_init;
    *top = localVal.bit_field.top;
}

__STATIC_INLINE uint8_t wdt_top_init_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_torr.bit_field.top_init;
}

__STATIC_INLINE uint8_t wdt_top_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_torr.bit_field.top;
}

__STATIC_INLINE void wdt_top_setf(uint32_t base, uint8_t top)
{
    ((T_HWP_WDT_T*)(base))->wdt_torr.bit_field.top = top;
}

__STATIC_INLINE uint32_t wdt_wdt_ccvr_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_ccvr.val;
}

__STATIC_INLINE void wdt_wdt_ccvr_unpack(uint32_t base, uint16_t* currentcountervalueregiste)
{
    T_WDT_WDT_CCVR localVal = ((T_HWP_WDT_T*)(base))->wdt_ccvr;

    *currentcountervalueregiste = localVal.bit_field.currentcountervalueregiste;
}

__STATIC_INLINE uint16_t wdt_currentcountervalueregiste_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_ccvr.bit_field.currentcountervalueregiste;
}

__STATIC_INLINE uint32_t wdt_wdt_crr_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_crr.val;
}

__STATIC_INLINE void wdt_wdt_crr_set(uint32_t base, uint32_t value)
{
    ((T_HWP_WDT_T*)(base))->wdt_crr.val = value;
}

__STATIC_INLINE void wdt_wdt_crr_pack(uint32_t base, uint8_t counterrestartregister)
{
    ((T_HWP_WDT_T*)(base))->wdt_crr.val = (((uint32_t)counterrestartregister << 0));
}

__STATIC_INLINE uint32_t wdt_wdt_stat_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_stat.val;
}

__STATIC_INLINE void wdt_wdt_stat_unpack(uint32_t base, uint8_t* interruptstatusregister)
{
    T_WDT_WDT_STAT localVal = ((T_HWP_WDT_T*)(base))->wdt_stat;

    *interruptstatusregister = localVal.bit_field.interruptstatusregister;
}

__STATIC_INLINE uint8_t wdt_interruptstatusregister_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_stat.bit_field.interruptstatusregister;
}

__STATIC_INLINE uint32_t wdt_wdt_eoi_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_eoi.val;
}

__STATIC_INLINE void wdt_wdt_eoi_unpack(uint32_t base, uint8_t* interruptclearregister)
{
    T_WDT_WDT_EOI localVal = ((T_HWP_WDT_T*)(base))->wdt_eoi;

    *interruptclearregister = localVal.bit_field.interruptclearregister;
}

__STATIC_INLINE uint8_t wdt_interruptclearregister_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_eoi.bit_field.interruptclearregister;
}

__STATIC_INLINE uint32_t wdt_wdt_comp_version_0_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_version_0.val;
}

__STATIC_INLINE void wdt_wdt_comp_version_0_unpack(uint32_t base, uint32_t* wdtcomponentversion0)
{
    T_WDT_WDT_COMP_VERSION_0 localVal = ((T_HWP_WDT_T*)(base))->wdt_comp_version_0;

    *wdtcomponentversion0 = localVal.bit_field.wdtcomponentversion0;
}

__STATIC_INLINE uint32_t wdt_wdtcomponentversion0_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_version_0.bit_field.wdtcomponentversion0;
}

__STATIC_INLINE uint32_t wdt_wdt_comp_params_5_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_5.val;
}

__STATIC_INLINE void wdt_wdt_comp_params_5_unpack(uint32_t base, uint32_t* cp_wdt_user_top_max)
{
    T_WDT_WDT_COMP_PARAMS_5 localVal = ((T_HWP_WDT_T*)(base))->wdt_comp_params_5;

    *cp_wdt_user_top_max = localVal.bit_field.cp_wdt_user_top_max;
}

__STATIC_INLINE uint32_t wdt_cp_wdt_user_top_max_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_5.bit_field.cp_wdt_user_top_max;
}

__STATIC_INLINE uint32_t wdt_wdt_comp_params_4_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_4.val;
}

__STATIC_INLINE void wdt_wdt_comp_params_4_unpack(uint32_t base, uint32_t* cp_wdt_user_top_init_max)
{
    T_WDT_WDT_COMP_PARAMS_4 localVal = ((T_HWP_WDT_T*)(base))->wdt_comp_params_4;

    *cp_wdt_user_top_init_max = localVal.bit_field.cp_wdt_user_top_init_max;
}

__STATIC_INLINE uint32_t wdt_cp_wdt_user_top_init_max_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_4.bit_field.cp_wdt_user_top_init_max;
}

__STATIC_INLINE uint32_t wdt_wdt_comp_params_3_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_3.val;
}

__STATIC_INLINE void wdt_wdt_comp_params_3_unpack(uint32_t base, uint32_t* cd_wdt_top_rst)
{
    T_WDT_WDT_COMP_PARAMS_3 localVal = ((T_HWP_WDT_T*)(base))->wdt_comp_params_3;

    *cd_wdt_top_rst = localVal.bit_field.cd_wdt_top_rst;
}

__STATIC_INLINE uint32_t wdt_cd_wdt_top_rst_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_3.bit_field.cd_wdt_top_rst;
}

__STATIC_INLINE uint32_t wdt_wdt_comp_params_2_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_2.val;
}

__STATIC_INLINE void wdt_wdt_comp_params_2_unpack(uint32_t base, uint32_t* cp_wdt_cnt_rst)
{
    T_WDT_WDT_COMP_PARAMS_2 localVal = ((T_HWP_WDT_T*)(base))->wdt_comp_params_2;

    *cp_wdt_cnt_rst = localVal.bit_field.cp_wdt_cnt_rst;
}

__STATIC_INLINE uint32_t wdt_cp_wdt_cnt_rst_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_2.bit_field.cp_wdt_cnt_rst;
}

__STATIC_INLINE uint32_t wdt_wdt_comp_params_1_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.val;
}

__STATIC_INLINE void wdt_wdt_comp_params_1_unpack(uint32_t base, uint8_t* cp_wdt_cnt_width, uint8_t* cp_wdt_dflt_top_init, uint8_t* cp_wdt_dflt_top, uint8_t* cp_wdt_dflt_rpl, uint8_t* cp_wdt_apb_data_width, uint8_t* cp_wdt_pause, uint8_t* cp_wdt_use_fix_top, uint8_t* cp_wdt_hc_top, uint8_t* cp_wdt_hc_rpl, uint8_t* cp_wdt_hc_rmod, uint8_t* cp_wdt_dual_top, uint8_t* cp_wdt_dflt_rmod, uint8_t* cp_wdt_always_en)
{
    T_WDT_WDT_COMP_PARAMS_1 localVal = ((T_HWP_WDT_T*)(base))->wdt_comp_params_1;

    *cp_wdt_cnt_width = localVal.bit_field.cp_wdt_cnt_width;
    *cp_wdt_dflt_top_init = localVal.bit_field.cp_wdt_dflt_top_init;
    *cp_wdt_dflt_top = localVal.bit_field.cp_wdt_dflt_top;
    *cp_wdt_dflt_rpl = localVal.bit_field.cp_wdt_dflt_rpl;
    *cp_wdt_apb_data_width = localVal.bit_field.cp_wdt_apb_data_width;
    *cp_wdt_pause = localVal.bit_field.cp_wdt_pause;
    *cp_wdt_use_fix_top = localVal.bit_field.cp_wdt_use_fix_top;
    *cp_wdt_hc_top = localVal.bit_field.cp_wdt_hc_top;
    *cp_wdt_hc_rpl = localVal.bit_field.cp_wdt_hc_rpl;
    *cp_wdt_hc_rmod = localVal.bit_field.cp_wdt_hc_rmod;
    *cp_wdt_dual_top = localVal.bit_field.cp_wdt_dual_top;
    *cp_wdt_dflt_rmod = localVal.bit_field.cp_wdt_dflt_rmod;
    *cp_wdt_always_en = localVal.bit_field.cp_wdt_always_en;
}

__STATIC_INLINE uint8_t wdt_cp_wdt_cnt_width_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.bit_field.cp_wdt_cnt_width;
}

__STATIC_INLINE uint8_t wdt_cp_wdt_dflt_top_init_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.bit_field.cp_wdt_dflt_top_init;
}

__STATIC_INLINE uint8_t wdt_cp_wdt_dflt_top_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.bit_field.cp_wdt_dflt_top;
}

__STATIC_INLINE uint8_t wdt_cp_wdt_dflt_rpl_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.bit_field.cp_wdt_dflt_rpl;
}

__STATIC_INLINE uint8_t wdt_cp_wdt_apb_data_width_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.bit_field.cp_wdt_apb_data_width;
}

__STATIC_INLINE uint8_t wdt_cp_wdt_pause_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.bit_field.cp_wdt_pause;
}

__STATIC_INLINE uint8_t wdt_cp_wdt_use_fix_top_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.bit_field.cp_wdt_use_fix_top;
}

__STATIC_INLINE uint8_t wdt_cp_wdt_hc_top_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.bit_field.cp_wdt_hc_top;
}

__STATIC_INLINE uint8_t wdt_cp_wdt_hc_rpl_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.bit_field.cp_wdt_hc_rpl;
}

__STATIC_INLINE uint8_t wdt_cp_wdt_hc_rmod_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.bit_field.cp_wdt_hc_rmod;
}

__STATIC_INLINE uint8_t wdt_cp_wdt_dual_top_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.bit_field.cp_wdt_dual_top;
}

__STATIC_INLINE uint8_t wdt_cp_wdt_dflt_rmod_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.bit_field.cp_wdt_dflt_rmod;
}

__STATIC_INLINE uint8_t wdt_cp_wdt_always_en_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_params_1.bit_field.cp_wdt_always_en;
}

__STATIC_INLINE uint32_t wdt_wdt_comp_version_1_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_version_1.val;
}

__STATIC_INLINE void wdt_wdt_comp_version_1_unpack(uint32_t base , uint32_t* wdtcomponentversion1)
{
    T_WDT_WDT_COMP_VERSION_1 localVal = ((T_HWP_WDT_T*)(base))->wdt_comp_version_1;

    *wdtcomponentversion1 = localVal.bit_field.wdtcomponentversion1;
}

__STATIC_INLINE uint32_t wdt_wdtcomponentversion1_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_version_1.bit_field.wdtcomponentversion1;
}

__STATIC_INLINE uint32_t wdt_wdt_comp_type_get(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_type.val;
}

__STATIC_INLINE void wdt_wdt_comp_type_unpack(uint32_t base , uint32_t* componenttyperegister)
{
    T_WDT_WDT_COMP_TYPE localVal = ((T_HWP_WDT_T*)(base))->wdt_comp_type;

    *componenttyperegister = localVal.bit_field.componenttyperegister;
}

__STATIC_INLINE uint32_t wdt_componenttyperegister_getf(uint32_t base)
{
    return ((T_HWP_WDT_T*)(base))->wdt_comp_type.bit_field.componenttyperegister;
}
#endif


