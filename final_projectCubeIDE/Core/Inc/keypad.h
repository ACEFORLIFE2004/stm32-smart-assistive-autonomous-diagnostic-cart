/*
 * keypad.h
 *
 *  Created on: Mar 25, 2026
 *      Author: akhan
 */

#ifndef INC_KEYPAD_H_
#define INC_KEYPAD_H_

#include "main.h"
//#include "ui_handler.h"

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 3

void keypad_update(void);

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

//extern void handle_keypad_input(char key);

#endif /* INC_KEYPAD_H_ */
