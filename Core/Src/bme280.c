/*
 * BMXX80.c
 *
 *	The MIT License.
 *	Based on Adafuit libraries.
 *  Created on: 10.08.2018
 *      Author: Mateusz Salamon
 *      www.msalamon.pl
 *
 */

#include "bme280.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "math.h"

#include "delays.h"
//
//	 Private variables
//
uint8_t _temperature_res, _pressure_oversampling, _huminidity_oversampling,  _mode, h1, h3;
int8_t h6;
int16_t t2, t3, p2, p3, p4, p5, p6, p7, p8, p9, h2, h4, h5;
uint16_t t1, p1;
int32_t t_fine;

//
//	Coeffs registers
//
#define	BME280_DIG_T1		0x88
#define	BME280_DIG_T2		0x8A
#define	BME280_DIG_T3		0x8C

#define	BME280_DIG_P1		0x8E
#define	BME280_DIG_P2		0x90
#define	BME280_DIG_P3		0x92
#define	BME280_DIG_P4		0x94
#define	BME280_DIG_P5		0x96
#define	BME280_DIG_P6		0x98
#define	BME280_DIG_P7		0x9A
#define	BME280_DIG_P8		0x9C
#define	BME280_DIG_P9		0x9E

#define	BME280_DIG_H1		0xA1
#define	BME280_DIG_H2		0xE1
#define	BME280_DIG_H3		0xE3
#define	BME280_DIG_H4		0xE4
#define	BME280_DIG_H5		0xE5
#define	BME280_DIG_H6		0xE7

//
//	Registers
//
#define	BME280_CHIPID			0xD0
#define	BME280_VERSION			0xD1
#define	BME280_SOFTRESET		0xE0
#define	BME280_CAL26			0xE1  // R calibration stored in 0xE1-0xF0
#define	BME280_HUM_CONTROL		0xF2
#define	BME280_STATUS			0xF3
#define	BME280_CONTROL			0xF4
#define	BME280_CONFIG			0xF5
#define	BME280_PRESSUREDATA		0xF7
#define	BME280_TEMPDATA			0xFA
#define	BME280_HUMIDDATA		0xFD

//
//	Control bits
//
#define	BME280_MEASURING			(1<<3) // Conversion in progress

