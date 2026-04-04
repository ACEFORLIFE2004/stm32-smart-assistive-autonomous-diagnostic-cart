/*
 * assistive_cart_logic.c
 *
 *  Created on: Mar 18, 2026
 *      Author: arnol
 */

#include "assistive_cart_logic.h"
#include "bluetooth.h"

extern uint32_t last_packet_time;

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

void cart_update() {
    extern float current_left, current_right;

    switch(current_cart_state) {
        case CART_INIT:
            initialize_cart_motors();
            // Transition to wait
            current_cart_state = CART_WAIT_CMD;
            request_cart_state(CART_WAIT_CMD); // Tell the system state machine
            print_msg("Cart: Initialized and Waiting\n");
            break;

        case CART_WAIT_CMD:
            BT_Update_Physics();
            // If the wheels start moving, transition to MOVING
            if (abs(current_left) > 1.0f || abs(current_right) > 1.0f) {
                current_cart_state = CART_MOVING;
                request_cart_state(CART_MOVING);
            }
            break;

        case CART_MOVING:
            BT_Update_Physics();
            // If we come to a full stop, go back to WAIT
            if (abs(current_left) < 0.1f && abs(current_right) < 0.1f) {
                current_cart_state = CART_WAIT_CMD;
                request_cart_state(CART_WAIT_CMD);
            }
            break;

        case CART_STOP:
            motor_apply_physics(0, 0);
            print_msg("Cart: Emergency Stop Active\n");
            break;
    }
}
