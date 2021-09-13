/*
 * parser.h
 *
 *  Created on: 11 wrz 2021
 *      Author: igur
 */

#ifndef INC_PARSER_H_
#define INC_PARSER_H_

#include "main.h"
#include "RingBuffer.h"
#include "string.h"


// function prototypes
void Parser_clean_string(RingBuffer_t* RX_buffer, uint8_t Destination_buffer[]);
uint8_t Parser_parse_message(char* message, uint8_t buffer[]);
uint8_t Parser_simple_parse(char* message, uint8_t buffer[]);

#endif /* INC_PARSER_H_ */
