//
// Created by fabien on 10/10/2019.
//

#include "ServoState.h"
#include "lcd.h"
#include "stdio.h"
#include "stdutil.h"
#include "printf.h"
#include "MainMenuState.h"

extern "C" {
	#include "servos.h"
}

ServoState servoState = ServoState();

char* titles_servo[] = {
	(char*)"Servo~Servo 1",
	(char*)"Servo~Servo 2",
	(char*)"Servo~Servo 3",
	(char*)"Servo~Servo 4",
	(char*)"Servo~Servo 5",
};

ServoState::ServoState() {
	pos_enc_init = 0;
	for(int i=0; i<5; i++) {
		servo_positions[i] = 1002;
	}
	driving_servo = false;
	no_servo = 0;
}

void ServoState::enter(int32_t pos_enc) {
	(void)pos_enc;
	pos_enc_init = pos_enc;
    char blank[] = "                ";
	lcdWriteString(&LCDD1, blank, 0);
	lcdWriteString(&LCDD1, blank, 40);
	lcdWriteString(&LCDD1, titles_servo[0], 0);
}

void ServoState::leave() {
    
}

AbstractState* ServoState::onUiEvent(struct UiState ui_state) {
	if(driving_servo) {
		if(ui_state.enc_changed) {
			servo_positions[no_servo] = CLAMP_TO(SERVO_MIN, SERVO_MAX, servo_positions[no_servo] + (ui_state.pos_enc - last_pos_enc) * ui_state.encoder_speed);
			last_pos_enc = ui_state.pos_enc;
			chsnprintf(txt_pos_servo, sizeof(txt_pos_servo), "T = %4d", servo_positions[no_servo]);
			lcdWriteString(&LCDD1, txt_pos_servo, 40);
			set_servo(no_servo + 1, servo_positions[no_servo]);
		}
		if(ui_state.ok_clicked) {
			// do nothing
		}
		if(ui_state.ret_clicked) {
			driving_servo = false;
			char blank[] = "                ";
			lcdWriteString(&LCDD1, blank, 40);
			pos_enc_init = ui_state.pos_enc - no_servo;
			const int i = (ui_state.pos_enc - pos_enc_init) % (sizeof(titles_servo)/sizeof(*titles_servo));
			lcdWriteString(&LCDD1, titles_servo[i], 0);
		}
	} else {
		const int i = (ui_state.pos_enc - pos_enc_init) % (sizeof(titles_servo)/sizeof(*titles_servo));
		if(ui_state.enc_changed) {
			lcdWriteString(&LCDD1, titles_servo[i], 0);
		}
		if(ui_state.ok_clicked) {
			chsnprintf(txt_pos_servo, sizeof(txt_pos_servo), "T = %d", servo_positions[i]);
			lcdWriteString(&LCDD1, txt_pos_servo, 40);
			driving_servo = true;
			no_servo = i;
			pos_enc_init = ui_state.pos_enc;
			last_pos_enc = ui_state.pos_enc;
			set_servo(no_servo + 1, servo_positions[no_servo]);
		}
		if(ui_state.ret_clicked) {
			return &mainMenuState;
		}
	}

	return nullptr;
}

AbstractState* ServoState::periodic() {
    return nullptr;
}
