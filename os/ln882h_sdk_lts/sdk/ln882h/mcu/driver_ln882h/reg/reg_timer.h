#ifndef __REG_TIMER_H__
#define __REG_TIMER_H__

#include "ln882h.h"

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer1loadcountregister: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timerloadcount;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer1currentvalueregister: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timercurrentvalue;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer1enable        : 1  ; // 0  : 0
        uint32_t    timer1mode          : 1  ; // 1  : 1
        uint32_t    timer1interruptmask : 1  ; // 2  : 2
        uint32_t    timer1pwm           : 1  ; // 3  : 3
        uint32_t    reserved_0          : 28 ; // 31 : 4
    } bit_field;
} t_timer_timercontrolreg;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer1endofinterruptregister: 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_timer_timereoi;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer1interruptstatusregister: 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_timer_timerintstatus;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer1loadcount2register: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timerloadcount2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer1loadcountregister: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timer1loadcount;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer1currentvalueregister: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timer1currentvalue;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer1enable        : 1  ; // 0  : 0
        uint32_t    timer1mode          : 1  ; // 1  : 1
        uint32_t    timer1interruptmask : 1  ; // 2  : 2
        uint32_t    timer1pwm           : 1  ; // 3  : 3
        uint32_t    reserved_0          : 28 ; // 31 : 4
    } bit_field;
} t_timer_timer1controlreg;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer1endofinterruptregister: 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_timer_timer1eoi;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer1interruptstatusregister: 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_timer_timer1intstatus;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer1loadcount2register: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timer1loadcount2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer2loadcountregister: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timer2loadcount;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer2currentvalueregister: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timer2currentvalue;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer2enable        : 1  ; // 0  : 0
        uint32_t    timer2mode          : 1  ; // 1  : 1
        uint32_t    timer2interruptmask : 1  ; // 2  : 2
        uint32_t    timer2pwm           : 1  ; // 3  : 3
        uint32_t    reserved_0          : 28 ; // 31 : 4
    } bit_field;
} t_timer_timer2controlreg;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer2endofinterruptregister: 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_timer_timer2eoi;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer2interruptstatusregister: 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_timer_timer2ntstatus;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer2loadcount2register: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timer2loadcount2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer3loadcountregister: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timer3loadcount;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer3currentvalueregister: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timer3currentvalue;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer3enable        : 1  ; // 0  : 0
        uint32_t    timer3mode          : 1  ; // 1  : 1
        uint32_t    timer3interruptmask : 1  ; // 2  : 2
        uint32_t    timer3pwm           : 1  ; // 3  : 3
        uint32_t    reserved_0          : 28 ; // 31 : 4
    } bit_field;
} t_timer_timer3controlreg;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer3endofinterruptregister: 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_timer_timer3eoi;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer3interruptstatusregister: 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_timer_timer3intstatus;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer3loadcount2register: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timer3loadcount2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer4loadcountregister: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timer4loadcount;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer4currentvalueregister: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timer4currentvalue;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer4enable        : 1  ; // 0  : 0
        uint32_t    timer4mode          : 1  ; // 1  : 1
        uint32_t    timer4interruptmask : 1  ; // 2  : 2
        uint32_t    timer4pwm           : 1  ; // 3  : 3
        uint32_t    reserved_0          : 28 ; // 31 : 4
    } bit_field;
} t_timer_timer4controlreg;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer4endofinterruptregister: 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_timer_timer4eoi;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer4interruptstatusregister: 1  ; // 0  : 0
        uint32_t    reserved_0          : 31 ; // 31 : 1
    } bit_field;
} t_timer_timer4ntstatus;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timer4loadcount2register: 24 ; // 23 : 0
        uint32_t    reserved_0          : 8  ; // 31 : 24
    } bit_field;
} t_timer_timer4loadcount2;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timerinterruptstatusregister: 4  ; // 3  : 0
        uint32_t    reserved_0          : 28 ; // 31 : 4
    } bit_field;
} t_timer_timersintstatus;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timersendofinterruptregister: 4  ; // 3  : 0
        uint32_t    reserved_0          : 28 ; // 31 : 4
    } bit_field;
} t_timer_timerseoi;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timersrawinterruptstatusregister: 4  ; // 3  : 0
        uint32_t    reserved_0          : 28 ; // 31 : 4
    } bit_field;
} t_timer_timersrawintstatus;

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    timerscomponentversionregister: 32 ; // 31 : 0
    } bit_field;
} t_timer_timerscomponentversion;


