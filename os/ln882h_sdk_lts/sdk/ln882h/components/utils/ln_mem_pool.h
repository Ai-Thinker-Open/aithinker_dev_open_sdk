#ifndef __LN_MEM_POOL_H__
#define __LN_MEM_POOL_H__

#include "ln882h.h"
#include "ln_types.h"
#include "ln_compiler.h"
#include "utils/debug/log.h"

#define CRITICAL_START()        do{  uint32_t __irq_mask = __get_PRIMASK();   \
                                        __disable_irq();
#define CRITICAL_END()               if(__irq_mask == 0) { __enable_irq(); } \
                                } while(0);

#define CHUNK_BUF_FLAG_ALLOC          (0xAA)
#define CHUNK_BUF_FLAG_FREE           (0x55)
#define MEM_POOL_CHUNK_INFO_SIZE      (2)

typedef struct
{
    volatile uint16_t free_chunk_cnt;       /**<  number of free chunks present   */
    uint16_t          total_chunk_cnt;      /**<  total chunks in the mem pool    */
    uint16_t          chunk_size;           /**<  size of the chunks in mem pool  */
    uint8_t          *mem_base;             /**<  point to the base of mem        */
    uint8_t         **free_chunk_ptr;       /**<  maintain the free chunks        */
} ln_mem_pool_t;

__STATIC_INLINE__ void ln_mem_pool_init(ln_mem_pool_t *mem_pool)
{
    /* +----------+-------------+----------------------+*/
    /* | chunk id |  chunk flag |   chunk buf          |*/
    /* +----------+-------------+----------------------+*/
    /* | 1 byte   |  1 byte     |   (buf_size) bytes   |*/
    /* +----------+-------------+----------------------+*/
    uint8_t * chunk           = mem_pool->mem_base;
    uint16_t  total_chunk_cnt = mem_pool->total_chunk_cnt;

    mem_pool->free_chunk_cnt  = 0;

    for(uint16_t i = 0; i < total_chunk_cnt; i++)
    {
        chunk[0] = i;
        chunk[1] = CHUNK_BUF_FLAG_FREE;

        mem_pool->free_chunk_ptr[mem_pool->free_chunk_cnt++] = chunk;
        chunk += mem_pool->chunk_size;
    }
}

__STATIC_INLINE__ void *ln_mem_pool_alloc(ln_mem_pool_t *mem_pool)
{
    uint8_t *chunk = NULL;

    CRITICAL_START();

    if(mem_pool->free_chunk_cnt == 0) {
        goto out;
    }

    chunk = mem_pool->free_chunk_ptr[--(mem_pool->free_chunk_cnt)];

    if(chunk[1] == CHUNK_BUF_FLAG_FREE) {
        chunk[1] = CHUNK_BUF_FLAG_ALLOC;
        chunk += MEM_POOL_CHUNK_INFO_SIZE;
    } else {
        LOG(LOG_LVL_ERROR, "[%s]alloc error(chunk flag)!\r\n", __func__);
        goto out;
    }

out:
    CRITICAL_END();
    return (void *)(chunk);
}

__STATIC_INLINE__ int ln_mem_pool_free(ln_mem_pool_t *mem_pool, void *addr)
{
    int ret = LN_TRUE;
 
    if ((addr == NULL) || (mem_pool == NULL)) {
        return LN_FALSE;
    }

    uint8_t * chunk = ((uint8_t *)addr) - MEM_POOL_CHUNK_INFO_SIZE;

    CRITICAL_START();

    // case 1: chunk id error.

    // case 2: alloc flag error.
    if(chunk[1] != CHUNK_BUF_FLAG_ALLOC) {
        ret =  LN_FALSE;
        LOG(LOG_LVL_ERROR, "[%s]free error(chunk flag)!\r\n", __func__);
        goto out;
    }

    // case 3: chunk index error.
    if(mem_pool->free_chunk_cnt >= mem_pool->total_chunk_cnt) {
        ret = LN_FALSE;
        LOG(LOG_LVL_ERROR, "[%s]free error(chunk index)!\r\n", __func__);
        goto out;
    }

    mem_pool->free_chunk_ptr[(mem_pool->free_chunk_cnt)++] = chunk;
    chunk[1] = CHUNK_BUF_FLAG_FREE;
    ret = LN_TRUE;

out:
    CRITICAL_END();
    return ret;
}


#endif /* __LN_MEM_POOL_H__ */

