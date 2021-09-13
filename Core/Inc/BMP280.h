/*
 * BMP280.h
 *
 *  Created on: Sep 7, 2021
 *      Author: Igor Soroczynski
 */

// In order to use this library first call BMP280_Init function on your main.c


#ifndef INC_BMP280_H_
#define INC_BMP280_H_

#include "main.h"

#define CHIP_ID			0x58
#define I2C_Timeout		100    // 100ms

// sensor power modes
#define SLEEP_MODE		0
#define FORCED_MODE		1
#define NORMAL_MODE		3

// user registers
#define ID				0xD0
#define ctrl_meas		0xF4
#define config 			0xF5
#define press_msb		0xF7
#define press_lsb		0xF8
#define press_xlsb		0xF9
#define temp_msb		0xFA
#define temp_lsb		0xFB
#define temp_xlsb		0xFC

// compensation registers
#define dig_T1			0x88
#define dig_T2			0x8A
#define dig_T3			0x8C
#define dig_P1			0x8E
#define dig_P2			0x90
#define dig_P3			0x92
#define dig_P4			0x94
#define dig_P5			0x96
#define dig_P6			0x98
#define dig_P7			0x9A
#define dig_P8			0x9C
#define dig_P9			0x9E

typedef enum BMP_Status
{
	BMP280_NOK = 0,
	BMP280_OK,
}BMP_Status;

typedef enum BMP_Mode
{
	SLEEP,
	FORCED,
	NORMAL
}BMP_Mode;

typedef enum BMP_OVS_rate
{
	SKIPPED,
	x1,
	x2,
	x4,
	x8,
	x16,
}BMP_OVS_rate;

typedef enum BMP_filter_rate
{
	OFF,
	f_2,
	f_4,
	f_8,
	f_16,

}BMP_filter_rate;

typedef struct BMP280_t
{
	I2C_HandleTypeDef* 		hi2c;
	uint8_t 				I2C_Addr;    // device I2C address

	// compensation coefficients
	uint16_t 				dig_t1;
	int16_t 				dig_t2;
	int16_t 				dig_t3;

	uint16_t 				dig_p1;
	int16_t 				dig_p2;
	int16_t 				dig_p3;
	int16_t 				dig_p4;
	int16_t 				dig_p5;
	int16_t 				dig_p6;
	int16_t 				dig_p7;
	int16_t 				dig_p8;
	int16_t 				dig_p9;

}BMP280_t;

// User functions
BMP_Status BMP280_Init(I2C_HandleTypeDef* hi2c, uint8_t I2C_Address);
void BMP280_set_mode(BMP_Mode mode);
void BMP280_set_temp_OVS(BMP_OVS_rate Rate);
void BMP280_set_press_OVS(BMP_OVS_rate Rate);
void BMP280_set_filter(BMP_filter_rate Rate);
void BMP280_get_data_FORCED(int32_t* temperature, uint32_t* pressure);



#endif /* INC_BMP280_H_ */