//--------------------------------------------------------------------------------
typedef struct
{
    volatile        t_timer_timerloadcount                  timerloadcount                        ; // 0x0
    volatile        t_timer_timercurrentvalue               timercurrentvalue                     ; // 0x4
    volatile        t_timer_timercontrolreg                 timercontrolreg                       ; // 0x8
    volatile        t_timer_timereoi                        timereoi                              ; // 0xc
    volatile        t_timer_timerintstatus                  timerintstatus                        ; // 0x10
    volatile        t_timer_timerloadcount2                 timerloadcount2                       ; // 0x14
}t_hwp_timer_tmp_t;


typedef struct
{
    volatile        t_timer_timer1loadcount                  timer1loadcount                        ; // 0x0
    volatile        t_timer_timer1currentvalue               timer1currentvalue                     ; // 0x4
    volatile        t_timer_timer1controlreg                 timer1controlreg                       ; // 0x8
    volatile        t_timer_timer1eoi                        timer1eoi                              ; // 0xc
    volatile        t_timer_timer1intstatus                  timer1intstatus                        ; // 0x10
    volatile        t_timer_timer1loadcount2                 timer1loadcount2                       ; // 0x14
    volatile        t_timer_timer2loadcount                  timer2loadcount                        ; // 0x18
    volatile        t_timer_timer2currentvalue               timer2currentvalue                     ; // 0x1c
    volatile        t_timer_timer2controlreg                 timer2controlreg                       ; // 0x20
    volatile        t_timer_timer2eoi                        timer2eoi                              ; // 0x24
    volatile        t_timer_timer2ntstatus                   timer2ntstatus                         ; // 0x28
    volatile        t_timer_timer2loadcount2                 timer2loadcount2                       ; // 0x2c
    volatile        t_timer_timer3loadcount                  timer3loadcount                        ; // 0x30
    volatile        t_timer_timer3currentvalue               timer3currentvalue                     ; // 0x34
    volatile        t_timer_timer3controlreg                 timer3controlreg                       ; // 0x38
    volatile        t_timer_timer3eoi                        timer3eoi                              ; // 0x3c
    volatile        t_timer_timer3intstatus                  timer3intstatus                        ; // 0x40
    volatile        t_timer_timer3loadcount2                 timer3loadcount2                       ; // 0x44
    volatile        t_timer_timer4loadcount                  timer4loadcount                        ; // 0x48
    volatile        t_timer_timer4currentvalue               timer4currentvalue                     ; // 0x4c
    volatile        t_timer_timer4controlreg                 timer4controlreg                       ; // 0x50
    volatile        t_timer_timer4eoi                        timer4eoi                              ; // 0x54
    volatile        t_timer_timer4ntstatus                   timer4ntstatus                         ; // 0x58
    volatile        t_timer_timer4loadcount2                 timer4loadcount2                       ; // 0x5c
    volatile        t_timer_timersintstatus                  timersintstatus                        ; // 0x60
    volatile        t_timer_timerseoi                        timerseoi                              ; // 0x64
    volatile        t_timer_timersrawintstatus               timersrawintstatus                     ; // 0x68
    volatile        t_timer_timerscomponentversion           timerscomponentversion                 ; // 0x6c
} t_hwp_timer_t;


