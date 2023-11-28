#ifndef __LN_RUNTIME_H__
#define __LN_RUNTIME_H__

#include "ln_types.h"

/**
 * @brief initialize runtime measure feature.
 * initialize runtime measure feature.
 *
 * @attention 1. just called once.
 *            2. call it in the first phase of the main function, after system clock init.
 *
 * @return  none
 */
void  ln_runtime_measure_init(void);

/**
 * @brief get the current number of running cycles.
 * get the current number of running cycles.
 *
 * @attention None
 *
 * @return  cycles   current number of running cycles.
 */
uint32_t  ln_runtime_get_curr_cycles(void);

/**
 * @brief get delta running cycles.
 * get delta running cycles.
 *
 * @attention None
 *
 * @param[in]    last    last running cycles.
 * @param[in]    curr    current running cycles.
 *
 * @return  cycles   delta running cycles.
 */
uint32_t  ln_runtime_get_delta_cycles(uint32_t last, uint32_t curr);

/**
 * @brief get delta running ticks.
 * get delta running ticks.
 *
 * @attention None
 *
 * @param[in]    last    last running cycles.
 * @param[in]    curr    current running cycles.
 *
 * @return  cycles   delta running ticks.
 */
uint32_t  ln_runtime_get_delta_ticks(uint32_t last, uint32_t curr);

/**
 * @brief usec delay.
 * usec delay.
 *
 * @attention CPU will be blocked.
 *
 * @param[in]    us   how long is the delay time.
 *
 * @return  none.
 */
void  ln_block_delayus(uint32_t us);

/**
 * @brief ms delay.
 * ms delay.
 *
 * @attention CPU will be blocked.
 *
 * @param[in]    ms   how long is the delay time.
 *
 * @return  none.
 */
void  ln_block_delayms(uint32_t ms);

/**
 * @brief get the current running cycles.
 * get the current number of running cycles.
 *
 * @attention Call it in the critical section.
 *
 * @return  cycles   current number of running cycles.
 */
uint32_t  ln_os_tick_comp_pre_get_cycles(void);

/**
 * @brief increment os ticks.
 * increment os ticks.
 *
 * @attention Call it in the critical section.
 *
 * @param[in]    last   last running cycles.
 *
 * @return  none.
 */
void  ln_os_tick_comp_val_inc(uint32_t last);

/**
 * @brief
 *
 * @param comp_val
 */
void ln_get_os_tick_comp_val(uint16_t ** comp_val);

#endif /* __LN_RUNTIME_H__ */

