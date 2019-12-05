/*
 * srf02.h
 *
 *  Created on: 3 Dec 2019
 *      Author: fabien
 */

#if !defined(SRF02_H_)
#define SRF02_H_

#include <ch.h>
#include <hal.h>

// See https://www.robot-electronics.co.uk/htm/srf02techI2C.htm for documentation about this module.

#define MIN_ADDRESS 0X70
#define MAX_ADDRESS 0X7E

int srf02_range(I2CDriver *i2cp, uint8_t address, uint16_t* range);
bool address_exists(I2CDriver *i2cp, uint8_t address);
uint8_t get_next_device(I2CDriver *i2cp);

int srf02_change_address(I2CDriver *i2cp, uint8_t address, uint8_t new_address);

#endif // SRF02_H_
