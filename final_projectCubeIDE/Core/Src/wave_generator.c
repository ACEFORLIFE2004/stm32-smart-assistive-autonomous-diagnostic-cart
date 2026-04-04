/*
 * wave_generator.c
 *
 *  Created on: Mar 21, 2026
 *      Author: arnol
 */

#include "wave_generator.h"
#include <math.h>

static float diag_debug_buf[MAX_SAMPLE_SIZE];   /* Floating point buffer for debugging purposes */

/* Circular DAC DMA buffer: first half + second half */
static uint16_t dac_dhr_buf[2 * ARB_WAV_CHUNK_SAMPLES];
#define DAC_BUF0   (&dac_dhr_buf[0])
#define DAC_BUF1   (&dac_dhr_buf[ARB_WAV_CHUNK_SAMPLES])

/* Stream sync flags - set only from DAC callbacks */
static volatile uint8_t s_dac_buf0_free = 0U;
static volatile uint8_t s_dac_buf1_free = 0U;

/* ---------- DAC-ready buffers ----------
 * Keep these separate from fixedpt DSP buffers.
 * fixedpt samples and DAC DHR samples are different representations.
 */
//static uint16_t s_dac_dma_buf_0[ARB_WAV_CHUNK_SAMPLES];
//static uint16_t s_dac_dma_buf_1[ARB_WAV_CHUNK_SAMPLES];

static fixedpt clamp_fxd(fixedpt x, fixedpt lo, fixedpt hi);
static fixedpt fxd_bipolar_to_dac_volts(fixedpt x);
static StorageStatus_t decode_wav_chunk_inplace_to_fxd(DiagnosticConfig_t *cfg, fixedpt *rw_buffer,
                                                       uint32_t samples_in_chunk, uint8_t bits_per_sample);
static void apply_selected_filter_block(DiagnosticConfig_t *cfg, const fixedpt *in, fixedpt *out, uint16_t sample_count);
static void pack_dac_block_from_fxd(const fixedpt *src, uint16_t *dst, uint16_t sample_count);
static StorageStatus_t rewind_wav_stream(DiagnosticConfig_t *cfg);

void stream_wave_function(DiagnosticConfig_t *func_wave){
	switch(func_wave->wav_function){
		case SINUSOIDAL_FUNC:
			func_wave->sample_size = generate_sine_wave(func_wave->wav_amplitude, func_wave->wav_frequency, func_wave->wave_samples);
			break;
		case SQUARE_FUNC:
			func_wave->sample_size = generate_square_wave(func_wave->wav_amplitude, func_wave->wav_frequency, func_wave->wave_samples);
			break;
		case TRIANGLE_FUNC:
			func_wave->sample_size = generate_triangle_wave(func_wave->wav_amplitude, func_wave->wav_frequency, func_wave->wave_samples);
			break;
		case SAWTOOTH_FUNC:
			func_wave->sample_size = generate_sawtooth_wave(func_wave->wav_amplitude, func_wave->wav_frequency, func_wave->wave_samples);
			break;
	}
}

uint16_t generate_sine_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer){
	uint16_t sample_size = MAX_SAMPLE_SIZE;

	for(uint16_t i = 0; i < sample_size; i++){
		sample_buffer[i] = FXD_MUL( amplitude , FXD_ADD(sine_LUT_fxd[i] , FXD_FROM_FLOAT(1.2f)) );
		dac_dhr_buf[i] 	 = fxd_to_dhr(sample_buffer[i]);
	}

	uint32_t conv_freq = frequency*sample_size;

	start_dac_conversion(conv_freq, sample_size);

	return sample_size;
}

