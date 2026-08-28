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

#include "axk_basic_ringbuff.h"

#include <string.h>

int32_t aiio_ring_buff_init(aiio_ring_buff_t *ring_buff, uint8_t *buff, uint32_t size)
{
    ring_buff->buffer     = buff;
    ring_buff->size       = size;
    ring_buff->readpoint  = 0;
    ring_buff->writepoint = 0;
    memset(ring_buff->buffer, 0, ring_buff->size);

    return AIIO_RINGBUFF_OK;
}

int32_t aiio_ring_buff_flush(aiio_ring_buff_t *ring_buff)
{
    ring_buff->readpoint  = 0;
    ring_buff->writepoint = 0;
    memset(ring_buff->buffer, 0, ring_buff->size);

    return AIIO_RINGBUFF_OK;
}

int32_t aiio_ring_buff_push_data(aiio_ring_buff_t *ring_buff, uint8_t *buff, uint32_t len)
{
    int i;

    if (len > ring_buff->size) {
        return AIIO_RINGBUFF_TOO_SHORT;
    }

    for (i = 0; i < len; i++) {
        if (((ring_buff->writepoint + 1) % ring_buff->size) == ring_buff->readpoint) {
            return AIIO_RINGBUFF_FULL;
        } else {
            if (ring_buff->writepoint < (ring_buff->size - 1)) {
                ring_buff->writepoint++;
            } else {
                ring_buff->writepoint = 0;
            }
            ring_buff->buffer[ring_buff->writepoint] = buff[i];
        }
    }

    return AIIO_RINGBUFF_OK;
}

int32_t aiio_ring_buff_pop_data(aiio_ring_buff_t *ring_buff, uint8_t *buff, uint32_t len)
{
    int i;

    if (len > ring_buff->size) {
        return AIIO_RINGBUFF_TOO_SHORT;
    }

    for (i = 0; i < len; i++) {
        if (ring_buff->writepoint == ring_buff->readpoint) {
            break;
        } else {
            if (ring_buff->readpoint == (ring_buff->size - 1)) {
                ring_buff->readpoint = 0;
            } else {
                ring_buff->readpoint++;
            }
            buff[i] = ring_buff->buffer[ring_buff->readpoint];
        }
    }

    return i;
}

uint32_t aiio_ring_buff_get_size(aiio_ring_buff_t *ring_buff)
{
    if (ring_buff->writepoint == ring_buff->readpoint) {
        return 0;
    }

    if (ring_buff->writepoint > ring_buff->readpoint)
        return ring_buff->writepoint - ring_buff->readpoint;
    else
        return ring_buff->size - (ring_buff->readpoint - ring_buff->writepoint);
}
