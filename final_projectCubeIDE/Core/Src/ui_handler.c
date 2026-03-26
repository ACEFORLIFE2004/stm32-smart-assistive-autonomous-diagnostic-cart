/*
 * ui_handler.c
 *
 *  Created on: Mar 21, 2026
 *      Author: akhan
 */

#include "ui_handler.h"
#include "tft_lcd.h"

SystemEnv_t* get_system_state(void);
GlobalState_t get_global_state(void);

//void request_system_mode(SystemMode_t new_system_mode);
//void request_cart_state(AssistiveCartSubstate_t new_cart_state);
//void request_diagnostic_state(DiagnosticSubstate_t new_diagnostic_state);

static GlobalState_t last_global_state = SYS_BOOT;
static SystemMode_t last_system_mode = MODE_NONE;
static DiagnosticSubstate_t last_diag_state = DIAGNOSTIC_INIT;

void ui_update(void) {
    GlobalState_t current_global = get_global_state();
    SystemEnv_t* env = get_system_state();

    // CASE: MAIN MENU
    if (current_global == SYS_IDLE) {
        if (last_global_state != SYS_IDLE) {
            LCD_DrawModeSelect();
            last_global_state = SYS_IDLE;
            last_system_mode = MODE_NONE;
            last_diag_state = DIAGNOSTIC_INIT;
        }
    }

    // CASE: RUNNING A MODE
    else if (current_global == SYS_RUNNING) {
        last_global_state = SYS_RUNNING;

        // IF WE ARE IN CART MODE
        if (env->system_mode == MODE_ASSIST) {
            if (last_system_mode != MODE_ASSIST) {
                LCD_DrawCartMain();
                last_system_mode = MODE_ASSIST;
            }
        }

        // IF WE ARE IN DIAGNOSTIC MODE
        else if (env->system_mode == MODE_DIAGNOSTIC) {

        	if (last_system_mode != MODE_DIAGNOSTIC) {
                LCD_DrawSigMain();
                last_system_mode = MODE_DIAGNOSTIC;
            }

            if (env->diagnostic_state == DIAGNOSTIC_CONFIG && last_diag_state != DIAGNOSTIC_CONFIG) {
                LCD_DrawSigInput();
                last_diag_state = DIAGNOSTIC_CONFIG;
            }
            else if (env->diagnostic_state == DIAGNOSTIC_RUNNING && last_diag_state != DIAGNOSTIC_RUNNING) {
                LCD_DrawSigMain();
                last_diag_state = DIAGNOSTIC_RUNNING;
            }
        }
    }
}

void handle_keypad_input(char key) {
    GlobalState_t current_global = get_global_state();
    SystemEnv_t* env = get_system_state();

    // IF WE ARE IN THE MAIN MENU
    if (current_global == SYS_IDLE) {
        if (key == '1') {
            request_system_mode(MODE_ASSIST);
        } else if (key == '2') {
            request_system_mode(MODE_DIAGNOSTIC);
        }
    }

    // IF WE ARE ALREADY RUNNING A MODE
    else if (current_global == SYS_RUNNING) {
        if (key == '*') {
            request_system_mode(MODE_NONE);
        }

        if (env->system_mode == MODE_DIAGNOSTIC) {
            if (key == '3') request_diagnostic_state(DIAGNOSTIC_CONFIG);
            if (key == '4') request_diagnostic_state(DIAGNOSTIC_RUNNING);
        }
    }
}
