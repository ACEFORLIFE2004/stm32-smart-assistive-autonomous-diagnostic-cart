/*
 * ui_handler.c
 *
 *  Created on: Mar 21, 2026
 *      Author: akhan
 */

//#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
//#include "project.h"
//#include "system.h"
#include "ui_handler.h"
//#include "fixedpt.h"
//#include "diagnostic_handler.h"
//#include "tft_lcd.h"

//SystemEnv_t* get_system_state(void);
//GlobalState_t get_global_state(void);

//extern DiagnosticConfig_t func_wave_1, func_wave_2, func_wave_3;

const char* arb_files[] = {
    "BEET-M~1.WAV",
    "MERRY-~1.WAV",
    "TWISTE~1.WAV",
    "WE-WIS~1.WAV"
};
#define NUM_ARB_FILES 4
static uint8_t file_idx = 0; // Tracks which file is selected

static uint8_t selection_made = 0;
static uint8_t edit_line = 0;
static char input_buffer[10];
static uint8_t buffer_index = 0;

static GlobalState_t last_global_state = SYS_BOOT;
static SystemMode_t last_system_mode = MODE_NONE;
static DiagnosticSubstate_t last_diag_state = DIAGNOSTIC_INIT;

void LCD_UpdateFuncValues() {
    char str[32];
    uint16_t highlight_color = 0xFFE0; // Yellow for active line
    uint16_t normal_color = 0xFFFF;    // White for inactive

    // --- SHAPE LINE ---
    char* shapes[] = {"SINE", "SQUARE", "TRIANGLE", "SAWTOOTH"};
    uint16_t c0 = (edit_line == 0) ? highlight_color : normal_color;

    // Clear the old text area with a black rectangle
    LCD_DrawRect(150, 50, 150, 20, 0x0000);
    LCD_WriteString(150, 50, shapes[func_wave_1.wav_function], Font_11x18, c0, 0x0000);

    // --- FREQUENCY LINE ---
    uint16_t c1 = (edit_line == 1) ? highlight_color : normal_color;
    LCD_DrawRect(150, 90, 150, 20, 0x0000);

    // If we are currently typing, show the buffer. Otherwise, show the saved value.
    if (edit_line == 1 && buffer_index > 0) {
        sprintf(str, "%s_", input_buffer); // Add an underscore cursor
    } else {
        sprintf(str, "%lu Hz", func_wave_1.wav_frequency);
    }
    LCD_WriteString(150, 90, str, Font_11x18, c1, 0x0000);

    // --- AMPLITUDE LINE ---
    uint16_t c2 = (edit_line == 2) ? highlight_color : normal_color;
    LCD_DrawRect(150, 130, 150, 20, 0x0000);

    if (edit_line == 2 && buffer_index > 0) {
    	// Get the raw number typed so far (e.g., "125")
		int raw_val = atoi(input_buffer);

		// Split it into whole and hundredths manually
		int whole = raw_val / 100;      // 125 -> 1
		int hundredths = raw_val % 100; // 125 -> 25

		// Display it with the dot so the user knows it's 1.25V
		sprintf(str, "%d.%02d V", whole, hundredths);
    } else {
    	// Convert fixed-point to a temporary float
		float amp_f = FXD_TO_FLOAT(func_wave_1.wav_amplitude);

		// Extract the whole number
		int whole = (int)amp_f;

		// Extract the hundredths
		int hundredths = (int)((amp_f - (float)whole) * 100.0f + 0.5f);

		// Safety check for rounding overflow
		if (hundredths >= 100) {
			whole += 1;
			hundredths = 0;
		}

		// Format using only integers
		sprintf(str, "%d.%02d V", whole, abs(hundredths));
    }
    LCD_WriteString(150, 130, str, Font_11x18, c2, 0x0000);
}

