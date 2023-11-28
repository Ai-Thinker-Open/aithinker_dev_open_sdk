#include "proj_config.h"
#include "ln882h.h"
#include "utils/runtime/runtime.h"

static uint16_t os_tick_compensate_val = 0;
static uint32_t remainder_cycles = 0;

void ln_runtime_measure_init(void)
{
    if (!((uint32_t)CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    }
    
    if(!(((uint32_t)DWT->CTRL) & DWT_CTRL_NOPRFCNT_Msk)) {
        DWT->CTRL |= DWT_CTRL_NOPRFCNT_Msk;
    }
    
    if(!(((uint32_t)DWT->CTRL) & DWT_CTRL_CYCCNTENA_Msk)) {
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

uint32_t ln_runtime_get_curr_cycles(void)
{
    return ((uint32_t)DWT->CYCCNT);
}

uint32_t ln_runtime_get_delta_cycles(uint32_t last, uint32_t curr)
{
    uint32_t delta = 0;
    if( curr >= last) {
        delta = curr - last;
    } else {
        delta = (0xFFFFFFFFUL - last) + curr;
    }
    return delta;
}

uint32_t ln_runtime_get_delta_ticks(uint32_t last, uint32_t curr)
{
    uint64_t ticks = 0;
    
    ticks = (ln_runtime_get_delta_cycles(last,curr)*1000LL) / SYSTEM_CLOCK;
    return (uint32_t)(ticks);
}

void ln_block_delayus(uint32_t us)
{
   uint32_t start = 0, end = 0, delta = 0;

   delta = us * ( SYSTEM_CLOCK/(1000UL*1000UL));
   start = DWT->CYCCNT;
   end   = start + delta;
    
   if(end > start) {
       while(DWT->CYCCNT < end) {};
   } else {
       while(DWT->CYCCNT > end) {};
       while(DWT->CYCCNT < end) {};
   }
}

void ln_block_delayms(uint32_t ms)
{
    uint32_t t = 0;
    
    if ( ms < ((0xFFFFFFFFUL/SYSTEM_CLOCK) * 1000UL)) {
        ln_block_delayus( 1000UL * ms);
    } else {
        for (t = 0; t < ms; t++) {
            ln_block_delayus(1000UL);        
        }
    }
}

uint32_t ln_os_tick_comp_pre_get_cycles(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    return ((uint32_t)DWT->CYCCNT); 
}

void ln_os_tick_comp_val_inc(uint32_t last)
{
    #define COMP_MISSED_CYCLES    (160*30)
    uint32_t delta = 0;

    delta += remainder_cycles;
    delta += ln_runtime_get_delta_cycles(last, (uint32_t)DWT->CYCCNT);
    delta += COMP_MISSED_CYCLES;
    remainder_cycles = delta % (SYSTEM_CLOCK/1000UL);

    os_tick_compensate_val += (delta / (SYSTEM_CLOCK/1000UL));

    SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk);  
}

void ln_get_os_tick_comp_val(uint16_t ** comp_val) 
{
    *comp_val = &os_tick_compensate_val;
}


