/**
 * @brief   The declare of ring buffer interface
 * 
 * @file    aiio_ringbuff.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       declare some common ring buffer interface
 * 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-13          <td>1.0.0            <td>zhuolm             <td> ring buffer interface
 */
#include "aiio_ringbuff.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


int ring_buff_init(ring_buff_t* ring_buff, char* buff, unsigned int size)
{
    ring_buff->buffer     = buff;
    ring_buff->size       = size;
    ring_buff->readpoint  = 0;
    ring_buff->writepoint = 0;
    memset(ring_buff->buffer, 0, ring_buff->size);
    ring_buff->full = false;

    return RINGBUFF_OK;
}

int ring_buff_flush(ring_buff_t* ring_buff)
{
    ring_buff->readpoint  = 0;
    ring_buff->writepoint = 0;
    memset(ring_buff->buffer, 0, ring_buff->size);
    ring_buff->full = false;

    return RINGBUFF_OK;
}

int ring_buff_push_data(ring_buff_t* ring_buff, char* pData, int len)
{
    int i;

    if (len > ring_buff->size) {
        return RINGBUFF_TOO_SHORT;
    }

    for (i = 0; i < len; i++) {
        if (((ring_buff->writepoint + 1) % ring_buff->size) == ring_buff->readpoint) {
            ring_buff->full = true;
            return RINGBUFF_FULL;
        } else {
            if (ring_buff->writepoint < (ring_buff->size - 1)) {
                ring_buff->writepoint++;
            } else {
                ring_buff->writepoint = 0;
            }
            ring_buff->buffer[ring_buff->writepoint] = pData[i];
        }
    }

    return RINGBUFF_OK;
}

int ring_buff_pop_data(ring_buff_t* ring_buff, char* pData, int len)
{
    int i;

    if (len > ring_buff->size) {
        return RINGBUFF_TOO_SHORT;
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
            pData[i] = ring_buff->buffer[ring_buff->readpoint];
        }
    }

    return i;
}

unsigned int ring_buff_get_size(ring_buff_t* ring_buff)
{
    if (ring_buff->writepoint == ring_buff->readpoint) {
        return 0;
    }
    if (ring_buff->writepoint > ring_buff->readpoint)
        return ring_buff->writepoint - ring_buff->readpoint;
    else
        return ring_buff->size - (ring_buff->readpoint - ring_buff->writepoint);
}
