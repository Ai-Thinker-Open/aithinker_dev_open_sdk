#include "stub_malloc.h"
#include "ln_compiler.h"

#ifndef INVALID_ADDR
#define INVALID_ADDR    0xFFFFFFFF
#endif

// 内存池
static uint8_t sg_membase[MEM_MAX_SIZE] __ALIGNED__(8);

// 内存分块大小
static const uint32_t sg_memblksize = MEM_BLOCK_SIZE;

// 内存管理表
static uint32_t sg_memmapbase[MEM_ALLOC_TABLE_SIZE];

// 内存表大小
static const uint32_t sg_memblkcnt = MEM_ALLOC_TABLE_SIZE;

// 内存总大小
static const uint32_t sg_memsize = MEM_MAX_SIZE;

typedef struct mm_ctrl
{
	void        (*init)(void);
	uint8_t     (*perused)(void);
	uint8_t     memrdy;
	uint8_t     *membase;
	uint32_t    *memmap;
} mm_ctrl_t;

static mm_ctrl_t sg_mm_ctrl = {
    .init       = stub_mem_init,
    .perused    = stub_mem_usepercent,
    .memrdy     = 0,
    .membase    = sg_membase,
    .memmap     = sg_memmapbase
};


/*******************************  API 声明  ***********************************/
static void     _stub_memset(void *s, uint8_t c, uint32_t count);
static void     _stub_memcpy(void *des, void *src, uint32_t n);
static uint32_t _stub_malloc(uint32_t size);
static uint8_t  _stub_mfree(uint32_t offset);

/*******************************  函数实现  ***********************************/
void _stub_memset(void *s, uint8_t c, uint32_t count)
{
    uint8_t *xs = s;

    for (uint32_t i = 0; i < count; i++) {
        *(xs + i) = c;
    }
}

void _stub_memcpy(void *des, void *src, uint32_t n)
{
    uint8_t *xdes = des;
    uint8_t *xsrc = src;

    for (int i = 0; i < n; i++) {
        *xdes++ = *xsrc++;
    }
}

uint32_t _stub_malloc(uint32_t size)
{
    long        offset = 0;
    uint32_t    nmemb;
    uint32_t    cmemb = 0;
    uint32_t    i;

    if (!sg_mm_ctrl.memrdy) {
        sg_mm_ctrl.init();
    }

    if (0 == size) {
        return INVALID_ADDR;
    }

    nmemb = size / sg_memblksize;
    if (size % sg_memblksize) {
        nmemb++;
    }

    for (offset = sg_memblkcnt - 1; offset >= 0; offset--) {
        if (!sg_mm_ctrl.memmap[offset]) {
            cmemb++;
        } else {
            cmemb = 0;
        }

        if (cmemb == nmemb)
        {
            for (i = 0; i < nmemb; i++)
            {
                sg_mm_ctrl.memmap[offset + i] = nmemb;
            }
            return (offset * sg_memblksize);
        }
    }
    return INVALID_ADDR;
}

uint8_t _stub_mfree(uint32_t offset)
{
    int i;

    if (!sg_mm_ctrl.memrdy) {
        sg_mm_ctrl.init();
        return ERR_RES;
    }

    if (offset < sg_memsize) {
        int index = offset / sg_memblksize;
        int nmemb = sg_mm_ctrl.memmap[index];
        for (i = 0; i < nmemb; i++) {
            sg_mm_ctrl.memmap[index + i] = 0;
        }
        return ERR_OK;
    }

    return ERR_INVALID;
}

void stub_mem_init(void)
{
    _stub_memset(sg_mm_ctrl.memmap, 0, sg_memblkcnt * sizeof(sg_mm_ctrl.memmap[0]));
    sg_mm_ctrl.memrdy = 1;
}

void stub_mem_free(void *ptr)
{
    uint32_t offset;
    if (ptr == NULL)
        return;
    offset = (uint32_t)ptr - (uint32_t)sg_mm_ctrl.membase;
    _stub_mfree(offset);
}

void *stub_mem_malloc(uint32_t size)
{
    uint32_t offset;
    offset = _stub_malloc(size);
    if (offset == INVALID_ADDR)
        return NULL;
    else
        return (void *)((uint32_t)sg_mm_ctrl.membase + offset);
}

void *stub_mem_realloc(void *ptr, uint32_t size)
{
    uint32_t offset;

    offset = _stub_malloc(size);
    if (offset == INVALID_ADDR) {
        return NULL;
    } else {
        _stub_memcpy((void *)((uint32_t)sg_mm_ctrl.membase + offset), ptr, size);
        stub_mem_free(ptr);
        return (void *)((uint32_t)sg_mm_ctrl.membase + offset);
    }
}

uint8_t stub_mem_usepercent(void)
{
    uint32_t used = 0;
    uint32_t i;

    for (i = 0; i < sg_memblkcnt; i++) {
        if (sg_mm_ctrl.memmap[i]) {
            used++;
        }
    }

    return (used * 100) / (sg_memblkcnt);
}
