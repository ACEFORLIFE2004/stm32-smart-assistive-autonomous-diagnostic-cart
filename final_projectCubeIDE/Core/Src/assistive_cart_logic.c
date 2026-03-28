/*
 * assistive_cart_logic.c
 *
 *  Created on: Mar 18, 2026
 *      Author: arnol
 */

#include "assistive_cart_logic.h"

/****************************************************************
 *				Imported public System APIs						*
 ****************************************************************/
typedef struct SystemEnv SystemEnv_t;	/* Assist with forward declaration of system's APIs */

SystemEnv_t* get_system_state(void);
void request_cart_state(AssistiveCartSubstate_t new_cart_state);

/* Private variables ---------------------------------------------------------*/
static AssistiveCartSubstate_t current_cart_state;

void reset_cart_state(void){
	current_cart_state = CART_INIT;
}

void cart_update(){
	;
}
