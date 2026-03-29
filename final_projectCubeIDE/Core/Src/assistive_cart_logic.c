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

//#define MOTOR_L1_Pin GPIO_PIN_7
//#define MOTOR_L1_GPIO_Port GPIOF
//#define MOTOR_L2_Pin GPIO_PIN_9
//#define MOTOR_L2_GPIO_Port GPIOF

//#define MOTOR_R1_Pin GPIO_PIN_10
//#define MOTOR_R1_GPIO_Port GPIOB
//#define MOTOR_R2_Pin GPIO_PIN_11
//#define MOTOR_R2_GPIO_Port GPIOB

/* Private variables ---------------------------------------------------------*/
static AssistiveCartSubstate_t current_cart_state;



void reset_cart_state(void){
	current_cart_state = CART_INIT;
}

void initialize_cart_motors(void){
	cart_brake();
	enable_motor_pwm();
}

void deinitialize_cart_motors(void){
	cart_brake();
	disable_motor_pwm();
}

void cart_update(){
	switch(current_cart_state){
		case CART_INIT:
			initialize_cart_motors();
			test_motor();				/* Notice: It is a pretty long test (well over 1 minute) */
			deinitialize_cart_motors();

			current_cart_state = CART_INIT;
			request_cart_state(current_cart_state);
			break;
		case CART_WAIT_CMD:
			break;
		case CART_MOVING:
			break;
		case CART_STOP:
			break;
	}
}
