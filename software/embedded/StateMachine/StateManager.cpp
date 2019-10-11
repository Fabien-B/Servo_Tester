#include "StateManager.h"
#include <ch.h>
#include <hal.h>
#include "stdutil.h"
#include "string.h"

#include "MainMenuState.h"

#define FAST_ENC_THRESHOLD TIME_MS2I(100)

void encoder_cb(void* arg);
void ok_cb(void* arg);
void ret_cb(void* arg);

struct AbstractState* current_state;
struct AbstractState* next_state;
volatile struct UiState ui_state;
volatile systime_t encoder_change_time = 0;

void StateManager::init() {
    current_state = &mainMenuState;
    palEnableLineEvent(LINE_ENC_A, PAL_EVENT_MODE_BOTH_EDGES);
	palSetLineCallbackI(LINE_ENC_A, &encoder_cb, NULL);

	palEnableLineEvent(LINE_OK_BTN, PAL_EVENT_MODE_FALLING_EDGE);
	palSetLineCallbackI(LINE_OK_BTN, &ok_cb, NULL);

	palEnableLineEvent(LINE_RET_BTN, PAL_EVENT_MODE_FALLING_EDGE);
	palSetLineCallbackI(LINE_RET_BTN, &ret_cb, NULL);

    current_state->enter(ui_state.pos_enc);    
}

void StateManager::run() {
    
    while(true) {
        if(ui_state.ok_clicked || ui_state.ret_clicked || ui_state.enc_changed) {
            AbstractState* next = current_state->onUiEvent(ui_state);
            ui_state.clearEvent();

            if(next != nullptr) {
                current_state->leave();
                next->enter(ui_state.pos_enc);
                current_state = next;
            }
        }
        //periodic stuff
    }
}

void encoder_cb(void* arg) {
	(void)arg;

	chSysLockFromISR();
	if(palReadLine(LINE_ENC_A)) {
		if(palReadLine(LINE_ENC_B)) {
			ui_state.pos_enc++;
		} else {
			ui_state.pos_enc--;
		}
	}else {
		if(palReadLine(LINE_ENC_B)) {
			ui_state.pos_enc--;
		} else {
			ui_state.pos_enc++;
		}
	}
	chSysUnlockFromISR();

	systime_t time = chVTGetSystemTimeX();
    ui_state.encoder_speed = CLAMP_TO(1, 500, (uint32_t)500/(time - encoder_change_time));
	encoder_change_time = time;
    ui_state.enc_changed = true;
}

void ok_cb(void* arg) {
	(void)arg;
	ui_state.ok_clicked = true;
}

void ret_cb(void* arg) {
	(void)arg;
	ui_state.ret_clicked = true;
}
