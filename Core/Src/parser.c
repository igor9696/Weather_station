/*
 * parser.c
 *
 *  Created on: 11 wrz 2021
 *      Author: igur
 */


#include "parser.h"

void Parser_clean_string(RingBuffer_t* RX_buffer, RingBuffer_t* Destination_buffer)
{
	uint8_t data_cnt = 0;

	data_cnt = RX_buffer->data_counter;
	for(int i=0; i < data_cnt - 2; i++)
	{
		uint8_t tmp;
		RB_Buff_Read(RX_buffer, &tmp);

		if((tmp=='\n') || (tmp=='\r'))
		{
			continue;
		}

		else
		{
			RB_Buff_Write(Destination_buffer, tmp);

//			Destination_buffer[j] = tmp;
//			j++;
		}
	}

	//Destination_buffer[j] = '\0';
}

uint8_t Parser_parse_message(char* message, uint8_t buffer[])
{
	if(strcmp(message, (char*)buffer) == 0)
	{
		return 1;
	}

	return 0;
}


uint8_t Parser_simple_parse(char* message, RingBuffer_t* source_buffer)
{
	char *ptr;

	ptr = strstr((char*)source_buffer->buffer, message);
	if(ptr == NULL)
	{
		return 0;
	}

	else
	{
		return 1;
	}
}




