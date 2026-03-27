/*
 * ui_handler.c
 *
 *  Created on: Mar 21, 2026
 *      Author: akhan
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_handler.h"
#include "fixedpt.h"
#include "wave_generator.h"
#include "tft_lcd.h"

SystemEnv_t* get_system_state(void);
GlobalState_t get_global_state(void);

extern DiagnosticConfig_t func_wave_1;

static uint8_t selection_made = 0;
static uint8_t edit_line = 0;       // Matches the logic below
static char input_buffer[10];       // Matches the logic below
static uint8_t buffer_index = 0;    // Matches the logic below

//void request_system_mode(SystemMode_t new_system_mode);
//void request_cart_state(AssistiveCartSubstate_t new_cart_state);
//void request_diagnostic_state(DiagnosticSubstate_t new_diagnostic_state);

static GlobalState_t last_global_state = SYS_BOOT;
static SystemMode_t last_system_mode = MODE_NONE;
static DiagnosticSubstate_t last_diag_state = DIAGNOSTIC_INIT;

void LCD_UpdateFuncValues() {
    char str[32];
    uint16_t highlight_color = 0xFFE0; // Yellow for active line
    uint16_t normal_color = 0xFFFF;    // White for inactive

    // --- 1. SHAPE LINE ---
    char* shapes[] = {"SINE", "SQUARE", "TRIANGLE", "SAWTOOTH"};
    uint16_t c0 = (edit_line == 0) ? highlight_color : normal_color;

    // Clear the old text area with a black rectangle first to prevent "ghosting"
    LCD_DrawRect(150, 50, 150, 20, 0x0000);
    LCD_WriteString(150, 50, shapes[func_wave_1.wav_function], Font_11x18, c0, 0x0000);

    // --- 2. FREQUENCY LINE ---
    uint16_t c1 = (edit_line == 1) ? highlight_color : normal_color;
    LCD_DrawRect(150, 90, 150, 20, 0x0000);

    // If we are currently typing, show the buffer. Otherwise, show the saved value.
    if (edit_line == 1 && buffer_index > 0) {
        sprintf(str, "%s_", input_buffer); // Add an underscore cursor
    } else {
        sprintf(str, "%lu Hz", func_wave_1.wav_frequency);
    }
    LCD_WriteString(150, 90, str, Font_11x18, c1, 0x0000);

    // --- 3. AMPLITUDE LINE ---
    uint16_t c2 = (edit_line == 2) ? highlight_color : normal_color;
    LCD_DrawRect(150, 130, 150, 20, 0x0000);

    if (edit_line == 2 && buffer_index > 0) {
    	// 1. Get the raw number typed so far (e.g., "125")
		int raw_val = atoi(input_buffer);

		// 2. Split it into whole and hundredths manually
		int whole = raw_val / 100;      // 125 -> 1
		int hundredths = raw_val % 100; // 125 -> 25

		// 3. Display it with the dot so the user knows it's 1.25V
		sprintf(str, "%d.%02d V", whole, hundredths);
    } else {
    	// 1. Convert Arnold's fixed-point to a temporary float
		float amp_f = FXD_TO_FLOAT(func_wave_1.wav_amplitude);

		// 2. Extract the whole number (e.g., 1.45 -> 1)
		int whole = (int)amp_f;

		// 3. Extract the hundredths (e.g., 0.45 * 100 -> 45)
		// We add 0.5 to the result before casting to int to handle rounding
		// (e.g., 0.44999 becomes 45 instead of 44)
		int hundredths = (int)((amp_f - (float)whole) * 100.0f + 0.5f);

		// Safety check for rounding overflow (e.g., 0.999 -> 1.00)
		if (hundredths >= 100) {
			whole += 1;
			hundredths = 0;
		}

		// 4. Format using only integers (%d)
		// %02d is CRITICAL: it ensures "1.05" doesn't look like "1.5"
		sprintf(str, "%d.%02d V", whole, abs(hundredths));
    }
    LCD_WriteString(150, 130, str, Font_11x18, c2, 0x0000);
}

void LCD_UpdateArbValues() {
    char str[16];
    char* filters[] = {"NONE", "FIR ", "IIR "}; // Added space to clear old text
    char* outputs[] = {"WAVE", "FFT "};

    uint16_t high = 0xFFE0; // Yellow for selected
    uint16_t norm = 0xFFFF; // White for unselected

    // --- 1. FILTER LINE (Row 2, Y=90) ---
    uint16_t c1 = (edit_line == 1) ? high : norm;
    // Clear value area only (X starts around 150)
    LCD_DrawRect(150, 90, 100, 20, 0x0000);
    LCD_WriteString(150, 90, filters[func_wave_1.digital_filter], Font_11x18, c1, 0x0000);

    // --- 2. OUTPUT MODE (Row 3, Y=130) ---
    uint16_t c2 = (edit_line == 2) ? high : norm;
    LCD_DrawRect(150, 130, 100, 20, 0x0000);
    LCD_WriteString(150, 130, outputs[func_wave_1.screen_output], Font_11x18, c2, 0x0000);

    // --- 3. SAMPLE SIZE (Row 4, Y=170) ---
    // This isn't editable in Arnold's current struct logic,
    // but we should show the constant value.
    sprintf(str, "%d", func_wave_1.sample_size);
    LCD_DrawRect(150, 170, 100, 20, 0x0000);
    LCD_WriteString(150, 170, str, Font_11x18, 0xFFFF, 0x0000);
}

void ui_update(void) {
    GlobalState_t current_global = get_global_state();
    SystemEnv_t* env = get_system_state();

    // CASE: MAIN MENU
    if (current_global == SYS_IDLE) {
		if (last_global_state != SYS_IDLE) {
			LCD_DrawModeSelect();

			// RESET EVERYTHING HERE
			last_global_state = SYS_IDLE;
			last_system_mode = MODE_NONE;
			last_diag_state = DIAGNOSTIC_INIT;
			selection_made = 0;

			print_msg("UI: Menu Redrawn & Flags Reset\r\n");
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

            // --- Draw the Selection Menu (Waiting for 3 or 4) ---
            // We only draw this if we haven't made a selection (3/4) yet
            if (env->diagnostic_state == DIAGNOSTIC_CONFIG && selection_made == 0) {
                if (last_diag_state != DIAGNOSTIC_CONFIG) {
                    LCD_DrawSigMain();
                    print_msg("UI: Drawing Signal Selection Menu (Wait for 3/4)\r\n");
                    last_diag_state = DIAGNOSTIC_CONFIG;
                }
            }

        }
    }
}

void handle_keypad_input(char key) {
    GlobalState_t current_global = get_global_state();
    SystemEnv_t* env = get_system_state();

    // 1. UNIVERSAL EXIT (Using '#' as requested)
    if (key == '#') {
    	print_msg("UI: Exit to Main Menu\r\n");
		request_system_mode(MODE_NONE); // This sets GlobalState to SYS_IDLE

		// 1. Reset UI Trackers
		selection_made = 0;
		edit_line = 0;
		buffer_index = 0;
		memset(input_buffer, 0, sizeof(input_buffer));

		// 2. THE FIX: Force the UI to notice the change
		// We set last_global_state to something impossible (like 99)
		// so the next 'ui_update' sees (SYS_IDLE != 99) and redraws the menu.
		last_global_state = 0xFF;

		return;
    }

    // 2. MAIN MENU NAVIGATION (Wait for 1 or 2)
    if (current_global == SYS_IDLE) {
        if (key == '1') {
            request_system_mode(MODE_ASSIST);
        } else if (key == '2') {
            request_system_mode(MODE_DIAGNOSTIC);
            selection_made = 0; // Show the 3/4 selection screen first
        }
    }

    // 3. DIAGNOSTIC MODE LOGIC
    else if (current_global == SYS_RUNNING && env->system_mode == MODE_DIAGNOSTIC) {

        // --- STEP 1: Choose between Function(3) or Arbitrary(4) ---
        if (selection_made == 0) {
            if (key == '3') {
                func_wave_1.wave_gen_mode = FUNCTION_WAVE_GEN_MODE;
                selection_made = 1;
                LCD_DrawFuncInput();     // Draw the labels/lines once
                LCD_UpdateFuncValues(); // Draw the current numbers
            } else if (key == '4') {
                func_wave_1.wave_gen_mode = ARBITRARY_WAVE_GEN_MODE;
                selection_made = 1;
                edit_line = 1;
                LCD_DrawArbInput();
                LCD_UpdateArbValues();
            }
        }

        // --- STEP 2: Function Mode ---
        else if (selection_made == 1 && func_wave_1.wave_gen_mode == FUNCTION_WAVE_GEN_MODE) {

            // NEXT LINE (Star Key)
            if (key == '*') {
                // Save the typed number to the struct before moving
                if (buffer_index > 0) {
                    if (edit_line == 1) {
                        uint32_t val = atoi(input_buffer);
                        // Bounds check based on Arnold's defines
                        if (val >= MIN_WAVE_FREQ && val <= MAX_WAVE_FREQ) {
                            func_wave_1.wav_frequency = val;
                        }
                    } else if (edit_line == 2) {
                    	// Convert string "125" to float 1.25
						float amp = (float)atoi(input_buffer) / 100.0f;

						// Safety Bounds Check (Typical DAC is 0V to 3.3V)
						if (amp > 3.3f) amp = 3.3f;
						if (amp < 0.0f) amp = 0.0f;

						// Convert to Arnold's Q16.16 format using the macro
						func_wave_1.wav_amplitude = FXD_FROM_FLOAT(amp);
                    }
                }

                // Move cursor and clear buffer
                edit_line = (edit_line + 1) % 3; // Cycle through 0, 1, 2
                buffer_index = 0;
                memset(input_buffer, 0, sizeof(input_buffer));

                LCD_UpdateFuncValues(); // Refresh screen to show new focus
            }

            // TYPE NUMBERS
            else if (key >= '0' && key <= '9') {
                if (edit_line == 0) {
                    // Shape selection is a direct map
                    uint8_t choice = key - '0';
                    if (choice <= 3) func_wave_1.wav_function = choice;
                } else {
                    // Frequency and Amplitude use the buffer
                    if (buffer_index < 8) {
                        input_buffer[buffer_index++] = key;
                        input_buffer[buffer_index] = '\0';
                    }
                }
                LCD_UpdateFuncValues();
            }
        }
        // --- STEP 3: Arbitrary Mode Logic ---
        else if (selection_made == 1 && func_wave_1.wave_gen_mode == ARBITRARY_WAVE_GEN_MODE) {

            // 1. NEXT LINE (Star Key - just like Function Mode)
            if (key == '*') {
                // Cycle edit_line between 1 (Filter) and 2 (Output)
                // We skip 0 because that's the Header/Source line
                edit_line = (edit_line == 1) ? 2 : 1;

                // Optional: If you want '*' to also START the wave when on the last line:
                // if (edit_line == 2) stream_arbitrary_wave(&func_wave_1);

                LCD_UpdateArbValues();
            }

            // 2. CHANGE SETTINGS (Number Keys)
            else if (key >= '0' && key <= '9') {
                if (edit_line == 1) {
                    // Cycle: NONE -> FIR -> IIR
                    func_wave_1.digital_filter = (func_wave_1.digital_filter + 1) % 3;
                }
                else if (edit_line == 2) {
                    // Toggle: WAVE -> FFT
                    func_wave_1.screen_output = (func_wave_1.screen_output + 1) % 2;
                }

                LCD_UpdateArbValues();
            }
        }
    }
}
