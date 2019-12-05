//
// Created by fabien on 10/10/2019.
//

#include "I2CState.h"
#include "lcd.h"
#include "stdio.h"
#include "stdutil.h"
#include "printf.h"
#include "MainMenuState.h"
#include "common_i2c.h"

extern "C" {
	#include "srf02.h"
}

char txt[16];

I2CState i2cState = I2CState();
SRF02State srf02State = SRF02State();
VL6180xState vl6180xState = VL6180xState();

char main_title[] = "I2C             "; 

char* titles_i2c[] = {
	(char*)"SRF02  ",
    (char*)"VL6180x"
};

I2CState::I2CState() {
	pos_enc_init = 0;
}

void I2CState::enter(int32_t pos_enc) {
	(void)pos_enc;
	pos_enc_init = pos_enc;
	lcdWriteString(&LCDD1, titles_i2c[0], 40);
    lcdWriteString(&LCDD1, main_title, 0);
}

void I2CState::leave() {
    
}

AbstractState* I2CState::onUiEvent(struct UiState ui_state) {
    int i = (ui_state.pos_enc - pos_enc_init) % (sizeof(titles_i2c)/sizeof(*titles_i2c));

    if(ui_state.enc_changed) {
        lcdWriteString(&LCDD1, titles_i2c[i], 40);
    }
    if(ui_state.ok_clicked) {
        switch (i)
        {
        case 0:
            lcdClear1stLine(&LCDD1);
            lcdWriteString(&LCDD1, titles_i2c[i], 0);
            return &srf02State;
            break;
        case 1:
            lcdClear1stLine(&LCDD1);
            lcdWriteString(&LCDD1, titles_i2c[i], 0);
            return &vl6180xState;
        default:
            break;
        }
        
    }
    if(ui_state.ret_clicked) {
        return &mainMenuState;
    }

	return nullptr;
}

AbstractState* I2CState::periodic() {
    return nullptr;
}



///////////////////////////  SRF02  ////////////////////////////////////

char* titles_srf02[] = {
    (char*)"Range           ",
    (char*)"Change addr     "
};

SRF02State::SRF02State() {
	pos_enc_init = 0;
    substate = SRF02_SCAN;
    current_addr = 0;
}

void SRF02State::enter(int32_t pos_enc) {
	(void)pos_enc;
	pos_enc_init = pos_enc;
    lcdClear2ndLine(&LCDD1);
	lcdWriteString(&LCDD1, titles_srf02[0], 40);
    next_address();
}

void SRF02State::leave() {
    
}

AbstractState* SRF02State::onUiEvent(struct UiState ui_state) {

    if(substate == SRF02_SCAN) {
        if(ui_state.enc_changed) {
            next_address();
        }
        if(ui_state.ok_clicked && current_addr != 0) {
            pos_enc_init = ui_state.pos_enc;
            substate = SRF02_RANGE_AND_MENU;
        }
        if(ui_state.ret_clicked) {
            lcdClear1stLine(&LCDD1);
            lcdWriteString(&LCDD1, titles_i2c[0], 0);
            lcdWriteString(&LCDD1, titles_srf02[0], 40);
            return &i2cState;
        }
    }
    else if(substate == SRF02_RANGE_AND_MENU) {
        int i = (ui_state.pos_enc - pos_enc_init) % (sizeof(titles_srf02)/sizeof(*titles_srf02));

        if(ui_state.enc_changed) {
            lcdWriteString(&LCDD1, titles_srf02[i], 40);
        }
        if(ui_state.ok_clicked) {
            if(i==0) {
                uint16_t range = 0;
                srf02_range(&I2CD1, current_addr, &range);
                chsnprintf(txt, sizeof(txt), "Range   %3d cm", range);
                lcdWriteString(&LCDD1, txt, 40);
            } else if(i==1) {
                chsnprintf(txt, sizeof(txt), "Change addr 0x%02X", MIN_ADDRESS);
                lcdWriteString(&LCDD1, txt, 40);
                pos_enc_init = ui_state.pos_enc;
                substate = SRF02_CHANGING_ADDR;
            }
        }
        if(ui_state.ret_clicked) {
            lcdWriteString(&LCDD1, titles_srf02[0], 40);
            substate = SRF02_SCAN;
        }
    }
    else if(substate == SRF02_CHANGING_ADDR) {
        uint8_t addr = (ui_state.pos_enc - pos_enc_init) % (MAX_ADDRESS - MIN_ADDRESS + 1) + MIN_ADDRESS;

        if(ui_state.enc_changed) {
            chsnprintf(txt, sizeof(txt), "Change addr 0x%02X", addr);
            lcdWriteString(&LCDD1, txt, 40);
        }
        if(ui_state.ok_clicked) {
            //set address
            if(srf02_change_address(&I2CD1, current_addr, addr) == I2C_OK) {
                current_addr = addr;
                chsnprintf(txt, sizeof(txt), "0x%02X", current_addr);
                lcdWriteString(&LCDD1, txt, 10);
                ui_state.ret_clicked = true;
            }
        }
        if(ui_state.ret_clicked) {
            lcdWriteString(&LCDD1, titles_srf02[1], 40);
            pos_enc_init = ui_state.pos_enc - 1;
            substate = SRF02_RANGE_AND_MENU;
        }
    }
    
	return nullptr;
}

AbstractState* SRF02State::periodic() {
    return nullptr;
}

void SRF02State::next_address() {
    current_addr = get_next_device(&I2CD1);
    if(current_addr == 0) {
        chsnprintf(txt, sizeof(txt), "END ");
    } else {
        chsnprintf(txt, sizeof(txt), "0x%02X", current_addr);
    }
    lcdWriteString(&LCDD1, txt, 10);
}


///////////////////////////  VL6280x  ///////////////////////////////////

VL6180xState::VL6180xState() {
	pos_enc_init = 0;
}

void VL6180xState::enter(int32_t pos_enc) {
	(void)pos_enc;
	pos_enc_init = pos_enc;
    vl6180x.init(&I2CD1);
	lcdWriteString(&LCDD1, titles_i2c[1], 0);
    lcdClear2ndLine(&LCDD1);
    range_and_display();
}

void VL6180xState::leave() {
    lcdClear2ndLine(&LCDD1);
}

AbstractState* VL6180xState::onUiEvent(struct UiState ui_state) {

    if(ui_state.ok_clicked) {
        range_and_display();
    }
    if(ui_state.ret_clicked) {
        return &i2cState;
    }
    
	return nullptr;
}

AbstractState* VL6180xState::periodic() {
    return nullptr;
}

void VL6180xState::range_and_display() {
    uint8_t range;
    uint8_t status;
    int ret = vl6180x.do_single_shot(&range, &status);
    if(ret==I2C_OK) {
        if((VL6180x::RangeError)status == VL6180x::NO_ERROR) {
            lcdClear2ndLine(&LCDD1);
            chsnprintf(txt, sizeof(txt), "%3d mm ", range);
        } else {
            chsnprintf(txt, sizeof(txt), "%s", vl6180x.get_error_msg((VL6180x::RangeError)status));
        }
    } else {
        chsnprintf(txt, sizeof(txt), "I2C Err");
    }
    lcdWriteString(&LCDD1, txt, 40);
}
