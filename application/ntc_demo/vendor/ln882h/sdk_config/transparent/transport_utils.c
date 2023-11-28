#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "transport_utils.h"

int resolve_dns(const char *host, struct sockaddr_in *ip) {

    struct hostent *he;
    struct in_addr **addr_list;
    he = gethostbyname(host);
    if (he == NULL) {
        return LN_FAIL;
    }
    addr_list = (struct in_addr **)he->h_addr_list;
    if (addr_list[0] == NULL) {
        return LN_FAIL;
    }
    ip->sin_family = AF_INET;
    memcpy(&ip->sin_addr, addr_list[0], sizeof(ip->sin_addr));
    return LN_OK;
}

void Fifo_2_Init_Buffer(fifo2_buffer_param_t *uart_no,uint8_t* buffer_addr,uint32_t buffer_max)
{
	uart_no -> buffer_p = buffer_addr;
	uart_no -> buffer_tail = 0;
	uart_no -> buffer_head = 0;
	uart_no -> set_flag = 0;
	uart_no -> blocks_flag = 0;
	uart_no -> blocks_num = 0;
	uart_no -> data_len = 0;
	uart_no -> buffer_len = buffer_max;
}

/**************************************FIFO**************************************/
uint32_t Fifo_2_Save_Char(fifo2_buffer_param_t *uart_no,uint8_t ch)
{
	if(uart_no->buffer_head - uart_no->buffer_tail == 1)
		return 0;

	if(uart_no->buffer_tail == uart_no->buffer_len)
		uart_no->buffer_tail = 0;

	uart_no->buffer_p[uart_no->buffer_tail] = ch;

	uart_no->buffer_tail++;

	return 0;
}

uint32_t Fifo_2_Save_Data(fifo2_buffer_param_t *uart_no,uint8_t *data,uint32_t len)
{
	int write_len, first_write_len, second_write_len;
	
	if((len == 0) || (uart_no->buffer_head - uart_no->buffer_tail == 1))
		return 0;
	
	if(uart_no->buffer_head <= uart_no->buffer_tail)
	{
		if(uart_no->buffer_head + uart_no->buffer_len - uart_no->buffer_tail > len)
			write_len = len;
		else
			write_len = uart_no->buffer_head + uart_no->buffer_len - uart_no->buffer_tail;

		first_write_len = write_len > uart_no->buffer_len - uart_no->buffer_tail? uart_no->buffer_len - uart_no->buffer_tail : write_len;
		second_write_len = write_len - first_write_len;
			
		memcpy(uart_no->buffer_p + uart_no->buffer_tail, data, first_write_len);

		if(second_write_len <= 0)
			uart_no->buffer_tail += first_write_len;
		else
		{
			memcpy(uart_no->buffer_p, data + first_write_len, second_write_len);
			uart_no->buffer_tail = second_write_len;
		}
	}
	else
	{
		if(uart_no->buffer_head - uart_no->buffer_tail > len)
			write_len = len;
		else
			write_len = uart_no->buffer_head - uart_no->buffer_tail -1; 

		memcpy(uart_no->buffer_p + uart_no->buffer_tail, data, write_len);
	}


	return write_len;
}

uint32_t Fifo_2_Get_Char(fifo2_buffer_param_t *uart_no,uint8_t *ch)
{
	if( uart_no->buffer_head == uart_no->buffer_tail)
		return 0;

	if(uart_no->buffer_head == uart_no->buffer_len)
		uart_no->buffer_head = 0;

	*ch = uart_no->buffer_p[uart_no->buffer_head];

	uart_no->buffer_head++;

	return 1;
}


uint32_t Fifo_2_Get_Data(fifo2_buffer_param_t *uart_no,uint8_t *data,uint32_t len)
{

	int read_len, first_read_len, second_read_len;;

	if(len == 0 || uart_no->buffer_head == uart_no->buffer_tail)
		return 0;

	if(uart_no->buffer_head < uart_no->buffer_tail)
	{
		if(uart_no->buffer_tail - uart_no->buffer_head >= read_len)
			read_len = len;
		else
			read_len = uart_no->buffer_tail - uart_no->buffer_head;
		
		memcpy(data, uart_no->buffer_p + uart_no->buffer_head, read_len);
		uart_no->buffer_head += read_len;
	}
	else
	{
		if(uart_no->buffer_len - uart_no->buffer_head + uart_no->buffer_tail > len)
			read_len = len;
		else
			read_len = uart_no->buffer_len - uart_no->buffer_head + uart_no->buffer_tail;

		first_read_len = read_len > uart_no->buffer_len - uart_no->buffer_head?  uart_no->buffer_len - uart_no->buffer_head : read_len;
		second_read_len = read_len - first_read_len;

		memcpy(data, uart_no->buffer_p + uart_no->buffer_head, first_read_len);
		if(second_read_len <= 0)
			uart_no->buffer_head += first_read_len;
		else
		{
			memcpy(data + first_read_len, uart_no->buffer_p, second_read_len);
			uart_no->buffer_head = second_read_len;
		}
	}
	
	return read_len;
}

uint32_t Fifo_2_Get_Buflen(fifo2_buffer_param_t *uart_no)
{
	if(uart_no->buffer_head <= uart_no->buffer_tail)
		return uart_no->buffer_tail - uart_no->buffer_head;
	else
		return uart_no->buffer_len - uart_no->buffer_head + uart_no->buffer_tail;
}