uint16_t generate_square_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer){
	uint16_t sample_size = MAX_SAMPLE_SIZE;

	for(uint16_t i = 0; i < sample_size; i++){
		fixedpt harmonics = FXD_FROM_FLOAT(0.0f);
		uint16_t LUT_index = 0;

		// Compute LUT_index for each harmonic (1, 3, ..., 2n+1) -> scale amplitudes and add to Fourier series sum
		for(uint8_t n = 0; n<=10; n++){
			uint32_t harmonic_index = 0;

			harmonic_index = ((uint32_t)i * ((2*n)+1)) % sample_size;
			LUT_index = (uint16_t)harmonic_index;

			harmonics = FXD_ADD(
				harmonics,
				FXD_DIV(
					sine_LUT_fxd[LUT_index],
					FXD_ADD(FXD_MUL(FXD_FROM_INT(2), FXD_FROM_INT(n)), FXD_FROM_INT(1))
				)
			);
		}

		harmonics = FXD_DIV( FXD_MUL(FXD_FROM_FLOAT(3.39f) , harmonics) , FIXEDPT_PI );

		sample_buffer[i] = FXD_MUL( amplitude , FXD_ADD(harmonics , FXD_FROM_FLOAT(1.2f)) );
		dac_dhr_buf[i] 	 = fxd_to_dhr(sample_buffer[i]);
	}

	uint32_t conv_freq = frequency*sample_size;

	start_dac_conversion(conv_freq, sample_size);

	return sample_size;

}

uint16_t generate_triangle_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer){
	uint16_t sample_size = MAX_SAMPLE_SIZE;

	for(uint16_t i = 0; i < sample_size; i++){
		fixedpt harmonics = FXD_FROM_FLOAT(0.0f);
		uint16_t LUT_index = 0;

		// Compute LUT_index for each harmonic (1, 3, ..., 2n+1) -> scale amplitudes and add to Fourier series sum
		for(uint8_t n = 0; n<=10; n++){
			uint32_t harmonic_index = 0;
			fixedpt term = FXD_FROM_FLOAT(0.0f);

			harmonic_index = ((uint32_t)i * ((2*n)+1)) % sample_size;
			LUT_index = (uint16_t)harmonic_index;

			term = 	FXD_DIV(
							sine_LUT_fxd[LUT_index],
							FXD_MUL(
									FXD_ADD(FXD_MUL(FXD_FROM_INT(2), FXD_FROM_INT(n)), FXD_FROM_INT(1)),
									FXD_ADD(FXD_MUL(FXD_FROM_INT(2), FXD_FROM_INT(n)), FXD_FROM_INT(1))
									)
							);

			if((n % 2) == 1){
				term = FXD_MUL(term, FXD_FROM_INT(-1));
			}

			harmonics = FXD_ADD(harmonics, term);
		}

		harmonics = FXD_DIV( FXD_MUL(FXD_FROM_INT(8) , harmonics) , FXD_MUL(FIXEDPT_PI , FIXEDPT_PI) );

		sample_buffer[i] = FXD_MUL( amplitude , FXD_ADD(harmonics , FXD_FROM_FLOAT(1.2f)) );
		dac_dhr_buf[i] 	 = fxd_to_dhr(sample_buffer[i]);
	}

	uint32_t conv_freq = frequency*sample_size;

	start_dac_conversion(conv_freq, sample_size);

	return sample_size;
}

uint16_t generate_sawtooth_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer){
	uint16_t sample_size = MAX_SAMPLE_SIZE;

	for(uint16_t i = 0; i < sample_size; i++){
		fixedpt harmonics = FXD_FROM_FLOAT(0.0f);
		uint16_t LUT_index = 0;

		// Compute LUT_index for each harmonic (1, 2, ..., n) -> scale amplitudes and add to Fourier series sum
		for(uint8_t n = 1; n<=21; n++){
			uint32_t harmonic_index = 0;
			fixedpt term = FXD_FROM_FLOAT(0.0f);

			harmonic_index = ((uint32_t)i * n) % sample_size;
			LUT_index = (uint16_t)harmonic_index;

			term = 	FXD_DIV( sine_LUT_fxd[LUT_index] , FXD_FROM_INT(n) );

			if((n % 2) == 0){ /* True if n is even */
				term = FXD_MUL(term, FXD_FROM_INT(-1));
			}

			harmonics = FXD_ADD(harmonics, term);
		}

		harmonics = FXD_DIV( FXD_MUL(FXD_FROM_FLOAT(1.77f) , harmonics) , FIXEDPT_PI );

		sample_buffer[i] = FXD_MUL( amplitude , FXD_ADD(harmonics , FXD_FROM_FLOAT(1.2f)) );
		dac_dhr_buf[i] 	 = fxd_to_dhr(sample_buffer[i]);
	}

	uint32_t conv_freq = frequency*sample_size;

	start_dac_conversion(conv_freq, sample_size);

	return sample_size;
}

