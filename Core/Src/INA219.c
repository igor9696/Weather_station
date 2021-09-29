/*
 * INA219.c
 *
 *  Created on: Sep 20, 2021
 *      Author: igors
 */


#include "INA219.h"
#include "math.h"
#include "stdio.h"

INA219_t INA_Sensor;

static uint16_t read16(uint8_t memory_addres)
{
	uint8_t tmp_buff[2];
	HAL_I2C_Mem_Read(INA_Sensor.hi2c, INA_Sensor.I2C_Addr, memory_addres, 1, tmp_buff, 2, INA_I2C_Timeout);

	return (uint16_t)(tmp_buff[0] << 8) | (tmp_buff[1]);
}


static void write16(uint8_t memory_addres, uint16_t data_to_write)
{
	uint8_t tmp_buff[2];
	tmp_buff[0] = data_to_write >> 8;
	tmp_buff[1] = data_to_write;
	HAL_I2C_Mem_Write(INA_Sensor.hi2c, INA_Sensor.I2C_Addr, memory_addres, 1, tmp_buff, 2, INA_I2C_Timeout);
}


void INA219_Set_VoltageRange(INA219_VoltageRange range)
{
	uint16_t tmp = read16(CONFIGURATION_REG);
	tmp &= 0xDFFF;

	tmp |= range << 13;

	write16(CONFIGURATION_REG, tmp);

}


void INA219_Set_Range(INA219_Modes Range)
{
	uint16_t tmp = read16(CONFIGURATION_REG);
	tmp &= 0xE7FF; // clear pending bits

	switch(Range)
	{
	case Range_40mV:
		break;
	case Range_80mV:
		tmp |= 1 << 11;
		break;
	case Range_160mV:
		tmp |= 1 << 12;
		break;
	case Range_320mV:
		tmp |= 0x03 << 11;
		break;
	}

	write16(CONFIGURATION_REG, tmp);
}


void INA219_Set_ADC_Mode(INA219_ADC_Mode ADC_resolution, uint8_t type)
{
	uint16_t tmp = read16(CONFIGURATION_REG);

	if(type == BUS_ADC)
	{
		tmp &= 0xF8F7; // clear pending bits
		switch(ADC_resolution)
		{
		case ADC_9bit:
			break;
		case ADC_10bit:
			tmp |= 1 << 7;
			break;
		case ADC_11bit:
			tmp |= 1 << 8;
			break;
		case ADC_12bit:
			tmp |= 1 << 10;
			break;
		}
	}

	else
	{
		tmp &= 0xFF87; // clear pending bits
		switch(ADC_resolution)
		{
		case ADC_9bit:
			break;
		case ADC_10bit:
			tmp |= 1 << 3;
			break;
		case ADC_11bit:
			tmp |= 1 << 4;
			break;
		case ADC_12bit:
			tmp |= 1 << 6;
			break;
		}
	}

	write16(CONFIGURATION_REG, tmp);
}


void INA219_Reset()
{
	uint16_t tmp = read16(CONFIGURATION_REG);

	tmp |= 1<<15; // set RST bit

	write16(CONFIGURATION_REG, tmp);
}

void INA219_Set_Power_Mode(INA219_Power_Modes Mode)
{
	uint16_t tmp = read16(CONFIGURATION_REG);

	tmp &= 0x7FF8;
	if(Mode < 0) Mode = 0;
	if(Mode > 7) Mode = 7;

	tmp |= Mode;

	write16(CONFIGURATION_REG, tmp);
}


void INA219_Get_Data_OneShot(uint16_t* bus_voltage, uint8_t* current)
{
	INA219_Set_Power_Mode(SHUNT_AND_BUS_TRIGG);
	uint8_t power_raw;
	int16_t current_raw;
	uint16_t shunt_raw;
	uint16_t bus_raw;
	uint16_t bus_voltage_val;

	power_raw = read16(POWER_REG);
	current_raw = read16(CURRENT_REG);
	shunt_raw = read16(SHUNT_VOLTAGE_REG);
	bus_raw = read16(BUS_VOLTAGE_REG);
	bus_voltage_val = bus_raw / 2;
	*bus_voltage = bus_voltage_val;
	//*power = (power_raw * INA_Sensor.Current_LSB * 20);
	*current = current_raw * INA_Sensor.Current_LSB * 1000;
}

uint8_t INA219_Init(I2C_HandleTypeDef* hi2c, uint8_t device_addr)
{
	INA_Sensor.I2C_Addr = device_addr << 1;
	INA_Sensor.hi2c = hi2c;

	INA219_Reset();
	// Check if connection works properly
	uint16_t config_value = 0x399F;
	uint16_t config_tmp_val = read16(CONFIGURATION_REG);
	if(config_value != config_tmp_val)
	{
		return 1; // error
	}

	// Programming CALIBRATION_REG register
	INA_Sensor.Current_LSB = MAXIMUM_EXPECTED_CURRENT / pow(2, 15);
	//if(INA_Sensor.Current_LSB < 1) INA_Sensor.Current_LSB = 1;

	INA_Sensor.Cal_Reg_Value = truncf(FIXED_SCALING_FACTOR / (INA_Sensor.Current_LSB * SHUNT_RESISTOR_VALUE));
 	write16(CALIBRATION_REG, INA_Sensor.Cal_Reg_Value);

	// Set sensor mode
	INA219_Set_Range(Range_40mV);
	INA219_Set_VoltageRange(Voltage_16V);

	// Set ADC resolution
	INA219_Set_ADC_Mode(ADC_12bit, BUS_ADC);
	INA219_Set_ADC_Mode(ADC_12bit, SHUNT_RESISTOR_ADC);

	// Set power mode
	INA219_Set_Power_Mode(SHUNT_AND_BUS_TRIGG);

	return 0;
}
