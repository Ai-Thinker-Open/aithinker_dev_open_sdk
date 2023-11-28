#ifndef __REG_PWM_H__
#define __REG_PWM_H__

#include "ln882h.h"

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    ise0_load            : 1  ; // 0  : 0
        uint32_t    ise0_trig            : 1  ; // 1  : 1
        uint32_t    ise0_cmpa            : 1  ; // 2  : 2
        uint32_t    ise0_cmpb            : 1  ; // 3  : 3

        uint32_t    ise1_load            : 1  ; // 4  : 4
        uint32_t    ise1_trig            : 1  ; // 5  : 5
        uint32_t    ise1_cmpa            : 1  ; // 6  : 6
        uint32_t    ise1_cmpb            : 1  ; // 7  : 7

        uint32_t    ise2_load            : 1  ; // 8  : 8
        uint32_t    ise2_trig            : 1  ; // 9  : 9
        uint32_t    ise2_cmpa            : 1  ; // 10  : 10
        uint32_t    ise2_cmpb            : 1  ; // 11  : 11

        uint32_t    ise3_load            : 1  ; // 12  : 12
        uint32_t    ise3_trig            : 1  ; // 13  : 13
        uint32_t    ise3_cmpa            : 1  ; // 14  : 14
        uint32_t    ise3_cmpb            : 1  ; // 15  : 15

        uint32_t    ise4_load            : 1  ; // 16  : 16
        uint32_t    ise4_trig            : 1  ; // 17  : 17
        uint32_t    ise4_cmpa            : 1  ; // 18  : 18
        uint32_t    ise4_cmpb            : 1  ; // 19  : 19

        uint32_t    ise5_load            : 1  ; // 20  : 20
        uint32_t    ise5_trig            : 1  ; // 21  : 21
        uint32_t    ise5_cmpa            : 1  ; // 22  : 22
        uint32_t    ise5_cmpb            : 1  ; // 23  : 23

        uint32_t    reserved_0           : 8  ; // 31 : 24
    } bit_field;
} t_pwm_ise;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    isrr0_load            : 1  ; // 0  : 0
        uint32_t    isrr0_trig            : 1  ; // 1  : 1
        uint32_t    isrr0_cmpa            : 1  ; // 2  : 2
        uint32_t    isrr0_cmpb            : 1  ; // 3  : 3

        uint32_t    isrr1_load            : 1  ; // 4  : 4
        uint32_t    isrr1_trig            : 1  ; // 5  : 5
        uint32_t    isrr1_cmpa            : 1  ; // 6  : 6
        uint32_t    isrr1_cmpb            : 1  ; // 7  : 7

        uint32_t    isrr2_load            : 1  ; // 8  : 8
        uint32_t    isrr2_trig            : 1  ; // 9  : 9
        uint32_t    isrr2_cmpa            : 1  ; // 10  : 10
        uint32_t    isrr2_cmpb            : 1  ; // 11  : 11

        uint32_t    isrr3_load            : 1  ; // 12  : 12
        uint32_t    isrr3_trig            : 1  ; // 13  : 13
        uint32_t    isrr3_cmpa            : 1  ; // 14  : 14
        uint32_t    isrr3_cmpb            : 1  ; // 15  : 15

        uint32_t    isrr4_load            : 1  ; // 16  : 16
        uint32_t    isrr4_trig            : 1  ; // 17  : 17
        uint32_t    isrr4_cmpa            : 1  ; // 18  : 18
        uint32_t    isrr4_cmpb            : 1  ; // 19  : 19

        uint32_t    isrr5_load            : 1  ; // 20  : 20
        uint32_t    isrr5_trig            : 1  ; // 21  : 21
        uint32_t    isrr5_cmpa            : 1  ; // 22  : 22
        uint32_t    isrr5_cmpb            : 1  ; // 23  : 23

        uint32_t    reserved_0          : 8  ; // 31 : 24

    } bit_field;
} t_pwm_isrr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    isr0_load            : 1  ; // 0  : 0
        uint32_t    isr0_trig            : 1  ; // 1  : 1
        uint32_t    isr0_cmpa            : 1  ; // 2  : 2
        uint32_t    isr0_cmpb            : 1  ; // 3  : 3

        uint32_t    isr1_load            : 1  ; // 4  : 4
        uint32_t    isr1_trig            : 1  ; // 5  : 5
        uint32_t    isr1_cmpa            : 1  ; // 6  : 6
        uint32_t    isr1_cmpb            : 1  ; // 7  : 7

        uint32_t    isr2_load            : 1  ; // 8  : 8
        uint32_t    isr2_trig            : 1  ; // 9  : 9
        uint32_t    isr2_cmpa            : 1  ; // 10  : 10
        uint32_t    isr2_cmpb            : 1  ; // 11  : 11

        uint32_t    isr3_load            : 1  ; // 12  : 12
        uint32_t    isr3_trig            : 1  ; // 13  : 13
        uint32_t    isr3_cmpa            : 1  ; // 14  : 14
        uint32_t    isr3_cmpb            : 1  ; // 15  : 15

        uint32_t    isr4_load            : 1  ; // 16  : 16
        uint32_t    isr4_trig            : 1  ; // 17  : 17
        uint32_t    isr4_cmpa            : 1  ; // 18  : 18
        uint32_t    isr4_cmpb            : 1  ; // 19  : 19

        uint32_t    isr5_load            : 1  ; // 20  : 20
        uint32_t    isr5_trig            : 1  ; // 21  : 21
        uint32_t    isr5_cmpa            : 1  ; // 22  : 22
        uint32_t    isr5_cmpb            : 1  ; // 23  : 23

        uint32_t    reserved_0          : 8  ; // 31 : 24

    } bit_field;
} t_pwm_isr;


