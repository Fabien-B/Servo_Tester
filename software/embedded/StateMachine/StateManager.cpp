#include "StateManager.h"
#include <ch.h>
#include <hal.h>
#include "stdutil.h"
#include "string.h"

#include "MainMenuState.h"


const volatile int32_t * const tim2CntPtr = (volatile int32_t *) &(STM32_TIM2->CNT);
#define TIM2_CNT (*tim2CntPtr)

struct AbstractState* current_state;
struct AbstractState* next_state;
volatile struct UiState ui_state;
volatile systime_t encoder_change_time = 0;


static void initTim2AsQuadratureDecoder (void) {
 rccEnableTIM2(NULL);
 rccResetTIM2();
 STM32_TIM2->SMCR =  TIM_SMCR_SMS_0;          // Encoder mode 3				(TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0)
 STM32_TIM2->CCER = TIM_CCER_CC1P;	// falling edge polarity
 STM32_TIM2->ARR = 0xFFFFFFFF;      // count from 0-ARR or ARR-0
 STM32_TIM2->CCMR1 = 0xC1C1;    // f_DTS/16, N=8, IC1->TI1, IC2->TI2   // TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;
 STM32_TIM2->CNT = 0;           // Initialize counter
 STM32_TIM2->EGR = 1;           // generate an update event
 STM32_TIM2->CR1 = TIM_CR1_CEN;//1;           // Enable the counter
}



void StateManager::init() {
    current_state = &mainMenuState;
	initTim2AsQuadratureDecoder();
    current_state->enter(ui_state.pos_enc);    
}

void StateManager::run() {
	static bool ok_released, ret_released = true;
    
    while(true) {

		if(ui_state.pos_enc != TIM2_CNT) {
			ui_state.pos_enc = TIM2_CNT;
			ui_state.enc_changed = true;

			systime_t time = chVTGetSystemTimeX();
    		ui_state.encoder_speed = CLAMP_TO(1, 800, (uint32_t)800/(time - encoder_change_time));
			encoder_change_time = time;
		}

		if(palReadLine(LINE_OK_BTN) == PAL_LOW && ok_released) {
			ui_state.ok_clicked = true;
			ok_released = false;
		}

		if(palReadLine(LINE_OK_BTN) == PAL_HIGH) {
			ok_released = true;
		}

		if(palReadLine(LINE_RET_BTN) == PAL_LOW && ret_released) {
			ui_state.ret_clicked = true;
			ret_released = false;
		}

		if(palReadLine(LINE_RET_BTN) == PAL_HIGH) {
			ret_released = true;
		}

        if(ui_state.ok_clicked || ui_state.ret_clicked || ui_state.enc_changed) {

            AbstractState* next = current_state->onUiEvent(ui_state);
            ui_state.clearEvents();

            if(next != nullptr) {
                current_state->leave();
                next->enter(ui_state.pos_enc);
                current_state = next;
            }
        }
        //periodic stuff

		chThdSleepMilliseconds(1);
    }
}

