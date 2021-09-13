/*
 * BMP280.c
 *
 *  Created on: Sep 7, 2021
 *      Author: igur
 */


#include "BMP280.h"

BMP280_t sensor;

// Read / Write functions
static uint8_t read8(uint8_t memory_address)
{
	uint8_t tmp;
	HAL_I2C_Mem_Read(sensor.hi2c, sensor.I2C_Addr, memory_address, 1, &tmp, 1, I2C_Timeout);
	return tmp;
}

static uint16_t read16(uint8_t memory_address)
{
	uint8_t tmp[2];
	HAL_I2C_Mem_Read(sensor.hi2c, sensor.I2C_Addr, memory_address, 1, tmp, 2, I2C_Timeout);
	return (tmp[1] << 8) | (tmp[0]);
}

static void temp_press_burst_read(uint8_t* buff)
{
	HAL_I2C_Mem_Read(sensor.hi2c, sensor.I2C_Addr, press_msb, 1, buff, 6, I2C_Timeout);
}

static void write8(uint8_t memory_address, uint8_t* data)
{
	HAL_I2C_Mem_Write(sensor.hi2c, sensor.I2C_Addr, memory_address, 1, data, 1, I2C_Timeout);
}

// User functions
void BMP280_set_mode(BMP_Mode mode)
{
	uint8_t tmp;
	tmp = read8(ctrl_meas);

	switch(mode)
	{
	case SLEEP:
		tmp &= 0xFC;
		write8(ctrl_meas, &tmp);
		break;

	case FORCED:
		tmp &= 0xFC;
		tmp |= 0x01;
		write8(ctrl_meas, &tmp);
		break;

	case NORMAL:
		tmp &= 0xFC;
		tmp |= 0x03;
		write8(ctrl_meas, &tmp);
		break;
	}
}

void BMP280_set_temp_OVS(BMP_OVS_rate Rate)
{
	uint8_t tmp = read8(ctrl_meas);

	tmp &= 0x1F; // CLEAR bits
	tmp |= (Rate << 5);
	write8(ctrl_meas, &tmp);
}


void BMP280_set_press_OVS(BMP_OVS_rate Rate)
{
	uint8_t tmp = read8(ctrl_meas);

	tmp &= 0xE3; // CLEAR bits
	tmp |= (Rate << 2);

	write8(ctrl_meas, &tmp);
}

void BMP280_set_filter(BMP_filter_rate Rate)
{
	uint8_t tmp = read8(config);
	tmp &= 0xE3; // CLEAR bits
	tmp |= (Rate << 2);

	write8(ctrl_meas, &tmp);
}


static int32_t BMP280_temp_compensate(int32_t temp_raw)
{
	int32_t var1, var2, t_fine;

	var1 = ((((temp_raw>>3)-((int32_t)sensor.dig_t1<<1))) * ((int32_t)sensor.dig_t2)) >> 11;
	var2 = (((((temp_raw>>4)-((int32_t)sensor.dig_t1)) * ((temp_raw>>4)-((int32_t)sensor.dig_t1))) >> 12) *  ((int32_t)sensor.dig_t3)) >> 14;
	t_fine = var1 + var2;

	return t_fine;
}

static uint32_t BMP280_press_compensate(int32_t press_raw, int32_t t_fine)
{
	int32_t var1, var2;
	uint32_t p;
	var1 = (((int32_t)t_fine)>>1)-(int32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)sensor.dig_p6);
	var2 = var2 + ((var1*((int32_t)sensor.dig_p5))<<1);
	var2 = (var2>>2)+(((int32_t)sensor.dig_p4)<<16);
	var1 = (((sensor.dig_p3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)sensor.dig_p2) * var1)>>1))>>18;
	var1 =((((32768+var1))*((int32_t)sensor.dig_p1))>>15);
	if (var1 == 0)
	{
	return 0; // avoid exception caused by division by zero
	}
	p = (((uint32_t)(((int32_t)1048576)-press_raw)-(var2>>12)))*3125;
	if (p < 0x80000000)
	{
	p = (p << 1) / ((uint32_t)var1);
	}
	else
	{
	p = (p / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)sensor.dig_p9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((int32_t)(p>>2)) * ((int32_t)sensor.dig_p8))>>13;
	p = (uint32_t)((int32_t)p + ((var1 + var2 + sensor.dig_p7) >> 4));
	return p;

}


void BMP280_get_data_FORCED(int32_t* temperature, uint32_t* pressure)
{
	BMP280_set_mode(FORCED); // start conversion
	int32_t temp_raw;
	int32_t press_raw;
	uint8_t tmp_buff[6];
	int32_t t_fine;


	temp_press_burst_read(tmp_buff);
	press_raw = tmp_buff[0] << 12 | (tmp_buff[1] << 4) | (tmp_buff[2] >> 4);
	temp_raw = tmp_buff[3] << 12 | (tmp_buff[4] << 4) | (tmp_buff[5] >> 4);

	// compensate temperature
	t_fine = BMP280_temp_compensate(temp_raw);
	*temperature = ((t_fine * 5 + 128) >> 8) / 100; // return temperature in [C]

	// compensate pressure
	*pressure = (BMP280_press_compensate(press_raw, t_fine)) / 100; // return pressure in [Pa]
}




BMP_Status BMP280_Init(I2C_HandleTypeDef* hi2c, uint8_t I2C_Address)
{
	sensor.hi2c = hi2c;
	sensor.I2C_Addr = (I2C_Address << 1);

	// check sensor ID
	uint8_t sensor_id = read8(ID);
	if(sensor_id != CHIP_ID)
	{
		return BMP280_NOK;
	}

	// load calibration coefficients
	sensor.dig_t1 = read16(dig_T1);
	sensor.dig_t2 = read16(dig_T2);
	sensor.dig_t3 = read16(dig_T3);
	sensor.dig_p1 = read16(dig_P1);
	sensor.dig_p2 = read16(dig_P2);
	sensor.dig_p3 = read16(dig_P3);
	sensor.dig_p4 = read16(dig_P4);
	sensor.dig_p5 = read16(dig_P5);
	sensor.dig_p6 = read16(dig_P6);
	sensor.dig_p7 = read16(dig_P7);
	sensor.dig_p8 = read16(dig_P8);
	sensor.dig_p9 = read16(dig_P9);

	// initial sensor setup
	// Mode - Forced, Oversampling - UltraLowPower, osrs_p - x1, osrs_t - x1, IIR - off
	// Timing - 1/min, ODR - 1/60Hz, BW - Full

	 BMP280_set_filter(OFF);
	 BMP280_set_temp_OVS(x1);
	 BMP280_set_press_OVS(x1);

	return BMP280_OK;
}