void perform_operation(DiagnosticConfig_t *result){
//	DiagnosticConfig_t *arg_1 = result->arg1;
//	DiagnosticConfig_t *arg_2 = result->arg2;
//	uint8_t operation 		  = result->operation;
//	fixedpt *result_arg 	  = result->wave_samples;

	return;
}

/* Read one chunk from file, decode, filter, and prepare DAC block.
 *
 * Uses rw_buffer memory as raw-byte staging first, then expands it in-place
 * into fixedpt samples.
 */
static StorageStatus_t prepare_next_wav_chunk(DiagnosticConfig_t *cfg, fixedpt *rw_buffer, fixedpt *dsp_buffer,
                                              uint16_t *dac_buffer, uint32_t *samples_prepared){
    StorageStatus_t status;
    uint32_t bytes_read = 0U;
    uint32_t bytes_per_sample;
    uint32_t total_samples;
    uint32_t samples_remaining;
    uint32_t samples_this_chunk;
    uint32_t bytes_to_read;
    uint32_t max_bytes_allowed;
    uint16_t i;

    if ((cfg == NULL) || (cfg->wav_header == NULL) || (rw_buffer == NULL) ||
        (dsp_buffer == NULL) || (dac_buffer == NULL) || (samples_prepared == NULL)) {
        return STORAGE_INVALID_ARG;
    }

    if (!cfg->stream_file_open) {
        return STORAGE_ERROR;
    }

    *samples_prepared = 0U;

    bytes_per_sample = (uint32_t)(cfg->wav_header->bits_per_sample / 8U);

    if ((cfg->wav_header->audio_format != 1U) || (cfg->wav_header->num_channels != 1U)) {
        return STORAGE_ERROR;
    }

    if (!((cfg->wav_header->bits_per_sample == 8U) ||
          (cfg->wav_header->bits_per_sample == 16U))) {
        return STORAGE_ERROR;
    }

    if ((cfg->sample_size == 0U) || (cfg->sample_size > ARB_WAV_CHUNK_SAMPLES)) {
        return STORAGE_ERROR;
    }

    total_samples = cfg->wav_header->data_size / bytes_per_sample;

    if (cfg->file_sample_index >= total_samples) {
        return STORAGE_OK; /* EOF */
    }

    samples_remaining = total_samples - cfg->file_sample_index;
    samples_this_chunk = (samples_remaining > cfg->sample_size) ? cfg->sample_size : samples_remaining;
    bytes_to_read = samples_this_chunk * bytes_per_sample;

    max_bytes_allowed = ARB_WAV_CHUNK_SAMPLES * sizeof(fixedpt);
    if (bytes_to_read > max_bytes_allowed) {
        return STORAGE_ERROR;
    }

    status = storage_read_stream(&cfg->stream_fil, (uint8_t *)rw_buffer, bytes_to_read, &bytes_read);
    if (status != STORAGE_OK) {
        return status;
    }

    if (bytes_read != bytes_to_read) {
        return STORAGE_ERROR;
    }

    status = decode_wav_chunk_inplace_to_fxd(cfg, rw_buffer, samples_this_chunk, cfg->wav_header->bits_per_sample);
    if (status != STORAGE_OK) {
        return status;
    }

    apply_selected_filter_block(cfg, rw_buffer, dsp_buffer, (uint16_t)samples_this_chunk);

    for (i = (uint16_t)samples_this_chunk; i < cfg->sample_size; i++) {
        dsp_buffer[i] = 0;
    }

    pack_dac_block_from_fxd(dsp_buffer, dac_buffer, cfg->sample_size);

    cfg->file_sample_index += samples_this_chunk;
    *samples_prepared = samples_this_chunk;

    return STORAGE_OK;
}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac_ptr)
{
    if (hdac_ptr == &hdac) {
        arbitrary_wave_mark_buf0_free();
    }
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac_ptr)
{
    if (hdac_ptr == &hdac) {
        arbitrary_wave_mark_buf1_free();
    }
}

