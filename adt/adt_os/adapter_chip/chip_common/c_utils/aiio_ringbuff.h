/**
 * @brief   The declare of ring buffer interface
 * 
 * @file    aiio_ringbuff.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       declare some common ring buffer interface
 * 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-13          <td>1.0.0            <td>zhuolm             <td> ring buffer interface
 */
#ifndef __AIIO_RING_BUFF_H__
#define __AIIO_RING_BUFF_H__

#include <stdbool.h>
#include "stdint.h"

#define RINGBUFF_OK        0  /* No error, everything OK. */
#define RINGBUFF_ERR       -1 /* Out of memory error.     */
#define RINGBUFF_EMPTY     -3 /* Timeout.                	    */
#define RINGBUFF_FULL      -4 /* Routing problem.          */
#define RINGBUFF_TOO_SHORT -5

/**
 * @brief  The member of the ring buffer structure
 */
typedef struct _ring_buff_ {
    unsigned int size;
    unsigned int readpoint;
    unsigned int writepoint;
    char*        buffer;
    bool         full;
} ring_buff_t;

/**
 * @brief   Initialize the member of the ring buffer
 * 
 * @param[in]   ring_buff               The paramter of ring buffer
 * @param[in]   buff                    The paramter of memory space
 * @param[in]   size                    The paramter of memory space size 
 * 
 * @return  int                         Reture RINGBUFF_OK value
 */
int ring_buff_init(ring_buff_t* ring_buff, char* buff, unsigned int size);

/**
 * @brief   Clean the ring buffter
 * 
 * @param[in]   ring_buff       The paramter of ring buffer
 * 
 * @return  int                 Reture RINGBUFF_OK value
 */
int ring_buff_flush(ring_buff_t* ring_buff);

/**
 * @brief   Push data into the ring buffer
 * 
 * @param[in]   ring_buff               The paramter of ring buffer
 * @param[in]   pData                   The data to be push into the ring buffer
 * @param[in]   len                     The Max len of the data that is push into the ring buffer
 * 
 * @return  int                 Reture RINGBUFF_OK value
 */
int ring_buff_push_data(ring_buff_t* ring_buff, char* pData, int len);

/**
 * @brief   Pop data from the ring buffer
 * 
 * @param[in]   ring_buff               The paramter of ring buffer
 * @param[in]   pData                   The data to be push into the ring buffer
 * @param[in]   len                     The Max len of the data that is push into the ring buffer
 * 
 * @return  int                 Reture RINGBUFF_OK value
 */
int ring_buff_pop_data(ring_buff_t* ring_buff, char* pData, int len);


/**
 * @brief   Get the size of the ring buffer
 * 
 * @param[in]   ring_buff               The paramter of ring buffer
 * 
 * @return  int                 Reture RINGBUFF_OK value
 */
unsigned int ring_buff_get_size(ring_buff_t* ring_buff);

#endif  // __ringbuff_h__
