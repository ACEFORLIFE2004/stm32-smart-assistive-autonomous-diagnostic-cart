/*
 * ui_interrupt_emulator.c
 *
 *  Created on: Mar 19, 2026
 *      Author: arnol
 */

#include "ui_interrupt_emulator.h"

uint8_t test_no;
emulcnt_t event_seq_num;

/*****************************************************************************
 * At the end of each test, please try to return Global State to SYS_IDLE
 */

/* Test 0: UI requests system switches to a non-existent system mode */
void test_0(void){
	switch(event_seq_num){
		case 0:	/* Request switch to an invalid system mode */
			print_msg("<||> Starting Test %d ...\r\n", test_no);
			project_assert(IS_NO_MODE(get_system_state()->system_mode));
			request_system_mode(MODE_INVALID);
			break;
		case 1: /* Check if global state switched to SYS_MODE_SELECTED */
			project_assert(IS_SYS_MODE_SELECTED(get_global_state()));
			break;
		case 2: /* Check if global state switched to SYS_RUNNING */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			break;
		case 3: /* Check if global state switched to SYS_IDLE */
			project_assert(IS_SYS_IDLE(get_global_state()));
			event_seq_num = 0xFF;	/* End of Test Sequence */ // Set to 0xFF is it resets to zero upon incrementation
			test_no++;				/* Point to next test */
//			print_msg(" \r\n", test_no);
			break;
	}
}

/* UI requests switch to assistive cart mode */
void test_1(void){
	switch(event_seq_num){
		case 0: /* Request switch to assistive cart mode */
			print_msg("<||> Starting Test %d ...\r\n", test_no);
			project_assert(IS_NO_MODE(get_system_state()->system_mode));
			request_system_mode(MODE_ASSIST);
			break;
		case 1:/* Check if global state switched to SYS_MODE_SELECTED & runtime state has correct system mode*/
			project_assert(IS_SYS_MODE_SELECTED(get_global_state()));
			project_assert(IS_ASSIST_MODE(get_system_state()->system_mode));
			break;
		case 2: /* Check if global state switched to SYS_RUNNING */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_ASSIST_MODE(get_system_state()->system_mode));
			break;
		case 3: /* Request system switch to MODE_NONE */
			request_system_mode(MODE_NONE);
			project_assert(IS_ASSIST_MODE(get_system_state()->system_mode));
			break;
		case 4: /* Check runtime state's system mode is set to MODE_NONE */
			project_assert(IS_NO_MODE(get_system_state()->system_mode));
			break;
		case 5: /* Check if global state switched to SYS_IDLE */
			project_assert(IS_SYS_IDLE(get_global_state()));
			event_seq_num = 0xFF;	/* End of Test Sequence */
			test_no++;				/* Point to next test */
//			print_msg(" \r\n", test_no);
			break;
	}
}

