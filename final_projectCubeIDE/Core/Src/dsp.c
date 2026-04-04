/*
 * dsp.c
 *
 *  Created on: Apr 1, 2026
 *      Author: arnol
 */

#include "dsp.h"

static fixedpt s_delay_line[DSP_DELAY_SAMPLES];
static uint16_t s_delay_idx = 0U;

static fixedpt s_prev_x = 0;

void reset_dsp_state(void){
    memset(s_delay_line, 0, sizeof(s_delay_line));
    s_delay_idx = 0U;
    s_prev_x = 0;
}

void apply_echo(const fixedpt *in, fixedpt *out, uint16_t count){
    uint16_t i;
    fixedpt delayed;
    fixedpt wet;
    fixedpt feedback_in;

    if ((in == NULL) || (out == NULL)) {
        return;
    }

    for (i = 0U; i < count; i++) {
        delayed = s_delay_line[s_delay_idx];

        /* Stronger wet mix */
        wet = FXD_MUL(delayed, FIXEDPT_CONST(0.75f));

        /* Output = dry + wet */
        out[i] = FXD_ADD(in[i], wet);

        /* Feedback makes repeats continue */
        feedback_in = FXD_ADD(in[i], FXD_MUL(delayed, FIXEDPT_CONST(0.55f)));
        s_delay_line[s_delay_idx] = feedback_in;

        s_delay_idx++;
        if (s_delay_idx >= DSP_DELAY_SAMPLES) {
            s_delay_idx = 0U;
        }
    }

    if (out[i] > FIXEDPT_CONST(1.0f)) {
        out[i] = FIXEDPT_CONST(1.0f);
    } else if (out[i] < FIXEDPT_CONST(-1.0f)) {
        out[i] = FIXEDPT_CONST(-1.0f);
    }
}

void apply_reverb(const fixedpt *in, fixedpt *out, uint16_t count){
    uint16_t i;
    uint16_t idx1, idx2, idx3, idx4;
    fixedpt r1, r2, r3, r4;
    fixedpt mix;
    fixedpt store_val;

    if ((in == NULL) || (out == NULL)) {
        return;
    }

    for (i = 0U; i < count; i++) {
        idx1 = (uint16_t)((s_delay_idx + DSP_DELAY_SAMPLES - 701U) % DSP_DELAY_SAMPLES);
        idx2 = (uint16_t)((s_delay_idx + DSP_DELAY_SAMPLES - 1451U) % DSP_DELAY_SAMPLES);
        idx3 = (uint16_t)((s_delay_idx + DSP_DELAY_SAMPLES - 2203U) % DSP_DELAY_SAMPLES);
        idx4 = (uint16_t)((s_delay_idx + DSP_DELAY_SAMPLES - 3119U) % DSP_DELAY_SAMPLES);

        r1 = s_delay_line[idx1];
        r2 = s_delay_line[idx2];
        r3 = s_delay_line[idx3];
        r4 = s_delay_line[idx4];

        mix = in[i];
        mix = FXD_ADD(mix, FXD_MUL(r1, FIXEDPT_CONST(0.55f)));
        mix = FXD_ADD(mix, FXD_MUL(r2, FIXEDPT_CONST(0.40f)));
        mix = FXD_ADD(mix, FXD_MUL(r3, FIXEDPT_CONST(0.28f)));
        mix = FXD_ADD(mix, FXD_MUL(r4, FIXEDPT_CONST(0.20f)));

        out[i] = mix;

        /* feed some of the reverberated signal back into the line */
        store_val = FXD_ADD(in[i], FXD_MUL(mix, FIXEDPT_CONST(0.35f)));
        s_delay_line[s_delay_idx] = store_val;

        s_delay_idx++;
        if (s_delay_idx >= DSP_DELAY_SAMPLES) {
            s_delay_idx = 0U;
        }
    }

    if (out[i] > FIXEDPT_CONST(1.0f)) {
        out[i] = FIXEDPT_CONST(1.0f);
    } else if (out[i] < FIXEDPT_CONST(-1.0f)) {
        out[i] = FIXEDPT_CONST(-1.0f);
    }
}

void apply_quadrature(const fixedpt *in, fixedpt *out, uint16_t count){
    uint16_t i;
    fixedpt current;

    if ((in == NULL) || (out == NULL)) {
        return;
    }

    for (i = 0U; i < count; i++) {
        current = in[i];
        out[i] = FXD_SUB(current, s_prev_x);
        s_prev_x = current;
    }
}