//--------------------------------------------------------------------------------
__STATIC_INLINE void timer_timerloadcount_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_tmp_t*)(base))->timerloadcount.val = value;
}

__STATIC_INLINE void timer_timerloadcountregister_setf(uint32_t base, uint32_t timerloadcountregister)
{
    ((t_hwp_timer_tmp_t*)(base))->timerloadcount.bit_field.timer1loadcountregister = timerloadcountregister;
}

__STATIC_INLINE void timer_timercontrolreg_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_tmp_t*)(base))->timercontrolreg.val = value;
}

__STATIC_INLINE void timer_timerpwm_setf(uint32_t base, uint8_t timerpwm)
{
    ((t_hwp_timer_tmp_t*)(base))->timercontrolreg.bit_field.timer1pwm = timerpwm;
}

__STATIC_INLINE void timer_timerinterruptmask_setf(uint32_t base, uint8_t timerinterruptmask)
{
    ((t_hwp_timer_tmp_t*)(base))->timercontrolreg.bit_field.timer1interruptmask = timerinterruptmask;
}

__STATIC_INLINE void timer_timermode_setf(uint32_t base, uint8_t timermode)
{
    ((t_hwp_timer_tmp_t*)(base))->timercontrolreg.bit_field.timer1mode = timermode;
}

__STATIC_INLINE void timer_timerenable_setf(uint32_t base, uint8_t timerenable)
{
    ((t_hwp_timer_tmp_t*)(base))->timercontrolreg.bit_field.timer1enable = timerenable;
}

__STATIC_INLINE void timer_timerloadcount2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_tmp_t*)(base))->timerloadcount2.val = value;
}

__STATIC_INLINE void timer_timerloadcount2register_setf(uint32_t base, uint32_t timerloadcount2register)
{
    ((t_hwp_timer_tmp_t*)(base))->timerloadcount2.bit_field.timer1loadcount2register = timerloadcount2register;
}



__STATIC_INLINE void timer_timer1loadcount_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_t*)(base))->timer1loadcount.val = value;
}

__STATIC_INLINE void timer_timer1loadcountregister_setf(uint32_t base, uint32_t timer1loadcountregister)
{
    ((t_hwp_timer_t*)(base))->timer1loadcount.bit_field.timer1loadcountregister = timer1loadcountregister;
}

__STATIC_INLINE void timer_timer1controlreg_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_t*)(base))->timer1controlreg.val = value;
}

__STATIC_INLINE void timer_timer1pwm_setf(uint32_t base, uint8_t timer1pwm)
{
    ((t_hwp_timer_t*)(base))->timer1controlreg.bit_field.timer1pwm = timer1pwm;
}

__STATIC_INLINE void timer_timer1interruptmask_setf(uint32_t base, uint8_t timer1interruptmask)
{
    ((t_hwp_timer_t*)(base))->timer1controlreg.bit_field.timer1interruptmask = timer1interruptmask;
}

__STATIC_INLINE void timer_timer1mode_setf(uint32_t base, uint8_t timer1mode)
{
    ((t_hwp_timer_t*)(base))->timer1controlreg.bit_field.timer1mode = timer1mode;
}

__STATIC_INLINE void timer_timer1enable_setf(uint32_t base, uint8_t timer1enable)
{
    ((t_hwp_timer_t*)(base))->timer1controlreg.bit_field.timer1enable = timer1enable;
}

__STATIC_INLINE void timer_timer1loadcount2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_t*)(base))->timer1loadcount2.val = value;
}

__STATIC_INLINE void timer_timer1loadcount2register_setf(uint32_t base, uint32_t timer1loadcount2register)
{
    ((t_hwp_timer_t*)(base))->timer1loadcount2.bit_field.timer1loadcount2register = timer1loadcount2register;
}

__STATIC_INLINE void timer_timer2loadcount_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_t*)(base))->timer2loadcount.val = value;
}

