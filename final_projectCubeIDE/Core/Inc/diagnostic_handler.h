/*
 * diagnostic_handler.h
 *
 *  Created on: Mar 18, 2026
 *      Author: arnol
 */

#ifndef INC_DIAGNOSTIC_HANDLER_H_
#define INC_DIAGNOSTIC_HANDLER_H_

//#include "project.h"
#include "wave_generator.h"
//#include "fixedpt.h"

extern uint8_t func_wav_no;
extern uint8_t diag_config_flag, diag_running_flag;
extern DiagnosticConfig_t func_wave_1, func_wave_2, func_wave_3;

typedef enum{
	DIAGNOSTIC_INIT		= 0U,
	DIAGNOSTIC_CONFIG 	= 1U,
	DIAGNOSTIC_PREPARE 	= 2U,
	DIAGNOSTIC_RUNNING 	= 3U
}DiagnosticSubstate_t;

#define GET_MAX_BUFFER_SIZE( __SIZE1__ , __SIZE2__ )	(__SIZE1__ > __SIZE2__) ? __SIZE1__ : __SIZE2__

void diagnostic_update(void);
void initialize_diagnostic_buffers(DiagnosticConfig_t *cfg, uint8_t slot);
void reset_diagnostic_state(void);

#endif /* INC_DIAGNOSTIC_HANDLER_H_ */
