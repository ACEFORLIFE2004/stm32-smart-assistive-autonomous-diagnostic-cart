/*
 * bluetooth.h
 *
 *  Created on: Mar 29, 2026
 *      Author: akhan
 */

#ifndef INC_BLUETOOTH_H_
#define INC_BLUETOOTH_H_

#include "stm32f4xx_hal.h"
#include "motor_driver.h" // So we can call drive_forward(), etc.

// External handle from main.c
extern UART_HandleTypeDef huart6;

// Global variable for the incoming byte
extern uint8_t bt_data;

// Function Prototypes
void BT_Init(void);
void BT_Process_Command(uint8_t cmd);

#endif /* INC_BLUETOOTH_H_ */
