/**
 ****************************************************************************************
 *
 * @file co_math.h
 *
 * @brief Common optimized math functions
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _CO_MATH_H_
#define _CO_MATH_H_

/**
 *****************************************************************************************
 * @defgroup CO_MATH Math functions
 * @ingroup COMMON
 * @brief  Optimized math functions and other computations.
 *
 * @{
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>        // standard integer definitions
#include <stdbool.h>       // boolean definitions
#include <stdlib.h>        // standard library
#include "ln_compiler.h"      // for __STATIC_INLINE__
#include "arch.h"          // for ASSERT_ERR

extern void srand (unsigned int seed);
extern int rand (void);

/*
 * MACROS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Return value with one bit set.
 *
 * @param[in] pos Position of the bit to set.
 *
 * @return Value with one bit set.  There is no return type since this is a macro and this
 * will be resolved by the compiler upon assignment to an l-value.
 ****************************************************************************************
 */
#define CO_BIT(pos) (1UL<<(pos))

/**
 ****************************************************************************************
 * @brief Align val on the multiple of 4 equal or nearest higher.
 * @param[in] val Value to align.
 * @return Value aligned.
 ****************************************************************************************
 */
#define CO_ALIGN4_HI(val) (((val)+3)&~3)


/**
 ****************************************************************************************
 * @brief Align val on the multiple of 4 equal or nearest lower.
 * @param[in] val Value to align.
 * @return Value aligned.
 ****************************************************************************************
 */
#define CO_ALIGN4_LO(val) ((val)&~3)

/**
 ****************************************************************************************
 * @brief Align val on the multiple of 2 equal or nearest higher.
 * @param[in] val Value to align.
 * @return Value aligned.
 ****************************************************************************************
 */
#define CO_ALIGN2_HI(val) (((val)+1)&~1)


/**
 ****************************************************************************************
 * @brief Align val on the multiple of 2 equal or nearest lower.
 * @param[in] val Value to align.
 * @return Value aligned.
 ****************************************************************************************
 */
#define CO_ALIGN2_LO(val) ((val)&~1)

/**
 ****************************************************************************************
 * Perform a division and ceil up the result
 *
 * @param[in] val Value to divide
 * @param[in] div Divide value
 * @return ceil(val/div)
 ****************************************************************************************
 */
#define CO_DIVIDE_CEIL(val, div) (((val) + ((div) - 1))/ (div))

/**
 ****************************************************************************************
 * Perform a division and round the result
 *
 * @param[in] val Value to divide
 * @param[in] div Divide value
 * @return round(val/div)
 ****************************************************************************************
 */
#define CO_DIVIDE_ROUND(val, div) (((val) + ((div) >> 1))/ (div))

/**
 ****************************************************************************************
 * Perform a modulo operation
 *
 * @param[in] val    Dividend
 * @param[in] div    Divisor
 * @return  val/div)
 ****************************************************************************************
 */
//#define CO_MOD(val, div) ((val) % (div))
__STATIC_INLINE__ uint32_t co_mod(uint32_t val, uint32_t div)
{
   ASSERT_ERR(div);
   return ((val) % (div));
}
#define CO_MOD(val, div) co_mod(val, div)


/*
 * FUNCTION DEFINTIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Count leading zeros.
 * @param[in] val Value to count the number of leading zeros on.
 * @return Number of leading zeros when value is written as 32 bits.
 ****************************************************************************************
 */
__STATIC_INLINE__ uint32_t co_clz(uint32_t val)
{
#if defined(__CC_ARM)
    return __CLZ(val);
#elif defined(__arm__)
    return __builtin_clz(val);
#elif defined(__GNUC__)
    if (val == 0)
    {
        return 32;
    }
    return __builtin_clz(val);
#else
    uint32_t i;
    for (i = 0; i < 32; i++)
    {
        if (val & CO_BIT(31 - i))
            break;
    }
    return i;
#endif // defined(__arm__)
}

/**
 ****************************************************************************************
 * @brief Count trailing zeros.
 * @param[in] val Value to count the number of trailing zeros on.
 * @return Number of trailing zeros when value is written as 32 bits.
 ****************************************************************************************
 */
__STATIC_INLINE__ uint32_t co_ctz(uint32_t val)
{
#if defined(__CC_ARM)
    uint32_t i;
    for (i = 0; i < 32; i++)
    {
        if (val & CO_BIT(i))
            break;
    }
    return i;
#elif defined(__arm__)
    return __builtin_ctz(val);
#elif defined(__GNUC__)
    if (val == 0)
    {
        return 32;
    }
    return __builtin_ctz(val);
#else
    uint32_t i;
    for (i = 0; i < 32; i++)
    {
        if (val & CO_BIT(i))
            break;
    }
    return i;
#endif // defined(__arm__)
}
/**
 ****************************************************************************************
 * @brief Function to initialize the random seed.
 * @param[in] seed The seed number to use to generate the random sequence.
 ****************************************************************************************
 */
__STATIC_INLINE__ void co_random_init(uint32_t seed)
{
    srand(seed);
}

/**
 ****************************************************************************************
 * @brief Function to get an 8 bit random number.
 * @return Random byte value.
 ****************************************************************************************
 */
__STATIC_INLINE__ uint8_t co_rand_byte(void)
{
    return (uint8_t)(rand() & 0xFF);
}

/**
 ****************************************************************************************
 * @brief Function to get an 16 bit random number.
 * @return Random half word value.
 ****************************************************************************************
 */
__STATIC_INLINE__ uint16_t co_rand_hword(void)
{
    return (uint16_t)(rand() & 0xFFFF);
}

/**
 ****************************************************************************************
 * @brief Function to get an 32 bit random number.
 * @return Random word value.
 ****************************************************************************************
 */
__STATIC_INLINE__ uint32_t co_rand_word(void)
{
    return (uint32_t)rand();
}

/**
 ****************************************************************************************
 * @brief Function to return the smallest of 2 unsigned 32 bits words.
 * @return The smallest value.
 ****************************************************************************************
 */
__STATIC_INLINE__ uint32_t co_min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

/**
 ****************************************************************************************
 * @brief Function to return the smallest of 2 signed 32 bits words.
 * @return The smallest value.
 ****************************************************************************************
 */
__STATIC_INLINE__ int32_t co_min_s(int32_t a, int32_t b)
{
    return a < b ? a : b;
}

/**
 ****************************************************************************************
 * @brief Function to return the greatest of 2 unsigned 32 bits words.
 * @return The greatest value.
 ****************************************************************************************
 */
__STATIC_INLINE__ uint32_t co_max(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}

/**
 ****************************************************************************************
 * @brief Function to return the absolute value of a signed integer.
 * @return The absolute value.
 ****************************************************************************************
 */
__STATIC_INLINE__ int co_abs(int val)
{
    return val < 0 ? val*(-1) : val;
}

/// @} CO_MATH


#endif // _CO_MATH_H_
