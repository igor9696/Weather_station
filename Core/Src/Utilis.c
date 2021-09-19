/*
 * Utilis.c
 *
 *  Created on: 9 wrz 2021
 *      Author: igur
 */

#include "Utilis.h"

void UART_send_string(char* message)
{
	uint8_t length;
	uint8_t message_buf[64];
	length = sprintf((char*)message_buf, message);
	HAL_UART_Transmit(UART, message_buf, length, 100);
}

void UART_send_message(char* string, uint8_t string_size)
{
	HAL_UART_Transmit(UART, (uint8_t*)string, string_size, 100);
}


