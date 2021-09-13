/*
 * delay.h
 *
 *  Created on: Jun 3, 2021
 *      Author: igur
 */

#ifndef INC_DELAY_H_
#define INC_DELAY_H_

#include "tim.h"

#define _TIMER &htim2 // in projects settings define proper prescaler value for selected _TIMER (1Mhz)

void delay_init();
void delay_us(uint16_t us);
void delay_ms(uint16_t ms);
uint32_t get_tick_us();


#endif /* INC_DELAY_H_ */
