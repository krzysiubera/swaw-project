#ifndef _BME280_APP_H_
#define _BME280_APP_H_

#include <zephyr.h>
#include <device.h>
#include <kernel.h>
#include <drivers/sensor.h>

struct bme280_state {
	uint32_t timestamp;
	struct sensor_value temp, press, humidity;
};

struct bme280_context {
	const struct device *dev;
	struct k_work wq_item;
	struct bme280_state state;
};

int bme280_get_device(const struct device **dev);

#endif 