void stream_arbitrary_wave(DiagnosticConfig_t *arby_wave){
    StorageStatus_t status;
    uint32_t samples_loaded = 0U;
    DiagnosticConfig_t *cfg = arby_wave;


    if (cfg == NULL) {
        return;
    }

    if (!cfg->stream_active) {
        return;
    }

    switch (cfg->stream_state) {

        case ARB_STREAM_IDLE:
        	print_msg("  ----> Entering ARB_STREAM_IDLE\r\n");
        	cfg->sample_size = ARB_WAV_CHUNK_SAMPLES;
        	cfg->stream_state = ARB_STREAM_LOAD_HEADER;
        	print_msg("  --> Leaving ARB_STREAM_IDLE\r\n");
            break;

        case ARB_STREAM_LOAD_HEADER:
            print_msg("  ----> Entering ARB_STREAM_LOAD_HEADER\r\n");

            if (storage_find_file_by_name(&g_storage_catalog, cfg->file_name, &(cfg->wav_file)) != STORAGE_OK) {
                print_msg("storage_find_file_by_name: Could not find file path\r\n");
                cfg->stream_error = 1U;
                cfg->stream_active = 0U;
                cfg->stream_state = ARB_STREAM_ERROR;
                return;
            }

            if (cfg->wav_file == NULL) {
                cfg->stream_error = 1U;
                cfg->stream_active = 0U;
                cfg->stream_state = ARB_STREAM_ERROR;
                return;
            }

            load_wav_header(cfg->wav_file, cfg->wav_header);

            if ((memcmp(cfg->wav_header->riff, "RIFF", 4) != 0) ||
                (memcmp(cfg->wav_header->wave, "WAVE", 4) != 0) ||
                (cfg->wav_header->audio_format != 1U) ||
                (cfg->wav_header->num_channels != 1U) ||
                !((cfg->wav_header->bits_per_sample == 8U) || (cfg->wav_header->bits_per_sample == 16U))) {
                cfg->stream_error = 1U;
                cfg->stream_active = 0U;
                cfg->stream_state = ARB_STREAM_ERROR;
                return;
            }

            if (storage_open_file_for_stream(cfg->wav_file, &cfg->stream_fil) != STORAGE_OK) {
                cfg->stream_error = 1U;
                cfg->stream_active = 0U;
                cfg->stream_state = ARB_STREAM_ERROR;
                return;
            }

            cfg->stream_file_open = 1U;
            cfg->data_offset = 44U;

            if (storage_seek_stream(&cfg->stream_fil, cfg->data_offset) != STORAGE_OK) {
                (void)storage_close_stream(&cfg->stream_fil);
                cfg->stream_file_open = 0U;
                cfg->stream_error = 1U;
                cfg->stream_active = 0U;
                cfg->stream_state = ARB_STREAM_ERROR;
                return;
            }

            cfg->file_sample_index = 0U;
            cfg->eof_reached = 0U;
            cfg->stream_state = ARB_STREAM_PRIME_BUF0;

            print_msg("  ----> Leaving ARB_STREAM_LOAD_HEADER\r\n");
            break;

        case ARB_STREAM_PRIME_BUF0:
        	print_msg("  ----> Entering ARB_STREAM_PRIME_BUF0\r\n");
        	status = prepare_next_wav_chunk(cfg, cfg->rw_buffer_0,
        	                                cfg->dsp_buf_0, DAC_BUF0, &samples_loaded);

        	if ((status != STORAGE_OK) || (samples_loaded == 0U)) {
        	    cfg->stream_error = 1U;
        	    cfg->stream_active = 0U;
        	    cfg->stream_state = ARB_STREAM_ERROR;
        	    return;
        	}

            cfg->stream_state = ARB_STREAM_PRIME_BUF1;
            print_msg("  ----> Leaving ARB_STREAM_PRIME_BUF0\r\n");
            break;

        case ARB_STREAM_PRIME_BUF1:
        	print_msg("  ----> Entering ARB_STREAM_PRIME_BUF1\r\n");
        	status = prepare_next_wav_chunk(cfg, cfg->rw_buffer_1,
        	                                cfg->dsp_buf_1, DAC_BUF1, &samples_loaded);

            if ((status != STORAGE_OK) || (samples_loaded == 0U)) {
                if (cfg->loop_enable) {
                    if (rewind_wav_stream(cfg) != STORAGE_OK) {
                        cfg->stream_error = 1U;
                        cfg->stream_active = 0U;
                        cfg->stream_state = ARB_STREAM_ERROR;
                        return;
                    }

                    status = prepare_next_wav_chunk(cfg, cfg->rw_buffer_1,
                                                    cfg->dsp_buf_1, DAC_BUF1, &samples_loaded);

                    if ((status != STORAGE_OK) || (samples_loaded == 0U)) {
                        cfg->stream_error = 1U;
                        cfg->stream_active = 0U;
                        cfg->stream_state = ARB_STREAM_ERROR;
                        return;
                    }
                } else {
                    memset(DAC_BUF1, 0, ARB_WAV_CHUNK_SAMPLES * sizeof(uint16_t));
                    cfg->eof_reached = 1U;
                }
            }

            cfg->stream_state = ARB_STREAM_START_DAC;
            print_msg("  ----> Leaving ARB_STREAM_PRIME_BUF1\r\n");
            break;

        case ARB_STREAM_START_DAC:
        	print_msg("  ----> Entering ARB_STREAM_START_DAC\r\n");
            start_dac_conversion(cfg->wav_header->sample_rate, 2*(cfg->sample_size));
            cfg->stream_state = ARB_STREAM_RUNNING;
            print_msg("  ----> Leaving ARB_STREAM_START_DAC\r\n");
            break;

        case ARB_STREAM_RUNNING:
        	if (cfg->abort_stream) {
				if (cfg->stream_file_open) {
					(void)storage_close_stream(&cfg->stream_fil);
					cfg->stream_file_open = 0U;
				}

				cfg->stream_active = 0U;
				cfg->stream_done = 1U;
				cfg->stream_state = ARB_STREAM_DONE;
				return;
			}

            if (s_dac_buf0_free) {
                s_dac_buf0_free = 0U;

                if (!cfg->eof_reached) {
                	status = prepare_next_wav_chunk(cfg, cfg->rw_buffer_0,
                	                                cfg->dsp_buf_0, DAC_BUF0, &samples_loaded);

                    if ((status != STORAGE_OK) || (samples_loaded == 0U)) {
                        if (cfg->loop_enable) {
                            if (rewind_wav_stream(cfg) != STORAGE_OK) {
                                cfg->stream_error = 1U;
                                cfg->stream_active = 0U;
                                cfg->stream_state = ARB_STREAM_ERROR;
                                return;
                            }

                            status = prepare_next_wav_chunk(cfg, cfg->rw_buffer_0,
                                                            cfg->dsp_buf_0, DAC_BUF0, &samples_loaded);

                            if ((status != STORAGE_OK) || (samples_loaded == 0U)) {
                                cfg->stream_error = 1U;
                                cfg->stream_active = 0U;
                                cfg->stream_state = ARB_STREAM_ERROR;
                                return;
                            }
                        } else {
                            memset(DAC_BUF0, 0, ARB_WAV_CHUNK_SAMPLES * sizeof(uint16_t));
                            cfg->eof_reached = 1U;
                        }
                    }
                } else {
                    memset(DAC_BUF0, 0, (ARB_WAV_CHUNK_SAMPLES/2));
                }
            }

            if (s_dac_buf1_free) {
                s_dac_buf1_free = 0U;

                if (!cfg->eof_reached) {
                	status = prepare_next_wav_chunk(cfg, cfg->rw_buffer_1,
                	                                cfg->dsp_buf_1, DAC_BUF1, &samples_loaded);

                    if ((status != STORAGE_OK) || (samples_loaded == 0U)) {
                        if (cfg->loop_enable) {
                            if (rewind_wav_stream(cfg) != STORAGE_OK) {
                                cfg->stream_error = 1U;
                                cfg->stream_active = 0U;
                                cfg->stream_state = ARB_STREAM_ERROR;
                                return;
                            }

                            status = prepare_next_wav_chunk(cfg, cfg->rw_buffer_1,
                                                            cfg->dsp_buf_1, DAC_BUF1, &samples_loaded);

                            if ((status != STORAGE_OK) || (samples_loaded == 0U)) {
                                cfg->stream_error = 1U;
                                cfg->stream_active = 0U;
                                cfg->stream_state = ARB_STREAM_ERROR;
                                return;
                            }
                        } else {
                            memset(DAC_BUF1, 0, ARB_WAV_CHUNK_SAMPLES * sizeof(uint16_t));
                            cfg->eof_reached = 1U;
                        }
                    }
                } else {
                    memset(DAC_BUF1, 0, (ARB_WAV_CHUNK_SAMPLES/2));
                }
            }

            /* Let higher-level logic decide when to stop DAC after EOF */
            if (cfg->eof_reached && !cfg->loop_enable) {
                cfg->stream_done = 1U;
            }
            break;

        case ARB_STREAM_DONE:
            if (cfg->stream_file_open) {
                (void)storage_close_stream(&cfg->stream_fil);
                cfg->stream_file_open = 0U;
            }
            cfg->stream_active = 0U;
            cfg->stream_state = ARB_STREAM_IDLE;
        	break;

        case ARB_STREAM_ERROR:
            if (cfg->stream_file_open) {
                (void)storage_close_stream(&cfg->stream_fil);
                cfg->stream_file_open = 0U;
            }
            cfg->stream_active = 0U;
            cfg->stream_state = ARB_STREAM_IDLE;
        	break;

        default:
        	break;
    }
}

