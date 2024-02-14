#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "ch.h"
#include "hal.h"
#include "microrl/microrlShell.h"
#include "ttyConsole.h"
#include "stdutil.h"
#include "globalVar.h"
#include "printf.h"
#include "portage.h"
#include "servos.h"
#include "DynamixelState.h"


/*===========================================================================*/
/* START OF EDITABLE SECTION                                           */
/*===========================================================================*/


static void cmd_mem (BaseSequentialStream *lchp, int argc,const char * const argv[]);
#if CH_DBG_THREADS_PROFILING
static void cmd_threads (BaseSequentialStream *lchp, int argc,const char * const argv[]);
#endif
static void cmd_uid (BaseSequentialStream *lchp, int argc,const char * const argv[]);
static void cmd_shutdown (BaseSequentialStream *lchp, int argc,const char * const argv[]);
static void cmd_bkp (BaseSequentialStream *lchp, int argc,const char * const argv[]);
static void cmd_conf (BaseSequentialStream *lchp, int argc,const char * const argv[]);
static void cmd_servo (BaseSequentialStream *lchp, int argc,const char * const argv[]);
static void cmd_dynamixel (BaseSequentialStream *lchp, int argc,const char * const argv[]);

static const ShellCommand commands[] = {
  {"mem", cmd_mem},
#if CH_DBG_THREADS_PROFILING
  {"threads", cmd_threads},
#endif
  {"uid", cmd_uid},
  {"shutdown", cmd_shutdown},
  {"bkp", cmd_bkp},
  {"conf", cmd_conf},
  {"servo", cmd_servo},
  {"dyn", cmd_dynamixel},
  {NULL, NULL}
};


static void cmd_servo(BaseSequentialStream *lchp, int argc,const char * const argv[]) {
  if(argc < 2) {
	  chprintf (lchp, "Usage : >servo 2 1400\r\n");
	  return;
  }
  int no_servo;
  int us;
  sscanf(argv[0], "%d", &no_servo);
  sscanf(argv[1], "%d", &us);
  chprintf (lchp, "Set servo %d to %d\r\n", no_servo, us);
  set_servo(no_servo, us);
}

static void cmd_dynamixel(BaseSequentialStream *lchp, int argc,const char * const argv[]) {
  if(argc < 2) {
	  chprintf (lchp, "Usage : >dyn 1 512 [200]\r\n");
	  return;
  }
  int dyn_id;
  int pos;
  sscanf(argv[0], "%d", &dyn_id);
  sscanf(argv[1], "%d", &pos);
  if(argc == 2) {
    chprintf (lchp, "Set dynamixel %d to %d\r\n", dyn_id, pos);
    dyn.move(dyn_id, pos);
  } else if(argc == 3) {
    int speed;
    sscanf(argv[2], "%d", &speed);
    chprintf (lchp, "Set dynamixel %d to %d with speed %d\r\n", dyn_id, pos, speed);
    dyn.moveSpeed(dyn_id, pos, speed);
  }
  
  
}

/*===========================================================================*/
/* START OF PRIVATE SECTION  : DO NOT CHANGE ANYTHING BELOW THIS LINE        */
/*===========================================================================*/

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/


#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(4096)

#if CONSOLE_DEV_USB == 0
static const SerialConfig ftdiConfig =  {
  115200,
  0,
  USART_CR2_STOP1_BITS | USART_CR2_LINEN,
  0
};
#endif


#define MAX_CPU_INFO_ENTRIES 20
typedef struct _ThreadCpuInfo {
  float    ticks[MAX_CPU_INFO_ENTRIES];
  float    cpu[MAX_CPU_INFO_ENTRIES];
  float    totalTicks;
} ThreadCpuInfo ;

#if CH_DBG_THREADS_PROFILING
static void stampThreadCpuInfo (ThreadCpuInfo *ti);
static float stampThreadGetCpuPercent (const ThreadCpuInfo *ti, const uint32_t idx);
#endif

