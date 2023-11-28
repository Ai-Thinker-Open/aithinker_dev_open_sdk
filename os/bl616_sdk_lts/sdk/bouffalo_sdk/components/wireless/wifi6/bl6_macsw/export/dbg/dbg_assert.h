/**
 ****************************************************************************************
 *
 * @file dbg_assert.h
 *
 * @brief File containing the definitions of the assertion macros.
 *
 * Copyright (C) RivieraWaves 2011-2021
 *
 ****************************************************************************************
 */

#ifndef _DBG_ASSERT_H_
#define _DBG_ASSERT_H_

#ifdef __cplusplus
extern "C" {
#endif
/**
 ****************************************************************************************
 * @defgroup ASSERT ASSERT
 * @ingroup DEBUG
 * @brief Assertion management module
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "export/rwnx_config.h"
#include "compiler.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#if NX_TRACE
/// Line number: Include file id if trace is enabled
#define LINE_NB ((TRACE_FILE_ID << 20) + (__LINE__ & 0xfffff))
#else
/// Line number
#define LINE_NB __LINE__
#endif

/*
 * ASSERTION CHECK
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Force the trigger of the logic analyzer
 *
 * @param[in] msg Error condition that caused the trigger to be forced.
 *
 ****************************************************************************************
 */
void dbg_force_trigger(const char *msg);

/**
 ****************************************************************************************
 * @brief Print the assertion error reason and trigger the recovery procedure.
 *
 * @param[in] condition C string containing the condition.
 * @param[in] file C string containing file where the assertion is located.
 * @param[in] line Line number in the file where the assertion is located.
 ****************************************************************************************
 */
void dbg_assert_rec(const char * file, int line);

/**
 ****************************************************************************************
 * @brief Print the assertion error reason and loop forever.
 *
 * @param[in] condition C string containing the condition.
 * @param[in] file C string containing file where the assertion is located.
 * @param[in] line Line number in the file where the assertion is located.
 ****************************************************************************************
 */
void dbg_assert_err(const char * file, int line);

/**
 ****************************************************************************************
 * @brief Print the assertion warning reason.
 *
 * @param[in] condition C string containing the condition.
 * @param[in] file C string containing file where the assertion is located.
 * @param[in] line Line number in the file where the assertion is located.
 ****************************************************************************************
 */
void dbg_assert_warn(const char * file, int line);

/// Macro defining the format of the assertion calls
#if (NX_DEBUG > 1)
#define ASSERT(type, cond) dbg_assert_##type(__MODULE__, LINE_NB)
#elif (NX_DEBUG > 0)
#define ASSERT(type, cond) dbg_assert_##type("", __MODULE__, LINE_NB)
#else
#define ASSERT(type, cond) dbg_assert_##type("", "", 0)
#endif

#if NX_DEBUG
/// Assertions showing a critical error that could require a full system reset
#define ASSERT_ERR(cond)                                                                 \
    do {                                                                                 \
        if (!(cond)) {                                                                   \
            ASSERT(err, #cond);                                                          \
        }                                                                                \
    } while(0)

/// Assertions showing a critical error that could require a full system reset
#define ASSERT_ERR2(cond, param0, param1) ASSERT_ERR(cond)

/// Assertions showing a non-critical problem that has to be fixed by the SW
#define ASSERT_WARN(cond)                                                                \
    do {                                                                                 \
        if (!(cond)) {                                                                   \
            ASSERT(warn, #cond);                                                         \
        }                                                                                \
    } while(0)
#else
/// Assertions showing a critical error that could require a full system reset
#define ASSERT_ERR(cond)
/// Assertions showing a critical error that could require a full system reset
#define ASSERT_ERR2(cond, param0, param1)
/// Assertions showing a non-critical problem that has to be fixed by the SW
#define ASSERT_WARN(cond)
#endif

#if NX_RECOVERY
/// Assertions that trigger the automatic recovery mechanism and return void
#define ASSERT_REC(cond)                                                                 \
    ({                                                                                   \
        if (!(cond)) {                                                                   \
            ASSERT(rec, #cond);                                                          \
            return;                                                                      \
        }                                                                                \
    })

/// Assertions that trigger the automatic recovery mechanism and return a value
#define ASSERT_REC_VAL(cond, ret)                                                        \
    ({                                                                                   \
        if (!(cond)) {                                                                   \
            ASSERT(rec, #cond);                                                          \
            return (ret);                                                                \
        }                                                                                \
    })

/// Assertions that trigger the automatic recovery mechanism and do not return
#define ASSERT_REC_NO_RET(cond)                                                          \
    ({                                                                                   \
        if (!(cond)) {                                                                   \
            ASSERT(rec, #cond);                                                          \
        }                                                                                \
    })
#else
/// Assertions that trigger the automatic recovery mechanism and return void
#define ASSERT_REC(cond)             ASSERT_ERR(cond)

/// Assertions that trigger the automatic recovery mechanism and return a value
#define ASSERT_REC_VAL(cond, ret)    ASSERT_ERR(cond)

/// Assertions that trigger the automatic recovery mechanism and do not return
#define ASSERT_REC_NO_RET(cond)      ASSERT_ERR(cond)
#endif


/// @}  // end of group ASSERT

#ifdef __cplusplus
}
#endif
#endif // _DBG_ASSERT_H_