void start_arbitrary_wave_stream(DiagnosticConfig_t *cfg){
    if (cfg == NULL) {
        return;
    }

    reset_dsp_state();

    cfg->file_sample_index = 0U;
    cfg->data_offset = 44U;
    cfg->eof_reached = 0U;

    cfg->abort_stream = 0U;
    cfg->stream_active = 1U;
    cfg->stream_done = 0U;
    cfg->stream_error = 0U;
    cfg->stream_file_open = 0U;

    memset(&cfg->stream_fil, 0, sizeof(FIL));

    cfg->stream_state = ARB_STREAM_LOAD_HEADER;

    s_dac_buf0_free = 0U;
    s_dac_buf1_free = 0U;

    memset(dac_dhr_buf, 0, sizeof(dac_dhr_buf));
}

void load_wav_header(File_t *wav_file, WAV_Header_t *wav_header)
{
    StorageStatus_t status;
    uint32_t bytes_read = 0;
    uint8_t header_buff[sizeof(WAV_Header_t)];

    if ((wav_file == NULL) || (wav_header == NULL)) {
        return;
    }

    memset(wav_header, 0, sizeof(WAV_Header_t));
    memset(header_buff, 0, sizeof(header_buff));

    status = storage_read_file(wav_file, header_buff, sizeof(header_buff), &bytes_read);
    if (status != STORAGE_OK) {
        print_msg("load_wav_header failed: could not read file. Status=%u\r\n", status);
        return;
    }

    if (bytes_read < sizeof(WAV_Header_t)) {
        print_msg("load_wav_header failed: incomplete header read (%lu bytes).\r\n", bytes_read);
        return;
    }

    memcpy(wav_header, header_buff, sizeof(WAV_Header_t));

    // Add this before the memcmp checks in load_wav_header
    print_msg("Header IDs: [%.4s] [%.4s] [%.4s] [%.4s]\r\n",
              wav_header->riff, wav_header->wave, wav_header->fmt, wav_header->data);

    /* Optional sanity checks */
    if ((memcmp(wav_header->riff, "RIFF", 4) != 0) ||
        (memcmp(wav_header->wave, "WAVE", 4) != 0) ||
        (memcmp(wav_header->fmt,  "fmt ", 4) != 0) ||
        (memcmp(wav_header->data, "data", 4) != 0)) {
        print_msg("load_wav_header warning: file does not look like a standard PCM WAV.\r\n");
        return;
    }

    print_msg("WAV Header Loaded Successfully:\r\n");
    print_msg("  Channels        : %u\r\n", wav_header->num_channels);
    print_msg("  Sample Rate     : %lu\r\n", wav_header->sample_rate);
    print_msg("  Byte Rate       : %lu\r\n", wav_header->byte_rate);
    print_msg("  Block Align     : %u\r\n", wav_header->block_align);
    print_msg("  Bits Per Sample : %u\r\n", wav_header->bits_per_sample);
    print_msg("  Data Size       : %lu\r\n", wav_header->data_size);
}