static void cmd_uid(BaseSequentialStream *lchp, int argc,const char * const argv[]) {
  (void)argv;
  if (argc > 0) {
    chprintf (lchp, "Usage: uid\r\n");
    return;
  }

  /* for (uint32_t i=0; i<2000; i++) { */
  /*   bkpram[i] = (uint16_t) i; */
  /* } */
  
  /* for (uint32_t i=0; i<2000; i++) { */
  /*   if (bkpram[i] != (uint16_t) i) { */
  /*     DebugTrace ("bkpram error"); */
  /*   } */
  /* } */

  chprintf (lchp, "uniq id : ");
  for (uint32_t i=0; i< UniqProcessorIdLen; i++)
    chprintf (lchp, "[%x] ", UniqProcessorId[i]);
  chprintf (lchp, "\r\n");
}


static void cmd_shutdown(BaseSequentialStream *lchp, int argc,const char * const argv[]) {
  (void)lchp;
  (void)argc;
  (void)argv;

  systemDeepSleep();
}

#define _XSTR(s) _STR(s)
#define _STR(s) #s

static void cmd_conf(BaseSequentialStream *lchp, int argc,const char * const argv[]) {
  (void)argc;
  (void)argv;

  chprintf (lchp, "SYSCLK = %.2f Mhz\r\n", STM32_SYSCLK/1e6f);
  if (PWR->CR1 & PWR_CR1_LPR) {
      chprintf (lchp, "Low Power Regulator (LPR) selected\r\n");
  } else {
    chprintf (lchp, "Voltage Scalable regulator (VOS) selected in mode : %s\r\n",
	      STM32_VOS ==  STM32_VOS_RANGE1 ? "Regular (1.2V)" :
	      "Low Power (1.0V)");
  }
  chprintf (lchp, "scheduler frequency = %d Hz\r\n", CH_CFG_ST_FREQUENCY);
  chprintf (lchp, "scheduler mode = %s\r\n", CH_CFG_ST_TIMEDELTA ? "Tickless" : "Periodic tick");
  chprintf (lchp, "scheduler idle use WFI_IDLE : %s\r\n", CORTEX_ENABLE_WFI_IDLE ? "Yes" : "No");
  chprintf (lchp, "compiler OPT : %s\r\n", _XSTR(GCCOPT));
}

static void cmd_bkp(BaseSequentialStream *lchp, int argc,const char * const argv[]) {
  (void)lchp;
  if (argc == 0) {
    DebugTrace ("BKP0= %d", (int) RTC->BKP0R);
  } else {
    RTC->BKP0R = atoi (argv[0]);
  }

}


static void cmd_mem(BaseSequentialStream *lchp, int argc,const char * const argv[]) {
  (void)argv;
  if (argc > 0) {
    chprintf (lchp, "Usage: mem\r\n");
    return;
  }

  chprintf (lchp, "core free memory : %u bytes\r\n", chCoreStatus());
  chprintf (lchp, "heap free memory : %u bytes\r\n", getHeapFree());

  void * ptr1 = malloc_m (100);
  void * ptr2 = malloc_m (100);

  chprintf (lchp, "(2x) malloc_m(1000) = %p ;; %p\r\n", ptr1, ptr2);
  chprintf (lchp, "heap free memory : %d bytes\r\n", getHeapFree());

  free_m (ptr1);
  free_m (ptr2);
}

#if  CH_DBG_THREADS_PROFILING
static void cmd_threads(BaseSequentialStream *lchp, int argc,const char * const argv[]) {
  static const char *states[] = {THD_STATE_NAMES};
  Thread *tp = chRegFirstThread();
  (void)argv;
  (void)argc;
  float totalTicks=0;
  float idleTicks=0;

  static ThreadCpuInfo threadCpuInfo = {
    .ticks = {0}, 
    .cpu =   {0}, 
    .totalTicks = 0.f
  };

  for(int i=0; i<MAX_CPU_INFO_ENTRIES; i++) {
    threadCpuInfo.cpu[i] = -1;
  }
  
  stampThreadCpuInfo (&threadCpuInfo);
  
  chprintf (lchp, "    addr    stack  frestk prio refs  state        time \t percent        name\r\n");
  uint32_t idx=0;
  do {
    chprintf (lchp, "%.8lx %.8lx %6lu %4lu %4lu %9s %9lu   %.1f    \t%s\r\n",
	      (uint32_t)tp, (uint32_t)tp->ctx.sp,
	      get_stack_free (tp),
	      (uint32_t)tp->prio, (uint32_t)(tp->refs - 1),
	      states[tp->state], (uint32_t)tp->time, 
	      stampThreadGetCpuPercent (&threadCpuInfo, idx),
	      chRegGetThreadName(tp));
    totalTicks+= (float) tp->time;
    if (strcmp (chRegGetThreadName(tp), "idle") == 0)
      idleTicks =  (float) tp->time;
    tp = chRegNextThread ((Thread *)tp);
    idx++;
  } while (tp != NULL);

  const float idlePercent = (idleTicks*100.f)/totalTicks;
  const float cpuPercent = 100.f - idlePercent;
  chprintf (lchp, "\r\ncpu load = %.2f %%\r\n", cpuPercent);
}
#endif

