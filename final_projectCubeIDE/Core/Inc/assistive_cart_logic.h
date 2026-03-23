/*
 * assistive_cart_logic.h
 *
 *  Created on: Mar 18, 2026
 *      Author: arnol
 */

#ifndef INC_ASSISTIVE_CART_LOGIC_H_
#define INC_ASSISTIVE_CART_LOGIC_H_

#include "project.h"

typedef enum{
	CART_INIT	= 0
}AssistiveCartSubstate_t;

//extern volatile SystemEnv_t system_request_state;

void cart_update(void);

void reset_cart_state(void);

#endif /* INC_ASSISTIVE_CART_LOGIC_H_ */