/* ---------- Fixed-point helpers ---------- */
static fixedpt clamp_fxd(fixedpt x, fixedpt lo, fixedpt hi){
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

/* Convert normalized bipolar [-1, 1) to unipolar [0, 3.3] for DAC */
static fixedpt fxd_bipolar_to_dac_volts(fixedpt x){
    const fixedpt half_scale = FIXEDPT_CONST(1.45f);
    const fixedpt full_scale = FIXEDPT_CONST(1.74f);

    /* v = 1.45*x + 1.74 = 1.45*(x + 1.2) */
    fixedpt v = FXD_ADD(FXD_MUL(x, half_scale), full_scale);
    return clamp_fxd(v, FIXEDPT_CONST(0.29f), FIXEDPT_CONST(3.19f));
}

/* ---------- Hook this into your DAC DMA complete logic ----------
 * Call these from the DMA callback / half-complete or full-complete logic
 * based on whichever ping-pong buffer has just been consumed.
 */
void arbitrary_wave_mark_buf0_free(void){
    s_dac_buf0_free = 1U;
}

void arbitrary_wave_mark_buf1_free(void){
    s_dac_buf1_free = 1U;
}
/* ---------- Optional integration helpers ----------
 * If your DAC layer needs access to the actual uint16_t DMA buffers, use these.
 */
uint16_t *arbitrary_wave_get_dac_buf0(void){
    return DAC_BUF0;
}

uint16_t *arbitrary_wave_get_dac_buf1(void){
    return DAC_BUF1;
}

/* ---------- WAV decode helpers ---------- */

/* Convert raw PCM chunk -> normalized fixedpt in [-1,1), in-place backwards.
 * We read raw bytes into the same rw_buffer memory, then expand backwards
 * into fixedpt samples so we do not need a separate byte staging buffer.
 *
 * Supports:
 *   - PCM mono 8-bit unsigned
 *   - PCM mono 16-bit signed little-endian
 */
/* Convert raw PCM chunk -> normalized fixedpt in [-1,1), in-place backwards.
 * Supports:
 * - PCM mono 8-bit unsigned (centered at 128)
 * - PCM mono 16-bit signed little-endian
 */
static StorageStatus_t decode_wav_chunk_inplace_to_fxd(DiagnosticConfig_t *cfg, fixedpt *rw_buffer,
                                                       uint32_t samples_in_chunk, uint8_t bits_per_sample) {
    int32_t i;
    uint8_t *raw = (uint8_t *)rw_buffer;

    if (bits_per_sample == 8U) {
        /* 8-bit is unsigned (0-255). Center at 128 for bipolar [-1, 1] */
        for (i = (int32_t)samples_in_chunk - 1; i >= 0; i--) {
            int32_t s = (int32_t)raw[i] - 128;
            rw_buffer[i] = FXD_DIV(FXD_FROM_INT(s), FXD_FROM_INT(128));
        }
    }
    else if (bits_per_sample == 16U) {
        /* 16-bit is signed little-endian. Expand 2 bytes to 4 bytes backwards */
        for (i = (int32_t)samples_in_chunk - 1; i >= 0; i--) {
            uint32_t b_idx = (uint32_t)i * 2U;
            int16_t s = (int16_t)(((uint16_t)raw[b_idx + 1] << 8U) | (uint16_t)raw[b_idx]);
            rw_buffer[i] = FXD_DIV(FXD_FROM_INT((int32_t)s), FXD_FROM_INT(32768));
        }
    }
    return STORAGE_OK;
}

/* ---------- DSP placeholder ----------
 * Replace the filter body with your actual DSP functions.
 * For now:
 *   - if filter disabled -> copy through
 *   - otherwise still copy through, but this is where your FIR/IIR/etc goes
 */
static void apply_selected_filter_block(DiagnosticConfig_t *cfg, const fixedpt *in, fixedpt *out, uint16_t sample_count){
    uint16_t i;

    if ((cfg == NULL) || (in == NULL) || (out == NULL)) {
        return;
    }

    switch (cfg->digital_filter){
        case NO_FILTER:
            for (i = 0U; i < sample_count; i++)
                out[i] = in[i];
            break;
        case ECHO:        /* use as ECHO */
            apply_echo(in, out, sample_count);
            break;
        case REVERB:        /* use as REVERB */
            apply_reverb(in, out, sample_count);
            break;
        case QUADRATURE:      /* quadrature */
            apply_quadrature(in, out, sample_count);
            break;
        default:
            for (i = 0U; i < sample_count; i++)
                out[i] = in[i];
            break;
    }
}

/* Convert filtered fixedpt block -> DAC-ready uint16_t block */
static void pack_dac_block_from_fxd(const fixedpt *src, uint16_t *dst, uint16_t sample_count){
    uint16_t i;

    if ((src == NULL) || (dst == NULL)) {
        return;
    }

    for (i = 0U; i < sample_count; i++) {
        fixedpt dac_volts = fxd_bipolar_to_dac_volts(src[i]);
        dst[i] = fxd_to_dhr(dac_volts);
    }
}

static StorageStatus_t rewind_wav_stream(DiagnosticConfig_t *cfg){
    if ((cfg == NULL) || !cfg->stream_file_open) {
        return STORAGE_ERROR;
    }

    if (storage_seek_stream(&cfg->stream_fil, cfg->data_offset) != STORAGE_OK) {
        return STORAGE_FATFS_ERROR;
    }

    cfg->file_sample_index = 0U;
    cfg->eof_reached = 0U;
    return STORAGE_OK;
}

/* DMA DAC Helper Functions */
void start_dac_conversion(uint32_t conversion_frequency, uint16_t sample_size){
	if(HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)dac_dhr_buf, sample_size, DAC_ALIGN_12B_R) != HAL_OK){
		(void)sprintf(error_msg_buf, "  <err> start_dac_conversion: Failed to start DMA for DAC conversion\r\n");
		print_error_msg();

		while(1){
			;
		}
	}

	enable_dac_dma_trigger(conversion_frequency);
	print_msg("  --> DMA DAC Conversion started\r\n");
}

void stop_dac_conversion(void){
	disable_dac_dma_trigger();

	(void)HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
}

uint16_t fxd_to_dhr(fixedpt sample_val){
	return FXD_TO_INT( FXD_DIV(FXD_MUL(sample_val , FXD_FROM_INT(4095)) , FXD_MUL(FXD_FROM_INT(2) , FXD_FROM_FLOAT(1.65f))) );
}



/* ---------- Main streamer ----------
 * Integration note:
 * start_dac_conversion(conversion_frequency, sample_size) is assumed to start
 * the DAC/DMA machinery that consumes your ping-pong DAC buffers.
 *
 * You still need your DAC side to alternate between s_dac_dma_buf_0 and
 * s_dac_dma_buf_1 and call:
 *   arbitrary_wave_mark_buf0_free();
 *   arbitrary_wave_mark_buf1_free();
 * when each buffer is finished.
 */
