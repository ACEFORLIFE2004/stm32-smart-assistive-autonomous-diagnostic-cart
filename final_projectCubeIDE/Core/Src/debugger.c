/*
 * debugger.c
 *
 *  Created on: Mar 17, 2026
 *      Author: arnol
 */

#include "debugger.h"
#include <stdarg.h>

/* Private variables ---------------------------------------------------------*/
uint8_t call_depth;
char debug_trace_buf[160];
debugtrc_t call_stack[MAX_FUNCTION_CALL_DEPTH];

static char system_msg_buf[128];

/* DON'T FORGET TO CLEAR STACK BY SETTING ALL VALUES TO 0xFF IN MAIN()
	<--> memset(call_stack, 0xFF, MAX_FUNCTION_CALL_DEPTH);
*/

/* Public variables ----------------------------------------------------------*/
char error_msg_buf[128];
//uint8_t error_msg_buf_len;

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Adds Function ID to top of current function call stack
  * @note   This function MUST be called at the start of every user-defined
  * 		function (main() will be handled separately)
  * @param  function_id: -- Function ID set by user. No two functions can have
  * 					 	the same Function ID.
  * 					 -- Function IDs can range from 0x01 to 0xFE.
  * 					 -- 0x00 and 0xFF are reserved IDs.
  * @retval void
  */
void append_stack(debugtrc_t function_id){
	call_stack[call_depth] = function_id;
	call_depth++;
}

/**
  * @brief  Removes Function ID at the top of current function call stack
  * @note   This function MUST be called just before returning from every
  * 		user-defined function
  * @param  void
  * @retval void
  */
void pop_stack(void){
	call_depth--;
	call_stack[call_depth] = 0xFF;
}

/**
  * @brief  Prints current function call stack
  * @note   --	This function is to be called before infinite while loop in
  * 			Error_Handler().
  * 		--	User may find other use for this function but try to avoid
  * 			otherwise.
  * @param  void
  * @retval void
  */
void print_stack(void){
	int buf_pos = 0, write_buf_len = 0;

	write_buf_len = sprintf(debug_trace_buf+buf_pos, "  <dbg> [:]->MAIN");
	buf_pos += write_buf_len;

	for(uint8_t i = 1; i < call_depth; i++){
		write_buf_len = sprintf(debug_trace_buf+buf_pos, NEXT_CALL_ARROW);
		buf_pos += write_buf_len;
		write_buf_len = sprintf(debug_trace_buf+buf_pos, "0x%02X", call_stack[i]);
		buf_pos += write_buf_len;
	}

	write_buf_len = sprintf(debug_trace_buf+buf_pos, "\r\n");

	HAL_UART_Transmit(&huart3, (uint8_t*)debug_trace_buf, strlen(debug_trace_buf), DEBUG_TX_TIMEOUT);
}

/* Public functions ----------------------------------------------------------*/
void print_error_msg(void){
	HAL_UART_Transmit(&huart3, (uint8_t*)error_msg_buf, strlen(error_msg_buf), DEBUG_TX_TIMEOUT);

	print_stack();
}

void print_msg(const char *msg, ...){
	va_list args;
	int len;

	va_start(args, msg);
	len = vsnprintf(system_msg_buf, sizeof(system_msg_buf), msg, args);
	va_end(args);

	if (len < 0) {
		return;
	}

	if ((size_t)len >= sizeof(system_msg_buf)) {
		len = sizeof(system_msg_buf) - 1;
	}

	HAL_UART_Transmit(&huart3, (uint8_t*)system_msg_buf, (uint16_t)len, HAL_MAX_DELAY);
}
