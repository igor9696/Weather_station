/*
 * dht11.h
 *
 *  Created on: Jul 25, 2021
 *      Author: igur
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include "main.h"

//#define DATA_PIN GPIO_PIN_1
//#define DATA_PORT GPIOA

#define LOW_HIGH_RESPONSE_TIME 80
#define TRANSMISION_LOGIC_0_TIME 40  // max time of high state that means '0'
#define RESPONSE_TIMEOUT 40


typedef struct dht11_sensor
{
	GPIO_TypeDef* Port;
	uint16_t Pin;

}dht11_sensor;


//typedef struct sensor_data
//{
//	uint8_t humidity;
//	uint8_t temperature;
//	uint8_t check_sum;
//
//}sensor_data;


// functions

void DHT11_Init(dht11_sensor* sensor, GPIO_TypeDef* _PORT, uint16_t _PIN);
void DHT11_get_data(dht11_sensor* sensor, uint8_t* humidity_val, int8_t* temp_val, uint8_t* check_sum);



#endif /* INC_DHT11_H_ */
