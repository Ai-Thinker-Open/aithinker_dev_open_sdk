#ifndef __LN882H_H__
#define __LN882H_H__

#ifdef __cplusplus
extern "C" {
#endif


/* -------------------------  Interrupt Number Definition  ------------------------ */
typedef enum IRQn
{
    /* -------------------  Processor Exceptions Numbers  ----------------------------- */
    Reset_IRQn              = -15,              /* -15  Reset Vector, invoked on Power up and warm reset                       */
    NMI_IRQn                = -14,              /* -14  Non maskable Interrupt, cannot be stopped or preempted                           */
    HardFault_IRQn          = -13,              /* -13  Hard Fault, all classes of Fault                                       */
    MemoryManagement_IRQn   = -12,              /* -12  Memory Management, MPU mismatch, including Access Violation and No Match */
    BusFault_IRQn           = -11,              /* -11  Bus Fault, Pre-Fetch-, Memory Access Fault, other address/memory
                                                        related Fault                                                          */
    UsageFault_IRQn         = -10,              /* -10  Usage Fault, i.e. Undef Instruction, Illegal State Transition          */
    SVCall_IRQn             =  -5,              /* -5   System Service Call via SVC instruction                                */
    DebugMonitor_IRQn       =  -4,              /* -4   Debug Monitor                                                          */
    PendSV_IRQn             =  -2,              /* -2   Pendable request for system service                                    */
    SysTick_IRQn            =  -1,              /* -1   System Tick Timer                                                      */

    /* -------------------  Processor Interrupt Numbers  ------------------------------ */
    WDT_IRQn                = 0,
    EXT_IRQn                = 1,
    RTC_IRQn                = 2,
    RFSLP_IRQn              = 3,
    MAC_IRQn                = 4,
    BLE_WAKE_IRQn           = 5,
    BLE_ERR_IRQn            = 6,
    BLE_MAC_IRQn            = 7,    
    DMA_IRQn                = 8,
    QSPI_IRQn               = 9,
    SDIO_1_IRQn             = 10,
    SDIO_2_IRQn             = 11,
    SDIO_3_IRQn             = 12,   
    FPIXC_IRQn              = 13,
    FPOFC_IRQn              = 14,
    FPUFC_IRQn              = 15,
    FPIOC_IRQn              = 16,
    FPDZC_IRQn              = 17,
    FPIDC_IRQn              = 18,
    I2C_IRQn                = 19,
    SPI0_IRQn               = 20,
    SPI1_IRQn               = 21,  
    UART0_IRQn              = 22,
    UART1_IRQn              = 23,
    UART2_IRQn              = 24, 
    ADC_IRQn                = 25,
    WS2811_IRQn             = 26, 
    I2S_IRQn                = 27,
    GPIOA_IRQn              = 28,
    GPIOB_IRQn              = 29,
    TIMER0_IRQn             = 30,
    TIMER1_IRQn             = 31,
    TIMER2_IRQn             = 32,
    TIMER3_IRQn             = 33,
    ADV_TIMER_IRQn          = 34,
    AES_IRQn                = 35,
    TRNG_IRQn               = 36,
    PAOTD_IRQn              = 37,  

} IRQn_Type;

/* ================================================================================ */
/* ================      Processor and Core Peripheral Section     ================ */
/* ================================================================================ */

#define __CM4_REV                 0x0201U    /*!< Core Revision r2p1 */
/* ToDo: define the correct core features for the <Device> */
#define __MPU_PRESENT             1U         /*!< Set to 1 if MPU is present */
#define __VTOR_PRESENT            1U         /*!< Set to 1 if VTOR is present */
#define __NVIC_PRIO_BITS          3U         /*!< Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0U         /*!< Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT             1U         /*!< Set to 1 if FPU is present */
#define __FPU_DP                  0U         /*!< Set to 1 if FPU is double precision FPU (default is single precision FPU) */
#define __ICACHE_PRESENT          0U         /*!< Set to 1 if I-Cache is present */
#define __DCACHE_PRESENT          0U         /*!< Set to 1 if D-Cache is present */
#define __DTCM_PRESENT            0U         /*!< Set to 1 if DTCM is present */

#include "core_cm4.h"                        /* Processor and core peripherals */
#include "system_ln882h.h"                   /* System Header */

#include <stddef.h>

#define CRITICAL_SECT_START()   do{                                            \
                                       uint32_t __irq_mask = __get_PRIMASK(); \
                                       __disable_irq();
#define CRITICAL_SECT_END()               if(__irq_mask==0) {__enable_irq();}    \
                                   }while(0);

