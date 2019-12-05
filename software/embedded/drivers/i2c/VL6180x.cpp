/*
 * VL6180X.cpp
 * 
 * driver for the VL6180X, tof range and ambiant light sensor
 * NOTE : this driver only support single-shot ranging (no continuous, no ambiant light)
 * See "AN4545 - VL6180X basic ranging application note"
 * (https://www.st.com/content/ccc/resource/technical/document/application_note/d5/bb/ec/94/7d/1e/40/a0/DM00122600.pdf/files/DM00122600.pdf/jcr:content/translations/en.DM00122600.pdf)
 *
 *  Created on: 4 Dec 2019
 *      Author: fabien
 */

#include "VL6180x.h"
#include "string.h"
#include "common_i2c.h"

VL6180x vl6180x = VL6180x();

/**
 * Write register 16 bits address to device and read rxbytes from it.
 * 
 */
int VL6180x::read_from(uint16_t reg, uint8_t* rxbuf, size_t rxbytes) {
    uint8_t txbuf[2];
    txbuf[0] = (reg&0XFF00)>>8;
    txbuf[1] = reg&0X00FF;

    if (i2c_driver->state == I2C_READY) {
        msg_t ret = i2cMasterTransmit(i2c_driver, address,
            txbuf, 2,
            rxbuf, rxbytes
        );
        if (ret != MSG_OK) {
			return I2C_ERROR;
		}
	} else {
		return I2C_NOT_READY;
	}
    return I2C_OK;
}

int VL6180x::write(uint16_t reg, uint8_t* data, size_t databytes, uint8_t* rxbuf, size_t rxbytes) {
    uint8_t txbuf[databytes + 2];
    txbuf[0] = (reg&0XFF00)>>8;
    txbuf[1] = reg&0X00FF;
    memcpy(txbuf+2, data, databytes);

    if (i2c_driver->state == I2C_READY) {
        msg_t ret = i2cMasterTransmit(i2c_driver, address,
		txbuf, databytes + 2,
		rxbuf, rxbytes
	    );
        if (ret != MSG_OK) {
			return I2C_ERROR;
		}
	} else {
		return I2C_NOT_READY;
	}
    return I2C_OK;
}

/**
 *  Initialise device
 */
int VL6180x::init(I2CDriver* i2cp, uint8_t addr) {
    i2c_driver = i2cp;
    address = addr;
    uint8_t val = 0;
    int ret = read_from(VL6180X_SYSTEM_FRESH_OUT_OF_RESET, &val, 1);
    if(ret != I2C_OK) {
        return ret;
    }
    if(val != 0X01) {
        // init ERROR, SYSTEM_FRESH_OUT_OF_RESET should be equal to 0X01. Maybe the device is already initialized ?
    }

    load_sr_settings();

    val = 0X00;
    ret = write(VL6180X_SYSTEM_FRESH_OUT_OF_RESET, &val, 1, NULL, 0);
    
    return ret;

}

/**
 * Performs a single-shot ranging.
 * return measured distance in mm.
 */
int VL6180x::do_single_shot(uint8_t* range, uint8_t* status) {
    uint8_t val = 0;
    int ret = read_from(VL6180X_RESULT_RANGE_STATUS, &val, 1);
    if(ret != I2C_OK) { return ret; }

    if((val & 0x01) == 0) {
        // device not ready ! Try it anyway
    }
    val = 0x01;
    ret = write(VL6180X_SYSRANGE_START, &val, 1, NULL, 0);
    if(ret != I2C_OK) { return ret; }

    // wait for range measurement to complete
    while(true) {       // TODO add a timeout ??? maybe not necessary since we know the device is responding
        ret = read_from(VL6180X_RESULT_INTERRUPT_STATUS_GPIO, &val, 1);
        if(ret != I2C_OK) { return ret; }

        if(val & 0b100) {
            break;
        }
        chThdSleepMicroseconds(100);    //release the CPU a bit. Maybe not the good solution ?
    }

    ret = read_from(VL6180X_RESULT_RANGE_VAL, range, 1);

    val = 0X07;
    ret = write(VL6180X_SYSTEM_INTERRUPT_CLEAR, &val, 1, NULL, 0);
    if(ret != I2C_OK) { return ret; }
    ret = read_from(VL6180X_RESULT_RANGE_STATUS, &val, 1);
    if(ret != I2C_OK) { return ret; }
    *status = val >> 4;

    return I2C_OK;
}


struct config_vl6180x{
    uint16_t reg;
    uint8_t data;
};

struct config_vl6180x sr_settings[] = {
    // Mandatory : private registers
    {0x0207, 0x01},
    {0x0208, 0x01},
    {0x0096, 0x00},
    {0x0097, 0xfd},
    {0x00e3, 0x01},
    {0x00e4, 0x03},
    {0x00e5, 0x02},
    {0x00e6, 0x01},
    {0x00e7, 0x03},
    {0x00f5, 0x02},
    {0x00d9, 0x05},
    {0x00db, 0xce},
    {0x00dc, 0x03},
    {0x00dd, 0xf8},
    {0x009f, 0x00},
    {0x00a3, 0x3c},
    {0x00b7, 0x00},
    {0x00bb, 0x3c},
    {0x00b2, 0x09},
    {0x00ca, 0x09},
    {0x0198, 0x01},
    {0x01b0, 0x17},
    {0x01ad, 0x00},
    {0x00ff, 0x05},
    {0x0100, 0x05},
    {0x0199, 0x05},
    {0x01a6, 0x1b},
    {0x01ac, 0x3e},
    {0x01a7, 0x1f},
    {0x0030, 0x00},
    
    // Recommended : Public registers - See data sheet for more detail
    {0x0011, 0x10}, // Enables polling for ‘New Sample ready’ when measurement completes
    {0x010a, 0x30}, // Set the averaging sample period (compromise between lower noise and increased execution time)
    {0x003f, 0x46}, // Sets the light and dark gain (upper nibble). Dark gain should not be changed.
    {0x0031, 0xFF}, // sets the # of range measurements after which auto calibration of system is performed 
    {0x0041, 0x63}, // Set ALS integration time to 100ms
    {0x002e, 0x01}, // perform a single temperature calibration of the ranging sensor Optional: Public registers - See data sheet for more detail
    {0x001b, 0x09}, // Set default ranging inter-measurement period to 100ms
    {0x003e, 0x31}, // Set default ALS inter-measurement period to 500ms
    {0x0014, 0x24}, // Configures interrupt on ‘New Sample Ready threshold event’
};

int VL6180x::load_sr_settings() {
    size_t nb_settings = sizeof(sr_settings)/sizeof(sr_settings[0]);
    for(size_t i=0; i< nb_settings; i++) {
        int ret = write(sr_settings[i].reg, &sr_settings[i].data, 1, NULL, 0);
        if(ret != I2C_OK) {
            return ret;
        }
    }
    return I2C_OK;
}


const char* error_msgs[] {
        "OK",
        "V CONT",
        "V WATCH T",
        "V WATCH",
        "PLL1_LOCK",
        "PLL2_LOCK",
        "EARLY CONV EST",
        "MAX CONVERGENCE",
        "NO_TARGET_IGNORE",
        "MAX SNR",
        "R RANG UNDERFLOW",
        "R RANG OVERFLOW",
        "RANG UNDERFLOW",
        "RANG OVERFLOW",
};

const char* VL6180x::get_error_msg(RangeError error) {
    return (char*)error_msgs[(int)error];
}