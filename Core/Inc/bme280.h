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

typedef struct {
	SPI_HandleTypeDef* spi_handler;
	GPIO_TypeDef* cs_port;
	uint16_t cs_pin;
} BME280;


//
// User functions
//
void BME280_InitHardware(BME280* bme_280, SPI_HandleTypeDef* spi_handler,
		GPIO_TypeDef* cs_port, uint16_t cs_pin);
void BME280_InitChip(BME280 *bme280, uint8_t temperature_resolution,
		uint8_t pressure_oversampling, uint8_t huminidity_oversampling, uint8_t mode);
void BME280_SetConfig(BME280* bme_280, uint8_t standby_time, uint8_t filter);

float BME280_ReadTemperature(BME280* bme_280);
int32_t BME280_ReadPressure(BME280* bme_280);
float BME280_ReadHuminidity(BME280* bme_280);
uint8_t BME280_ReadTemperatureAndPressureAndHuminidity(BME280* bme_280, float *temperature,
		int32_t *pressure, float *huminidity);
float BME280_ReadAltitude(BME280* bme_280, float sea_level_pa);

#endif /* BME280_H */