static const ShellConfig shell_cfg1 = {
#if CONSOLE_DEV_USB == 0
  (BaseSequentialStream *) &CONSOLE_DEV_SD,
#else
  (BaseSequentialStream *) &SDU1,
#endif
  commands
};



void consoleInit (void)
{
  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * USBD1 : FS, USBD2 : HS
   */

#if CONSOLE_DEV_USB != 0
  usbSerialInit(&SDU1, &USBDRIVER); 
  chp = (BaseSequentialStream *) &SDU1;
#else
  sdStart(&CONSOLE_DEV_SD, &ftdiConfig);
  chp = (BaseSequentialStream *) &CONSOLE_DEV_SD;
#endif
  /*
   * Shell manager initialization.
   */
  shellInit();
}


void consoleLaunch (void)
{
  Thread *shelltp = NULL;

 
#if CONSOLE_DEV_USB != 0
   while (TRUE) {
    if (!shelltp) {
      systime_t time=90;


      while (usbGetDriver()->state != USB_ACTIVE) {
	if (time != 100) {
	  time++;
	  chThdSleepMilliseconds(100);
	} else {
	  time=90;
	  //usbSerialReset(&SDU1);
	}
      }
      
      // activate driver, giovani workaround
      chnGetTimeout(&SDU1, TIME_IMMEDIATE);
      while (!isUsbConnected()) {
	chThdSleepMilliseconds(100);
      }
      
      shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
    } else if (shelltp && (chThdTerminated(shelltp))) {
      chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
      shelltp = NULL;           /* Triggers spawning of a new shell.        */
    }
    chThdSleepMilliseconds(100);
  }

#else

   while (TRUE) {
     if (!shelltp) {
       //       palSetPad (BOARD_LED3_P, BOARD_LED3);
       shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
     } else if (chThdTerminated(shelltp)) {
       chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
       shelltp = NULL;           /* Triggers spawning of a new shell.        */
       //       palClearPad (BOARD_LED3_P, BOARD_LED3);
     }
     chThdSleepMilliseconds(100);
   }
   
   
#endif //CONSOLE_DEV_USB

}


#if  CH_DBG_THREADS_PROFILING 
static void stampThreadCpuInfo (ThreadCpuInfo *ti)
{
  const Thread *tp =  chRegFirstThread();
  uint32_t idx=0;
  
  float totalTicks =0;
  do {
    totalTicks+= (float) tp->time;
    ti->cpu[idx] = (float) tp->time - ti->ticks[idx];;
    ti->ticks[idx] = (float) tp->time;
    tp = chRegNextThread ((Thread *)tp);
    idx++;
  } while ((tp != NULL) && (idx < MAX_CPU_INFO_ENTRIES));
  
  const float diffTotal = totalTicks- ti->totalTicks;
  ti->totalTicks = totalTicks;
  
  tp =  chRegFirstThread();
  idx=0;
  do {
    ti->cpu[idx] =  (ti->cpu[idx]*100.f)/diffTotal;
    tp = chRegNextThread ((Thread *)tp);
    idx++;
  } while ((tp != NULL) && (idx < MAX_CPU_INFO_ENTRIES));
}


static float stampThreadGetCpuPercent (const ThreadCpuInfo *ti, const uint32_t idx)
{
  if (idx >= MAX_CPU_INFO_ENTRIES) 
    return -1.f;

  return ti->cpu[idx];
}
#endif
