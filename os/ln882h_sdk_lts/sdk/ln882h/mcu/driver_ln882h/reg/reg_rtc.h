#ifndef __REG_RTC_H__
#define __REG_RTC_H__
//Auto-gen by fr

#include "ln882h.h"

//rtc_ccvr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t            currentcountervalue : 32; /*31: 0, When read, this register is the current value of the internal counter.
                                                            This value always is read coherently. Bits from RTC_CNT_WIDTH
                                                            to 31 are read as 0 when RTC_CNT_WIDTH is less than 31.*/
    } bit_field;
} t_rtc_rtc_ccvr;

//rtc_cmr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   countermatch : 32; /*31: 0, Interrupt Match Register. When the internal counter matches this
                                                        register, an interrupt is generated, provided interrupt generation is
                                                        enabled. When appropriate, this value is written coherently. Only when all the bytes are written is the register used by the interrupt detection logic. Bits from RTC_CNT_WIDTH and above are read and written as 0 when RTC_CNT_WIDTH is less than 31.*/
    } bit_field;
} t_rtc_rtc_cmr;

//rtc_clr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                    counterload : 32; /*31: 0, Loaded into the counter as the loaded value, which is written coherently. Bits from RTC_CNT_WIDTH and above are read and written as 0 when RTC_CNT_WIDTH is less than 31.*/
    } bit_field;
} t_rtc_rtc_clr;

//rtc_ccr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        rtc_ien :  1; /* 0: 0, Allows the user to disable interrupt generation.
                                                        0 = Interrupt disabled
                                                        1 = Interrupt enabled*/
        uint32_t                       rtc_mask :  1; /* 1: 1, Allows the user to mask a generated interrupt.
                                                        0 = Interrupt unmasked
                                                        1 = Interrupt masked*/
        uint32_t                         rtc_en :  1; /* 2: 2, Optional. Allows the user to control counting in the counter.
                                                        0 = Counter disabled
                                                        1 = Counter enabled
                                                        This bit does not exit if RTC_EN_MODE = 0. Internally, the counter always is enabled.*/
        uint32_t                        rtc_wen :  1; /* 3: 3, Optional. Allows the user to force the counter to wrap when a match occurs instead of waiting until the maximum count is reached.
                                                        0 = Wrap disabled
                                                        1 = Wrap enabled
                                                        This bit is writable only when RTC_WRAP_MODE = 1.*/
        uint32_t                     reserved_0 : 28; /*31: 4,                             NA*/
    } bit_field;
} t_rtc_rtc_ccr;

//rtc_stat
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                       rtc_stat :  1; /* 0: 0, This register is the masked raw status
                                                        0 = Interrupt is inactive
                                                        1 = Interrupt is active (regardless of polarity)*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} t_rtc_rtc_stat;

//rtc_rstat
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                      rtc_rstat :  1; /* 0: 0, 0 = Interrupt is inactive
                                                        1 = Interrupt is active (regardless of polarity)*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} t_rtc_rtc_rstat;

//rtc_eoi
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        rtc_eoi :  1; /* 0: 0, By reading this location, the match interrupt is cleared. Performing read-to-clear on interrupts, the interrupt is cleared at the end of the read.*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} t_rtc_rtc_eoi;

//rtc_comp_version
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t               rtc_comp_version : 32; /*31: 0, ASCII value for each number in the version, followed by *. For example 32_30_31_2A represents the version 2.01*.*/
    } bit_field;
} t_rtc_rtc_comp_version;

//Registers Mapping to the same address

typedef struct
{
    volatile                  t_rtc_rtc_ccvr                       rtc_ccvr; /*  0x0,    RO, 0x00000000, Current Counter Value Register*/
    volatile                   t_rtc_rtc_cmr                        rtc_cmr; /*  0x4,    RW, 0x00000000,          Counter Load Register*/
    volatile                   t_rtc_rtc_clr                        rtc_clr; /*  0x8,    RW, 0x00000000,          Counter Load Register*/
    volatile                   t_rtc_rtc_ccr                        rtc_ccr; /*  0xc,    RW, 0x00000000,       Counter Control Register*/
    volatile                  t_rtc_rtc_stat                       rtc_stat; /* 0x10,    RO, 0x00000000,      Interrupt Status Register*/
    volatile                 t_rtc_rtc_rstat                      rtc_rstat; /* 0x14,    RO, 0x00000000,  Interrupt Raw Status Register*/
    volatile                   t_rtc_rtc_eoi                        rtc_eoi; /* 0x18,    RO, 0x00000000,      End of Interrupt Register*/
    volatile          t_rtc_rtc_comp_version               rtc_comp_version; /* 0x1c,    RO, 0x3230352A,     Component Version Register*/
} t_hwp_rtc_t;