//--------------------------------------------------------------------------------

typedef struct
{
    volatile        t_pwm_ise                                ise                                    ; // 0x0
    volatile        t_pwm_isrr                               isrr                                   ; // 0x4
    volatile        t_pwm_isr                                isr                                    ; // 0x8
} t_hwp_pwm_t0;


__STATIC_INLINE void pwm_ise_set(uint32_t base , uint32_t value)
{
    ((t_hwp_pwm_t0*)(base))->ise.val = value;
}

__STATIC_INLINE void pwm_ise0_load_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise0_load = ise;
}

__STATIC_INLINE void pwm_ise0_trig_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise0_trig = ise;
}

__STATIC_INLINE void pwm_ise0_cmpa_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise0_cmpa = ise;
}

__STATIC_INLINE void pwm_ise0_cmpb_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise0_cmpb = ise;
}

__STATIC_INLINE void pwm_ise1_load_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise1_load = ise;
}


__STATIC_INLINE void pwm_ise1_trig_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise1_trig = ise;
}

__STATIC_INLINE void pwm_ise1_cmpa_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise1_cmpa = ise;
}

__STATIC_INLINE void pwm_ise1_cmpb_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise1_cmpb = ise;
}

__STATIC_INLINE void pwm_ise2_load_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise2_load = ise;
}


__STATIC_INLINE void pwm_ise2_trig_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise2_trig = ise;
}

__STATIC_INLINE void pwm_ise2_cmpa_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise2_cmpa = ise;
}

__STATIC_INLINE void pwm_ise2_cmpb_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise2_cmpb = ise;
}

__STATIC_INLINE void pwm_ise3_load_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise3_load = ise;
}


__STATIC_INLINE void pwm_ise3_trig_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise3_trig = ise;
}

__STATIC_INLINE void pwm_ise3_cmpa_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise3_cmpa = ise;
}

__STATIC_INLINE void pwm_ise3_cmpb_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise3_cmpb = ise;
}

__STATIC_INLINE void pwm_ise4_load_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise4_load = ise;
}


__STATIC_INLINE void pwm_ise4_trig_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise4_trig = ise;
}

__STATIC_INLINE void pwm_ise4_cmpa_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise4_cmpa = ise;
}

__STATIC_INLINE void pwm_ise4_cmpb_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise4_cmpb = ise;
}

__STATIC_INLINE void pwm_ise5_load_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise5_load = ise;
}


__STATIC_INLINE void pwm_ise5_trig_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise5_trig = ise;
}

__STATIC_INLINE void pwm_ise5_cmpa_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise5_cmpa = ise;
}

__STATIC_INLINE void pwm_ise5_cmpb_setf(uint32_t base , uint32_t ise)
{
    ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise5_cmpb = ise;
}





__STATIC_INLINE void pwm_isr_set(uint32_t base , uint32_t value)
{
    ((t_hwp_pwm_t0*)(base))->isr.val = value;
}

__STATIC_INLINE void pwm_isr0_load_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr0_load = isr;
}

__STATIC_INLINE void pwm_isr0_trig_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr0_trig = isr;
}

__STATIC_INLINE void pwm_isr0_cmpa_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr0_cmpa = isr;
}

__STATIC_INLINE void pwm_isr0_cmpb_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr0_cmpb = isr;
}

__STATIC_INLINE void pwm_isr1_load_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr1_load = isr;
}

__STATIC_INLINE void pwm_isr1_trig_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr1_trig = isr;
}

