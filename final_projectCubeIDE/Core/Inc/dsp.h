/*
 * dsp.h
 *
 *  Created on: Mar 21, 2026
 *      Author: arnol
 */

#ifndef INC_DSP_H_
#define INC_DSP_H_

#include "project.h"
#include "fixedpt.h"

#define DSP_DELAY_SAMPLES 4096U

void reset_dsp_state(void);
void apply_echo(const fixedpt *in, fixedpt *out, uint16_t count);
void apply_reverb(const fixedpt *in, fixedpt *out, uint16_t count);
void apply_quadrature(const fixedpt *in, fixedpt *out, uint16_t count);

#endif /* INC_DSP_H_ */