#define GLOBAL_INT_START()      __enable_irq()
#define GLOBAL_INT_STOP()       __disable_irq()
                                    
#define GLOBAL_INT_DISABLE()    CRITICAL_SECT_START()
#define GLOBAL_INT_RESTORE()    CRITICAL_SECT_END()

/* -------  Start of section using anonymous unions and disabling warnings  ------- */
#if   defined (__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined (__ICCARM__)
  #pragma language=extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wc11-extensions"
  #pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning 586
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif


/* =========================================================================================================================== */
/* ================                            Device Specific Peripheral Section                             ================ */
/* =========================================================================================================================== */



/* --------  End of section using anonymous unions and disabling warnings  -------- */
#if   defined (__CC_ARM)
  #pragma pop
#elif defined (__ICCARM__)
  /* leave anonymous unions enabled */
#elif (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
  #pragma clang diagnostic pop
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning restore
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif


/* =========================================================================================================================== */
/* ================                          Device Specific Peripheral Address Map                           ================ */
/* =========================================================================================================================== */
#if   defined (LN882H)
  #include "base_addr_ln882h.h"
  #include "mem_map_ln882h.h"
#else 
  #error Not supported device type
#endif

/**
 * @brief ln882h sdk version
 *
 * @Official Release URL: https://gitee.com/lightningsemi/ln882h
 *
 * LN882H_SDK_MAJOR_V:    0-255
 * LN882H_SDK_MINOR_V:    0-255
 */
#define LN882H_SDK_MAJOR_V        1
#define LN882H_SDK_MINOR_V        8

#define LN882H_SDK_RC_V           255/* 0: development, 255: release, 1-254: release candidates */

#define LN882H_SDK_RC_RELEASE     255
#define LN882H_SDK_RC_DEVELOPMENT 0

#define LN882H_SDK_VERSTR2(x)     #x
#define LN882H_SDK_VERSTR(x)      LN882H_SDK_VERSTR2(x)

#define LN882H_SDK_VERSION_IS_RELEASE     (LN882H_SDK_RC_V == LN882H_SDK_RC_RELEASE)
#define LN882H_SDK_VERSION_IS_DEVELOPMENT (LN882H_SDK_RC_V == LN882H_SDK_RC_DEVELOPMENT)
#define LN882H_SDK_VERSION_IS_RC          ((LN882H_SDK_RC_V != LN882H_SDK_RC_RELEASE) && (LN882H_SDK_RC_V != LN882H_SDK_RC_DEVELOPMENT))

#if LN882H_SDK_VERSION_IS_RELEASE
#define LN882H_SDK_VERSION_STRING_SUFFIX  ""
#elif LN882H_SDK_VERSION_IS_DEVELOPMENT
#define LN882H_SDK_VERSION_STRING_SUFFIX  "_alpha"
#else
#define LN882H_SDK_VERSION_STRING_SUFFIX  "_rc" LN882H_SDK_VERSTR(LN882H_SDK_RC_V)
#endif

#define LN882H_SDK_VERSION          ((uint32_t)(LN882H_SDK_MAJOR_V) << 24 |     \
                                     (uint32_t)(LN882H_SDK_MINOR_V) << 16  |    \
                                     (uint32_t)(LN882H_SDK_RC_V))
#define LN882H_SDK_VERSION_STRING   LN882H_SDK_VERSTR(LN882H_SDK_MAJOR_V) "."   \
                                    LN882H_SDK_VERSTR(LN882H_SDK_MINOR_V)       \
                                    LN882H_SDK_VERSION_STRING_SUFFIX
#define LN882H_SDK_BUILD_DATE_TIME  ""__DATE__" "__TIME__


#ifdef __cplusplus
}
#endif

#endif  /* __LN882H_H__ */