__STATIC_INLINE uint32_t rtc_rtc_ccvr_get(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_ccvr.val;
}

__STATIC_INLINE uint32_t rtc_currentcountervalue_getf(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_ccvr.bit_field.currentcountervalue;
}

__STATIC_INLINE uint32_t rtc_rtc_cmr_get(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_cmr.val;
}

__STATIC_INLINE void rtc_rtc_cmr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_rtc_t*)(base))->rtc_cmr.val = value;
}

__STATIC_INLINE uint32_t rtc_countermatch_getf(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_cmr.bit_field.countermatch;
}

__STATIC_INLINE void rtc_countermatch_setf(uint32_t base, uint32_t countermatch)
{
    ((t_hwp_rtc_t*)(base))->rtc_cmr.bit_field.countermatch = countermatch;
}

__STATIC_INLINE uint32_t rtc_rtc_clr_get(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_clr.val;
}

__STATIC_INLINE void rtc_rtc_clr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_rtc_t*)(base))->rtc_clr.val = value;
}

__STATIC_INLINE uint32_t rtc_counterload_getf(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_clr.bit_field.counterload;
}

__STATIC_INLINE void rtc_counterload_setf(uint32_t base, uint32_t counterload)
{
    ((t_hwp_rtc_t*)(base))->rtc_clr.bit_field.counterload = counterload;
}

__STATIC_INLINE uint32_t rtc_rtc_ccr_get(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_ccr.val;
}

__STATIC_INLINE void rtc_rtc_ccr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_rtc_t*)(base))->rtc_ccr.val = value;
}

__STATIC_INLINE uint8_t rtc_rtc_wen_getf(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_ccr.bit_field.rtc_wen;
}

__STATIC_INLINE void rtc_rtc_wen_setf(uint32_t base, uint8_t rtc_wen)
{
    ((t_hwp_rtc_t*)(base))->rtc_ccr.bit_field.rtc_wen = rtc_wen;
}

__STATIC_INLINE uint8_t rtc_rtc_en_getf(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_ccr.bit_field.rtc_en;
}

__STATIC_INLINE void rtc_rtc_en_setf(uint32_t base, uint8_t rtc_en)
{
    ((t_hwp_rtc_t*)(base))->rtc_ccr.bit_field.rtc_en = rtc_en;
}

__STATIC_INLINE uint8_t rtc_rtc_mask_getf(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_ccr.bit_field.rtc_mask;
}

__STATIC_INLINE void rtc_rtc_mask_setf(uint32_t base, uint8_t rtc_mask)
{
    ((t_hwp_rtc_t*)(base))->rtc_ccr.bit_field.rtc_mask = rtc_mask;
}

__STATIC_INLINE uint8_t rtc_rtc_ien_getf(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_ccr.bit_field.rtc_ien;
}

__STATIC_INLINE void rtc_rtc_ien_setf(uint32_t base, uint8_t rtc_ien)
{
    ((t_hwp_rtc_t*)(base))->rtc_ccr.bit_field.rtc_ien = rtc_ien;
}


__STATIC_INLINE uint32_t rtc_rtc_stat_get(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_stat.val;
}

__STATIC_INLINE uint8_t rtc_rtc_stat_getf(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_stat.bit_field.rtc_stat;
}

__STATIC_INLINE uint32_t rtc_rtc_rstat_get(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_rstat.val;
}

__STATIC_INLINE uint8_t rtc_rtc_rstat_getf(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_rstat.bit_field.rtc_rstat;
}

__STATIC_INLINE uint32_t rtc_rtc_eoi_get(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_eoi.val;
}

__STATIC_INLINE uint8_t rtc_rtc_eoi_getf(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_eoi.bit_field.rtc_eoi;
}

__STATIC_INLINE uint32_t rtc_rtc_comp_version_get(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_comp_version.val;
}

__STATIC_INLINE uint32_t rtc_rtc_comp_version_getf(uint32_t base)
{
    return ((t_hwp_rtc_t*)(base))->rtc_comp_version.bit_field.rtc_comp_version;
}
#endif