__STATIC_INLINE void pwm_isr1_cmpa_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr1_cmpa = isr;
}

__STATIC_INLINE void pwm_isr1_cmpb_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr1_cmpb = isr;
}

__STATIC_INLINE void pwm_isr2_load_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr2_load = isr;
}


__STATIC_INLINE void pwm_isr2_trig_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr2_trig = isr;
}

__STATIC_INLINE void pwm_isr2_cmpa_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr2_cmpa = isr;
}

__STATIC_INLINE void pwm_isr2_cmpb_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr2_cmpb = isr;
}

__STATIC_INLINE void pwm_isr3_load_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr3_load = isr;
}


__STATIC_INLINE void pwm_isr3_trig_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr3_trig = isr;
}

__STATIC_INLINE void pwm_isr3_cmpa_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr3_cmpa = isr;
}

__STATIC_INLINE void pwm_isr3_cmpb_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr3_cmpb = isr;
}

__STATIC_INLINE void pwm_isr4_load_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr4_load = isr;
}


__STATIC_INLINE void pwm_isr4_trig_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr4_trig = isr;
}

__STATIC_INLINE void pwm_isr4_cmpa_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr4_cmpa = isr;
}

__STATIC_INLINE void pwm_isr4_cmpb_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr4_cmpb = isr;
}

__STATIC_INLINE void pwm_isr5_load_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr5_load = isr;
}


__STATIC_INLINE void pwm_isr5_trig_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr5_trig = isr;
}

__STATIC_INLINE void pwm_isr5_cmpa_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr5_cmpa = isr;
}

__STATIC_INLINE void pwm_isr5_cmpb_setf(uint32_t base , uint32_t isr)
{
    ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr5_cmpb = isr;
}

/**********************************************************************************/
__STATIC_INLINE void pwm_isrr_set(uint32_t base , uint32_t value)
{
    ((t_hwp_pwm_t0*)(base))->isrr.val = value;
}


__STATIC_INLINE void pwm_isrr0_load_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr0_load = isrr;
}

__STATIC_INLINE void pwm_isrr0_trig_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr0_trig = isrr;
}

__STATIC_INLINE void pwm_isrr0_cmpa_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr0_cmpa = isrr;
}

__STATIC_INLINE void pwm_isrr0_cmpb_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr0_cmpb = isrr;
}

__STATIC_INLINE void pwm_isrr1_load_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr1_load = isrr;
}

__STATIC_INLINE void pwm_isrr1_trig_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr1_trig = isrr;
}

__STATIC_INLINE void pwm_isrr1_cmpa_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr1_cmpa = isrr;
}

__STATIC_INLINE void pwm_isrr1_cmpb_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr1_cmpb = isrr;
}

__STATIC_INLINE void pwm_isrr2_load_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr2_load = isrr;
}


__STATIC_INLINE void pwm_isrr2_trig_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr2_trig = isrr;
}

__STATIC_INLINE void pwm_isrr2_cmpa_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr2_cmpa = isrr;
}

__STATIC_INLINE void pwm_isrr2_cmpb_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr2_cmpb = isrr;
}

__STATIC_INLINE void pwm_isrr3_load_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr3_load = isrr;
}


__STATIC_INLINE void pwm_isrr3_trig_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr3_trig = isrr;
}

__STATIC_INLINE void pwm_isrr3_cmpa_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr3_cmpa = isrr;
}

__STATIC_INLINE void pwm_isrr3_cmpb_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr3_cmpb = isrr;
}

__STATIC_INLINE void pwm_isrr4_load_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr4_load = isrr;
}


__STATIC_INLINE void pwm_isrr4_trig_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr4_trig = isrr;
}

__STATIC_INLINE void pwm_isrr4_cmpa_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr4_cmpa = isrr;
}

__STATIC_INLINE void pwm_isrr4_cmpb_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr4_cmpb = isrr;
}

__STATIC_INLINE void pwm_isrr5_load_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr5_load = isrr;
}


__STATIC_INLINE void pwm_isrr5_trig_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr5_trig = isrr;
}

__STATIC_INLINE void pwm_isrr5_cmpa_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr5_cmpa = isrr;
}

__STATIC_INLINE void pwm_isrr5_cmpb_setf(uint32_t base , uint32_t isrr)
{
    ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr5_cmpb = isrr;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE uint32_t pwm_ise_get(uint32_t base)
{
    return ((t_hwp_pwm_t0*)(base))->ise.val;
}

__STATIC_INLINE uint8_t pwm_ise0_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise0_load;
}

__STATIC_INLINE uint8_t pwm_ise0_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise0_trig;
}

