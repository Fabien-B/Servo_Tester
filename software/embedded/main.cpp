
extern "C" {
#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"
#include "ttyConsole.h"
#include "lcd.h"
#include "servos.h"
#include "hardware_config.h"
}
#include "ui.h"
#include "StateMachine/StateManager.h"


static THD_WORKING_AREA(waUiStateMachine, 500);

StateManager stateManager = StateManager();
static void runStateMachine (void *arg)
{
  (void)arg;
  chRegSetThreadName("uiStateMachine");
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

  stateManager.init();

  chThdCreateStatic(waUiStateMachine, sizeof(waUiStateMachine), NORMALPRIO, &runStateMachine, NULL);

  consoleInit(); // initialisation de la liaison série du shell
  consoleLaunch();  // lancement du thread qui gère le shell sur la liaison série


  // main thread does nothing
  chThdSleep(TIME_INFINITE);
}


