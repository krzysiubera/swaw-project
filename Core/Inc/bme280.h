/*
 * BMPXX0.h
 *
 *  The MIT License.
 *  Based on Adafuit libraries.
 *  Created on: 10.08.2018
 *      Author: Mateusz Salamon
 *      www.msalamon.pl
 *
 */

#ifndef BME280_H
#define BME280_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

//
//	Mode
//
// Huminidity oversampling
#define BME280_HUMINIDITY_DISABLE			0
#define BME280_HUMINIDITY_ULTRALOWPOWER		1
#define BME280_HUMINIDITY_LOWPOWER			2
#define BME280_HUMINIDITY_STANDARD			3
#define BME280_HUMINIDITY_HIGHRES			4
#define BME280_HUMINIDITY_ULTRAHIGH			5

// Temperature resolution
#define BME280_TEMPERATURE_DISABLE	0
#define BME280_TEMPERATURE_16BIT 	1
#define BME280_TEMPERATURE_17BIT 	2
#define BME280_TEMPERATURE_18BIT 	3
#define BME280_TEMPERATURE_19BIT 	4
#define BME280_TEMPERATURE_20BIT 	5

// Pressure oversampling
#define BME280_PRESSURE_DISABLE			0
#define BME280_PRESSURE_ULTRALOWPOWER	1
#define BME280_PRESSURE_LOWPOWER		2
#define BME280_PRESSURE_STANDARD		3
#define BME280_PRESSURE_HIGHRES			4
#define BME280_PRESSURE_ULTRAHIGHRES	5

// t_standby time
#define BME280_STANDBY_MS_0_5	0
#define BME280_STANDBY_MS_10	6
#define BME280_STANDBY_MS_20	7
#define BME280_STANDBY_MS_62_5	1
#define BME280_STANDBY_MS_125	2
#define BME280_STANDBY_MS_250 	3
#define BME280_STANDBY_MS_500	4
#define BME280_STANDBY_MS_1000	5

// IIR filter
#define BME280_FILTER_OFF	0
#define BME280_FILTER_X2 	1
#define BME280_FILTER_X4 	2
#define BME280_FILTER_X8	3
#define BME280_FILTER_X16 	4

// Mode
#define BME280_SLEEPMODE		0
#define BME280_FORCEDMODE		1
#define BME280_NORMALMODE		3
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
// User functions
//
void BME280_Init(SPI_HandleTypeDef *spi_handler, uint8_t temperature_resolution, uint8_t pressure_oversampling, uint8_t huminidity_oversampling, uint8_t mode);
void BME280_SetConfig(uint8_t standby_time, uint8_t filter);

float BME280_ReadTemperature(void);
int32_t BME280_ReadPressure(void);
float BME280_ReadHuminidity(void);
uint8_t BME280_ReadTemperatureAndPressureAndHuminidity(float *temperature, int32_t *pressure, float *huminidity);
float BME280_ReadAltitude(float sea_level_pa);

#endif /* BME280_H */