/* UI requests switch to diagnostic mode */
void test_2(void){
	switch(event_seq_num){
		case 0: /* Request switch to diagnostic mode */
			print_msg("<||> Starting Test %d ...\r\n", test_no);
			project_assert(IS_NO_MODE(get_system_state()->system_mode));
			request_system_mode(MODE_DIAGNOSTIC);
			break;
		case 1:/* Check if global state switched to SYS_MODE_SELECTED & runtime state has correct system mode*/
			project_assert(IS_SYS_MODE_SELECTED(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			break;
		case 2: /* Check if global state switched to SYS_RUNNING */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			break;
		case 3: /* Request system switch to MODE_NONE */
			request_system_mode(MODE_NONE);
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			break;
		case 4: /* Check runtime state's system mode is set to MODE_NONE */
			project_assert(IS_NO_MODE(get_system_state()->system_mode));
			break;
		case 5: /* Check if global state switched to SYS_IDLE */
			project_assert(IS_SYS_IDLE(get_global_state()));
			event_seq_num = 0xFF;	/* End of Test Sequence */
			test_no++;				/* Point to next test */
//			print_msg(" \r\n", test_no);
			break;
	}
}

/* Display sine function with DAC on oscilloscope */
void test_3(void){
//	if(diag_running_flag == 1){
//		event_seq_num--;
//	}

	switch(event_seq_num){
		case 0: /* Request switch to diagnostic mode */
			print_msg("<||> Starting Test %d ...\r\n", test_no);
			project_assert(IS_NO_MODE(get_system_state()->system_mode));
			request_system_mode(MODE_DIAGNOSTIC);
			break;
		case 1:/* Check if global state switched to SYS_MODE_SELECTED & runtime state has correct system mode*/
			project_assert(IS_SYS_MODE_SELECTED(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			break;
		case 2: /* Check if global state switched to SYS_RUNNING */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_INIT(get_system_state()->diagnostic_state));
			break;
		case 3: /* Configure sine wave: freq = 1kHz, ampl = 1V */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_CONFIG(get_system_state()->diagnostic_state));

			func_wave_1.wave_gen_mode = FUNCTION_WAVE_GEN_MODE;
			func_wave_1.wav_function = SAWTOOTH_FUNC;	/* SQUARE_FUNC TRIANGLE_FUNC SAWTOOTH_FUNC */
			func_wave_1.wav_amplitude = (FXD_FROM_FLOAT(1.45f) > MAX_AMPLITUDE) ? MAX_AMPLITUDE : FXD_FROM_FLOAT(1.45f);
			func_wave_1.wav_frequency = (10000 > MAX_WAVE_FREQ) ? MAX_WAVE_FREQ : 10000;
			func_wave_1.operation = OP_NONE;
			func_wave_1.digital_filter = NO_FILTER;

			diag_config_flag = 1;
			break;
		case 4: /* Check if diagnostic handler is in DIAGNOSTIC_PREPARE state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_PREP(get_system_state()->diagnostic_state));
			break;
		case 5: /* Check if diagnostic handler is in DIAGNOSTIC_RUNNING state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));
			project_assert(func_wave_1.sample_size == MAX_SAMPLE_SIZE);
			break;
		case 6: /* Display sine wave for 10 seconds then exit diagnostic mode*/
			HAL_Delay(15000);

			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));

			diag_running_flag = 0;
			request_system_mode(MODE_NONE);
			break;
		case 7: /* Check runtime state's system mode is set to MODE_NONE */
			project_assert(IS_NO_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_INIT(get_system_state()->diagnostic_state));
			break;
		case 8: /* Check if global state switched to SYS_IDLE */
			project_assert(IS_SYS_IDLE(get_global_state()));
			event_seq_num = 0xFF;	/* End of Test Sequence */
			test_no++;				/* Point to next test */
			break;
	}
}

/* Configure 3 different function generated waveforms and cycle between them
 *
 * Wave 1:
 * 	-> function  = sinusoidal
 * 	-> amplitude = 1V
 * 	-> frequency = 1kHz
 *
 * Wave 2:
 * 	-> function  = sinusoidal
 * 	-> amplitude = 0.5V
 * 	-> frequency = 2kHz
 *
 * Wave 3:
 * 	-> function  = sinusoidal
 * 	-> amplitude = 0.25V
 * 	-> frequency = 500Hz
 * 	*/
