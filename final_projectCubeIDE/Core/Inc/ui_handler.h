/*
 * ui_handler.h
 *
 *  Created on: Mar 21, 2026
 *      Author: akhan
 */

#ifndef INC_UI_HANDLER_H_
#define INC_UI_HANDLER_H_

#include "system.h"
//#include "diagnostic_handler.h"
//#include "assistive_cart_logic.h"
#include "keypad.h"
#include "tft_lcd.h"

void LCD_UpdateFuncValues();

void ui_update(void);

void handle_keypad_input(char key);

#endif /* INC_UI_HANDLER_H_ */
