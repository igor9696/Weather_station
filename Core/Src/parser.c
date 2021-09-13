/*
 * parser.c
 *
 *  Created on: 11 wrz 2021
 *      Author: igur
 */


#include "parser.h"

void Parser_clean_string(RingBuffer_t* RX_buffer, uint8_t Destination_buffer[])
{
	uint8_t data_cnt = 0;


	uint8_t j = 0;

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
			Destination_buffer[j] = tmp;
			j++;
		}
	}

	Destination_buffer[j] = '\0';
}

uint8_t Parser_parse_message(char* message, uint8_t buffer[])
{
	if(strcmp(message, (char*)buffer) == 0)
	{
		return 1;
	}

	return 0;
}


uint8_t Parser_simple_parse(char* message, uint8_t buffer[])
{
	char *ptr;

	ptr = strstr((char*)buffer, message);
	if(ptr == NULL)
	{
		return 0;
	}

	else
	{
		return 1;
	}
}




