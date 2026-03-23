/*
 * system.c
 *
 *  Created on: Mar 18, 2026
 *      Author: arnol
 */

#include "system.h"
#include "fatfs.h"

/* Private variables ---------------------------------------------------------*/
static GlobalState_t global_state;
static SystemEnv_t system_runtime_state, system_request_state;

static uint8_t deinit_flag;
//static char sys_msg_buf[50];

/* Public variables ----------------------------------------------------------*/
/* PV ENDS HERE */

/* Public function implementations -------------------------------------------*/
SystemEnv_t* get_system_state(void){
	append_stack(FN_SYS_GET_S_STATE_ID);
	pop_stack();
	return &system_runtime_state;
}
GlobalState_t get_global_state(void){
	append_stack(FN_SYS_GET_G_STATE_ID);
	pop_stack();
	return global_state;
}
void request_system_mode(SystemMode_t new_system_mode){
	append_stack(FN_SYS_REQ_S_MODE_ID);

	system_request_state.system_mode = new_system_mode;

	pop_stack();
}
void request_cart_state(AssistiveCartSubstate_t new_cart_state){
	append_stack(FN_SYS_REQ_C_SSTATE_ID);

	system_request_state.cart_state = new_cart_state;

	pop_stack();
}
void request_diagnostic_state(DiagnosticSubstate_t new_diagnostic_state){
	append_stack(FN_SYS_REQ_D_SSTATE_ID);
	print_msg("  --> Requesting new diagnostic state\r\n");
	system_request_state.diagnostic_state = new_diagnostic_state;
	print_msg("  --> Request made\r\n");

	pop_stack();
}

/* Private function implementations ------------------------------------------*/
static void system_init_core_peripherals(void){
	append_stack(FN_SYS_INIT_CORE_ID);

	SYS_Core_Peripheral_Init();

	pop_stack();
}

static void system_init_selected_mode(void){
	append_stack(FN_SYS_INIT_MODE_ID);

	if(system_runtime_state.system_mode == MODE_ASSIST){
		SYS_Cart_Mode_Init();
		print_msg("  -> Selected system mode peripherals initialized\r\n");
		deinit_flag = 0x00;
	}else if(system_runtime_state.system_mode == MODE_DIAGNOSTIC){
		SYS_Diagnostic_Mode_Init();
		print_msg("  -> Selected system mode peripherals initialized\r\n");
		deinit_flag = 0x01;
	}else{
		/* Either still in MODE_NONE or Invalid mode selected */
		(void)sprintf(error_msg_buf, "  <err> system_init_selected_mode: No valid system mode selected\r\n");
		print_error_msg();
	}

	pop_stack();
}

static void system_deinit_selected_mode(void){
	append_stack(FN_SYS_DEINIT_CORE_ID);

	/* Looks like we may need to deinitialize all peripherals. Edit: Simple flag to save the day */
	if(deinit_flag == 0x00){
		SYS_Cart_Mode_DeInit();
		system_runtime_state.cart_state = CART_INIT;
	}else if(deinit_flag == 0x01){
		stop_dac_conversion();
		SYS_Diagnostic_Mode_DeInit();
		system_runtime_state.diagnostic_state = DIAGNOSTIC_INIT;
	}

	pop_stack();
}

/* Need to execute in main before entering Infinite While Loop */
void set_system_boot_state(void){
	append_stack(FN_SYS_SET_BOOT_ID);

	global_state = SYS_BOOT;
	reset_cart_state();
	reset_diagnostic_state();

	pop_stack();
}

void system_update(void){
	append_stack(FN_SYS_UPDATE_ID);

	switch(global_state){
		case SYS_BOOT:
//			measure_exec_time(DEFAULT_RANGE);												  /* Start Test */
			system_init_core_peripherals();														/* <=== Section Under Test */
//			(void)get_exec_time("system_init_core_peripherals", DEFAULT_RANGE, NANO_SECONDS); /* End Test */
			print_msg("\r\nBOOTING:\r\n  -> Core peripherals initialized successfully\r\n");

//			measure_exec_time(DEFAULT_RANGE);												/* Start Test */
			system_runtime_state.system_mode = MODE_NONE;
			system_runtime_state.cart_state = CART_INIT;
			system_runtime_state.diagnostic_state = DIAGNOSTIC_INIT;
//			(void)get_exec_time("system_init_runtime_state", DEFAULT_RANGE, NANO_SECONDS);	/* End Test */
			print_msg("  -> System runtime states initialized successfully\r\n");

			print_msg("  -> Global state transitioning to IDLE ...\r\n");
			global_state = SYS_IDLE;
			print_msg("IDLE:\r\n");
			break;

		case SYS_IDLE:
			if(system_request_state.system_mode != MODE_NONE){
				system_runtime_state.system_mode = system_request_state.system_mode;
				print_msg("  -> System runtime system mode updated\r\n");
				print_msg("  -> Global state transitioning to MODE_SELECTED ...\r\n");
				global_state = SYS_MODE_SELECTED;
				print_msg("MODE SELECTED:\r\n");
			}
			break;

		case SYS_MODE_SELECTED:
			system_init_selected_mode();
			print_msg("  -> Global state transitioning to RUNNING ...\r\n");
			global_state = SYS_RUNNING;
			print_msg("RUNNING:\r\n");
			break;

		case SYS_RUNNING:
			/* Make any updates to system state requested by UI */
			system_runtime_state.system_mode = system_request_state.system_mode;

			if(system_runtime_state.system_mode == MODE_ASSIST){
				/* Continue running assistive cart mode */
				cart_update();
				print_msg("  -> Cart logic updated ...\r\n");

				/* Make any updates to subsystem state requested by cart submodule */
				system_runtime_state.cart_state = system_request_state.cart_state;

			}else if(system_runtime_state.system_mode == MODE_DIAGNOSTIC){
				/* Continue running diagnostic mode */
				diagnostic_update();
				print_msg("  -> Diagnostic logic updated ...\r\n");

				/* Make any updates to subsystem state requested by diagnostic submodule */
				system_runtime_state.diagnostic_state = system_request_state.diagnostic_state;

			}else if(system_runtime_state.system_mode == MODE_NONE){
				system_deinit_selected_mode();
				print_msg("  -> Current system mode peripherals deinitialized\r\n");

				system_request_state.cart_state = CART_INIT;
				system_request_state.diagnostic_state = DIAGNOSTIC_INIT;

				system_runtime_state.cart_state = system_request_state.cart_state;
				system_runtime_state.diagnostic_state = system_request_state.diagnostic_state;

				reset_cart_state();
				reset_diagnostic_state();
				/* Return to Home / Main Menu */
				print_msg("  -> Global state transitioning to IDLE ...\r\n");
				global_state = SYS_IDLE;
				print_msg("IDLE:\r\n");
				break;
			}else{
				/* Invalid mode selected --> Return to SYS_IDLE state */
				(void)sprintf(error_msg_buf, "  <err> system_update: Unrecognized system mode\r\n");
				print_error_msg();

				system_request_state.system_mode = MODE_NONE;
				system_runtime_state.system_mode = system_request_state.system_mode;
				print_msg("  -> Global state transitioning to IDLE ...\r\n");
				global_state = SYS_IDLE;
				print_msg("IDLE:\r\n");
				break;

			}

//			/* Make any updates to system state requested by UI */
//			system_runtime_state.system_mode = system_request_state.system_mode;
			break;
	}

	pop_stack();
}
