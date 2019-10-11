
extern "C" {
#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"
#include "ttyConsole.h"
#include "lcd.h"
#include "servos.h"
#include "i2c_features.h"
}
#include "ui.h"
#include "StateMachine/StateManager.h"

//const volatile int32_t * const tim2CntPtr = (volatile int32_t *) &(STM32_TIM2->CNT);
//#define TIM2_CNT (*tim2CntPtr)
//
//static void initTim2AsQuadratureDecoder (void) {
//  rccEnableTIM2(NULL);
//  rccResetTIM2();
//  STM32_TIM2->SMCR =  3;          // Encoder mode 3				(TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0)
//  STM32_TIM2->CCER = 0;          // rising edge polarity
//  STM32_TIM2->ARR = 0xFFFFFFFF;      // count from 0-ARR or ARR-0
//  STM32_TIM2->CCMR1 = 0xC1C1;    // f_DTS/16, N=8, IC1->TI1, IC2->TI2
//  STM32_TIM2->CNT = 0;           // Initialize counter
//  STM32_TIM2->EGR = 1;           // generate an update event
//  STM32_TIM2->CR1 = 1;           // Enable the counter
//}




// static THD_WORKING_AREA(waBlinker, 384);
// static void blinker (void *arg)
// {

//   (void)arg;
//   chRegSetThreadName("blinker");

//   while(true) {
// 	  chThdSleepMilliseconds(blinkPeriod);
// 	  palToggleLine(LINE_LED_GREEN);
// 	  palToggleLine(LINE_LED1);
// 	  palToggleLine(LINE_LED2);
//   }
// }

static THD_WORKING_AREA(waUiStateMachine, 500);//384);

StateManager stateManager = StateManager();
static void runStateMachine (void *arg)
{
  (void)arg;
  chRegSetThreadName("blinker");
  stateManager.run();
}


int main(void) {
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();
  initHeap();

  init_servos();
  init_I2C();


  init_ui();

  //init_ui_state_machine(&state_main_menu);
  stateManager.init();


  //chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO, &blinker, NULL); // thread qui fait clignoter la led
  //chThdCreateStatic(waUiStateMachine, sizeof(waUiStateMachine), NORMALPRIO, &states_run, NULL); // thread qui fait clignoter la led
  chThdCreateStatic(waUiStateMachine, sizeof(waUiStateMachine), NORMALPRIO, &runStateMachine, NULL);

  consoleInit(); // initialisation de la liaison série du shell
  consoleLaunch();  // lancement du thread qui gère le shell sur la liaison série


  // main thread does nothing
  chThdSleep(TIME_INFINITE);
}


