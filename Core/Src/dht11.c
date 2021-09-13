/*
 * dht11.c
 *
 *  Created on: Jul 25, 2021
 *      Author: igur
 */


#include "dht11.h"
#include "main.h"
#include "delay.h"


void DHT11_Init(dht11_sensor* sensor, GPIO_TypeDef* _PORT, uint16_t _PIN)
{
	sensor->Port = _PORT;
	sensor->Pin = _PIN;
}



static void change_to_output(dht11_sensor* sensor)
{
	GPIO_InitTypeDef gpio_data = {0};
	gpio_data.Pin = sensor->Pin;
	gpio_data.Mode = GPIO_MODE_OUTPUT_OD;
	gpio_data.Pull = GPIO_NOPULL;
	gpio_data.Speed = GPIO_SPEED_FREQ_LOW;


	HAL_GPIO_Init(sensor->Port, &gpio_data);
}


static void send_start_signal(dht11_sensor* sensor)
{
	HAL_GPIO_WritePin(sensor->Port, sensor->Pin, GPIO_PIN_RESET); // set Data pin to LOW
	delay_ms(18);
	HAL_GPIO_WritePin(sensor->Port, sensor->Pin, GPIO_PIN_SET); // set Data pin to HIGH
}


static void change_to_input(dht11_sensor* sensor)
{
	GPIO_InitTypeDef gpio_data = {0};
	gpio_data.Pin = sensor->Pin;
	gpio_data.Mode = GPIO_MODE_INPUT;
	gpio_data.Pull = GPIO_NOPULL;
	gpio_data.Speed = GPIO_SPEED_FREQ_LOW;


	HAL_GPIO_Init(sensor->Port, &gpio_data);

}


static void check_response(dht11_sensor* sensor)
{
	delay_us(RESPONSE_TIMEOUT);
	if(!(HAL_GPIO_ReadPin(sensor->Port, sensor->Pin)))
	{
		delay_us(LOW_HIGH_RESPONSE_TIME);
	}

	while((HAL_GPIO_ReadPin(sensor->Port, sensor->Pin)));
}


static uint8_t read_byte(dht11_sensor* sensor)
{
	uint8_t byte;
	for(int i=0; i<8; i++)
	{
		while(!(HAL_GPIO_ReadPin(sensor->Port, sensor->Pin)));
		delay_us(TRANSMISION_LOGIC_0_TIME);
		if(HAL_GPIO_ReadPin(sensor->Port, sensor->Pin))
		{
			//there is logic '1'
			byte |= (1<<(7-i));
		}

		else
		{
			// there is logic '0'
			byte &= ~(1<<(7-i));
		}
		//wait for pin to go low
		while(HAL_GPIO_ReadPin(sensor->Port, sensor->Pin));
	}

	return byte;
}


static void read_data(dht11_sensor* sensor, uint8_t* humidity_val, int8_t* temp_val, uint8_t* check_sum)
{
	uint8_t humidity_integral;
	uint8_t humidity_dec;
	uint8_t temp_integral;
	uint8_t temp_dec;

	humidity_integral = read_byte(sensor);
	humidity_dec = read_byte(sensor);
	temp_integral = read_byte(sensor);
	temp_dec = read_byte(sensor);

	*check_sum = read_byte(sensor);
	*humidity_val = humidity_integral;
	*temp_val = temp_integral;
}


void DHT11_get_data(dht11_sensor* sensor, uint8_t* humidity_val, int8_t* temp_val, uint8_t* check_sum)
{
	send_start_signal(sensor);
	change_to_input(sensor);
	check_response(sensor);
	read_data(sensor, humidity_val, temp_val, check_sum);
	change_to_output(sensor);
}



