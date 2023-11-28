#include "ln882h.h"

// TODO: cmbacktrace 支持 ARMCC 和 GCC
#if defined ( __GNUC__ ) && !defined(__ARMCC_VERSION)
#define CFG_USING_CM_BACKTRACE
#endif

#ifdef CFG_USING_CM_BACKTRACE
#include "cm_backtrace.h"
#endif /* CFG_USING_CM_BACKTRACE */

#include <stdbool.h>
#include "hal/hal_common.h"
#include "hal/hal_interrupt.h"

void set_interrupt_priority(void)
{
    NVIC_SetPriorityGrouping(4);

    NVIC_SetPriority(SysTick_IRQn,   (1<<__NVIC_PRIO_BITS) - 1);
    NVIC_SetPriority(PendSV_IRQn,    (1<<__NVIC_PRIO_BITS) - 1);

    NVIC_SetPriority(WDT_IRQn,       4);
    NVIC_SetPriority(EXT_IRQn,       4);
    NVIC_SetPriority(RTC_IRQn,       4);
    NVIC_SetPriority(RFSLP_IRQn,     4);
    NVIC_SetPriority(MAC_IRQn,       2);

    NVIC_SetPriority(BLE_WAKE_IRQn,  4);
    NVIC_SetPriority(BLE_ERR_IRQn,   4);
    NVIC_SetPriority(BLE_MAC_IRQn,   1);
    NVIC_SetPriority(DMA_IRQn,       4);
    NVIC_SetPriority(QSPI_IRQn,      4);

    NVIC_SetPriority(SDIO_1_IRQn,    4);
    NVIC_SetPriority(SDIO_2_IRQn,    4);
    NVIC_SetPriority(SDIO_3_IRQn,    4);

    NVIC_SetPriority(FPIXC_IRQn,     4);
    NVIC_SetPriority(FPOFC_IRQn,     4);
    NVIC_SetPriority(FPUFC_IRQn,     4);
    NVIC_SetPriority(FPIOC_IRQn,     4);
    NVIC_SetPriority(FPDZC_IRQn,     4);
    NVIC_SetPriority(FPIDC_IRQn,     4);

    NVIC_SetPriority(I2C_IRQn,       4);
    NVIC_SetPriority(SPI0_IRQn,      4);
    NVIC_SetPriority(SPI1_IRQn,      4);

    NVIC_SetPriority(UART0_IRQn,     4);
    NVIC_SetPriority(UART1_IRQn,     2);
    NVIC_SetPriority(UART2_IRQn,     2);

    NVIC_SetPriority(ADC_IRQn,       4);
    NVIC_SetPriority(WS2811_IRQn,    4);
    NVIC_SetPriority(I2S_IRQn,       4);

    NVIC_SetPriority(GPIOA_IRQn,     4);
    NVIC_SetPriority(GPIOB_IRQn,     4);

    NVIC_SetPriority(TIMER0_IRQn,    4);
    NVIC_SetPriority(TIMER1_IRQn,    4);
    NVIC_SetPriority(TIMER2_IRQn,    4);
    NVIC_SetPriority(TIMER3_IRQn,    4);
    NVIC_SetPriority(ADV_TIMER_IRQn, 4);

    NVIC_SetPriority(AES_IRQn,       4);
    NVIC_SetPriority(TRNG_IRQn,      4);
    NVIC_SetPriority(PAOTD_IRQn,     2);
}

void switch_global_interrupt(hal_en_t enable)
{
    if(enable)
        __enable_irq();
    else
        __disable_irq();
}


#if defined (__ARMCC_VERSION)
    __asm void NMI_Handler (void)
    {
        MOV    R0, LR
        MOV    R1, SP
#ifdef CFG_USING_CM_BACKTRACE
        BL     __cpp(cm_backtrace_fault)
#endif
NMI_Fault_Loop
        B      NMI_Fault_Loop
    }

    __asm void HardFault_Handler (void)
    {
        MOV    R0, LR
        MOV    R1, SP
#ifdef CFG_USING_CM_BACKTRACE
        BL     __cpp(cm_backtrace_fault)
#endif
Hard_Fault_Loop
        B      Hard_Fault_Loop
    }

    __asm void MemManage_Handler (void)
    {
        MOV    R0, LR
        MOV    R1, SP
#ifdef CFG_USING_CM_BACKTRACE
        BL     __cpp(cm_backtrace_fault)
#endif
MM_Fault_Loop
        B      MM_Fault_Loop
    }

    __asm void BusFault_Handler (void)
    {
        MOV    R0, LR
        MOV    R1, SP
#ifdef CFG_USING_CM_BACKTRACE
        BL     __cpp(cm_backtrace_fault)
#endif
Bus_Fault_Loop
        B      Bus_Fault_Loop
    }

    __asm void UsageFault_Handler (void)
    {
        MOV    R0, LR
        MOV    R1, SP
#ifdef CFG_USING_CM_BACKTRACE
        BL     __cpp(cm_backtrace_fault)
#endif
Usage_Fault_Loop
        B      Usage_Fault_Loop
    }

    __asm void DebugMon_Handler (void)
    {
        MOV    R0, LR
        MOV    R1, SP
#ifdef CFG_USING_CM_BACKTRACE
        BL     __cpp(cm_backtrace_fault)
#endif
DBG_Fault_Loop
        B      DBG_Fault_Loop
    }

#elif defined ( __GNUC__ )

     void NMI_Handler (void)
    {
        #if 0
       __asm__(" MOV    R0, LR;"
        "MOV    R1, SP;"
#ifdef CFG_USING_CM_BACKTRACE
        "BL     _asm_cm_backtrace_fault;"
#endif
        "B   .");
        #else
       // another way
       asm("MOV  R0,LR\n\t"
            "MOV R1,SP\n\t"
            "BL cm_backtrace_fault\n\t"
            "B .\n\t"
            ::"m"(cm_backtrace_fault):"cc");
       #endif
    }

    void HardFault_Handler (void)
    {
         __asm__("MOV  R0,LR\n\t"
        "MOV R1,SP\n\t"
        "BL cm_backtrace_fault\n\t"
        "B .\n\t"
        ::"m"(cm_backtrace_fault):"cc");
    }

    void MemManage_Handler (void)
    {
        __asm__("MOV  R0,LR\n\t"
        "MOV R1,SP\n\t"
        "BL cm_backtrace_fault\n\t"
        "B .\n\t"
        ::"m"(cm_backtrace_fault):"cc");
    }

    void BusFault_Handler (void)
    {
       __asm__("MOV  R0,LR\n\t"
        "MOV R1,SP\n\t"
        "BL cm_backtrace_fault\n\t"
        "B .\n\t"
        ::"m"(cm_backtrace_fault):"cc");
    }

    void UsageFault_Handler (void)
    {
        __asm__("MOV  R0,LR\n\t"
        "MOV R1,SP\n\t"
        "BL cm_backtrace_fault\n\t"
        "B .\n\t"
        ::"m"(cm_backtrace_fault):"cc");
    }

    void DebugMon_Handler (void)
    {
        __asm__("MOV  R0,LR\n\t"
        "MOV R1,SP\n\t"
        "BL cm_backtrace_fault\n\t"
        "B .\n\t"
        ::"m"(cm_backtrace_fault):"cc");
    }
#elif defined ( __ICCARM__ )
    #error "iar compiler!!!"
#else
    #error "Unsupported compiler!!!"
#endif