__STATIC_INLINE void timer_timer2loadcountregister_setf(uint32_t base, uint32_t timer2loadcountregister)
{
    ((t_hwp_timer_t*)(base))->timer2loadcount.bit_field.timer2loadcountregister = timer2loadcountregister;
}

__STATIC_INLINE void timer_timer2controlreg_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_t*)(base))->timer2controlreg.val = value;
}

__STATIC_INLINE void timer_timer2pwm_setf(uint32_t base, uint8_t timer2pwm)
{
    ((t_hwp_timer_t*)(base))->timer2controlreg.bit_field.timer2pwm = timer2pwm;
}

__STATIC_INLINE void timer_timer2interruptmask_setf(uint32_t base, uint8_t timer2interruptmask)
{
    ((t_hwp_timer_t*)(base))->timer2controlreg.bit_field.timer2interruptmask = timer2interruptmask;
}

__STATIC_INLINE void timer_timer2mode_setf(uint32_t base, uint8_t timer2mode)
{
    ((t_hwp_timer_t*)(base))->timer2controlreg.bit_field.timer2mode = timer2mode;
}

__STATIC_INLINE void timer_timer2enable_setf(uint32_t base, uint8_t timer2enable)
{
    ((t_hwp_timer_t*)(base))->timer2controlreg.bit_field.timer2enable = timer2enable;
}

__STATIC_INLINE void timer_timer2loadcount2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_t*)(base))->timer2loadcount2.val = value;
}

__STATIC_INLINE void timer_timer2loadcount2register_setf(uint32_t base, uint32_t timer2loadcount2register)
{
    ((t_hwp_timer_t*)(base))->timer2loadcount2.bit_field.timer2loadcount2register = timer2loadcount2register;
}

__STATIC_INLINE void timer_timer3loadcount_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_t*)(base))->timer3loadcount.val = value;
}

__STATIC_INLINE void timer_timer3loadcountregister_setf(uint32_t base, uint32_t timer3loadcountregister)
{
    ((t_hwp_timer_t*)(base))->timer3loadcount.bit_field.timer3loadcountregister = timer3loadcountregister;
}

__STATIC_INLINE void timer_timer3controlreg_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_t*)(base))->timer3controlreg.val = value;
}

__STATIC_INLINE void timer_timer3pwm_setf(uint32_t base, uint8_t timer3pwm)
{
    ((t_hwp_timer_t*)(base))->timer3controlreg.bit_field.timer3pwm = timer3pwm;
}

__STATIC_INLINE void timer_timer3interruptmask_setf(uint32_t base, uint8_t timer3interruptmask)
{
    ((t_hwp_timer_t*)(base))->timer3controlreg.bit_field.timer3interruptmask = timer3interruptmask;
}

__STATIC_INLINE void timer_timer3mode_setf(uint32_t base, uint8_t timer3mode)
{
    ((t_hwp_timer_t*)(base))->timer3controlreg.bit_field.timer3mode = timer3mode;
}

__STATIC_INLINE void timer_timer3enable_setf(uint32_t base, uint8_t timer3enable)
{
    ((t_hwp_timer_t*)(base))->timer3controlreg.bit_field.timer3enable = timer3enable;
}

__STATIC_INLINE void timer_timer3loadcount2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_t*)(base))->timer3loadcount2.val = value;
}

__STATIC_INLINE void timer_timer3loadcount2register_setf(uint32_t base, uint32_t timer3loadcount2register)
{
    ((t_hwp_timer_t*)(base))->timer3loadcount2.bit_field.timer3loadcount2register = timer3loadcount2register;
}

__STATIC_INLINE void timer_timer4loadcount_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_t*)(base))->timer4loadcount.val = value;
}

__STATIC_INLINE void timer_timer4loadcountregister_setf(uint32_t base, uint32_t timer4loadcountregister)
{
    ((t_hwp_timer_t*)(base))->timer4loadcount.bit_field.timer4loadcountregister = timer4loadcountregister;
}

