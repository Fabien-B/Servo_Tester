/*
    PLAY Embedded - Copyright (C) 2006..2015 Rocco Marco Guglielmi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
    Special thanks to Giovanni Di Sirio for teachings, his moral support and
    friendship. Note that some or every piece of this file could be part of
    the ChibiOS project that is intellectual property of Giovanni Di Sirio.
    Please refer to ChibiOS/RT license before use this file.
	
	For suggestion or Bug report - guglielmir@playembedded.org
 */

/**
 * @file    lcd.c
 * @brief   LCD complex driver code.
 *
 * @addtogroup LCD
 * @{
 */

#include "lcd.h"

#if USERLIB_USE_LCD || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

char blank[] = "                ";

/* LCD REGISTERS */
#define LCD_INSTRUCTION_R               PAL_LOW
#define LCD_DATA_R                      PAL_HIGH

/* LCD_INSTRUCTIONS */
#define LCD_CLEAR_DISPLAY               0x01

#define LCD_RETURN_HOME                 0x02

#define LCD_EMS                         0x04
#define LCD_EMS_S                       0x01
#define LCD_EMS_ID                      0x02

#define LCD_DC                          0x08
#define LCD_DC_B                        0x01
#define LCD_DC_C                        0x02
#define LCD_DC_D                        0x04

#define LCD_CDS                         0x10
#define LCD_CDS_RL                      0x04
#define LCD_CDS_SC                      0x08

#define LCD_FS                          0x20
#define LCD_FS_F                        0x04
#define LCD_FS_N                        0x08
#define LCD_FS_DL                       0x10

#define LCD_SET_CGRAM_ADDRESS           0x40
#define LCD_SET_CGRAM_ADDRESS_MASK      0X3F

#define LCD_SET_DDRAM_ADDRESS           0x80
#define LCD_SET_DDRAM_ADDRESS_MASK      0X7F

#define LCD_BUSY_FLAG                   0X80

#define PCA9554_INPUT_PORT 0
#define PCA9554_OUTPUT_PORT 1
#define PCA9554_POLARITY_INVERSION 2
#define PCA9554_CONFIGURATION 3

#define PCA9554_OUTPUT 0
#define PCA9554_INPUT  1

#define SET_MODE_INPUT(reg, mask) (reg |= mask)
#define SET_MODE_OUTPUT(reg, mask) (reg &= ~mask)

//#define SET_BIT(reg, mask) (reg |= mask)
//#define CLEAR_BIT(reg, mask) (reg &= ~mask)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   LCDD1 driver identifier.
 */
LCDDriver LCDD1;

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/


uint8_t pca9554_read(uint8_t reg) {
	uint8_t read;
	i2cMasterTransmitTimeout(&I2CD1,
		0X20, &reg, 1,
		&read, 1,
		10);
	return read;

}

msg_t pca9554_write(uint8_t reg, uint8_t data) {
	uint8_t bytes[2] = {reg, data};
	msg_t ret = i2cMasterTransmitTimeout(&I2CD1,
			0X20, bytes, 2,
			NULL, 0,
			10);
	return ret;
}


/**
 * @brief   Get the busy flag
 *
 * @param[in] lcdp          LCD driver
 *
 * @return                  The LCD status.
 * @retval TRUE             if the LCD is busy on internal operation.
 * @retval FALSE            if the LCD is in idle.
 *
 * @notapi
 */
static bool hd44780IsBusy(LCDDriver *lcdp) {
  bool busy;
  unsigned ii;

  /* Configuring Data PINs as Input. */
  uint8_t configuration = pca9554_read(PCA9554_CONFIGURATION);
  for(ii = 0; ii < LINE_DATA_LEN; ii++) {
	  SET_MODE_INPUT(configuration, lcdp->config->pinmap->D[ii]);
  }
  pca9554_write(PCA9554_CONFIGURATION, configuration);

  uint8_t output_port = pca9554_read(PCA9554_OUTPUT_PORT);
  SET_BIT(output_port, lcdp->config->pinmap->RW);
  CLEAR_BIT(output_port, lcdp->config->pinmap->RS);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);
  SET_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);

  busy = ((pca9554_read(PCA9554_INPUT_PORT) & lcdp->config->pinmap->D[LINE_DATA_LEN - 1]) != 0);
  CLEAR_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);

