//
// Created by fabien on 10/10/2019.
//

#include "MainMenuState.h"
#include "lcd.h"
#include "stdio.h"
#include "stdutil.h"
#include "servos.h"
#include "ServoState.h"
#include "DynamixelState.h"
#include "I2CState.h"

MainMenuState mainMenuState = MainMenuState();

char* titles_mainmenu[] = {
	(char*)"Servos    ",
	(char*)"Dynamixels",
	(char*)"I2C       ",
	(char*)"CAN       "
};

void MainMenuState::enter(int32_t pos_enc) {
	pos_enc_init = pos_enc;
    char title[] = "                ";
	lcdWriteString(&LCDD1, title, 0);
	lcdWriteString(&LCDD1, titles_mainmenu[0], 0);
	char blank[] = "                ";
	lcdWriteString(&LCDD1, blank, 40);
}

void MainMenuState::leave() {
    
}

AbstractState* MainMenuState::onUiEvent(struct UiState ui_state) {
	int i = (ui_state.pos_enc - pos_enc_init) % (sizeof(titles_mainmenu)/sizeof(*titles_mainmenu));
	
	if(ui_state.enc_changed) {	
		lcdWriteString(&LCDD1, titles_mainmenu[i], 0);
	}

	if(ui_state.ok_clicked) {
		switch (i) {
		case 0:
			return &servoState;
			break;
		case 1:
			return &dynamixelIDState;
			break;
		case 2:
			return &i2cState;
			break;
		default:
			return nullptr;
			break;
		}
	}

	return nullptr;
}

AbstractState* MainMenuState::periodic() {
    return nullptr;
}