__STATIC_INLINE void timer_timer4controlreg_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_t*)(base))->timer4controlreg.val = value;
}

__STATIC_INLINE void timer_timer4pwm_setf(uint32_t base, uint8_t timer4pwm)
{
    ((t_hwp_timer_t*)(base))->timer4controlreg.bit_field.timer4pwm = timer4pwm;
}

__STATIC_INLINE void timer_timer4interruptmask_setf(uint32_t base, uint8_t timer4interruptmask)
{
    ((t_hwp_timer_t*)(base))->timer4controlreg.bit_field.timer4interruptmask = timer4interruptmask;
}

__STATIC_INLINE void timer_timer4mode_setf(uint32_t base, uint8_t timer4mode)
{
    ((t_hwp_timer_t*)(base))->timer4controlreg.bit_field.timer4mode = timer4mode;
}

__STATIC_INLINE void timer_timer4enable_setf(uint32_t base, uint8_t timer4enable)
{
    ((t_hwp_timer_t*)(base))->timer4controlreg.bit_field.timer4enable = timer4enable;
}

__STATIC_INLINE void timer_timer4loadcount2_set(uint32_t base, uint32_t value)
{
    ((t_hwp_timer_t*)(base))->timer4loadcount2.val = value;
}

__STATIC_INLINE void timer_timer4loadcount2register_setf(uint32_t base, uint32_t timer4loadcount2register)
{
    ((t_hwp_timer_t*)(base))->timer4loadcount2.bit_field.timer4loadcount2register = timer4loadcount2register;
}


//--------------------------------------------------------------------------------
__STATIC_INLINE uint32_t timer_timerloadcount_get(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timerloadcount.val;
}

__STATIC_INLINE uint32_t timer_timerloadcountregister_getf(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timerloadcount.bit_field.timer1loadcountregister;
}

__STATIC_INLINE uint32_t timer_timercurrentvalue_get(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timercurrentvalue.val;
}

__STATIC_INLINE uint32_t timer_timercurrentvalueregister_getf(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timercurrentvalue.bit_field.timer1currentvalueregister;
}

__STATIC_INLINE uint32_t timer_timercontrolreg_get(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timercontrolreg.val;
}

__STATIC_INLINE uint8_t timer_timerpwm_getf(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timercontrolreg.bit_field.timer1pwm;
}

__STATIC_INLINE uint8_t timer_timerinterruptmask_getf(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timercontrolreg.bit_field.timer1interruptmask;
}

__STATIC_INLINE uint8_t timer_timermode_getf(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timercontrolreg.bit_field.timer1mode;
}

__STATIC_INLINE uint8_t timer_timerenable_getf(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timercontrolreg.bit_field.timer1enable;
}

__STATIC_INLINE uint32_t timer_timereoi_get(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timereoi.val;
}

__STATIC_INLINE uint8_t timer_timerendofinterruptregister_getf(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timereoi.bit_field.timer1endofinterruptregister;
}

__STATIC_INLINE uint32_t timer_timerintstatus_get(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timerintstatus.val;
}

__STATIC_INLINE uint8_t timer_timerxinterruptstatusregister_getf(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timerintstatus.bit_field.timer1interruptstatusregister;
}

__STATIC_INLINE uint32_t timer_timerloadcount2_get(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timerloadcount2.val;
}

__STATIC_INLINE uint32_t timer_timerloadcount2register_getf(uint32_t base)
{
    return ((t_hwp_timer_tmp_t*)(base))->timerloadcount2.bit_field.timer1loadcount2register;
}



__STATIC_INLINE uint32_t timer_timer1loadcount_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1loadcount.val;
}

__STATIC_INLINE uint32_t timer_timer1loadcountregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1loadcount.bit_field.timer1loadcountregister;
}

__STATIC_INLINE uint32_t timer_timer1currentvalue_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1currentvalue.val;
}