#if LCD_USE_4_BIT_MODE
  SET_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);
  CLEAR_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);
#endif
  return busy;
}

/**
 * @brief   Write a data into a register for the lcd
 *
 * @param[in] lcdp          LCD driver
 * @param[in] reg           Register id
 * @param[in] value         Writing value
 *
 * @notapi
 */
static void hd44780WriteRegister(LCDDriver *lcdp, uint8_t reg, uint8_t value){

  unsigned ii;

  while (hd44780IsBusy(lcdp));

//  uint8_t output_port = pca9554_read(PCA9554_OUTPUT_PORT);
//  CLEAR_BIT(output_port, lcdp->config->pinmap->E);
//  CLEAR_BIT(output_port, lcdp->config->pinmap->RW);
//  if(reg) {
//	  SET_BIT(output_port, lcdp->config->pinmap->RS);
//  } else {
//	  CLEAR_BIT(output_port, lcdp->config->pinmap->RS);
//  }
//  pca9554_write(PCA9554_OUTPUT_PORT, output_port);

  /* Configuring Data PINs as Output Push Pull. */
  uint8_t configuration = pca9554_read(PCA9554_CONFIGURATION);
  for(ii = 0; ii < LINE_DATA_LEN; ii++) {
    SET_MODE_OUTPUT(configuration, lcdp->config->pinmap->D[ii]);
  }
  pca9554_write(PCA9554_CONFIGURATION, configuration);

  uint8_t output_port = pca9554_read(PCA9554_OUTPUT_PORT);
  CLEAR_BIT(output_port, lcdp->config->pinmap->E);
  CLEAR_BIT(output_port, lcdp->config->pinmap->RW);
  if(reg) {
	  SET_BIT(output_port, lcdp->config->pinmap->RS);
  } else {
	  CLEAR_BIT(output_port, lcdp->config->pinmap->RS);
  }
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);

#if LCD_USE_4_BIT_MODE
  for(ii = 0; ii < LINE_DATA_LEN; ii++) {
    if(value & (1 << (ii + 4)))
      SET_BIT(output_port, lcdp->config->pinmap->D[ii]);
    else
      CLEAR_BIT(output_port, lcdp->config->pinmap->D[ii]);
  }
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  SET_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);
  CLEAR_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);

  for(ii = 0; ii < LINE_DATA_LEN; ii++) {
    if(value & (1 << ii))
    	SET_BIT(output_port, lcdp->config->pinmap->D[ii]);
    else
    	CLEAR_BIT(output_port, lcdp->config->pinmap->D[ii]);
  }
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  SET_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);
  CLEAR_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);
#else
  for(ii = 0; ii < LINE_DATA_LEN; ii++){
      if(value & (1 << ii))
        palSetLine(lcdp->config->pinmap->D[ii]);
      else
        palClearLine(lcdp->config->pinmap->D[ii]);
  }
  palSetLine(lcdp->config->pinmap->E);
  osalThreadSleepMilliseconds(1);
  palClearLine(lcdp->config->pinmap->E);
  osalThreadSleepMilliseconds(1);
#endif
}

/**
 * @brief   Perform a initialization by instruction as explained in HD44780
 *          datasheet.
 * @note    This reset is required after a mis-configuration or if there aren't
 *          condition to enable internal reset circuit.
 *
 * @param[in] lcdp          LCD driver
 *
 * @notapi
 */
static void hd44780InitByIstructions(LCDDriver *lcdp) {
  unsigned ii;

  osalThreadSleepMilliseconds(50);
  uint8_t output_port = pca9554_read(PCA9554_OUTPUT_PORT);
  uint8_t configuration = pca9554_read(PCA9554_CONFIGURATION);
  for(ii = 0; ii < LINE_DATA_LEN; ii++) {
    SET_MODE_OUTPUT(configuration, lcdp->config->pinmap->D[ii]);
    CLEAR_BIT(output_port, lcdp->config->pinmap->D[ii]);
  }
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  pca9554_write(PCA9554_CONFIGURATION, configuration);

  CLEAR_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  CLEAR_BIT(output_port, lcdp->config->pinmap->RW);
  CLEAR_BIT(output_port, lcdp->config->pinmap->RS);
  SET_BIT(output_port, lcdp->config->pinmap->D[LINE_DATA_LEN - 3]);
  SET_BIT(output_port, lcdp->config->pinmap->D[LINE_DATA_LEN - 4]);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);

  	  //osalThreadSleepMilliseconds(1);/////////////////////////
  SET_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);
  CLEAR_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(5);

  SET_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);
  CLEAR_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);

  SET_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);
  CLEAR_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);