void LCD_UpdateArbValues() {
    // Clear only the dynamic area to prevent flicker
    LCD_DrawRect(0, 60, 320, 140, 0x0000);

    // --- FILE SELECTION ---
    uint16_t color0 = (edit_line == 0) ? 0x07FF : 0xFFFF;
    LCD_WriteString(10, 70, "FILE:", Font_11x18, 0x07E0, 0x0000);
    LCD_WriteString(100, 70, (char*)arb_files[file_idx], Font_11x18, color0, 0x0000);

    // --- FILTER ---
    uint16_t color1 = (edit_line == 1) ? 0x07FF : 0xFFFF;
    char* filter_names[] = {"NONE", "FIR", "IIR"};
    LCD_WriteString(10, 110, "FLTR:", Font_11x18, 0x07E0, 0x0000);
    LCD_WriteString(100, 110, filter_names[func_wave_1.digital_filter], Font_11x18, color1, 0x0000);

    // --- OUTPUT MODE ---
    uint16_t color2 = (edit_line == 2) ? 0x07FF : 0xFFFF;
    char* out_names[] = {"WAVE", "FFT"};
    LCD_WriteString(10, 150, "DISP:", Font_11x18, 0x07E0, 0x0000);
    LCD_WriteString(100, 150, out_names[func_wave_1.screen_output], Font_11x18, color2, 0x0000);

    // --- FOOTER ---
    LCD_DrawRect(10, 210, 300, 1, 0x7BEF);
    LCD_WriteString(20, 215, "[*] NEXT  [0-9] TOGGLE", Font_11x18, 0x07E0, 0x0000);
}

void LCD_UpdateCartSpeed(float speed) {
    // Check if we are in the right mode
    SystemEnv_t* env = get_system_state();
    if (get_global_state() != SYS_RUNNING || env->system_mode != MODE_ASSIST) {
        return;
    }

    char speed_str[16];

    // Split float into Whole and Fractional parts
    int whole = (int)speed;
    int tenths = (int)((speed - (float)whole) * 10.0f);

    // Format using only integers
    sprintf(speed_str, "%d.%d m/s", whole, abs(tenths));

    // Clear and Draw
    LCD_WriteString(120, 120, speed_str, Font_11x18, 0x07E0, 0x0000);
}

void LCD_UpdateCartStatus(const char* status, uint16_t color) {
    // Safety Guard
    SystemEnv_t* env = get_system_state();
    if (get_global_state() != SYS_RUNNING || env->system_mode != MODE_ASSIST) {
        return;
    }

    // Clear the old status area first
    LCD_DrawRect(120, 80, 150, 18, 0x0000);

    // Draw the new status
    LCD_WriteString(120, 80, status, Font_11x18, color, 0x0000);
}

void ui_update(void) {
    GlobalState_t current_global = get_global_state();
    SystemEnv_t* env = get_system_state();

    // CASE: MAIN MENU
    if (current_global == SYS_IDLE) {
		if (last_global_state != SYS_IDLE) {
			LCD_DrawModeSelect();

			// RESET EVERYTHING
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
            if (env->diagnostic_state == DIAGNOSTIC_CONFIG && selection_made == 0) {
                if (last_diag_state != DIAGNOSTIC_CONFIG) {
                    LCD_DrawSigMain();
                    print_msg("UI: Drawing Signal Selection Menu (Wait for 3/4)\r\n");
                    last_diag_state = DIAGNOSTIC_CONFIG;
                }
            }

        }
    }

    /* Handle any keypad inputs and make requests to system where necessary/valid */
	keypad_update();
}