__STATIC_INLINE uint32_t timer_timer1currentvalueregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1currentvalue.bit_field.timer1currentvalueregister;
}

__STATIC_INLINE uint32_t timer_timer1controlreg_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1controlreg.val;
}

__STATIC_INLINE uint8_t timer_timer1pwm_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1controlreg.bit_field.timer1pwm;
}

__STATIC_INLINE uint8_t timer_timer1interruptmask_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1controlreg.bit_field.timer1interruptmask;
}

__STATIC_INLINE uint8_t timer_timer1mode_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1controlreg.bit_field.timer1mode;
}

__STATIC_INLINE uint8_t timer_timer1enable_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1controlreg.bit_field.timer1enable;
}

__STATIC_INLINE uint32_t timer_timer1eoi_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1eoi.val;
}

__STATIC_INLINE uint8_t timer_timer1endofinterruptregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1eoi.bit_field.timer1endofinterruptregister;
}

__STATIC_INLINE uint32_t timer_timer1intstatus_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1intstatus.val;
}

__STATIC_INLINE uint8_t timer_timer1interruptstatusregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1intstatus.bit_field.timer1interruptstatusregister;
}

__STATIC_INLINE uint32_t timer_timer1loadcount2_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1loadcount2.val;
}

__STATIC_INLINE uint32_t timer_timer1loadcount2register_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer1loadcount2.bit_field.timer1loadcount2register;
}

__STATIC_INLINE uint32_t timer_timer2loadcount_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2loadcount.val;
}

__STATIC_INLINE uint32_t timer_timer2loadcountregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2loadcount.bit_field.timer2loadcountregister;
}

__STATIC_INLINE uint32_t timer_timer2currentvalue_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2currentvalue.val;
}

__STATIC_INLINE uint32_t timer_timer2currentvalueregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2currentvalue.bit_field.timer2currentvalueregister;
}

__STATIC_INLINE uint32_t timer_timer2controlreg_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2controlreg.val;
}

__STATIC_INLINE uint8_t timer_timer2pwm_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2controlreg.bit_field.timer2pwm;
}

__STATIC_INLINE uint8_t timer_timer2interruptmask_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2controlreg.bit_field.timer2interruptmask;
}

__STATIC_INLINE uint8_t timer_timer2mode_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2controlreg.bit_field.timer2mode;
}

__STATIC_INLINE uint8_t timer_timer2enable_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2controlreg.bit_field.timer2enable;
}

__STATIC_INLINE uint32_t timer_timer2eoi_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2eoi.val;
}

__STATIC_INLINE uint8_t timer_timer2endofinterruptregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2eoi.bit_field.timer2endofinterruptregister;
}

__STATIC_INLINE uint32_t timer_timer2ntstatus_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2ntstatus.val;
}

__STATIC_INLINE uint8_t timer_timer2interruptstatusregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2ntstatus.bit_field.timer2interruptstatusregister;
}

__STATIC_INLINE uint32_t timer_timer2loadcount2_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2loadcount2.val;
}

__STATIC_INLINE uint32_t timer_timer2loadcount2register_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer2loadcount2.bit_field.timer2loadcount2register;
}

__STATIC_INLINE uint32_t timer_timer3loadcount_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3loadcount.val;
}

__STATIC_INLINE uint32_t timer_timer3loadcountregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3loadcount.bit_field.timer3loadcountregister;
}

__STATIC_INLINE uint32_t timer_timer3currentvalue_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3currentvalue.val;
}

__STATIC_INLINE uint32_t timer_timer3currentvalueregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3currentvalue.bit_field.timer3currentvalueregister;
}

__STATIC_INLINE uint32_t timer_timer3controlreg_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3controlreg.val;
}

__STATIC_INLINE uint8_t timer_timer3pwm_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3controlreg.bit_field.timer3pwm;
}