#if LCD_USE_4_BIT_MODE
  SET_BIT(output_port, lcdp->config->pinmap->D[LINE_DATA_LEN - 3]);
  CLEAR_BIT(output_port, lcdp->config->pinmap->D[LINE_DATA_LEN - 4]);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  SET_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  osalThreadSleepMilliseconds(1);
  CLEAR_BIT(output_port, lcdp->config->pinmap->E);
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
#endif

  /* Configuring data interface */
  hd44780WriteRegister(lcdp, LCD_INSTRUCTION_R, LCD_FS | LCD_DATA_LENGHT |
                       lcdp->config->font | lcdp->config->lines);

  /* Turning off display and clearing */
  hd44780WriteRegister(lcdp, LCD_INSTRUCTION_R, LCD_DC);
  hd44780WriteRegister(lcdp, LCD_INSTRUCTION_R, LCD_CLEAR_DISPLAY);

  /* Setting display control turning on display */
  hd44780WriteRegister(lcdp, LCD_INSTRUCTION_R, LCD_DC | LCD_DC_D |
                       lcdp->config->cursor | lcdp->config->blinking);

  /* Setting Entry Mode */
  hd44780WriteRegister(lcdp, LCD_INSTRUCTION_R, LCD_EMS | LCD_EMS_ID);
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   LCD Complex Driver initialization.
 *
 * @init
 */
void lcdInit(void) {
  /* Object initialization */
  lcdObjectInit(&LCDD1);
}

/**
 * @brief   Initializes an instance.
 *
 * @param[out] lcdp         pointer to the @p LCDDriver object
 *
 * @init
 */
void lcdObjectInit(LCDDriver *lcdp){

  lcdp->state  = LCD_STOP;
  lcdp->config = NULL;
}

/**
 * @brief   Configures and activates the LCD Complex Driver  peripheral.
 *
 * @param[in] lcd   pointer to the @p LCDDriver object
 * @param[in] config    pointer to the @p LCDConfig object
 *
 * @api
 */
void lcdStart(LCDDriver *lcdp, const LCDConfig *config) {

  osalDbgCheck((lcdp != NULL) && (config != NULL));

  osalDbgAssert((lcdp->state == LCD_STOP) || (lcdp->state == LCD_ACTIVE),
              "lcdStart(), invalid state");

  lcdp->config = config;

  //PCA9554 configuration
  uint8_t configuration = 0b11111111;	//default configuration as input
  uint8_t output_port = 0;
  SET_MODE_OUTPUT(configuration, lcdp->config->pinmap->E);
  SET_MODE_OUTPUT(configuration, lcdp->config->pinmap->RS);
  SET_MODE_OUTPUT(configuration, lcdp->config->pinmap->RW);
  for(int ii = 0; ii < LINE_DATA_LEN; ii++) {
	  SET_MODE_INPUT(configuration, lcdp->config->pinmap->D[ii]);
  }
  pca9554_write(PCA9554_OUTPUT_PORT, output_port);
  pca9554_write(PCA9554_CONFIGURATION, configuration);

  /* Initializing HD44780 by instructions. */
  hd44780InitByIstructions(lcdp);

  lcdp->state = LCD_ACTIVE;
}

/**
 * @brief   Deactivates the LCD Complex Driver  peripheral.
 *
 * @param[in] lcdp      pointer to the @p LCDDriver object
 *
 * @api
 */
void lcdStop(LCDDriver *lcdp) {

  osalDbgCheck(lcdp != NULL);

  osalDbgAssert((lcdp->state == LCD_STOP) || (lcdp->state == LCD_ACTIVE),
              "lcdStop(), invalid state");
  hd44780WriteRegister(lcdp, LCD_INSTRUCTION_R, LCD_DC);
  hd44780WriteRegister(lcdp, LCD_INSTRUCTION_R, LCD_CLEAR_DISPLAY);
  lcdp->state = LCD_STOP;
}

