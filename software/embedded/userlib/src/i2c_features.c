/*
 * i2c_features.c
 *
 *  Created on: 18 juin 2019
 *      Author: fabien
 */

#include "i2c_features.h"
#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"

I2CConfig i2ccfg = { .timingr = STM32_TIMINGR_PRESC(3)
		| STM32_TIMINGR_SCLL(0x13) | STM32_TIMINGR_SCLH(0xF)
		| STM32_TIMINGR_SDADEL(0X2) | STM32_TIMINGR_SCLDEL(0X4), .cr1 = 0,
		.cr2 = 0 };

void init_I2C() {
	i2cStart(&I2CD1, &i2ccfg);
}

int srf02_range(int address) {
	uint8_t do_range[2] = { 0X00, 0X51 };
	uint8_t read_reg[1] = { 0X02 };
	uint8_t data[4];

	if (I2CD1.state == I2C_READY) {
		msg_t ret = i2cMasterTransmitTimeout(
				&I2CD1,
				address, do_range, 2,
				NULL, 0,
				10);
		if (ret != MSG_OK) {
			return -1;
		}
	}

	chThdSleepMilliseconds(70);

	if (I2CD1.state == I2C_READY) {
		msg_t ret = i2cMasterTransmitTimeout(&I2CD1,
				address, read_reg, 1,
				data, 2,
				10);
		if (ret != MSG_OK) {
			return -1;
		}
		int range_meas = data[0] << 8 | data[1];
		return range_meas;
	}
	return -1;
}


int address_exists(int address) {
	if(address <= 7 || address >= 0x78) {
		//reserved addresses
		return 0;
	}

	uint8_t do_range[2] = { 0X00, 0X51 };

	msg_t ret = i2cMasterTransmitTimeout(
			&I2CD1,
			address, do_range, 2,
			NULL, 0,
			10);
	if (ret == MSG_OK) {
		return 1;
	} else {
		return 0;
	}

}


