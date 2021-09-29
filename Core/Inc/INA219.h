/*
 * INA219.h
 *
 *  Created on: Sep 20, 2021
 *      Author: igors
 */

#ifndef INC_INA219_H_
#define INC_INA219_H_

#include "main.h"

#define SHUNT_RESISTOR_VALUE			0.1 // 100mohm
#define MAXIMUM_EXPECTED_CURRENT		0.4	// Amps
#define FIXED_SCALING_FACTOR			0.04096
#define INA_I2C_Timeout					100 // miliseconds

// for selecting ADC mode
#define BUS_ADC							0
#define SHUNT_RESISTOR_ADC				1

// Registers
#define CONFIGURATION_REG				0x00
#define SHUNT_VOLTAGE_REG				0x01
#define BUS_VOLTAGE_REG					0x02
#define POWER_REG						0x03
#define CURRENT_REG						0x04
#define CALIBRATION_REG					0x05


typedef struct INA219_t
{
	uint8_t 					I2C_Addr;
	I2C_HandleTypeDef* 			hi2c;

	float 						Current_LSB;
	uint16_t					Cal_Reg_Value;

}INA219_t;

typedef enum INA219_Power_Modes
{
	POWER_DOWN,
	SHUNT_VOLTAGE_TRIGG,
	BUS_VOLTAGE_TRIGG,
	SHUNT_AND_BUS_TRIGG,
	ADC_OFF,
	SHUNT_VOLTAGE_CONT,
	BUS_VOLTAGE_CONT,
	SHUNT_AND_BUS_CONT,
}INA219_Power_Modes;

typedef enum INA219_Modes
{
	Range_40mV,
	Range_80mV,
	Range_160mV,
	Range_320mV,
}INA219_Modes;

typedef enum INA219_ADC_Mode
{
	ADC_9bit,
	ADC_10bit,
	ADC_11bit,
	ADC_12bit,
}INA219_ADC_Mode;

typedef enum INA219_VoltageRange
{
	Voltage_16V,
	Voltage_32V,
}INA219_VoltageRange;

// function prototypes
uint8_t INA219_Init(I2C_HandleTypeDef* hi2c, uint8_t device_addr);
void INA219_Set_Range(INA219_Modes Range);
void INA219_Set_ADC_Mode(INA219_ADC_Mode ADC_resolution, uint8_t type);
void INA219_Set_Power_Mode(INA219_Power_Modes Mode);
void INA219_Set_VoltageRange(INA219_VoltageRange range);
void INA219_Get_Data_OneShot(uint16_t* bus_voltage, uint8_t* current);


#endif /* INC_INA219_H_ */