/**
 * @brief   Clears display and return cursor in the first position.
 *
 * @param[in] lcdp      pointer to the @p LCDDriver object
 *
 * @api
 */
void lcdClearDisplay(LCDDriver *lcdp){

  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == LCD_ACTIVE), "lcdClearDisplay(), invalid state");
  hd44780WriteRegister(lcdp, LCD_INSTRUCTION_R, LCD_CLEAR_DISPLAY);
}

void lcdClear1stLine(LCDDriver *lcdp) {
  lcdWriteString(lcdp, blank, 0);
}

void lcdClear2ndLine(LCDDriver *lcdp) {
  lcdWriteString(lcdp, blank, 40);
}

/**
 * @brief   Return cursor in the first position.
 *
 * @param[in] lcdp      pointer to the @p LCDDriver object
 *
 * @api
 */
void lcdReturnHome(LCDDriver *lcdp){

  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == LCD_ACTIVE), "lcdReturnHome(), invalid state");
  hd44780WriteRegister(lcdp, LCD_INSTRUCTION_R, LCD_RETURN_HOME);
}

/**
 * @brief   Set DDRAM address position leaving data unchanged.
 *
 * @param[in] lcdp      pointer to the @p LCDDriver object
 * @param[in] add       DDRAM address (from 0 to LCD_DDRAM_MAX_ADDRESS)
 *
 * @api
 */
void lcdSetAddress(LCDDriver *lcdp, uint8_t add){

  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == LCD_ACTIVE),
                "lcdSetAddress(), invalid state");
  if(add > LCD_SET_DDRAM_ADDRESS_MASK)
    return;
  hd44780WriteRegister(lcdp, LCD_INSTRUCTION_R, LCD_SET_DDRAM_ADDRESS | add);
}

/**
 * @brief   Writes string starting from a certain position.
 *
 * @detail  If string lenght exceeds, then is cutted
 *
 * @param[in] lcdp      pointer to the @p LCDDriver object
 * @param[in] string    string to write
 * @param[in] pos       position for cursor (from 0 to LCD_DDRAM_MAX_ADDRESS)
 *
 * @api
 */
void lcdWriteString(LCDDriver *lcdp, char* string, uint8_t pos){
  int32_t iteration;
  osalDbgCheck((lcdp != NULL) && (string != NULL));
  osalDbgAssert((lcdp->state == LCD_ACTIVE),
                "lcdWriteString(), invalid state");

  iteration = LCD_SET_DDRAM_ADDRESS_MASK - pos + 1;
  if(iteration > 0){
    lcdSetAddress(lcdp, pos);
    while((*string != '\0') && (iteration > 0)){
      hd44780WriteRegister(lcdp, LCD_DATA_R, *string);
      string++;
      iteration--;
    }
  }
  else
    return;
}

void lcdCreateChar(LCDDriver *lcdp, uint8_t location, uint8_t charmap[]) {
	  osalDbgCheck(lcdp != NULL);
	  osalDbgAssert((lcdp->state == LCD_ACTIVE),
	                "lcdSetAddress(), invalid state");

	location &= 0x7; // we only have 8 locations 0-7
	hd44780WriteRegister(lcdp, LCD_INSTRUCTION_R, LCD_SET_CGRAM_ADDRESS  | (location << 3));
	  for (int i=0; i<8; i++) {
		  hd44780WriteRegister(lcdp, LCD_DATA_R, charmap[i]);
	  }
}

/**
 * @brief   Makes a shift according to an arbitrary direction
 *
 * @param[in] lcdp      pointer to the @p LCDDriver object
 * @param[in] dir       direction (LCD_RIGHT or LCD_LEFT)
 *
 * @api
 */
void lcdDoDisplayShift(LCDDriver *lcdp, uint8_t dir){

  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == LCD_ACTIVE),
                "lcdDoDisplayShift(), invalid state");
  hd44780WriteRegister(lcdp, LCD_INSTRUCTION_R, LCD_CDS | LCD_CDS_SC | dir);
}

/** @} */
#endif /* USERLIB_USE_LCD */
