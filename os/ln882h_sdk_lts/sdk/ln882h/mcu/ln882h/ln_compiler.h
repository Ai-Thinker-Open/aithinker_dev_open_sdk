#ifndef  __LN_COMPILER_H__
#define  __LN_COMPILER_H__

#include "cmsis_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

/*------------------ RealView Compiler -----------------*/
#if defined(__CC_ARM)
  #define ARMCC_V5
  #define likely(x)             __builtin_expect(!!(x), 1)
  #define unlikely(x)           __builtin_expect(!!(x), 0)
  #define __UNUSED__            __attribute__((__unused__))
  #define __USED__              __attribute__((__used__))
  #define __PACKED__            __attribute__((packed))
  #define __ALIGNED__(x)        __attribute__((aligned(x)))
  #define __PURE__              __attribute__((__pure__))
  #define __CONST__             __attribute__((__const__))
  #define __NO_RETURN__         __attribute__((noreturn))
  #define __WEAK__              __attribute__((weak))
  #define __ZERO_INIT__         __attribute__((zero_init))

/*------------------ ARM Compiler V6 -------------------*/
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #define ARMCC_V6
  #define likely(x)             __builtin_expect(!!(x), 1)
  #define unlikely(x)           __builtin_expect(!!(x), 0)
  #define __UNUSED__            __attribute__((__unused__))
  #define __USED__              __attribute__((__used__))
  #define __PACKED__            __attribute__((packed))
  #define __ALIGNED__(x)        __attribute__((aligned(x)))
  #define __PURE__              __attribute__((__pure__))
  #define __CONST__             __attribute__((__const__))
  #define __NO_RETURN__         __attribute__((noreturn))
  #define __NAKED__             __attribute__((naked))
  #define __WEAK__              __attribute__((weak))
  #define __ZERO_INIT__         __attribute__((zero_init))

/*------------------ ICC Compiler ----------------------*/
#elif defined(__ICCARM__)  || defined(__ICC430__) // __IAR_SYSTEMS_ICC__
  #if (__VER__ >= 8000000)
    #define __ICCARM_V8 1
  #else
    #define __ICCARM_V8 0
  #endif

  #define likely(x)             (x)
  #define unlikely(x)           (x)
  #define __UNUSED__
  #define __USED__

  #if __ICCARM_V8
    #define __PACKED__          __attribute__((packed, aligned(1)))
  #else
    /* Needs IAR language extensions */
    #define __PACKED__          __packed
  #endif

  #if __ICCARM_V8
    #define __ALIGNED__(x)      __attribute__((aligned(x)))
  #elif (__VER__ >= 7080000)
    /* Needs IAR language extensions */
    #define __ALIGNED__(x)      __attribute__((aligned(x)))
  #else
    #warning No compiler specific solution for __ALIGNED.__ALIGNED is ignored.
    #define __ALIGNED__(x)
  #endif

  #define __ALIGNED__(x)
  #define __PURE__
  #define __CONST__
  #define __NO_RETURN__
  #define __NAKED__
  #define __WEAK__              __weak
  #define __ZERO_INIT__         // TODO:

/*------------------ GNU Compiler ----------------------*/
#elif defined(__GNUC__)
  #define likely(x)             __builtin_expect(!!(x), 1)
  #define unlikely(x)           __builtin_expect(!!(x), 0)
  #define __UNUSED__            __attribute__((__unused__))
  #define __USED__              __attribute__((__used__))
  #define __PACKED__            __attribute__((packed))
  #define __ALIGNED__(x)        __attribute__((aligned(x)))
  #define __PURE__              __attribute__((__pure__))
  #define __CONST__             __attribute__((__const__))
  #define __NO_RETURN__         __attribute__((noreturn))
  #define __NAKED__             __attribute__((naked))
  #define __WEAK__              __attribute__((weak))
  #define __ZERO_INIT__         // NOTE: not supported
#endif

// compiler same define
#ifndef __ASM__
  #define __ASM__               __asm
#endif

#ifndef __VOLATILE__
  #define __VOLATILE__          volatile
#endif

#ifndef __STATIC__
  #define __STATIC__            static
#endif

#ifndef __STATIC
  #define __STATIC              static
#endif

#ifndef __INLINE__
  #define __INLINE__            inline
#endif

#ifndef __STATIC_INLINE__
  #define __STATIC_INLINE__     static inline
#endif

#ifndef __ARRAY_EMPTY
    #define __ARRAY_EMPTY       0
#endif

#ifndef __MODULE__
    #define __MODULE__          __FILE__
#endif

#ifndef __BLEIRQ
    #define __BLEIRQ
#endif

#ifdef __cplusplus
}
#endif
#endif /* __LN_COMPILER_H__ */

