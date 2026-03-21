/*
 * system.h
 *
 *  Created on: Mar 18, 2026
 *      Author: arnol
 */

#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

#include "project.h"
#include "diagnostic_handler.h"
#include "assistive_cart_logic.h"

#define FN_SYS_UPDATE_ID		0x01
#define FN_SYS_SET_BOOT_ID		0x02
#define FN_SYS_INIT_CORE_ID		0x03
#define FN_SYS_INIT_MODE_ID		0x04
#define FN_SYS_DEINIT_CORE_ID	0x05
#define FN_SYS_GET_S_STATE_ID	0x06
#define FN_SYS_GET_G_STATE_ID	0x07
#define FN_SYS_REQ_S_MODE_ID	0x08
#define FN_SYS_REQ_C_SSTATE_ID	0x09
#define FN_SYS_REQ_D_SSTATE_ID	0x0A

typedef enum{
	SYS_BOOT			= 0,
	SYS_IDLE			= 1,
	SYS_MODE_SELECTED	= 2,
	SYS_RUNNING			= 3
}GlobalState_t;

typedef enum{
	MODE_NONE		= 0,	/* Main Menu??? */
	MODE_ASSIST 	= 1,	/* Assistive Cart Controller */
	MODE_DIAGNOSTIC	= 2		/* Includes Basic Function Generation and Arbitrary Waveform Generation */
}SystemMode_t;

typedef struct SystemEnv{
	SystemMode_t system_mode;
	AssistiveCartSubstate_t cart_state;
	DiagnosticSubstate_t diagnostic_state;
}SystemEnv_t;

/* Private function prototypes -----------------------------------------------*/
void system_update(void);
void set_system_boot_state(void);

//static void system_init_core_peripherals(void);
//static void system_init_selected_mode(void);
//static void system_deinit_selected_mode(void);

/* Public function prototypes ------------------------------------------------*/
SystemEnv_t* get_system_state(void);
GlobalState_t get_global_state(void);

void request_system_mode(SystemMode_t new_system_mode);
void request_cart_state(AssistiveCartSubstate_t new_cart_state);
void request_diagnostic_state(DiagnosticSubstate_t new_diagnostic_state);

#endif /* INC_SYSTEM_H_ */
