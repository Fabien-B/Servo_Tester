/*
 * srf02.c
 *
 *  Created on: 3 Dec 2019
 *      Author: fabien
 */

#include "srf02.h"
#include "common_i2c.h"


int srf02_range(I2CDriver *i2cp, uint8_t address, uint16_t* range) {
	uint8_t do_range[2] = { 0X00, 0X51 };
	uint8_t read_reg[1] = { 0X02 };
	uint8_t data[4];

	if (i2cp->state == I2C_READY) {
		msg_t ret = i2cMasterTransmitTimeout(
				i2cp,
				address, do_range, 2,
				NULL, 0,
				10);
		if (ret != MSG_OK) {
			return I2C_ERROR;
		}
	} else {
		return I2C_NOT_READY;
	}

	chThdSleepMilliseconds(70);

	if (i2cp->state == I2C_READY) {
		msg_t ret = i2cMasterTransmitTimeout(i2cp,
				address, read_reg, 1,
				data, 2,
				10);
		if (ret == MSG_OK) {
			*range = data[0] << 8 | data[1];
			return I2C_OK;
		} else {
			return I2C_ERROR;
		}
		
	} else {
		return I2C_NOT_READY;
	}
}

int srf02_change_address(I2CDriver *i2cp, uint8_t address, uint8_t new_address) {
	uint8_t change_addr_sequence[4] = {0XA0, 0XAA, 0XA5, 2*MIN_ADDRESS};
	change_addr_sequence[3] = 2*new_address;
	for(int i=0; i<4; i++) {
		uint8_t tx_buf[2];
		tx_buf[0] = 0X00;
		tx_buf[1] = change_addr_sequence[i];

		if (i2cp->state == I2C_READY) {
			msg_t ret = i2cMasterTransmit(
					i2cp,
					address, tx_buf, 2,
					NULL, 0);
			if (ret != MSG_OK) {
				return I2C_ERROR;
			}
		} else {
			return I2C_NOT_READY;
		}
	}
	return I2C_OK;
}

bool address_exists(I2CDriver *i2cp, uint8_t address) {
	if(address < MIN_ADDRESS || address > MAX_ADDRESS) {
		return false;
	}

	uint16_t range;
	int ret = srf02_range(i2cp, address, &range);

	if (ret == I2C_OK) {
		return true;
	} else {
		return false;
	}

}

/**
 * Scan for existing SRF02 device on the bus. Scan addresses from 0X70 (0XE0) to 0X7F (0XFE).
 */
uint8_t get_next_device(I2CDriver *i2cp) {
	static uint8_t address = MIN_ADDRESS;
	uint8_t addr_ret = 0; // 0 means that no addresses responded

	while(address <= MAX_ADDRESS) {
		if(address_exists(i2cp, address)) {
			addr_ret = address;
			address += 1;
			break;
		}
		address += 1;
	}

	if(address > MAX_ADDRESS) {
		address = MIN_ADDRESS;	// no more devices, get ready for a new scan
	}
	
	return addr_ret;
}
