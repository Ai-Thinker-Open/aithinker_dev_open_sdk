/** @brief      Ringbuffer.
 *
 *  @file       aiio_chip_init.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/28      <td>1.0.0       <td>lins        <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_RING_BUFF_H__
#define __AIIO_AT_RING_BUFF_H__

#include "aiio_type.h"

#define AIIO_RINGBUFF_OK        0  /* No error, everything OK. */
#define AIIO_RINGBUFF_ERR       -1 /* Out of memory error.     */
#define AIIO_RINGBUFF_EMPTY     -3 /* Timeout.                      */
#define AIIO_RINGBUFF_FULL      -4 /* Routing problem.          */
#define AIIO_RINGBUFF_TOO_SHORT -5

typedef struct {
    uint32_t size;
    uint32_t readpoint;
    uint32_t writepoint;
    uint8_t *buffer;
} aiio_ring_buff_t;

CORE_API int32_t aiio_ring_buff_init(aiio_ring_buff_t *ring_buff, uint8_t *buff, uint32_t size);
CORE_API int32_t aiio_ring_buff_flush(aiio_ring_buff_t *ring_buff);
CORE_API int32_t aiio_ring_buff_push_data(aiio_ring_buff_t *ring_buff, uint8_t *buff, uint32_t len);
CORE_API int32_t aiio_ring_buff_pop_data(aiio_ring_buff_t *ring_buff, uint8_t *buff, uint32_t len);
CORE_API uint32_t aiio_ring_buff_get_size(aiio_ring_buff_t *ring_buff);

/**
  * @brief  Get the number of bytes available in the ring buffer
  *
  * @param  ring_buff
  * @return The number of bytes available.
  */
CORE_API static inline uint32_t aiio_ring_buff_avail(aiio_ring_buff_t* ring_buff)
{
    return ring_buff->size - aiio_ring_buff_get_size(ring_buff);
}

#endif
