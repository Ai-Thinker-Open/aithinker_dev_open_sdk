#ifndef STUB_MALLOC_H
#define STUB_MALLOC_H

#include <stdint.h>

#ifndef NULL
#define NULL 0
#endif

#define ERR_OK          0
#define ERR_RES         1
#define ERR_INVALID     2


// 内存池大小 60KB
#define MEM_MAX_SIZE           (1024 * 60)

// 分块最小单元 64B
#define MEM_BLOCK_SIZE         64

// 分块个数
#define MEM_ALLOC_TABLE_SIZE   (MEM_MAX_SIZE / MEM_BLOCK_SIZE)

#ifdef __cplusplus
extern "C" {
#endif

void    stub_mem_init(void);
void    stub_mem_free(void *ptr);
void    *stub_mem_malloc(uint32_t size);
void    *stub_mem_realloc(void *ptr, uint32_t size);
uint8_t stub_mem_usepercent(void);

#ifdef __cplusplus
}
#endif
#endif // !STUB_MALLOC_H
