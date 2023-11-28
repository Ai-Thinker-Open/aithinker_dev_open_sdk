#include "fifobuf.h"
#include "utils/debug/ln_assert.h"
#include "ln882h.h"

                                   
static int8_t fifo_isfull_nonsafety(const fifo_buffer_t *fb)
{
	return ((fb->head == fb->begin) && (fb->tail == fb->end)) || (fb->tail == fb->head - 1);
}

int8_t fifo_isfull(const fifo_buffer_t *fb)
{
	int8_t result;

    CRITICAL_SECT_START();
	result = fifo_isfull_nonsafety(fb);    
    CRITICAL_SECT_END();
    
	return result;
}

int8_t fifo_isempty(const fifo_buffer_t *fb)
{
	return fb->head == fb->tail;
}


void fifo_push(fifo_buffer_t *fb, uint8_t c)
{
    CRITICAL_SECT_START();    
	if(!fifo_isfull_nonsafety(fb))
	{
		/* fifo not full */
		if ((fb->tail >= fb->begin) && (fb->tail < fb->end))
		{
			/* Write at tail position */
			*(fb->tail) = c;
			/* Move tail forward */
			fb->tail++;
		}
		else if (fb->tail == fb->end)
		{
			/* Write at tail position */
			*(fb->tail) = c;
			/* wrap tail around */
			fb->tail = fb->begin;
		}
		else
		{
			/* invalid */
			LN_ASSERT(0);
		}
	}
    CRITICAL_SECT_END();
}

uint8_t fifo_pop(fifo_buffer_t *fb)
{
	uint8_t ch = 0;

    CRITICAL_SECT_START();
	if (!fifo_isempty(fb))
	{
		if ((fb->head >= fb->begin) && (fb->head < fb->end))
		{
			/* get char and move head forward */
			ch = *(fb->head++);
		}
		else if (fb->head == fb->end)
		{
			/* get char and wrap head around */
			ch = *(fb->end);
			fb->head = fb->begin;
		}
		else
		{
			/* invalid */
			LN_ASSERT(0);
		}
	}
    CRITICAL_SECT_END();

	return ch;
}

void fifo_flush(fifo_buffer_t *fb)
{
    CRITICAL_SECT_START();
    fb->head = fb->tail = fb->begin;
    CRITICAL_SECT_END();
}


void fifo_init(fifo_buffer_t *fb, uint8_t *buf, uint32_t size)
{
	/* FIFO buffers have a known bug with 1-byte buffers. */
    LN_ASSERT(size > 1);

    fb->head = fb->tail = fb->begin = buf;
    fb->end = buf + size - 1;
}

uint32_t fifo_len(fifo_buffer_t *fb)
{
	return fb->end - fb->begin + 1;
}


