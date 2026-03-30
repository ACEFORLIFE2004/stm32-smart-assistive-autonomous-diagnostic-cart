/*
 * motor_driver.h
 *
 *  Created on: Mar 28, 2026
 *      Author: arnol
 */

#ifndef INC_MOTOR_DRIVER_H_
#define INC_MOTOR_DRIVER_H_

#include "project.h"

#define ABS_MAX_ACCELERATION	10
#define ABS_MIN_ACCELERATION	0

typedef struct AssistiveCartParam{
	int8_t accel_x, accel_y;				/* Must be between -10 and 10 */

	uint8_t motor_speed_l, motor_speed_r;	/* Must be between 20 and 100*/

	uint8_t vert_dir;
	uint8_t hori_dir;

	uint8_t spin;
}AssistiveCartParam_t;



//void avoid_obstacles(void);

void enable_motor_pwm(void);
void disable_motor_pwm(void);

void drive_forward(uint8_t speed);
void drive_reverse(uint8_t speed);

void turn_right(uint8_t speed);
void turn_left(uint8_t speed);

void forward_spin_right(uint16_t angle);
void forward_spin_left(uint16_t angle);
void reverse_spin_right(uint16_t angle);
void reverse_spin_left(uint16_t angle);

void cart_brake(void);
void cart_coast(void);

void test_motor(void);

#endif /* INC_MOTOR_DRIVER_H_ */
