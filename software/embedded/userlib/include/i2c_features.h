/*
 * i2c_features.h
 *
 *  Created on: 18 juin 2019
 *      Author: fabien
 */

#ifndef USERLIB_INCLUDE_I2C_FEATURES_H_
#define USERLIB_INCLUDE_I2C_FEATURES_H_

#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"

void init_I2C(void);

int srf02_range(int address);
int address_exists(int address);


#endif /* USERLIB_INCLUDE_I2C_FEATURES_H_ */