__STATIC_INLINE uint8_t timer_timer3interruptmask_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3controlreg.bit_field.timer3interruptmask;
}

__STATIC_INLINE uint8_t timer_timer3mode_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3controlreg.bit_field.timer3mode;
}

__STATIC_INLINE uint8_t timer_timer3enable_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3controlreg.bit_field.timer3enable;
}

__STATIC_INLINE uint32_t timer_timer3eoi_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3eoi.val;
}

__STATIC_INLINE uint8_t timer_timer3endofinterruptregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3eoi.bit_field.timer3endofinterruptregister;
}

__STATIC_INLINE uint32_t timer_timer3intstatus_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3intstatus.val;
}

__STATIC_INLINE uint8_t timer_timer3interruptstatusregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3intstatus.bit_field.timer3interruptstatusregister;
}

__STATIC_INLINE uint32_t timer_timer3loadcount2_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3loadcount2.val;
}

__STATIC_INLINE uint32_t timer_timer3loadcount2register_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer3loadcount2.bit_field.timer3loadcount2register;
}

__STATIC_INLINE uint32_t timer_timer4loadcount_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4loadcount.val;
}

__STATIC_INLINE uint32_t timer_timer4loadcountregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4loadcount.bit_field.timer4loadcountregister;
}

__STATIC_INLINE uint32_t timer_timer4currentvalue_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4currentvalue.val;
}

__STATIC_INLINE uint32_t timer_timer4currentvalueregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4currentvalue.bit_field.timer4currentvalueregister;
}

__STATIC_INLINE uint32_t timer_timer4controlreg_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4controlreg.val;
}

__STATIC_INLINE uint8_t timer_timer4pwm_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4controlreg.bit_field.timer4pwm;
}

__STATIC_INLINE uint8_t timer_timer4interruptmask_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4controlreg.bit_field.timer4interruptmask;
}

__STATIC_INLINE uint8_t timer_timer4mode_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4controlreg.bit_field.timer4mode;
}

__STATIC_INLINE uint8_t timer_timer4enable_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4controlreg.bit_field.timer4enable;
}

__STATIC_INLINE uint32_t timer_timer4eoi_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4eoi.val;
}

__STATIC_INLINE uint8_t timer_timer4endofinterruptregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4eoi.bit_field.timer4endofinterruptregister;
}

__STATIC_INLINE uint32_t timer_timer4ntstatus_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4ntstatus.val;
}

__STATIC_INLINE uint8_t timer_timer4interruptstatusregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4ntstatus.bit_field.timer4interruptstatusregister;
}

__STATIC_INLINE uint32_t timer_timer4loadcount2_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4loadcount2.val;
}

__STATIC_INLINE uint32_t timer_timer4loadcount2register_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timer4loadcount2.bit_field.timer4loadcount2register;
}

__STATIC_INLINE uint32_t timer_timersintstatus_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timersintstatus.val;
}

__STATIC_INLINE uint8_t timer_timerinterruptstatusregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timersintstatus.bit_field.timerinterruptstatusregister;
}

__STATIC_INLINE uint32_t timer_timerseoi_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timerseoi.val;
}

__STATIC_INLINE uint8_t timer_timersendofinterruptregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timerseoi.bit_field.timersendofinterruptregister;
}

__STATIC_INLINE uint32_t timer_timersrawintstatus_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timersrawintstatus.val;
}

__STATIC_INLINE uint8_t timer_timersrawinterruptstatusregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timersrawintstatus.bit_field.timersrawinterruptstatusregister;
}

__STATIC_INLINE uint32_t timer_timerscomponentversion_get(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timerscomponentversion.val;
}

__STATIC_INLINE uint32_t timer_timerscomponentversionregister_getf(uint32_t base)
{
    return ((t_hwp_timer_t*)(base))->timerscomponentversion.bit_field.timerscomponentversionregister;
}


//--------------------------------------------------------------------------------
#endif