__STATIC_INLINE uint8_t pwm_ise0_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise0_cmpa;
}

__STATIC_INLINE uint8_t pwm_ise0_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise0_cmpb ;
}

__STATIC_INLINE uint8_t pwm_ise1_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise1_load;
}

__STATIC_INLINE uint8_t pwm_ise1_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise1_trig;
}

__STATIC_INLINE uint8_t pwm_ise1_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise1_cmpa;
}

__STATIC_INLINE uint8_t pwm_ise1_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise1_cmpb ;
}

__STATIC_INLINE uint8_t pwm_ise2_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise2_load;
}

__STATIC_INLINE uint8_t pwm_ise2_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise2_trig;
}

__STATIC_INLINE uint8_t pwm_ise2_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise2_cmpa;
}

__STATIC_INLINE uint8_t pwm_ise2_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise2_cmpb ;
}

__STATIC_INLINE uint8_t pwm_ise3_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise3_load;
}

__STATIC_INLINE uint8_t pwm_ise3_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise3_trig;
}

__STATIC_INLINE uint8_t pwm_ise3_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise3_cmpa;
}

__STATIC_INLINE uint8_t pwm_ise3_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise3_cmpb ;
}

__STATIC_INLINE uint8_t pwm_ise4_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise4_load;
}

__STATIC_INLINE uint8_t pwm_ise4_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise4_trig;
}

__STATIC_INLINE uint8_t pwm_ise4_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise4_cmpa;
}

__STATIC_INLINE uint8_t pwm_ise4_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise4_cmpb ;
}

__STATIC_INLINE uint8_t pwm_ise5_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise5_load;
}

__STATIC_INLINE uint8_t pwm_ise5_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise5_trig;
}

__STATIC_INLINE uint8_t pwm_ise5_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise5_cmpa;
}

__STATIC_INLINE uint8_t pwm_ise5_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->ise.bit_field.ise5_cmpb ;
}



__STATIC_INLINE uint32_t pwm_isr_get(uint32_t base)
{
    return ((t_hwp_pwm_t0*)(base))->isr.val;
}

__STATIC_INLINE uint8_t pwm_isr0_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr0_load;
}

__STATIC_INLINE uint8_t pwm_isr0_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr0_trig;
}

__STATIC_INLINE uint8_t pwm_isr0_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr0_cmpa;
}

__STATIC_INLINE uint8_t pwm_isr0_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr0_cmpb ;
}

__STATIC_INLINE uint8_t pwm_isr1_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr1_load;
}

__STATIC_INLINE uint8_t pwm_isr1_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr1_trig;
}

__STATIC_INLINE uint8_t pwm_isr1_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr1_cmpa;
}

__STATIC_INLINE uint8_t pwm_isr1_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr1_cmpb ;
}

__STATIC_INLINE uint8_t pwm_isr2_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr2_load;
}

__STATIC_INLINE uint8_t pwm_isr2_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr2_trig;
}

__STATIC_INLINE uint8_t pwm_isr2_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr2_cmpa;
}

__STATIC_INLINE uint8_t pwm_isr2_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr2_cmpb ;
}

__STATIC_INLINE uint8_t pwm_isr3_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr3_load;
}

__STATIC_INLINE uint8_t pwm_isr3_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr3_trig;
}

__STATIC_INLINE uint8_t pwm_isr3_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr3_cmpa;
}

__STATIC_INLINE uint8_t pwm_isr3_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr3_cmpb ;
}

__STATIC_INLINE uint8_t pwm_isr4_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr4_load;
}

__STATIC_INLINE uint8_t pwm_isr4_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr4_trig;
}

__STATIC_INLINE uint8_t pwm_isr4_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr4_cmpa;
}

__STATIC_INLINE uint8_t pwm_isr4_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr4_cmpb ;
}

__STATIC_INLINE uint8_t pwm_isr5_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr5_load;
}

__STATIC_INLINE uint8_t pwm_isr5_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr5_trig;
}

__STATIC_INLINE uint8_t pwm_isr5_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr5_cmpa;
}

__STATIC_INLINE uint8_t pwm_isr5_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isr.bit_field.isr5_cmpb ;
}


__STATIC_INLINE uint32_t pwm_isrr_get(uint32_t base)
{
    return ((t_hwp_pwm_t0*)(base))->isrr.val;
}

__STATIC_INLINE uint8_t pwm_isrr0_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr0_load;
}

__STATIC_INLINE uint8_t pwm_isrr0_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr0_trig;
}

__STATIC_INLINE uint8_t pwm_isrr0_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr0_cmpa;
}

