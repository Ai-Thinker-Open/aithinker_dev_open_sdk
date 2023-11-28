#ifndef __HAL_INTERRUPT_LN882H_H__
#define __HAL_INTERRUPT_LN882H_H__

#include <stdbool.h>
#include "hal/hal_common.h"

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

void set_interrupt_priority(void);
void switch_global_interrupt(hal_en_t enable);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __HAL_INTERRUPT_LN882H_H__ */
