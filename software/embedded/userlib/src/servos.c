/*
 * servos.c
 *
 *  Created on: 18 juin 2019
 *      Author: fabien
 */

#include "servos.h"
#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"

PWMConfig pwmcfg1 = {
	.frequency = 1000000,
	.period = 20000,
	.callback = NULL,
	.channels = {
			{								//ch1
					.mode = PWM_OUTPUT_ACTIVE_HIGH,
					.callback = NULL
			},
			{								//ch2n
					.mode = PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH,
					.callback = NULL
			},
			{								//ch3n
					.mode = PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH,
					.callback = NULL
			},
			{PWM_OUTPUT_DISABLED, NULL}
	},
	.cr2 = 0,
	.bdtr = 0,
	.dier = 0
};

// PWMConfig pwmcfg16 = {
// 	.frequency = 1000000,
// 	.period = 20000,
// 	.callback = NULL,
// 	.channels = {
// 			{								//ch1
// 					.mode = PWM_OUTPUT_ACTIVE_HIGH,
// 					.callback = NULL
// 			},
// 			{PWM_OUTPUT_DISABLED, NULL},	//ch2
// 			{PWM_OUTPUT_DISABLED, NULL},	//ch3
// 			{PWM_OUTPUT_DISABLED, NULL}		//ch4
// 	},
// 	.cr2 = 0,
// 	.bdtr = 0,
// 	.dier = 0
// };

PWMDriver* _servo_drivers[5] = {
		&PWMD1, &PWMD1, &PWMD1,
		NULL,	//&PWMD16,		TODO fix issue with TIM16 !!!
		NULL, //&PWMD2
};

int _servo_channels[5] = {
		1, 2, 0, 0, 3
};

void init_servos() {
	pwmStart(&PWMD1, &pwmcfg1);
	//pwmStart(&PWMD2, &pwmcfg2);
	//pwmStart(&PWMD16, &pwmcfg16);

}

void enable_servos() {
	  for(int i=0; i<5; i++) {
		  set_servo(i, 1000);
	  }
}

void set_servo(int servo_nb, int us) {
	if (servo_nb < 1 || servo_nb > 3) {	// TODO FixMe: only works for TIM1 for now, ie first 3 servos.
		return;
	}
	servo_nb -= 1;
	us = CLAMP_TO(100, 3000, us);
	pwmEnableChannel(_servo_drivers[servo_nb], _servo_channels[servo_nb], us);
}
