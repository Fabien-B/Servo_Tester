/*
 * ui.c
 *
 *  Created on: 19 juin 2019
 *      Author: fabien
 */

#include "ui.h"
#include "lcd.h"

#define LCD_RS	4
#define LCD_RW	5
#define LCD_E	6
#define LCD_D4	3
#define LCD_D5	2
#define LCD_D6	0
#define LCD_D7	1

static const lcd_pins_t lcdpins = {
  (1<<LCD_RS),
  (1<<LCD_RW),
  (1<<LCD_E),
  {
   (1<<LCD_D4),
   (1<<LCD_D5),
   (1<<LCD_D6),
   (1<<LCD_D7)
  }
};

static const LCDConfig lcdcfg = {
  LCD_CURSOR_OFF,           /* Cursor disabled */
  LCD_BLINKING_OFF,         /* Blinking disabled */
  LCD_SET_FONT_5X8,        /* Font 5x10 */
  LCD_SET_2LINES,           /* 2 lines */
  &lcdpins,                 /* pin map */
};





void init_ui() {
	lcdInit();
	lcdStart(&LCDD1, &lcdcfg);

	  uint8_t mid[8] = {
	        0b00000,
	        0b00100,
	        0b00100,
	        0b00100,
	        0b11110,
	        0b11111,
			0b01110,
			0b00000,
	      };

	  lcdCreateChar(&LCDD1, 2, mid);


}