__STATIC_INLINE uint8_t pwm_isrr0_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr0_cmpb ;
}

__STATIC_INLINE uint8_t pwm_isrr1_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr1_load;
}

__STATIC_INLINE uint8_t pwm_isrr1_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr1_trig;
}

__STATIC_INLINE uint8_t pwm_isrr1_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr1_cmpa;
}

__STATIC_INLINE uint8_t pwm_isrr1_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr1_cmpb ;
}

__STATIC_INLINE uint8_t pwm_isrr2_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr2_load;
}

__STATIC_INLINE uint8_t pwm_isrr2_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr2_trig;
}

__STATIC_INLINE uint8_t pwm_isrr2_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr2_cmpa;
}

__STATIC_INLINE uint8_t pwm_isrr2_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr2_cmpb ;
}

__STATIC_INLINE uint8_t pwm_isrr3_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr3_load;
}

__STATIC_INLINE uint8_t pwm_isrr3_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr3_trig;
}

__STATIC_INLINE uint8_t pwm_isrr3_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr3_cmpa;
}

__STATIC_INLINE uint8_t pwm_isrr3_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr3_cmpb ;
}

__STATIC_INLINE uint8_t pwm_isrr4_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr4_load;
}

__STATIC_INLINE uint8_t pwm_isrr4_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr4_trig;
}

__STATIC_INLINE uint8_t pwm_isrr4_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr4_cmpa;
}

__STATIC_INLINE uint8_t pwm_isrr4_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr4_cmpb ;
}

__STATIC_INLINE uint8_t pwm_isrr5_load_getf(uint32_t base )
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr5_load;
}

__STATIC_INLINE uint8_t pwm_isrr5_trig_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr5_trig;
}

__STATIC_INLINE uint8_t pwm_isrr5_cmpa_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr5_cmpa;
}

__STATIC_INLINE uint8_t pwm_isrr5_cmpb_getf(uint32_t base)
{
   return ((t_hwp_pwm_t0*)(base))->isrr.bit_field.isrr5_cmpb ;
}

//--------------------------------------------------------------------------------

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pwm_ena             : 1  ; // 0  : 0
        uint32_t    pwm_enb             : 1  ; // 1  : 1
        uint32_t    dben                : 1  ; // 2  : 2
        uint32_t    tim_en              : 1  ; // 3  : 3
        uint32_t    cap_en              : 1  ; // 4  : 4
        uint32_t    reserved_0          : 27 ; // 31 : 5
    } bit_field;
} t_pwm_cr;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pwm_inva            : 1  ; // 0  : 0
        uint32_t    pwm_invb            : 1  ; // 1  : 1
        uint32_t    cnt_mode            : 2  ; // 3  : 2
        uint32_t    trig_mod            : 2  ; // 5  : 4
        uint32_t    trig_sel            : 3  ; // 8  : 6
        uint32_t    trig_clr_en         : 1  ; // 9  : 9
        uint32_t    cap_edg             : 2  ; // 11 : 10
        uint32_t    cap_mod             : 1  ; // 12 : 12
        uint32_t    div                 : 6  ; // 18 : 13
        uint32_t    tim_once            : 1  ; // 19 : 19
        uint32_t    int_cmp_a_mod       : 2  ; // 21 : 20
        uint32_t    int_cmp_b_mod       : 2  ; // 23 : 22
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_pwm_cfg;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    load                : 16 ; // 15 : 0
        uint32_t    pwm_db              : 12 ; // 27 : 16
        uint32_t    reserved_0          : 4  ; // 31 : 28
    } bit_field;
} t_pwm_load;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    cmp_a               : 16 ; // 15 : 0
        uint32_t    cmp_b               : 16 ; // 31 : 16
    } bit_field;
} t_pwm_cmp;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    pwm_trig            : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_pwm_trig;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    count               : 16 ; // 15 : 0
        uint32_t    reserved_0          : 16 ; // 31 : 16
    } bit_field;
} t_pwm_count;


//--------------------------------------------------------------------------------

typedef struct
{
    volatile        t_pwm_cr                                 cr                                     ; // 0x0
    volatile        t_pwm_cfg                                cfg                                    ; // 0x4
    volatile        t_pwm_load                               load                                   ; // 0x8
    volatile        t_pwm_cmp                                cmp                                    ; // 0xc
    volatile        t_pwm_trig                               trig                                   ; // 0x10
    volatile        t_pwm_count                              count                                  ; // 0x14
} t_hwp_pwm_t;


//--------------------------------------------------------------------------------

__STATIC_INLINE void pwm_cr_set(uint32_t base, uint32_t value)
{
    ((t_hwp_pwm_t*)(base))->cr.val = value;
}

