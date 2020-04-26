#include "DynamixelState.h"
#include "DynamixelSerial.h"
#include "lcd.h"
#include "printf.h"
#include "MainMenuState.h"
#include "stdutil.h"

SerialConfig uart1conf = {
		.speed = 1000000,
		.cr1 = 0,
		.cr2 = USART_CR2_STOP1_BITS,
		.cr3 = USART_CR3_HDSEL
};

DynamixelSerial dyn = DynamixelSerial(1);

DynamixelIDState dynamixelIDState = DynamixelIDState();
DynamixelFnMenuState dynamixelFnMenuState = DynamixelFnMenuState();
DynamixelMoveState dynamixelMoveState = DynamixelMoveState();


void dynInit() {
    sdStop(&SD1);
    uart1conf.speed = 500000;
    sdStart(&SD1, &uart1conf);
}


DynamixelIDState::DynamixelIDState() {
    pos_enc_init = 0;
    dynamixel_id = 0;
}

void DynamixelIDState::enter(int32_t pos_enc) {
    pos_enc_init = pos_enc - dynamixel_id;
    chsnprintf(txt_id, sizeof(txt_id), "Dynamixel ID=%3d", dynamixel_id);
    lcdWriteString(&LCDD1, txt_id, 0);
    char blank[] = "                ";
    lcdWriteString(&LCDD1, blank, 40);
    sdStart(&SD1, &uart1conf);
}

AbstractState* DynamixelIDState::onUiEvent(struct UiState ui_state) {

    if(ui_state.enc_changed) {
        dynamixel_id = (((ui_state.pos_enc - pos_enc_init) % 255) + 255) % 255;     // max ID is 254, the broadcats ID
        chsnprintf(txt_id, sizeof(txt_id), "Dynamixel ID=%3d", dynamixel_id);
        lcdWriteString(&LCDD1, txt_id, 0);
    }

    if(ui_state.ok_clicked) {
        sdStop(&SD1);
        uart1conf.speed = 1000000;
        sdStart(&SD1, &uart1conf);
        dyn.setBD(dynamixel_id, 500000);
        sdStop(&SD1);
        uart1conf.speed = 500000;
        sdStart(&SD1, &uart1conf);
        return &dynamixelFnMenuState;
    }

    if(ui_state.ret_clicked) {
        return &mainMenuState;
    }

    return nullptr;
}

AbstractState* DynamixelIDState::periodic() {
    return nullptr;
}

void DynamixelIDState::leave() {
    
}



////////////////////////////////////////////////////////////////////////////////


char* titles_dyn_fn[] = {
	(char*)"Move            ",
	(char*)"Set ID          ",
	(char*)"Move Speed      ",
	(char*)"Set Torque      "
};

DynamixelFnMenuState::DynamixelFnMenuState() {
    pos_enc_init = 0;
    selectedFn = E_DynamixelMove;
}

void DynamixelFnMenuState::enter(int32_t pos_enc) {
    pos_enc_init = pos_enc - (int) selectedFn;
    //pos_enc_init = pos_enc;
    char txt_id[17];
    chsnprintf(txt_id, sizeof(txt_id), "Dynamixel ID=%3d", dynamixelIDState.get_dynamixel_id());
    lcdWriteString(&LCDD1, txt_id, 0);
    lcdWriteString(&LCDD1, titles_dyn_fn[(int) selectedFn], 40);
}

AbstractState* DynamixelFnMenuState::onUiEvent(struct UiState ui_state) {
    int i = (ui_state.pos_enc - pos_enc_init) % (sizeof(titles_dyn_fn)/sizeof(*titles_dyn_fn));
    selectedFn = (DynamixelFunction) i;
    
    if(ui_state.enc_changed) {
        lcdWriteString(&LCDD1, titles_dyn_fn[i], 40);
    }

    if(ui_state.ok_clicked) {
        return &dynamixelMoveState;
    }

    if(ui_state.ret_clicked) {
        return &dynamixelIDState;
    }

    

    return nullptr;
}

AbstractState* DynamixelFnMenuState::periodic() {
    return nullptr;
}

void DynamixelFnMenuState::leave() {

}


////////////////////////////////////////////////////////////////////////////////


DynamixelMoveState::DynamixelMoveState() {
    pos_enc_init = 0;
    last_enc = 0;
    pos = 512;
    endless_direction = DynamixelSerial::Clockwise;
}

