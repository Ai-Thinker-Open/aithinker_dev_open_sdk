

#ifndef __BLE_STACK_ARCH_H__
#define __BLE_STACK_ARCH_H__


#include "rwip_config.h"       // SW configuration
#include <stdint.h>            // standard integer definition
#include "ln_compiler.h"       // standard integer definition


#define CPU_WORD_SIZE   4      // ARM is a 32-bit CPU
#define CPU_LE          1      // ARM is little endian

/*
 * DEBUG configuration
 */
#if defined(CFG_DBG)
#define PLF_DEBUG          1
#else //CFG_DBG
#define PLF_DEBUG          0
#endif //CFG_DBG

// NVDS
#ifdef CFG_NVDS
#define PLF_NVDS           1
#else
#define PLF_NVDS           0
#endif//CFG_NVDS

// Possible errors detected by FW
#define RESET_NO_ERROR          (0x00000000)/// Reset platform (ok)
#define RESET_MEM_ALLOC_FAIL    (0xF2F2F2F2)/// Reset platform (mem alloc filed)
#define RESET_TO_ROM            (0xA5A5A5A5)/// Reset platform and stay in ROM
#define RESET_AND_LOAD_FW       (0xC3C3C3C3)/// Reset platform and reload FW

/// Exchange memory size limit
#if (BT_DUAL_MODE)
    #define    EM_SIZE_LIMIT          0x10000
#else
    #define    EM_SIZE_LIMIT          0x8000
#endif 


/*
 * ASSERTION CHECK
 */
#if PLF_DEBUG
    /// Assertions showing a critical error that could require a full system reset
    #define ASSERT_ERR(cond)                                                   \
        ASSERT_COMMON(cond, 0, 0, ASSERT_TYPE_ERROR)

    /// Assertions showing a critical error that could require a full system reset
    #define ASSERT_INFO(cond, param0, param1)                                  \
        ASSERT_COMMON(cond, param0, param1, ASSERT_TYPE_INFO)

    /// Assertions showing a non-critical problem that has to be fixed by the SW
    #define ASSERT_WARN(cond, param0, param1)                                  \
        ASSERT_COMMON(cond, param0, param1, ASSERT_TYPE_WARNING)

    #define ASSERT_COMMON(cond, param0, param1, type)                          \
        do {                                                                   \
            if (!(cond)) {                                                     \
                blib_assert((int)param0, (int)param1,                          \
                (int)type, __MODULE__, __LINE__);                              \
            }                                                                  \
        } while(0)
#else
    /// Assertions showing a critical error that could require a full system reset
    #define ASSERT_ERR(cond)
    
    /// Assertions showing a critical error that could require a full system reset
    #define ASSERT_INFO(cond, param0, param1)
    
    /// Assertions showing a non-critical problem that has to be fixed by the SW
    #define ASSERT_WARN(cond, param0, param1)
#endif //PLF_DEBUG


// required to define GLOBAL_INT_** macros as inline assembly. This file is included after
// definition of ASSERT macros as they are used inside ll.h
#ifndef CRITICAL_SECT_START
#define CRITICAL_SECT_START()   do{ uint32_t __irq_mask = __get_PRIMASK();  __disable_irq();
#endif

#ifndef CRITICAL_SECT_END 
#define CRITICAL_SECT_END()     if(__irq_mask==0) {__enable_irq();} }while(0);
#endif

#ifndef GLOBAL_INT_START 
#define GLOBAL_INT_START()      __enable_irq()
#endif

#ifndef GLOBAL_INT_STOP 
#define GLOBAL_INT_STOP()       __disable_irq()
#endif

#ifndef GLOBAL_INT_DISABLE 
#define GLOBAL_INT_DISABLE()    CRITICAL_SECT_START()
#endif

#ifndef GLOBAL_INT_RESTORE 
#define GLOBAL_INT_RESTORE()    CRITICAL_SECT_END()
#endif


enum assert_type
{
    ASSERT_TYPE_WARNING = 0,
    ASSERT_TYPE_ERROR   = 1,
    ASSERT_TYPE_INFO    = 2,
};


/**
 ****************************************************************************************
 * @brief  send ke message
 *
 * @param[in] param_ptr             Pointer to the parameter member of the message that
 *                                                         should be sent.
 ****************************************************************************************
 */
void ln_ke_msg_send(void const *param_ptr);

/**
 * @brief set ble mac address
 * @param[in] addr
 * @return 0 on success, -1 on failure
*/
int8_t ln_ble_mac_set(const uint8_t *addr);

/**
 * @brief set ble mac address
 * @param[out] addr
 * @return 0 on success, -1 on failure
*/
int8_t ln_ble_mac_get(uint8_t *addr);

void platform_reset(uint32_t error);
void rw_init(uint8_t mac[6]);
void rw_deinit(void);

#endif // __BLE_STACK_ARCH_H__
