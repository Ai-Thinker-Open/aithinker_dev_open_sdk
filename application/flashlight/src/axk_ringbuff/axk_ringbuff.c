#include "axk_ringbuff.h"
#include "aiio_adapter_include.h"

#define BUFFER_SIZE (4096)

static int32_t axk_ringbuff_init(void);
static int32_t axk_ringbuff_flush(void);
static int32_t axk_ringbuff_push_data(uint8_t *buff, uint32_t len);
static int32_t axk_ringbuff_pop_data(uint8_t *buff, uint32_t len);
static int32_t axk_ringbuff_get_size(void);

static aiio_ring_buff_t g_ringbuff = {0};
static uint8_t buffer[BUFFER_SIZE] = {0};

axk_ringbuff_t g_ringbuff_ctrl = 
{
    .init = axk_ringbuff_init,
    .flush = axk_ringbuff_flush,
    .push_data = axk_ringbuff_push_data,
    .pop_data = axk_ringbuff_pop_data,
    .get_size = axk_ringbuff_get_size,
};

static int32_t axk_ringbuff_init(void)
{
    return aiio_ring_buff_init(&g_ringbuff, buffer, BUFFER_SIZE);
}

static int32_t axk_ringbuff_flush(void)
{
    return aiio_ring_buff_flush(&g_ringbuff);
}

static int32_t axk_ringbuff_push_data(uint8_t *buff, uint32_t len)
{
    return aiio_ring_buff_push_data(&g_ringbuff, buff, len);
}

static int32_t axk_ringbuff_pop_data(uint8_t *buff, uint32_t len)
{
    return aiio_ring_buff_pop_data(&g_ringbuff, buff, len);
}

static int32_t axk_ringbuff_get_size(void)
{
    return aiio_ring_buff_get_size(&g_ringbuff);
}