void test_4(void){
//	if(diag_running_flag == 1){
//		event_seq_num--;
//	}

	switch(event_seq_num){
		case 0: /* Request switch to diagnostic mode */
			print_msg("<||> Starting Test %d ...\r\n", test_no);
			project_assert(IS_NO_MODE(get_system_state()->system_mode));
			request_system_mode(MODE_DIAGNOSTIC);
			break;
		case 1:/* Check if global state switched to SYS_MODE_SELECTED & runtime state has correct system mode*/
			project_assert(IS_SYS_MODE_SELECTED(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			break;
		case 2: /* Check if global state switched to SYS_RUNNING */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_INIT(get_system_state()->diagnostic_state));
			break;
		case 3: /* Configure Wave 1 */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_CONFIG(get_system_state()->diagnostic_state));

			func_wave_1.wave_gen_mode  = FUNCTION_WAVE_GEN_MODE;
			func_wave_1.wav_function   = SINUSOIDAL_FUNC;
			func_wave_1.wav_amplitude  = (FXD_FROM_FLOAT(1.0f) > MAX_AMPLITUDE) ? MAX_AMPLITUDE : FXD_FROM_FLOAT(1.0f);
			func_wave_1.wav_frequency  = (1000 > MAX_WAVE_FREQ) ? MAX_WAVE_FREQ : 1000;
			func_wave_1.operation 	   = OP_NONE;
			func_wave_1.digital_filter = NO_FILTER;

			diag_config_flag = 1;
			break;
		case 4: /* Check if diagnostic handler is in DIAGNOSTIC_PREPARE state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_PREP(get_system_state()->diagnostic_state));
			break;
		case 5: /* Check if diagnostic handler is in DIAGNOSTIC_RUNNING state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));
			project_assert(func_wave_1.sample_size == MAX_SAMPLE_SIZE);
			break;
		case 6: /* Display Wave 1 for 15 seconds then return to DIAGNOSTIC_CONFIG */
			HAL_Delay(15000);

			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));

			/* Set flag to return to DIAGNOSTIC_CONFIG */
			diag_running_flag = 0;
			diag_config_flag  = 0;
			break;
		case 7: /* Configure Wave 2 */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_CONFIG(get_system_state()->diagnostic_state));

			func_wave_2.wave_gen_mode  = FUNCTION_WAVE_GEN_MODE;
			func_wave_2.wav_function   = SINUSOIDAL_FUNC;
			func_wave_2.wav_amplitude  = (FXD_FROM_FLOAT(0.5f) > MAX_AMPLITUDE) ? MAX_AMPLITUDE : FXD_FROM_FLOAT(0.5f);
			func_wave_2.wav_frequency  = (2000 > MAX_WAVE_FREQ) ? MAX_WAVE_FREQ : 2000;
			func_wave_2.operation      = OP_NONE;
			func_wave_2.digital_filter = NO_FILTER;

			diag_config_flag = 1;
			break;
		case 8: /* Check if diagnostic handler is in DIAGNOSTIC_PREPARE state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_PREP(get_system_state()->diagnostic_state));
			break;
		case 9: /* Check if diagnostic handler is in DIAGNOSTIC_RUNNING state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));
			project_assert(func_wave_2.sample_size == MAX_SAMPLE_SIZE);
			break;
		case 10: /* Display Wave 2 for 15 seconds then return to DIAGNOSTIC_CONFIG */
			HAL_Delay(15000);

			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));

			/* Set flag to return to DIAGNOSTIC_CONFIG */
			diag_running_flag = 0;
			diag_config_flag  = 0;
			break;
		case 11: /* Configure Wave 3 */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_CONFIG(get_system_state()->diagnostic_state));

			func_wave_3.wave_gen_mode  = FUNCTION_WAVE_GEN_MODE;
			func_wave_3.wav_function   = SINUSOIDAL_FUNC;
			func_wave_3.wav_amplitude  = (func_wave_3.wav_amplitude > MAX_AMPLITUDE) ? MAX_AMPLITUDE : FXD_FROM_FLOAT(0.25f);
			func_wave_3.wav_frequency  = (func_wave_3.wav_frequency > MAX_WAVE_FREQ) ? MAX_WAVE_FREQ : 500;
			func_wave_3.operation      = OP_NONE;
			func_wave_3.digital_filter = NO_FILTER;

			diag_config_flag = 1;
			break;
		case 12: /* Check if diagnostic handler is in DIAGNOSTIC_PREPARE state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_PREP(get_system_state()->diagnostic_state));
			break;
		case 13: /* Check if diagnostic handler is in DIAGNOSTIC_RUNNING state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));
			project_assert(func_wave_3.sample_size == MAX_SAMPLE_SIZE);
			break;
		case 14: /* Display Wave 3 for 15 seconds then return to DIAGNOSTIC_CONFIG */
			HAL_Delay(15000);

			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));

			/* Set flag to return to DIAGNOSTIC_CONFIG */
			diag_running_flag = 0;
			diag_config_flag  = 0;
			break;
		case 15: /* Switch to Wave 1 */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_CONFIG(get_system_state()->diagnostic_state));

			diag_config_flag = 1;
			break;
		case 16: /* Check if diagnostic handler is in DIAGNOSTIC_PREPARE state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_PREP(get_system_state()->diagnostic_state));
			break;
		case 17: /* Check if diagnostic handler is in DIAGNOSTIC_RUNNING state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));
			project_assert(func_wave_1.sample_size == MAX_SAMPLE_SIZE);
			break;
		case 18: /* Display Wave 1 for 15 seconds then return to DIAGNOSTIC_CONFIG */
			HAL_Delay(15000);

			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));

			/* Set flag to return to DIAGNOSTIC_CONFIG */
			diag_running_flag = 0;
			diag_config_flag  = 0;
			break;
		case 19: /* Switch to Wave 2 */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_CONFIG(get_system_state()->diagnostic_state));

			diag_config_flag = 1;
			break;
		case 20: /* Check if diagnostic handler is in DIAGNOSTIC_PREPARE state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_PREP(get_system_state()->diagnostic_state));
			break;
		case 21: /* Check if diagnostic handler is in DIAGNOSTIC_RUNNING state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));
			project_assert(func_wave_2.sample_size == MAX_SAMPLE_SIZE);
			break;
		case 22: /* Display Wave 2 for 15 seconds then return to DIAGNOSTIC_CONFIG */
			HAL_Delay(15000);

			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));

			/* Set flag to return to DIAGNOSTIC_CONFIG */
			diag_running_flag = 0;
			diag_config_flag  = 0;
			break;
		case 23: /* Switch to Wave 3 */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_CONFIG(get_system_state()->diagnostic_state));

			diag_config_flag = 1;
			break;
		case 24: /* Check if diagnostic handler is in DIAGNOSTIC_PREPARE state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_PREP(get_system_state()->diagnostic_state));
			break;
		case 25: /* Check if diagnostic handler is in DIAGNOSTIC_RUNNING state */
			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));
			project_assert(func_wave_3.sample_size == MAX_SAMPLE_SIZE);
			break;
		case 26: /* Display Wave 3 for 15 seconds then return to Main Menu */
			HAL_Delay(15000);

			project_assert(IS_SYS_RUNNING(get_global_state()));
			project_assert(IS_DIAGNOSTIC_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_RUN(get_system_state()->diagnostic_state));

			/* Set flag to return to Main Menu */
			diag_running_flag = 0;
			request_system_mode(MODE_NONE);
			break;
		case 27: /* Check runtime state's system mode is set to MODE_NONE */
			project_assert(IS_NO_MODE(get_system_state()->system_mode));
			project_assert(IS_DIAG_INIT(get_system_state()->diagnostic_state));
			break;
		case 28: /* Check if global state switched to SYS_IDLE */
			project_assert(IS_SYS_IDLE(get_global_state()));
			event_seq_num = 0xFF;	/* End of Test Sequence */
			test_no++;				/* Point to next test */
			break;
	}
}

/***************************************************************************************
 * 					MODE SWITCHING LOGIC & GLOBAL STATE MACHINE TEST				   *
 ***************************************************************************************/
void ui_interrupt_emulator(void){
	switch(test_no){
		case 0:
			test_0();
			break;
		case 1:
			test_1();
			break;
		case 2:
			test_2();
			break;
		case 3:
			test_3();
			break;
		case 4:
			test_4();
			break;
		case 5:
			print_msg("<||> All %d tests are over\r\n", test_no);
			while(1){
				;
			}
			break;
	}

}

/***************************************************************************************
 * 																					   *
 ***************************************************************************************/