//
//	Functions
//
uint8_t BME280_Read8(BME280* bme_280, uint8_t addr) {
    uint8_t tmp[2];
	tmp[0] = addr;
	tmp[0] |= (1<<7);
	HAL_GPIO_WritePin(bme_280->cs_port, bme_280->cs_pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(bme_280->spi_handler, tmp, tmp, 2, 10);
	HAL_GPIO_WritePin(bme_280->cs_port, bme_280->cs_pin, GPIO_PIN_SET);
	return tmp[1];
}

uint16_t BME280_Read16(BME280* bme_280, uint8_t addr) {
	uint8_t tmp[3];
	tmp[0] = addr;
	tmp[0] |= (1<<7);
	HAL_GPIO_WritePin(bme_280->cs_port, bme_280->cs_pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(bme_280->spi_handler, tmp, tmp, 3, 10);
	HAL_GPIO_WritePin(bme_280->cs_port, bme_280->cs_pin, GPIO_PIN_SET);
	return ((tmp[1] << 8) | tmp[2]);
}

uint16_t BME280_Read16LE(BME280* bme_280, uint8_t addr) {
	uint16_t tmp;
	tmp = BME280_Read16(bme_280, addr);
	return (tmp >> 8) | (tmp << 8);
}

void BME280_Write8(BME280* bme_280, uint8_t address, uint8_t data) {
	uint8_t tmp[2];
	tmp[0] = address;
	tmp[0] &= ~(1<<7);
	tmp[1] = data;
	HAL_GPIO_WritePin(bme_280->cs_port, bme_280->cs_pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(bme_280->spi_handler, tmp, tmp, 2, 10);
	HAL_GPIO_WritePin(bme_280->cs_port, bme_280->cs_pin, GPIO_PIN_SET);
}

uint32_t BME280_Read24(BME280* bme_280, uint8_t addr) {
	uint8_t tmp[4];
	tmp[0] = addr;
	tmp[0] |= (1<<7);
	HAL_GPIO_WritePin(bme_280->cs_port, bme_280->cs_pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(bme_280->spi_handler, tmp, tmp, 3, 10);
	HAL_GPIO_WritePin(bme_280->cs_port, bme_280->cs_pin, GPIO_PIN_SET);
	return ((tmp[1] << 16) | tmp[2] << 8 | tmp[3]);
}

uint8_t BME280_IsReadingCalibration(BME280* bme_280) {
	uint8_t Status = BME280_Read8(bme_280, BME280_STATUS);
	return ((Status & 1) != 0);
}

void BME280_SetConfig(BME280* bme_280, uint8_t standby_time, uint8_t filter) {
	BME280_Write8(bme_280, BME280_CONFIG,
			(uint8_t)(((standby_time & 0x7) << 5) | ((filter & 0x7) << 2)) & 0xFC);
}

void BME280_InitHardware(BME280* bme_280, SPI_HandleTypeDef* spi_handler,
		GPIO_TypeDef* cs_port, uint16_t cs_pin) {
	bme_280->spi_handler = spi_handler;
	bme_280->cs_port = cs_port;
	bme_280->cs_pin = cs_pin;
}

void BME280_InitChip(BME280* bme_280, uint8_t temperature_resolution,
		uint8_t pressure_oversampling, uint8_t huminidity_oversampling, uint8_t mode) {
	HAL_GPIO_WritePin(bme_280->cs_port, bme_280->cs_pin, GPIO_PIN_RESET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(bme_280->cs_port, bme_280->cs_pin, GPIO_PIN_SET);

	uint8_t HumReg, i;

	if (mode > BME280_NORMALMODE)
	    mode = BME280_NORMALMODE;
	_mode = mode;
	if(mode == BME280_FORCEDMODE)
		mode = BME280_SLEEPMODE;

	if (temperature_resolution > BME280_TEMPERATURE_20BIT)
		temperature_resolution = BME280_TEMPERATURE_20BIT;
	_temperature_res = temperature_resolution;

	if (pressure_oversampling > BME280_PRESSURE_ULTRAHIGHRES)
		pressure_oversampling = BME280_PRESSURE_ULTRAHIGHRES;
	_pressure_oversampling = pressure_oversampling;

	if (huminidity_oversampling > BME280_HUMINIDITY_ULTRAHIGH)
		huminidity_oversampling = BME280_HUMINIDITY_ULTRAHIGH;
	_huminidity_oversampling = huminidity_oversampling;

	BME280_Write8(bme_280, BME280_SOFTRESET, 0xB6);

	for(i = 0; i<30; i++)
		Delay_us(1000); // Wait  300 msfor wake up

	while(BME280_IsReadingCalibration(bme_280))
		for(i = 0; i<10; i++)
			Delay_us(1000);

	/* read calibration data */
	t1 = BME280_Read16LE(bme_280, BME280_DIG_T1);
	t2 = BME280_Read16LE(bme_280, BME280_DIG_T2);
	t3 = BME280_Read16LE(bme_280, BME280_DIG_T3);

	p1 = BME280_Read16LE(bme_280, BME280_DIG_P1);
	p2 = BME280_Read16LE(bme_280, BME280_DIG_P2);
	p3 = BME280_Read16LE(bme_280, BME280_DIG_P3);
	p4 = BME280_Read16LE(bme_280, BME280_DIG_P4);
	p5 = BME280_Read16LE(bme_280, BME280_DIG_P5);
	p6 = BME280_Read16LE(bme_280, BME280_DIG_P6);
	p7 = BME280_Read16LE(bme_280, BME280_DIG_P7);
	p8 = BME280_Read16LE(bme_280, BME280_DIG_P8);
	p9 = BME280_Read16LE(bme_280, BME280_DIG_P9);

	h1 = BME280_Read8(bme_280, BME280_DIG_H1);
	h2 = BME280_Read16LE(bme_280, BME280_DIG_H2);
	h3 = BME280_Read8(bme_280, BME280_DIG_H3);
	h4 = ((BME280_Read8(bme_280, BME280_DIG_H4) << 4 ) | (BME280_Read8(bme_280, BME280_DIG_H4+1) & 0xF));
	h5 = ((BME280_Read8(bme_280, BME280_DIG_H5+1) << 4) | (BME280_Read8(bme_280, BME280_DIG_H5) >> 4));
	h6 = (int8_t)BME280_Read8(bme_280, BME280_DIG_H6);

	HumReg = BME280_Read8(bme_280, BME280_HUM_CONTROL);
	HumReg &= 0xF8;
	HumReg |= _huminidity_oversampling;
	BME280_Write8(bme_280, BME280_HUM_CONTROL, HumReg);
	HumReg = BME280_Read8(bme_280, BME280_HUM_CONTROL);
	BME280_Write8(bme_280, BME280_CONTROL, ((temperature_resolution<<5) | (pressure_oversampling<<2) | mode));

	if(mode == BME280_NORMALMODE)
	{
		BME280_SetConfig(bme_280, BME280_STANDBY_MS_0_5, BME280_FILTER_OFF);
	}
}

float BME280_ReadTemperature(BME280* bme_280) {
  int32_t var1, var2;

  if(_mode == BME280_FORCEDMODE)
  {
	  uint8_t mode;
	  uint8_t ctrl = BME280_Read8(bme_280, BME280_CONTROL);
	  ctrl &= ~(0x03);
	  ctrl |= BME280_FORCEDMODE;
	  BME280_Write8(bme_280, BME280_CONTROL, ctrl);

	  mode = BME280_Read8(bme_280, BME280_CONTROL); 	// Read written mode
	  mode &= 0x03;							// Do not work without it...

	  if(mode == BME280_FORCEDMODE)
	  {
		  while(1) // Wait for end of conversion
		  {
			  mode = BME280_Read8(bme_280, BME280_CONTROL);
			  mode &= 0x03;
			  if(mode == BME280_SLEEPMODE)
				  break;
		  }
	  }
  }

  int32_t adc_T = BME280_Read24(bme_280, BME280_TEMPDATA);
  if (adc_T == 0x800000)
	  return -99;

  adc_T >>= 4;

  var1  = ((((adc_T>>3) - ((int32_t)t1 <<1))) *
		  ((int32_t)t2)) >> 11;

  var2  = (((((adc_T>>4) - ((int32_t)t1)) *
		  ((adc_T>>4) - ((int32_t)t1))) >> 12) *
		  ((int32_t)t3)) >> 14;

  t_fine = var1 + var2;

  float T  = (t_fine * 5 + 128) >> 8;
  return T/100;

  return -99;
}

int32_t BME280_ReadPressure(BME280* bme_280) {
	int64_t var1, var2, p;

	// Must be done first to get the t_fine variable set up
	BME280_ReadTemperature(bme_280);

	int32_t adc_P = BME280_Read24(bme_280, BME280_PRESSUREDATA);
	adc_P >>= 4;

	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)p6;
	var2 = var2 + ((var1*(int64_t)p5)<<17);
	var2 = var2 + (((int64_t)p4)<<35);
	var1 = ((var1 * var1 * (int64_t)p3)>>8) +
			((var1 * (int64_t)p2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)p1)>>33;

	if (var1 == 0) {
		return 0;  // avoid exception caused by division by zero
	}
	p = 1048576 - adc_P;
	p = (((p<<31) - var2)*3125) / var1;
	var1 = (((int64_t)p9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)p8) * p) >> 19;

	p = ((p + var1 + var2) >> 8) + (((int64_t)p7)<<4);
	return (int32_t)p/256;
}

float BME280_ReadHuminidity(BME280* bme_280)
{
	// Must be done first to get the t_fine variable set up
	BME280_ReadTemperature(bme_280);

	int32_t adc_H = BME280_Read16(bme_280, BME280_HUMIDDATA);
	if (adc_H == 0x8000) // value in case humidity measurement was disabled
	   return -99; //error
	int32_t v_x1_u32r;

	v_x1_u32r = (t_fine - ((int32_t)76800));

	  v_x1_u32r = (((((adc_H << 14) - (((int32_t)h4) << 20) -
					  (((int32_t)h5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
				   (((((((v_x1_u32r * ((int32_t)h6)) >> 10) *
						(((v_x1_u32r * ((int32_t)h3)) >> 11) + ((int32_t)32768))) >> 10) +
					  ((int32_t)2097152)) * ((int32_t)h2) + 8192) >> 14));

	  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
								 ((int32_t)h1)) >> 4));

	  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
	  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
	  float h = (v_x1_u32r>>12);
	  return  h / 1024.0;
}

uint8_t BME280_ReadTemperatureAndPressureAndHuminidity(BME280* bme_280, float *temperature,
		int32_t *pressure, float *huminidity) {
	int64_t var1, var2, p;

	// Must be done first to get the t_fine variable set up
	*temperature = BME280_ReadTemperature(bme_280);

	if(*temperature == -99)
	  return -1;

	int32_t adc_P = BME280_Read24(bme_280, BME280_PRESSUREDATA);
	adc_P >>= 4;

	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)p6;
	var2 = var2 + ((var1*(int64_t)p5)<<17);
	var2 = var2 + (((int64_t)p4)<<35);
	var1 = ((var1 * var1 * (int64_t)p3)>>8) +
			((var1 * (int64_t)p2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)p1)>>33;

	if (var1 == 0) {
		return 0;  // avoid exception caused by division by zero
	}
	p = 1048576 - adc_P;
	p = (((p<<31) - var2)*3125) / var1;
	var1 = (((int64_t)p9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)p8) * p) >> 19;

	p = ((p + var1 + var2) >> 8) + (((int64_t)p7)<<4);
	*pressure = (int32_t)p/256;

	// Pressure section
	int32_t adc_H = BME280_Read16(bme_280, BME280_HUMIDDATA);
	if (adc_H == 0x8000) // value in case humidity measurement was disabled
		return -1; //error

	int32_t v_x1_u32r;

	v_x1_u32r = (t_fine - ((int32_t)76800));

	v_x1_u32r = (((((adc_H << 14) - (((int32_t)h4) << 20) -
				  (((int32_t)h5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
			   (((((((v_x1_u32r * ((int32_t)h6)) >> 10) *
					(((v_x1_u32r * ((int32_t)h3)) >> 11) + ((int32_t)32768))) >> 10) +
				  ((int32_t)2097152)) * ((int32_t)h2) + 8192) >> 14));

	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
							 ((int32_t)h1)) >> 4));

	v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
	v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
	float h = (v_x1_u32r>>12);
	*huminidity = h / 1024.0;

	return 0;
}

float BME280_ReadAltitude(BME280* bme_280, float sea_level_pa)
{
	  float altitude;
	  float pressure = BME280_ReadPressure(bme_280); // in Si units for Pascal
	  altitude = 44330 * (1.0 - pow(pressure / sea_level_pa, 0.1903));
	  return altitude;
}
