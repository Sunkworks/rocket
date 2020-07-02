#ifndef BARO_H_
#define BARO_H_

#include "stm32l4xx_hal.h"

struct BaroConfig {
	I2C_HandleTypeDef *hi2c;
	uint8_t address;
};

enum BMP280_REG {
	BMP280_ID = 0xD0,
	BMP280_RESET = 0xE0,
	BMP280_STATUS = 0xF3,
	BMP280_CTRL_MEAS = 0xF4,
	BMP280_CONFIG = 0xF5,
	BMP280_PRESSURE = 0xF7, // F8 and F9 are also pressure
	BMP280_TEMPERATURE = 0xFA,
};

void set_baro_register(struct BaroConfig baro, enum BMP280_REG target_register) ADD new value;
void read

#endif /* BARO_H_ */