__STATIC_INLINE void pwm_cap_en_setf(uint32_t base, uint8_t cap_en)
{
    ((t_hwp_pwm_t*)(base))->cr.bit_field.cap_en = cap_en;
}

__STATIC_INLINE void pwm_tim_en_setf(uint32_t base, uint8_t tim_en)
{
    ((t_hwp_pwm_t*)(base))->cr.bit_field.tim_en = tim_en;
}

__STATIC_INLINE void pwm_dben_setf(uint32_t base, uint8_t dben)
{
    ((t_hwp_pwm_t*)(base))->cr.bit_field.dben = dben;
}

__STATIC_INLINE void pwm_enb_setf(uint32_t base, uint8_t pwm_enb)
{
    ((t_hwp_pwm_t*)(base))->cr.bit_field.pwm_enb = pwm_enb;
}

__STATIC_INLINE void pwm_ena_setf(uint32_t base, uint8_t pwm_ena)
{
    ((t_hwp_pwm_t*)(base))->cr.bit_field.pwm_ena = pwm_ena;
}

__STATIC_INLINE void pwm_cfg_set(uint32_t base, uint32_t value)
{
    ((t_hwp_pwm_t*)(base))->cfg.val = value;
}

__STATIC_INLINE void pwm_int_cmp_b_mod_setf(uint32_t base, uint8_t int_cmp_b_mod)
{
    ((t_hwp_pwm_t*)(base))->cfg.bit_field.int_cmp_b_mod = int_cmp_b_mod;
}

__STATIC_INLINE void pwm_int_cmp_a_mod_setf(uint32_t base, uint8_t int_cmp_a_mod)
{
    ((t_hwp_pwm_t*)(base))->cfg.bit_field.int_cmp_a_mod = int_cmp_a_mod;
}

__STATIC_INLINE void pwm_tim_once_setf(uint32_t base, uint8_t tim_once)
{
    ((t_hwp_pwm_t*)(base))->cfg.bit_field.tim_once = tim_once;
}

__STATIC_INLINE void pwm_div_setf(uint32_t base, uint8_t div)
{
    ((t_hwp_pwm_t*)(base))->cfg.bit_field.div = div;
}

__STATIC_INLINE void pwm_cap_mod_setf(uint32_t base, uint8_t cap_mod)
{
    ((t_hwp_pwm_t*)(base))->cfg.bit_field.cap_mod = cap_mod;
}

__STATIC_INLINE void pwm_cap_edg_setf(uint32_t base, uint8_t cap_edg)
{
    ((t_hwp_pwm_t*)(base))->cfg.bit_field.cap_edg = cap_edg;
}

__STATIC_INLINE void pwm_trig_clr_en_setf(uint32_t base, uint8_t trig_clr_en)
{
    ((t_hwp_pwm_t*)(base))->cfg.bit_field.trig_clr_en = trig_clr_en;
}

__STATIC_INLINE void pwm_trig_sel_setf(uint32_t base, uint8_t trig_sel)
{
    ((t_hwp_pwm_t*)(base))->cfg.bit_field.trig_sel = trig_sel;
}

__STATIC_INLINE void pwm_trig_mod_setf(uint32_t base, uint8_t trig_mod)
{
    ((t_hwp_pwm_t*)(base))->cfg.bit_field.trig_mod = trig_mod;
}

__STATIC_INLINE void pwm_cnt_mode_setf(uint32_t base, uint8_t cnt_mode)
{
    ((t_hwp_pwm_t*)(base))->cfg.bit_field.cnt_mode = cnt_mode;
}

__STATIC_INLINE void pwm_invb_setf(uint32_t base, uint8_t pwm_invb)
{
    ((t_hwp_pwm_t*)(base))->cfg.bit_field.pwm_invb = pwm_invb;
}

__STATIC_INLINE void pwm_inva_setf(uint32_t base, uint8_t pwm_inva)
{
    ((t_hwp_pwm_t*)(base))->cfg.bit_field.pwm_inva = pwm_inva;
}

__STATIC_INLINE void pwm_load_set(uint32_t base, uint32_t value)
{
    ((t_hwp_pwm_t*)(base))->load.val = value;
}

__STATIC_INLINE void pwm_db_setf(uint32_t base, uint16_t pwm_db)
{
    ((t_hwp_pwm_t*)(base))->load.bit_field.pwm_db = pwm_db;
}

__STATIC_INLINE void pwm_load_setf(uint32_t base, uint16_t load)
{
    ((t_hwp_pwm_t*)(base))->load.bit_field.load = load;
}

