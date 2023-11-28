/**
 ****************************************************************************************
 *
 * @file co_int.h
 *
 * @brief This file replaces the need to include stdint or stdbool typical headers,
 *        which may not be available in all toolchains, and adds new types
 *
 * Copyright (C) RivieraWaves 2011-2021
 *
 * $Rev: $
 *
 ****************************************************************************************
 */

#ifndef _CO_INT_H_
#define _CO_INT_H_


/**
 ****************************************************************************************
 * @addtogroup CO_INT
 * @ingroup COMMON
 * @brief Common integer standard types (removes use of stdint)
 *
 * @{
 ****************************************************************************************
 */


/*
 * DEFINES
 ****************************************************************************************
 */


//STDINT----------------------------------------------------------------------------------

#if defined(CFG_RWTL) || defined(CFG_RWX1)
/// Exact-width integer types
#ifndef __int8_t_defined
    #define __int8_t_defined
    typedef signed char int8_t;
    typedef short int16_t;
    typedef int int32_t;
    typedef long long int64_t;
#endif

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

#ifndef __uint32_t_defined
    #define __uint32_t_defined
    typedef unsigned int uint32_t;
#endif
typedef unsigned long long uint64_t;
#else
#include <stdint.h>
#endif


/// @} CO_INT
#endif // _CO_INT_H_
