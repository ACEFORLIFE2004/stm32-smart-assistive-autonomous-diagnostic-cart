/*
 * debugger.h
 *
 *  Created on: Mar 17, 2026
 *      Author: arnol
 */

#ifndef INC_DEBUGGER_H_
#define INC_DEBUGGER_H_

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define DEBUG_TX_TIMEOUT		500 // Will need to calculate based on huart3 configuration
#define MAX_FUNCTION_CALL_DEPTH	20
#define NEXT_CALL_ARROW			"->"

typedef uint8_t debugtrc_t;

extern uint8_t call_depth;
//extern char debug_trace[60];
extern debugtrc_t call_stack[MAX_FUNCTION_CALL_DEPTH];

extern char error_msg_buf[128];
//extern uint8_t error_msg_buf_len;

/* Private function prototypes -----------------------------------------------*/
void append_stack(debugtrc_t function_id);
void pop_stack(void);
void print_stack(void);

/* Public function prototypes ------------------------------------------------*/
void print_error_msg(void);
void print_msg(const char *msg, ...);


#endif /* INC_DEBUGGER_H_ */