void DynamixelMoveState::enter(int32_t pos_enc) {
    
    last_enc = pos_enc_init = pos_enc;
    char txt_id[17];
    

    switch (dynamixelFnMenuState.get_function())
    {
    case E_DynamixelMove:
        pos = 512;
        dyn.setEndless(dynamixelIDState.get_dynamixel_id(), false);
        {
            int current_pos = dyn.readPosition(dynamixelIDState.get_dynamixel_id());
            if(current_pos >= 0) {
                pos = current_pos;
                dyn.moveSpeed(dynamixelIDState.get_dynamixel_id(), pos, 0);     // after going back from endless mode, speed must be set ?
            }
        }
        chsnprintf(txt_id, sizeof(txt_id), "Dyn Move     %3d", dynamixelIDState.get_dynamixel_id());
        chsnprintf(txt_position, sizeof(txt_position), "Pos = %4d      ", pos);
        break;
    case E_DynamixelSetId:
        chsnprintf(txt_id, sizeof(txt_id), "Dyn SetId    %3d", dynamixelIDState.get_dynamixel_id());
        chsnprintf(txt_position, sizeof(txt_position), "ID : %3d       ", 0);
        break;
    case E_DynamixelSetSpeed:
        pos = 512;
        dyn.setEndless(dynamixelIDState.get_dynamixel_id(), true);
        dyn.turn(dynamixelIDState.get_dynamixel_id(), endless_direction, 0);
        chsnprintf(txt_id, sizeof(txt_id), "Dyn Speed    %3d", dynamixelIDState.get_dynamixel_id());
        chsnprintf(txt_position, sizeof(txt_position), "Speed = %4d CW ", pos);
    default:
        break;
    }

    lcdWriteString(&LCDD1, txt_id, 0);
    lcdWriteString(&LCDD1, txt_position, 40);
}

AbstractState* DynamixelMoveState::onUiEvent(struct UiState ui_state) {
    (void) ui_state.encoder_speed;
    if(ui_state.enc_changed) {
        
        if(dynamixelFnMenuState.get_function() == E_DynamixelMove) {
            pos = CLAMP_TO(0, 1023, pos + (ui_state.pos_enc - last_enc) * ui_state.encoder_speed);
            last_enc = ui_state.pos_enc;
            chsnprintf(txt_position, sizeof(txt_position), "Pos = %4d      ", pos);
            lcdWriteString(&LCDD1, txt_position, 40);
            dyn.move(dynamixelIDState.get_dynamixel_id(), pos);
        }
        else if(dynamixelFnMenuState.get_function() == E_DynamixelSetId) {
            pos = (((ui_state.pos_enc - pos_enc_init) % 254) + 254) % 254;
            chsnprintf(txt_position, sizeof(txt_position), "Id : %4d      ", pos);
            lcdWriteString(&LCDD1, txt_position, 40);
        }
        else if(dynamixelFnMenuState.get_function() == E_DynamixelSetSpeed) {
            pos = CLAMP_TO(0, 1023, pos + (ui_state.pos_enc - last_enc)*ui_state.encoder_speed);
            last_enc = ui_state.pos_enc;

            if(endless_direction == DynamixelSerial::Clockwise) {
                chsnprintf(txt_position, sizeof(txt_position), "Speed = %4d CW ", pos);
            } else {
                chsnprintf(txt_position, sizeof(txt_position), "Speed = %4d CCW", pos);
            }

            
            lcdWriteString(&LCDD1, txt_position, 40);
            dyn.turn(dynamixelIDState.get_dynamixel_id(), endless_direction, pos);
        }
        
        
    }

    if(ui_state.ok_clicked) {
        if(dynamixelFnMenuState.get_function() == E_DynamixelSetId) {
            dyn.setID(dynamixelIDState.get_dynamixel_id(), pos);
            dynamixelIDState.set_dynamixel_id(pos);
            char txt_id[17];
            chsnprintf(txt_id, sizeof(txt_id), "Dyn SetId    %3d", pos);
            chsnprintf(txt_position, sizeof(txt_position), "Id set to %3d   ", pos);
            lcdWriteString(&LCDD1, txt_id, 0);
            lcdWriteString(&LCDD1, txt_position, 40);
        }
        else if(dynamixelFnMenuState.get_function() == E_DynamixelSetSpeed) {
            if(endless_direction == DynamixelSerial::Clockwise) {
                endless_direction = DynamixelSerial::Counterclockwise;
                chsnprintf(txt_position, sizeof(txt_position), "Speed = %4d CCW", pos);
            } else {
                endless_direction = DynamixelSerial::Clockwise;
                chsnprintf(txt_position, sizeof(txt_position), "Speed = %4d CW ", pos);
            }
            dyn.turn(dynamixelIDState.get_dynamixel_id(), endless_direction, pos);
            lcdWriteString(&LCDD1, txt_position, 40);
        }
    }

    if(ui_state.ret_clicked) {
        return &dynamixelFnMenuState;
    }

    

    return nullptr;
}

AbstractState* DynamixelMoveState::periodic() {
    return nullptr;
}

void DynamixelMoveState::leave() {

}