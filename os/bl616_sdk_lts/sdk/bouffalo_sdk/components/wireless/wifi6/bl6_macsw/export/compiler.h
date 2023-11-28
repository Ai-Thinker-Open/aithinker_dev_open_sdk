/**
 ****************************************************************************************
 *
 * @file riscv32/compiler.h
 *
 * @brief Definitions of compiler specific directives.
 *
 * Copyright (C) RivieraWaves 2011-2020
 *
 ****************************************************************************************
 */

#ifndef _COMPILER_H_
#define _COMPILER_H_

/// define the force inlining attribute for this compiler
#define __INLINE static __attribute__((__always_inline__)) inline

/// define the IRQ handler attribute for this compiler
#define __IRQ __attribute__((interrupt))

/// function has no side effect and return depends only on arguments
#define __PURE __attribute__((const))

/// Align instantiated lvalue or struct member on 4 bytes
#define __ALIGN4 __attribute__((aligned(4)))

/// Pack a structure field
#define __PACKED16 __attribute__ ((__packed__))
/// Pack a structure field
#define __PACKED __attribute__ ((__packed__))

/// __MODULE__ comes from the RVDS compiler that supports it
#define __MODULE__ __FILENAME__

/// define a variable as maybe unused, to avoid compiler warnings on it
#define __MAYBE_UNUSED __attribute__((unused))

#define __FALLTHROUGH __attribute__((fallthrough))

// Mapping of these different elements is already handled in the map.txt file, so no need
// to define anything here
/// SHARED RAM for IPC structure
#define __SHAREDRAMIPC __attribute__ ((section("SHAREDRAMIPC")))
/// SHARED RAM
#define __SHAREDRAM __attribute__ ((section("SHAREDRAM")))
/// LA RAM
#define __LARAMMAC __attribute__ ((section("LARAM")))
/// MIB memory
#define __MIB __attribute__ ((section("MACHWMIB")))

#endif // _COMPILER_H_