__STATIC_INLINE void pwm_cmp_set(uint32_t base, uint32_t value)
{
    ((t_hwp_pwm_t*)(base))->cmp.val = value;
}

__STATIC_INLINE void pwm_cmp_b_setf(uint32_t base, uint16_t cmp_b)
{
    ((t_hwp_pwm_t*)(base))->cmp.bit_field.cmp_b = cmp_b;
}

__STATIC_INLINE void pwm_cmp_a_setf(uint32_t base, uint16_t cmp_a)
{
    ((t_hwp_pwm_t*)(base))->cmp.bit_field.cmp_a = cmp_a;
}

__STATIC_INLINE void pwm_trig_set(uint32_t base, uint32_t value)
{
    ((t_hwp_pwm_t*)(base))->trig.val = value;
}

__STATIC_INLINE void pwm_trig_setf(uint32_t base, uint16_t pwm_trig)
{
    ((t_hwp_pwm_t*)(base))->trig.bit_field.pwm_trig = pwm_trig;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE uint32_t pwm_cr_get(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cr.val;
}

__STATIC_INLINE uint8_t pwm_cap_en_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cr.bit_field.cap_en;
}

__STATIC_INLINE uint8_t pwm_tim_en_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cr.bit_field.tim_en;
}

__STATIC_INLINE uint8_t pwm_dben_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cr.bit_field.dben;
}

__STATIC_INLINE uint8_t pwm_enb_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cr.bit_field.pwm_enb;
}

__STATIC_INLINE uint8_t pwm_ena_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cr.bit_field.pwm_ena;
}

__STATIC_INLINE uint32_t pwm_cfg_get(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cfg.val;
}

__STATIC_INLINE uint8_t pwm_int_cmp_b_mod_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cfg.bit_field.int_cmp_b_mod;
}

__STATIC_INLINE uint8_t pwm_int_cmp_a_mod_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cfg.bit_field.int_cmp_a_mod;
}

__STATIC_INLINE uint8_t pwm_tim_once_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cfg.bit_field.tim_once;
}

__STATIC_INLINE uint8_t pwm_div_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cfg.bit_field.div;
}

__STATIC_INLINE uint8_t pwm_cap_mod_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cfg.bit_field.cap_mod;
}

__STATIC_INLINE uint8_t pwm_cap_edg_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cfg.bit_field.cap_edg;
}

__STATIC_INLINE uint8_t pwm_trig_clr_en_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cfg.bit_field.trig_clr_en;
}

__STATIC_INLINE uint8_t pwm_trig_sel_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cfg.bit_field.trig_sel;
}

__STATIC_INLINE uint8_t pwm_trig_mod_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cfg.bit_field.trig_mod;
}

__STATIC_INLINE uint8_t pwm_cnt_mode_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cfg.bit_field.cnt_mode;
}

__STATIC_INLINE uint8_t pwm_invb_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cfg.bit_field.pwm_invb;
}

__STATIC_INLINE uint8_t pwm_inva_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cfg.bit_field.pwm_inva;
}

__STATIC_INLINE uint32_t pwm_load_get(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->load.val;
}

__STATIC_INLINE uint16_t pwm_db_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->load.bit_field.pwm_db;
}

__STATIC_INLINE uint16_t pwm_load_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->load.bit_field.load;
}

__STATIC_INLINE uint32_t pwm_cmp_get(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cmp.val;
}

__STATIC_INLINE uint16_t pwm_cmp_b_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cmp.bit_field.cmp_b;
}

__STATIC_INLINE uint16_t pwm_cmp_a_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->cmp.bit_field.cmp_a;
}

__STATIC_INLINE uint32_t pwm_trig_get(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->trig.val;
}

__STATIC_INLINE uint16_t pwm_trig_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->trig.bit_field.pwm_trig;
}

__STATIC_INLINE uint32_t pwm_count_get(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->count.val;
}

