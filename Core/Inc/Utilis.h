/*
 * Utilis.h
 *
 *  Created on: 9 wrz 2021
 *      Author: igur
 */

#ifndef INC_UTILIS_H_
#define INC_UTILIS_H_

#include "main.h"
#include "stdio.h"
#include "usart.h"
#include "usart.h"


#define UART		&huart2 //define uart handler

// Function prototypes
void UART_send_string(char* message);
void UART_send_message(char* string, uint8_t string_size);



#endif /* INC_UTILIS_H_ */
