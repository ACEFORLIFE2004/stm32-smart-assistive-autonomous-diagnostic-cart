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
		case 0: /* Request switch to an assistive cart mode */
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
		case 0: /* Request switch to an assistive cart mode */
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
