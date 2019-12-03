/*
 * hardware_config.c
 *
 *  Created on: 3 Dec 2019
 *      Author: fabien
 */

#include "hardware_config.h"
#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"

//400k : 0x00300F38
//100k : 0x00707CBB

I2CConfig i2ccfg = {
	// .timingr = STM32_TIMINGR_PRESC(3)
	// | STM32_TIMINGR_SCLL(0x13) | STM32_TIMINGR_SCLH(0xF)
	// | STM32_TIMINGR_SDADEL(0X2) | STM32_TIMINGR_SCLDEL(0X4),
	.timingr = 0x00707CBB,
	.cr1 = 0,
	.cr2 = 0
	};

void init_I2C() {
	i2cStart(&I2CD1, &i2ccfg);
}
