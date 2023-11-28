
#ifndef __FIFO_BUF_H__
#define __FIFO_BUF_H__

#include "ln_types.h"

#ifndef EOF
#define EOF (-1)
#endif

typedef struct
{
	uint8_t * volatile head;
	uint8_t * volatile tail;
	uint8_t *begin;
	uint8_t *end;
} fifo_buffer_t;


void     fifo_init(fifo_buffer_t *fb, uint8_t *buf, uint32_t size);

int8_t   fifo_isempty(const fifo_buffer_t *fb);
int8_t   fifo_isfull(const fifo_buffer_t *fb);

void     fifo_push(fifo_buffer_t *fb, uint8_t c);
uint8_t  fifo_pop(fifo_buffer_t *fb);

void     fifo_flush(fifo_buffer_t *fb);
uint32_t fifo_len(fifo_buffer_t *fb);



#endif /* __FIFO_BUF_H__ */
