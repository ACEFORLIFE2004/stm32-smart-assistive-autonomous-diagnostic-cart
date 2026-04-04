/*
 * keypad.c
 *
 *  Created on: Mar 25, 2026
 *      Author: akhan
 */

#include "keypad.h";

/* Import from ui_handler.c/.h via forward declaration */
void handle_keypad_input(char key);

volatile int8_t current_row = -1;
volatile int8_t key_pressed = 0;
volatile int8_t pressed_row = -1;
volatile int8_t current_col = -1;

char keypad_map[4][3] = {
    {'1','2','3'},
    {'4','5','6'},
    {'7','8','9'},
    {'*','0','#'}
};

void keypad_update(void) {
    current_row = 0;
    HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_SET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_RESET);

    current_row = 1;
    HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_SET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_RESET);

    current_row = 2;
    HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_SET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_RESET);

    current_row = 3;
    HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_SET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_RESET);

    if (key_pressed) {
        char key = keypad_map[pressed_row][current_col];

        handle_keypad_input(key);

        // Debounce
        HAL_Delay(200);
        key_pressed = 0;
    }
}
