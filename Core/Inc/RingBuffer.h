/*
 * RingBuffer.h
 *
 *  Created on: Aug 28, 2021
 *      Author: igur
 */

#ifndef INC_RINGBUFFER_H_
#define INC_RINGBUFFER_H_
#include "main.h"

#define BUFFER_SIZE		128


typedef struct RingBuffer_t
{
	uint8_t 	tail;
	uint8_t 	head;
	uint8_t 	buffer[BUFFER_SIZE];

	uint8_t 	BUFFER_FULL_FLAG;
	uint8_t		BUFFER_EMPTY_FLAG;

	uint8_t 	data_counter;

}RingBuffer_t;

// function prototypes

void RingBuffer_Init(RingBuffer_t* RingBuffer);
void RB_Buff_Write_String(RingBuffer_t* RingBuffer, char* string);
void RB_Buff_Write(RingBuffer_t* RingBuffer, uint8_t value);
void RB_Buff_Read(RingBuffer_t* RingBuffer, uint8_t* data);
void RB_Flush(RingBuffer_t* RingBuffer);

uint8_t RB_is_empty(RingBuffer_t* RingBuffer);
uint8_t RB_is_full(RingBuffer_t* RingBuffer);
uint8_t RB_get_data_counter(RingBuffer_t* RingBuffer);


#endif /* INC_RINGBUFFER_H_ */
