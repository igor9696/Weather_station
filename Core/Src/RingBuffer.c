/*
 * RingBuffer.c
 *
 *  Created on: Aug 28, 2021
 *      Author: igur
 */

#include "RingBuffer.h"


void RingBuffer_Init(RingBuffer_t* RingBuffer)
{
	RingBuffer->head = 0;
	RingBuffer->tail = 0;
	RingBuffer->BUFFER_FULL_FLAG = 0;
	RingBuffer->BUFFER_EMPTY_FLAG = 1;
	RingBuffer->data_counter = 0;
}


static void buffer_write_char(RingBuffer_t* RingBuffer, char* c)
{
	if(c >= 0)
	{
		if(((RingBuffer->head + 1) % BUFFER_SIZE) != RingBuffer->tail)
		{
			RingBuffer->BUFFER_EMPTY_FLAG = 0;
			RingBuffer->buffer[RingBuffer->head] = *c;
			RingBuffer->head++;
			RingBuffer->head %= BUFFER_SIZE;
			RingBuffer->data_counter++;
		}

		else
		{
			RingBuffer->BUFFER_FULL_FLAG = 1;
		}
	}
}


void RB_Buff_Write_String(RingBuffer_t* RingBuffer, char* string)
{
	do
	{
		buffer_write_char(RingBuffer, string);
	}
	while(*string++ && (!RingBuffer->BUFFER_FULL_FLAG));
}


void RB_Buff_Write(RingBuffer_t* RingBuffer, uint8_t value)
{
	if(((RingBuffer->head + 1) % BUFFER_SIZE) != RingBuffer->tail)
	{
		RingBuffer->BUFFER_EMPTY_FLAG = 0;
		RingBuffer->buffer[RingBuffer->head] = value;
		RingBuffer->head++;
		RingBuffer->head %= BUFFER_SIZE;
		RingBuffer->data_counter++;
	}
	else
	{
		RingBuffer->BUFFER_FULL_FLAG = 1;
	}
}



void RB_Buff_Read(RingBuffer_t* RingBuffer, uint8_t* data)
{
	if(RingBuffer->tail != RingBuffer->head)
	{
		RingBuffer->BUFFER_FULL_FLAG = 0;
		*data = RingBuffer->buffer[RingBuffer->tail];
		RingBuffer->tail++;
		RingBuffer->tail %= BUFFER_SIZE;
		RingBuffer->data_counter--;
	}

	else
	{
		RingBuffer->BUFFER_EMPTY_FLAG = 1;
	}
}


uint8_t RB_is_empty(RingBuffer_t* RingBuffer)
{
	if(RingBuffer->BUFFER_EMPTY_FLAG) return 1;

	return 0;
}


uint8_t RB_is_full(RingBuffer_t* RingBuffer)
{
	if(RingBuffer->BUFFER_FULL_FLAG) return 1;

	return 0;
}


uint8_t RB_get_data_counter(RingBuffer_t* RingBuffer)
{
	return RingBuffer->data_counter;
}

void RB_Flush(RingBuffer_t* RingBuffer)
{
	// clear buffer
	RingBuffer->head = 0;
	RingBuffer->tail = 0;
	RingBuffer->data_counter = 0;
}
