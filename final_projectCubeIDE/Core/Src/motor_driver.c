/*
 * motor_driver.c
 *
 *  Created on: Mar 28, 2026
 *      Author: arnol
 */

#include "motor_driver.h"

void enable_motor_pwm(void){
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
}

void disable_motor_pwm(void){
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4);
}

void drive_forward(uint8_t speed){
	motor_set_duty(speed, LEFT_MOTOR, FORWARD_DIR);
	motor_set_duty(speed, RIGHT_MOTOR, FORWARD_DIR);
}

void turn_right(uint8_t speed);
void turn_left(uint8_t speed);

void forward_spin_right(uint16_t angle);
void forward_spin_left(uint16_t angle);
void reverse_spin_right(uint16_t angle);
void reverse_spin_left(uint16_t angle);

void cart_brake(void){
	stop_motor(LEFT_MOTOR);
	stop_motor(RIGHT_MOTOR);
}

void cart_coast(void){
	motor_set_duty(0, LEFT_MOTOR, NO_DIR);
	motor_set_duty(0, RIGHT_MOTOR, NO_DIR);
}


void test_motor(void){
	/* Hard Brake to stop */
	drive_forward(100);
	HAL_Delay(10000);
	cart_brake();
	HAL_Delay(1000);

	drive_forward(50);
	HAL_Delay(10000);
	cart_brake();
	HAL_Delay(1000);

	drive_forward(20);
	HAL_Delay(10000);
	cart_brake();
	HAL_Delay(1000);

	/* Coast to a stop */
	drive_forward(100);
	HAL_Delay(10000);
	cart_coast();
	HAL_Delay(1000);

	drive_forward(50);
	HAL_Delay(10000);
	cart_coast();
	HAL_Delay(1000);

	drive_forward(20);
	HAL_Delay(10000);
	cart_coast();
	HAL_Delay(1000);
}