__STATIC_INLINE uint16_t pwm_count_getf(uint32_t base)
{
    return ((t_hwp_pwm_t*)(base))->count.bit_field.count;
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void pwm_cr_pack(uint32_t base, uint8_t cap_en, uint8_t tim_en, uint8_t dben, uint8_t pwm_enb, uint8_t pwm_ena)
{
    ((t_hwp_pwm_t*)(base))->cr.val = ( \
                                    ((uint32_t )cap_en            << 4) \
                                  | ((uint32_t )tim_en            << 3) \
                                  | ((uint32_t )dben              << 2) \
                                  | ((uint32_t )pwm_enb           << 1) \
                                  | ((uint32_t )pwm_ena) \
                                  );
}

__STATIC_INLINE void pwm_cfg_pack(uint32_t base, uint8_t int_cmp_b_mod, uint8_t int_cmp_a_mod, uint8_t tim_once, uint8_t div, uint8_t cap_mod, uint8_t cap_edg, uint8_t trig_clr_en, uint8_t trig_sel, uint8_t trig_mod, uint8_t cnt_mode, uint8_t pwm_invb, uint8_t pwm_inva)
{
    ((t_hwp_pwm_t*)(base))->cfg.val = ( \
                                    ((uint32_t )int_cmp_b_mod     << 22) \
                                  | ((uint32_t )int_cmp_a_mod     << 20) \
                                  | ((uint32_t )tim_once          << 19) \
                                  | ((uint32_t )div               << 13) \
                                  | ((uint32_t )cap_mod           << 12) \
                                  | ((uint32_t )cap_edg           << 10) \
                                  | ((uint32_t )trig_clr_en       << 9) \
                                  | ((uint32_t )trig_sel          << 6) \
                                  | ((uint32_t )trig_mod          << 4) \
                                  | ((uint32_t )cnt_mode          << 2) \
                                  | ((uint32_t )pwm_invb          << 1) \
                                  | ((uint32_t )pwm_inva) \
                                  );
}

__STATIC_INLINE void pwm_load_pack(uint32_t base, uint16_t pwm_db, uint16_t load)
{
    ((t_hwp_pwm_t*)(base))->load.val = ( \
                                    ((uint32_t )pwm_db            << 16) \
                                  | ((uint32_t )load) \
                                  );
}

__STATIC_INLINE void pwm_cmp_pack(uint32_t base, uint16_t cmp_b, uint16_t cmp_a)
{
    ((t_hwp_pwm_t*)(base))->cmp.val = ( \
                                    ((uint32_t )cmp_b             << 16) \
                                  | ((uint32_t )cmp_a) \
                                  );
}


//--------------------------------------------------------------------------------

__STATIC_INLINE void pwm_cr_unpack(uint32_t base, uint8_t * cap_en, uint8_t * tim_en, uint8_t * dben, uint8_t * pwm_enb, uint8_t * pwm_ena)
{
    t_pwm_cr local_val = ((t_hwp_pwm_t*)(base))->cr;

    *cap_en             = local_val.bit_field.cap_en;
    *tim_en             = local_val.bit_field.tim_en;
    *dben               = local_val.bit_field.dben;
    *pwm_enb            = local_val.bit_field.pwm_enb;
    *pwm_ena            = local_val.bit_field.pwm_ena;
}

__STATIC_INLINE void pwm_cfg_unpack(uint32_t base, uint8_t * int_cmp_b_mod, uint8_t * int_cmp_a_mod, uint8_t * tim_once, uint8_t * div, uint8_t * cap_mod, uint8_t * cap_edg, uint8_t * trig_clr_en, uint8_t * trig_sel, uint8_t * trig_mod, uint8_t * cnt_mode, uint8_t * pwm_invb, uint8_t * pwm_inva)
{
    t_pwm_cfg local_val = ((t_hwp_pwm_t*)(base))->cfg;

    *int_cmp_b_mod      = local_val.bit_field.int_cmp_b_mod;
    *int_cmp_a_mod      = local_val.bit_field.int_cmp_a_mod;
    *tim_once           = local_val.bit_field.tim_once;
    *div                = local_val.bit_field.div;
    *cap_mod            = local_val.bit_field.cap_mod;
    *cap_edg            = local_val.bit_field.cap_edg;
    *trig_clr_en        = local_val.bit_field.trig_clr_en;
    *trig_sel           = local_val.bit_field.trig_sel;
    *trig_mod           = local_val.bit_field.trig_mod;
    *cnt_mode           = local_val.bit_field.cnt_mode;
    *pwm_invb           = local_val.bit_field.pwm_invb;
    *pwm_inva           = local_val.bit_field.pwm_inva;
}

__STATIC_INLINE void pwm_load_unpack(uint32_t base, uint16_t * pwm_db, uint16_t * load)
{
    t_pwm_load local_val = ((t_hwp_pwm_t*)(base))->load;

    *pwm_db             = local_val.bit_field.pwm_db;
    *load               = local_val.bit_field.load;
}

__STATIC_INLINE void pwm_cmp_unpack(uint32_t base, uint16_t * cmp_b, uint16_t * cmp_a)
{
    t_pwm_cmp local_val = ((t_hwp_pwm_t*)(base))->cmp;

    *cmp_b              = local_val.bit_field.cmp_b;
    *cmp_a              = local_val.bit_field.cmp_a;
}

#endif




