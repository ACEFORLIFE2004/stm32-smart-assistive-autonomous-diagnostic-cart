/*
 * ui_interrupt_emulator.h
 *
 *  Created on: Mar 19, 2026
 *      Author: arnol
 */

#ifndef INC_UI_INTERRUPT_EMULATOR_H_
#define INC_UI_INTERRUPT_EMULATOR_H_

#include "project_assert.h"
#include "project.h"
#include "system.h"

#define MODE_INVALID	4

typedef uint8_t emulcnt_t;

extern uint8_t test_no;
extern emulcnt_t event_seq_num;


#define IS_SYS_BOOT(__STATE__)				(__STATE__ == 0x00)

#define IS_SYS_IDLE(__STATE__)				(__STATE__ == 0x01)

#define IS_SYS_MODE_SELECTED(__STATE__)		(__STATE__ == 0x02)

#define IS_SYS_RUNNING(__STATE__)			(__STATE__ == 0x03)


#define IS_NO_MODE(__MODE__)				(__MODE__ == 0x00)

#define IS_ASSIST_MODE(__MODE__)			(__MODE__ == 0x01)

#define IS_DIAGNOSTIC_MODE(__MODE__)		(__MODE__ == 0x02)


#define IS_DIAG_INIT(__STATE__)				(__STATE__ == 0x00)

#define IS_DIAG_CONFIG(__STATE__)			(__STATE__ == 0x01)

#define IS_DIAG_PREP(__STATE__)				(__STATE__ == 0x02)

#define IS_DIAG_RUN(__STATE__)				(__STATE__ == 0x03)


void test_0(void);
void test_1(void);
void test_2(void);
void test_3(void);
void test_4(void);

void ui_interrupt_emulator(void);

#endif /* INC_UI_INTERRUPT_EMULATOR_H_ */
