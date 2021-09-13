/*
 * delay.c
 *
 *  Created on: Jun 3, 2021
 *      Author: igur
 */


#include "main.h"
#include "delay.h"

void delay_init()
{
	HAL_TIM_Base_Start(_TIMER);
}



void delay_us(uint16_t us)
{
	__HAL_TIM_SET_COUNTER(_TIMER, 0); // set CNT register to 0
	while(__HAL_TIM_GET_COUNTER(_TIMER) < us); // wait until timer reach us
}


void delay_ms(uint16_t ms)
{
	for(int i=0;i<ms;i++)
	{
		delay_us(1000);
	}
}


uint32_t get_tick_us()
{
	uint32_t tick = __HAL_TIM_GET_COUNTER(_TIMER);
	return tick;
}
