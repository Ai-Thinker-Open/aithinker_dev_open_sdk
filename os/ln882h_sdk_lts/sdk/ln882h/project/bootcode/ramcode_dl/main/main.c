#include "proj_config.h"
#include "ln882h.h"
#include "hal/hal_cache.h"
#include "hal/hal_flash.h"
#include "mode_ctrl.h"

static void set_interrupt_priority(void)
{
    __NVIC_SetPriorityGrouping(4);

    NVIC_SetPriority(SysTick_IRQn, 1);
    NVIC_SetPriority(UART0_IRQn, 4);
    NVIC_SetPriority(UART1_IRQn, 4);
}

int main(int argc, char* argv[])
{
    SetSysClock();
    set_interrupt_priority();
    __enable_irq();

    hal_flash_init();
    flash_cache_disable();

    bootram_ctrl_init();
    bootram_ctrl_loop();

    while (1)
        ;
}

void MemManage_Handler(void)
{
    while (1)
        ;
}

void BusFault_Handler(void)
{
    while (1)
        ;
}

void UsageFault_Handler(void)
{
    while (1)
        ;
}