void handle_keypad_input(char key) {
    GlobalState_t current_global = get_global_state();
    SystemEnv_t* env = get_system_state();

    // UNIVERSAL EXIT
    if (key == '#') {
    	print_msg("UI: Exit to Main Menu\r\n");
		request_system_mode(MODE_NONE); // This sets GlobalState to SYS_IDLE

		// Reset UI Trackers
		selection_made = 0;
		edit_line = 0;
		buffer_index = 0;
		memset(input_buffer, 0, sizeof(input_buffer));

		// Reset the Data Struct
		func_wave_1.wav_function = 0;       // Back to Sine
		func_wave_1.wav_frequency = 0;      // 0 Hz
		func_wave_1.wav_amplitude = 0;      // 0 V

		// Force the UI to notice the change
		last_global_state = 0xFF;

		return;
    }

    // MAIN MENU NAVIGATION
    if (current_global == SYS_IDLE) {
        if (key == '1') {
        	print_msg("UI: Go into Cart Mode\r\n");
            request_system_mode(MODE_ASSIST);
        } else if (key == '2') {
        	print_msg("UI: Go into Diagnostic Mode\r\n");
            request_system_mode(MODE_DIAGNOSTIC);
            selection_made = 0;
        }
    }

    // DIAGNOSTIC MODE LOGIC
    else if (current_global == SYS_RUNNING && env->system_mode == MODE_DIAGNOSTIC) {

    	if (selection_made == 0) {
    	    if (key == '3') {
    	        func_wave_1.wave_gen_mode = FUNCTION_WAVE_GEN_MODE;
    	        selection_made = 1;
    	        edit_line = 0;
    	        LCD_DrawFuncInput();
    	        LCD_UpdateFuncValues();
    	    } else if (key == '4') {
    	        func_wave_1.wave_gen_mode = ARBITRARY_WAVE_GEN_MODE;
    	        selection_made = 1;

    	        edit_line = 0;

    	        LCD_DrawArbInput();
    	        LCD_UpdateArbValues();
    	    }
    	}

        // --- Function Mode ---
        else if (selection_made == 1 && func_wave_1.wave_gen_mode == FUNCTION_WAVE_GEN_MODE) {

            // NEXT LINE (Star Key)
        	if (key == '*') {
        	    // Save the typed number to the struct before moving
        	    if (buffer_index > 0) {
        	        if (edit_line == 1) {
        	            uint32_t val = atoi(input_buffer);
        	            if (val >= MIN_WAVE_FREQ && val <= MAX_WAVE_FREQ) {
        	                func_wave_1.wav_frequency = val;
        	            }
        	        } else if (edit_line == 2) {
        	            float amp = (float)atoi(input_buffer) / 100.0f;
        	            if (amp > 3.3f) amp = 3.3f;
        	            if (amp < 0.0f) amp = 0.0f;
        	            func_wave_1.wav_amplitude = FXD_FROM_FLOAT(amp);

        	            // --- TRIGGER WAVE GENERATION HERE ---

        	            print_msg("DSP: Generating new waveform...\r\n");

        	            diag_config_flag = 1;

        	            print_msg("DSP: Waveform streaming active.\r\n");
        	        }
        	    }

        	    // Move cursor and clear buffer
        	    edit_line = (edit_line + 1) % 3;
        	    buffer_index = 0;
        	    memset(input_buffer, 0, sizeof(input_buffer));

        	    LCD_UpdateFuncValues();
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
        // --- Arbitrary Mode Logic ---
        else if (selection_made == 1 && func_wave_1.wave_gen_mode == ARBITRARY_WAVE_GEN_MODE) {

            // NAVIGATION
        	if (key == '*') {
        	    // If we are currently on the LAST line (Output) and press '*',
        	    // it means we are looping back.
        	    if (edit_line == 2) {
        	        print_msg("DSP: Finalizing Arbitrary Config...\r\n");
        	        diag_config_flag = 1;
        	    }

        	    edit_line = (edit_line + 1) % 3;

        	    // Reset buffer
        	    buffer_index = 0;
        	    memset(input_buffer, 0, sizeof(input_buffer));

        	    LCD_UpdateArbValues();
        	}

            // CHANGE SETTINGS
            else if (key >= '0' && key <= '9') {
                if (edit_line == 0) {
                    // --- FILE SELECTION ---
                    file_idx = (file_idx + 1) % NUM_ARB_FILES;
                    strncpy(func_wave_1.file_name, arb_files[file_idx], STORAGE_NAME_LEN);
                    func_wave_1.file_name[STORAGE_NAME_LEN - 1] = '\0';

                    print_msg("File Selected: %s\r\n", func_wave_1.file_name);
                }
                else if (edit_line == 1) {
                    // --- FILTER SELECTION ---
                    func_wave_1.digital_filter = (func_wave_1.digital_filter + 1) % 3;
                }
                else if (edit_line == 2) {
                    // --- OUTPUT SELECTION ---
                    func_wave_1.screen_output = (func_wave_1.screen_output + 1) % 2;
                }

				print_msg("Updating Arb Wave Config...\r\n");
                LCD_UpdateArbValues();
            }
        }
    }
}
