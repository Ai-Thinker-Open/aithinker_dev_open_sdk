#ifndef _AXK_RINGBUFF_H_
#define _AXK_RINGBUFF_H_

#include <stdint.h>
#include "axk_basic_ringbuff.h"

typedef struct axk_ringbuff
{
    int32_t (*init)(void);
    int32_t (*flush)(void);
    int32_t (*push_data)(uint8_t *buff, uint32_t len);
    int32_t (*pop_data)(uint8_t *buff, uint32_t len);
    int32_t (*get_size)(void);
} axk_ringbuff_t;

extern axk_ringbuff_t g_ringbuff_ctrl;


#endif //_AXK_RINGBUFFER_H_
